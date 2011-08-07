#include <buddytree.h>
#include <string.h>

//
// The map between the QQBuddy uin and the tree row.
//
typedef struct{
    GString *uin;
    GtkTreeRowReference *row_ref;
}QQTreeMap;

enum{
    BDY_IMAGE = 0,      //The face image of buddy or group
    BDY_MARKNAME,       //Group name or buddy markname
    BDY_NICK,           //buddy nick
    BDY_LONGNICK,       //buddy long nick
    BDY_TYPE,           //The client type
    BDY_STATUS,         //The status

    CATE_CNT,           //Used by category. The online buddies's number
    CATE_TOTAL,         //Used by category. The total number of buddies.
    COLUMNS             //The number of columns
};

static GHashTable *tree_map = NULL;

//
// Set the text columns' values
//
static void qq_buddy_tree_text_cell_data_func(GtkTreeViewColumn *col
                                            , GtkCellRenderer *renderer
                                            , GtkTreeModel *model
                                            , GtkTreeIter *iter
                                            , gpointer data)
{
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    gchar text[500];
    if(gtk_tree_path_get_depth(path) > 1){
        //
        // Buddy
        //
        gchar *markname, *nick, *long_nick, *type;
        gtk_tree_model_get(model, iter
                            , BDY_MARKNAME, &markname
                            , BDY_NICK, &nick
                            , BDY_LONGNICK, &long_nick
                            , BDY_TYPE, &type, -1);
        GString *fmt = g_string_new(NULL);
        if(strlen(markname) <= 0){
            g_string_append(fmt, "<b>%s%s</b>");
        }else{
            g_string_append(fmt, "<b>%s</b>(%s)");
        }
        //type
        if(strlen(type) <= 0){
            g_string_append(fmt, "%s");
        }else{
            g_string_append(fmt, "<span color='blue'>[login with %s]</span>");
        }
        //long nick
        g_string_append(fmt, "<span color='grey'>%s</span>");
        g_snprintf(text, 500, fmt -> str, markname, nick, type, long_nick);
        g_string_free(fmt, TRUE);
        g_object_set(renderer, "markup", text, NULL);
        g_free(markname);
        g_free(nick);
        g_free(long_nick);
        g_free(type);
    }else{
        //
        // Category
        //
        gchar *name;
        gint count, total;
        gtk_tree_model_get(model, iter
                            , BDY_MARKNAME, &name
                            , CATE_CNT, &count
                            , CATE_TOTAL, &total, -1);
        g_snprintf(text, 500, "%s [%d/%d]", name, count, total);
        g_object_set(renderer, "text", text, NULL);
        g_free(name);
    }
}

//
// Set the pixbuf columns' values
//
static void qq_buddy_tree_pixbuf_cell_data_func(GtkTreeViewColumn *col
                                            , GtkCellRenderer *renderer
                                            , GtkTreeModel *model
                                            , GtkTreeIter *iter
                                            , gpointer data)
{
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    if(gtk_tree_path_get_depth(path) > 1){
        //
        // Buddy
        //
        g_object_set(renderer, "visible", TRUE, NULL);
        gchar *status;
        gtk_tree_model_get(model, iter, BDY_STATUS, &status, -1);
        if(g_strcmp0("online", status) == 0 
                        || g_strcmp0("away", status) == 0){
            g_object_set(renderer, "sensitive", TRUE, NULL);
        }else{
            g_object_set(renderer, "sensitive", FALSE, NULL);
        }
        g_free(status);
    }else{
        //
        // Category
        //
        g_object_set(renderer, "visible", FALSE, NULL);
    }
    gtk_tree_path_free(path);
}

GtkWidget* qq_buddy_tree_new()
{
    GtkWidget *view= gtk_tree_view_new();
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // The image column
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", BDY_IMAGE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer
                                        , qq_buddy_tree_pixbuf_cell_data_func
                                        , NULL, NULL);

    // The text column
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer
                                        , qq_buddy_tree_text_cell_data_func
                                        , NULL, NULL);


    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
    gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(view), -35);

    tree_map = g_hash_table_new(g_str_hash, g_str_equal);
    return view;
}

//
// Create the QQTreeMap hash table.
//
void clear_treemap_ht(gpointer key, gpointer value, gpointer data)
{
    QQTreeMap *map = value;
    g_string_free(map -> uin, TRUE);
    gtk_tree_row_reference_free(map -> row_ref);
    g_slice_free(QQTreeMap, map);
}


//
// Create the model of the contact tree
//
static GtkTreeModel* create_contact_model(QQInfo *info)
{
    //Clear
    g_hash_table_foreach(tree_map, clear_treemap_ht, NULL);

    GtkTreeIter iter, child;
    GtkTreeStore *store = gtk_tree_store_new(COLUMNS
                                            , GDK_TYPE_PIXBUF
                                            , G_TYPE_STRING
                                            , G_TYPE_STRING
                                            , G_TYPE_STRING
                                            , G_TYPE_STRING
                                            , G_TYPE_STRING
                                            , G_TYPE_INT
                                            , G_TYPE_INT);
    QQCategory *cate;
    QQTreeMap *map;
    QQBuddy *bdy;
    GtkTreeRowReference *ref;
    GtkTreePath *path;
    GdkPixbuf *pb;
    gint i, j, k;
    gint num;
    for(i = 0; i < info -> categories -> len; ++i){
        //add the categories

        //find the ith catogory.
        k = 0;
        do{
            cate = (QQCategory*)info -> categories -> pdata[k];
            if(cate -> index == i){
                break;
            }
            ++k;
        }while(k < info -> categories -> len);

        num = cate -> members -> len;
        gtk_tree_store_append(store, &iter, NULL);
        gtk_tree_store_set(store, &iter, BDY_MARKNAME, cate -> name -> str
                                        , CATE_CNT, 0
                                        , CATE_TOTAL, num, -1);

        //add the buddies in this category.
        for(j = 0; j < cate -> members -> len; ++j){
            gtk_tree_store_append(store, &child, &iter);
            bdy = (QQBuddy*) cate -> members -> pdata[j];
            // set face image
            pb = gdk_pixbuf_new_from_file_at_size(IMGDIR"/avatar.png"
                                        , 20, 20, NULL);
            gtk_tree_store_set(store, &child, BDY_IMAGE, pb, -1);
            g_object_unref(pb);

            // set markname and nick
            if(bdy -> markname == NULL || bdy -> markname -> len <=0){
                gtk_tree_store_set(store, &child
                                    , BDY_MARKNAME, ""
                                    , BDY_NICK, bdy -> nick -> str, -1);
            }else{
                gtk_tree_store_set(store, &child
                                    , BDY_MARKNAME, bdy -> markname -> str
                                    , BDY_NICK, bdy -> nick -> str, -1);
            }

            // set long nick
            gtk_tree_store_set(store, &child
                                    , BDY_LONGNICK, bdy -> lnick -> str
                                    , BDY_STATUS, bdy -> status -> str, -1);
            const gchar *type = "";
            switch(bdy -> client_type)
            {
            case 1:
                type = "PC";
                break;
            case 21:
                type = "Phone";
                break;
            case 41:
                type = "WebQQ";
                break;
            default:
                break;
            }
            gtk_tree_store_set(store, &child, BDY_TYPE, type, -1);
            //get the tree row reference
            path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &child);
            ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(store), path);
            gtk_tree_path_free(path);

            //create and add a tree map
            map = g_slice_new0(QQTreeMap);
            map -> uin = g_string_new(bdy -> uin -> str);
            map -> row_ref = ref;
            g_hash_table_insert(tree_map, bdy -> uin -> str, map);
        }
    }

    return GTK_TREE_MODEL(store);
}

//
// Update the face image of uin
//
static void update_face_image(GtkWidget *tree, QQInfo *info, const gchar *uin)
{
    QQBuddy *bdy = qq_info_lookup_buddy_by_uin(info , uin);
    if(bdy == NULL){
        return;
    }
    gchar buf[500];
    // test if we have the image file;
    g_snprintf(buf, 500, CONFIGDIR"/faces/%s", bdy -> qqnumber -> str);

    GtkTreeModel *model;
    QQTreeMap *map;
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
    map = (QQTreeMap*)g_hash_table_lookup(tree_map, uin);
    if(map == NULL){
        g_warning("No TreeMap for %s(%s, %d)", uin, __FILE__, __LINE__);
        return;
    }

    GError *err = NULL;
    GdkPixbuf *pb = gdk_pixbuf_new_from_file_at_size(buf, 20, 20, &err);
    if(pb == NULL){
        g_error_free(err);
        err = NULL;
        pb = gdk_pixbuf_new_from_file_at_size(
                                    IMGDIR"/avatar.gif", 20, 20, &err);
        if(pb == NULL){
            g_warning("Load default face image error. %s (%s, %d)"
                                    , err -> message, __FILE__, __LINE__);
            g_error_free(err);
        }
    }
    GtkTreePath *path;
    GtkTreeIter iter;
    path = gtk_tree_row_reference_get_path(map -> row_ref);
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, BDY_IMAGE, pb, -1);
    g_object_unref(pb);
}


void qq_buddy_tree_update(GtkWidget *tree, QQInfo *info)
{
    //update the contact tree
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree)
                            , create_contact_model(info));
    //update the face images
    gint i;
    QQBuddy *bdy;
    for(i = 0; i < info -> buddies -> len; ++i){
        bdy = (QQBuddy*)g_ptr_array_index(info -> buddies, i);
        if(bdy == NULL){
            continue;
        }
        update_face_image(tree, info, bdy -> uin -> str);
    }

}

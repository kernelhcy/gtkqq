#include <buddylist.h>

static void qq_buddy_list_init(QQBuddyList *self);
static void qq_buddy_list_class_init(QQBuddyListClass *klass, gpointer data);

static void qq_buddy_list_finalize(GObject *obj);

//
// List store columns
//
enum{
    BDY_LIST_TYPE = 0,      // GDK_TYPE_PIXBUF, client type image
    BDY_LIST_IMG,           // GDK_TYPE_PIXBUF, face image
    BDY_LIST_UIN,           // G_TYPE_STRING, buddy uin or group uin
    BDY_LIST_CLASS,         // G_TYPE_INT, buddy or group. 1: buddy, 2:group
    BDY_LIST_NAME,          // G_TYPE_STRING, buddy markname or group name
    BDY_LIST_LNICK,         // G_TYPE_STRING, buddy long nick 
    BDY_LIST_NUMBER,        // G_TYPE_STRING, qq number of the group number
                            // or group fingermemo
    BDY_LIST_COLUMNS        // column number
};

//
// map between the uin and row reference
//
typedef struct{
    GString *uin;
    GtkTreeRowReference *row_ref;
}QQBuddyListMap;

//
// Private members
//
typedef struct{
    GHashTable *row_map;
}QQBuddyListPriv;

GType qq_buddy_list_get_type()
{
    static volatile gsize g_define_type_id__volatile = 0;
    static GType type_id = 0;
    if (g_once_init_enter(&g_define_type_id__volatile)) {
            if(type_id == 0){
                    GTypeInfo type_info={
                            sizeof(QQBuddyListClass),     /* class size */
                            NULL,                       /* base init*/
                            NULL,                       /* base finalize*/
                            /* class init */
                            (GClassInitFunc)qq_buddy_list_class_init,
                            NULL,                       /* class finalize */
                            NULL,                       /* class data */
                                
                            sizeof(QQBuddyList),          /* instance size */
                            0,                          /* prealloc bytes */
                            /* instance init */
                            (GInstanceInitFunc)qq_buddy_list_init,
                            NULL                        /* value table */
                    };
                    type_id = g_type_register_static(
                                            GTK_TYPE_TREE_VIEW,
                                            "QQBuddyList",
                                            &type_info,
                                            0);
            }
            g_once_init_leave(&g_define_type_id__volatile, type_id);
    }
    return type_id;
}

GtkWidget* qq_buddy_list_new()
{
    return GTK_WIDGET(g_object_new(qq_buddy_list_get_type(), NULL));
}


// Set the text columns' values
//
static void qq_buddy_list_text_cell_data_func(GtkTreeViewColumn *col
                                            , GtkCellRenderer *renderer
                                            , GtkTreeModel *model
                                            , GtkTreeIter *iter
                                            , gpointer data)
{
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    gchar text[500];
    gchar *name, *lnick;
    gtk_tree_model_get(model, iter
                            , BDY_LIST_NAME, &name
                            , BDY_LIST_LNICK, &lnick, -1);
    g_snprintf(text, 500, "<b>%s</b>  <span color='grey'>%s</span>"
                                    , name, lnick);
    g_object_set(renderer, "markup", text, NULL);
    g_free(name);
    g_free(lnick);
    gtk_tree_path_free(path);
}

//
// Set the pixbuf columns' values
//
static void qq_buddy_list_pixbuf_cell_data_func(GtkTreeViewColumn *col
                                            , GtkCellRenderer *renderer
                                            , GtkTreeModel *model
                                            , GtkTreeIter *iter
                                            , gpointer data)
{
}

static gboolean qq_buddy_list_on_show_tooltip(GtkWidget* widget
                                            , int x
                                            , int y
                                            , gboolean keybord_mode
                                            , GtkTooltip* tip
                                            , gpointer data)
{
    return TRUE;
}

static void qq_buddy_list_on_double_click(GtkTreeView *tree
                                    , GtkTreePath *path 
                                    , GtkTreeViewColumn  *col 
                                    , gpointer data)
{
}

static void row_map_destroy_value(gpointer value)
{
    if(value == NULL){
        return;
    }
    GtkTreeRowReference *ref = value;
    gtk_tree_row_reference_free(ref);
}

static void qq_buddy_list_init(QQBuddyList *self)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // The client type column
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", BDY_LIST_TYPE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer
                                        , qq_buddy_list_pixbuf_cell_data_func
                                        , NULL, NULL);

    // The image column
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", BDY_LIST_IMG);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer
                                        , qq_buddy_list_pixbuf_cell_data_func
                                        , NULL, NULL);

    // The text column
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer
                                        , qq_buddy_list_text_cell_data_func
                                        , NULL, NULL);


    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self), FALSE);
    //gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(self), -35);
    gtk_tree_view_set_hover_selection(GTK_TREE_VIEW(self), TRUE);

    GtkListStore *store = gtk_list_store_new(BDY_LIST_COLUMNS
                                            , GDK_TYPE_PIXBUF
                                            , GDK_TYPE_PIXBUF
                                            , G_TYPE_STRING
                                            , G_TYPE_INT
                                            , G_TYPE_STRING
                                            , G_TYPE_STRING
                                            , G_TYPE_STRING); 
    gtk_tree_view_set_model(GTK_TREE_VIEW(self), GTK_TREE_MODEL(store));

    gtk_widget_set_has_tooltip(GTK_WIDGET(self), TRUE);
	g_signal_connect(self, "query-tooltip"
                        , G_CALLBACK(qq_buddy_list_on_show_tooltip) , NULL);

	g_signal_connect(self
				   , "row-activated"
				   , G_CALLBACK(qq_buddy_list_on_double_click)
				   , NULL);

    QQBuddyListPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(self
                                                , qq_buddy_list_get_type()
                                                , QQBuddyListPriv); 
    priv -> row_map = g_hash_table_new_full(g_str_hash, g_str_equal
                                            , g_free
                                            , row_map_destroy_value);
}

static void qq_buddy_list_class_init(QQBuddyListClass *klass, gpointer data)
{
    g_type_class_add_private(klass, sizeof(QQBuddyListPriv));
    G_OBJECT_CLASS(klass)-> finalize = qq_buddy_list_finalize;
}

static gboolean clear_row_map(gpointer key, gpointer value, gpointer data)
{
    // remove all the key/value
    return TRUE;
}

static void qq_buddy_list_finalize(GObject *obj)
{
    QQBuddyListPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(obj
                                                , qq_buddy_list_get_type()
                                                , QQBuddyListPriv); 
    g_hash_table_foreach_remove(priv -> row_map, clear_row_map, NULL);
    g_hash_table_unref(priv -> row_map);

    // chain up
    GObjectClass *klass = (GObjectClass*)g_type_class_peek_parent(
                                g_type_class_peek(qq_buddy_list_get_type()));
    klass -> finalize(obj);
}

//
// Set the buddy info
//
static void qq_buddy_list_set_buddy(GtkListStore *store, GtkTreeIter *iter
                                    , QQBuddy *bdy)
{
    gchar *name;
    if(bdy -> markname == NULL || bdy -> markname -> len <= 0){
        name = bdy -> nick -> str;
    }else{
        name = bdy -> markname -> str;
    }

    gtk_list_store_set(store, iter
                        , BDY_LIST_UIN, bdy -> uin -> str
                        , BDY_LIST_CLASS, 1
                        , BDY_LIST_NAME, name
                        , BDY_LIST_LNICK, bdy -> lnick -> str
                        , -1);
    gchar buf[500];
    GdkPixbuf *pb = NULL;
    g_snprintf(buf, 500, CONFIGDIR"/faces/%s", bdy -> qqnumber -> str);
    pb = gdk_pixbuf_new_from_file_at_size(buf, 22, 22, NULL);
    if(pb == NULL){
        pb = gdk_pixbuf_new_from_file_at_size(IMGDIR"/avatar.gif"
                                                    , 22, 22, NULL);
    }
    gtk_list_store_set(store, iter, BDY_LIST_IMG, pb, -1);
}

void qq_buddy_list_add_buddy(GtkWidget *widget, QQBuddy *bdy)
{
    if(widget == NULL || bdy == NULL){
        return;
    }

    GtkTreeView *view = GTK_TREE_VIEW(widget);
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(view));
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    qq_buddy_list_set_buddy(store, &iter, bdy);

    // add reference map
    GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    GtkTreeRowReference *ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(store)
                                                            , path);
    gtk_tree_path_free(path);
    QQBuddyListPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                                , qq_buddy_list_get_type()
                                                , QQBuddyListPriv); 
    g_hash_table_insert(priv -> row_map, g_strdup(bdy -> uin -> str), ref);
    return;
}

//
// Set the group info
//
static void qq_buddy_list_set_group(GtkListStore *store, GtkTreeIter *iter
                                    , QQGroup *grp)
{
    gtk_list_store_set(store, iter
                        , BDY_LIST_NUMBER, grp -> gnumber -> str
                        , BDY_LIST_UIN, grp -> gid -> str
                        , BDY_LIST_CLASS, 2
                        , BDY_LIST_NAME, grp -> name -> str
                        , BDY_LIST_LNICK, grp -> fingermemo -> str
                        , -1);
    gchar buf[500];
    GdkPixbuf *pb = NULL;
    g_snprintf(buf, 500, CONFIGDIR"/faces/%s", grp -> gnumber -> str);
    pb = gdk_pixbuf_new_from_file_at_size(buf, 22, 22, NULL);
    if(pb == NULL){
        pb = gdk_pixbuf_new_from_file_at_size(IMGDIR"/avatar.gif", 22, 22, NULL);
    }
    gtk_list_store_set(store, iter, BDY_LIST_IMG, pb, -1);
}

void qq_buddy_list_add_group(GtkWidget *widget, QQGroup *grp)
{
    if(widget == NULL || grp == NULL){
        return;
    }

    GtkTreeView *view = GTK_TREE_VIEW(widget);
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(view));
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    qq_buddy_list_set_group(store, &iter, grp);
    
    // add reference map
    GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    GtkTreeRowReference *ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(store)
                                                            , path);
    gtk_tree_path_free(path);
    QQBuddyListPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                                , qq_buddy_list_get_type()
                                                , QQBuddyListPriv); 
    g_hash_table_insert(priv -> row_map, g_strdup(grp -> gid -> str), ref);
    return;
}

void qq_buddy_list_add_buddies(GtkWidget *widget, GPtrArray *bdies)
{
    if(widget == NULL || bdies == NULL){
        return;
    }
    gint i;
    QQBuddy *bdy;
    for(i = 0; i < bdies -> len; ++i){
        bdy = g_ptr_array_index(bdies, i);
        qq_buddy_list_add_buddy(widget, bdy);
    }
}

void qq_buddy_list_add_groups(GtkWidget *widget, GPtrArray *grps)
{
    if(widget == NULL || grps == NULL){
        return;
    }
    gint i;
    QQGroup *grp;
    for(i = 0; i < grps -> len; ++i){
        grp = g_ptr_array_index(grps, i);
        qq_buddy_list_add_group(widget, grp);
    }
}

void qq_buddy_list_update_buddy_info(GtkWidget *widget, QQBuddy *bdy)
{
    if(widget == NULL || bdy == NULL){
        return;
    }

    QQBuddyListPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                                , qq_buddy_list_get_type()
                                                , QQBuddyListPriv); 
    GtkTreeRowReference *ref = g_hash_table_lookup(priv -> row_map
                                            , bdy -> uin -> str);
    if(ref == NULL){
        return;
    }
    GtkTreeView *view = GTK_TREE_VIEW(widget);
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(view));
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_row_reference_get_path(ref);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    qq_buddy_list_set_buddy(store, &iter, bdy);
    gtk_tree_path_free(path);
}

void qq_buddy_list_update_group_info(GtkWidget *widget, QQGroup *grp)
{
    if(widget == NULL || grp == NULL){
        return;
    }

    QQBuddyListPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                                , qq_buddy_list_get_type()
                                                , QQBuddyListPriv); 
    GtkTreeRowReference *ref = g_hash_table_lookup(priv -> row_map
                                            , grp -> gid -> str);
    if(ref == NULL){
        return;
    }
    GtkTreeView *view = GTK_TREE_VIEW(widget);
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(view));
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_row_reference_get_path(ref);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    qq_buddy_list_set_group(store, &iter, grp);
    gtk_tree_path_free(path);
}
void qq_buddy_list_update_buddies_info(GtkWidget *widget, GPtrArray *bdies)
{
    if(widget == NULL || bdies == NULL){
        return;
    }
    gint i;
    QQBuddy *bdy;
    for(i = 0; i < bdies -> len; ++i){
        bdy = g_ptr_array_index(bdies, i);
        qq_buddy_list_update_buddy_info(widget, bdy);
    }
}

void qq_buddy_list_update_groups_info(GtkWidget *widget, GPtrArray *grps)
{
    if(widget == NULL || grps == NULL){
        return;
    }
    gint i;
    QQGroup *grp;
    for(i = 0; i < grps -> len; ++i){
        grp = g_ptr_array_index(grps, i);
        qq_buddy_list_update_group_info(widget, grp);
    }
}


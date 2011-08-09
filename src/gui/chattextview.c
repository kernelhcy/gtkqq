#include <chattextview.h>
#include <gqqconfig.h>
#include <stdlib.h>

extern QQInfo *info;
extern GQQConfig *cfg;

static void qq_chat_textview_init(QQChatTextview *view);
static void qq_chat_textviewclass_init(QQChatTextviewClass *klass);

typedef struct{
    //QQMsgFont array
    GPtrArray *msgfonts;

    GtkTextMark *end_mark, *inserted_left_mark;
}QQChatTextviewPriv;

GType qq_chat_textview_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQChatTextviewClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_chat_textviewclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQChatTextview),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_chat_textview_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_TEXT_VIEW, "QQChatTextview"
                    , &info, 0);
    }
    return t;
}

GtkWidget* qq_chat_textview_new()
{
    return GTK_WIDGET(g_object_new(qq_chat_textview_get_type(), NULL));
}

//
// Create the tags for the text buffer
//
static void qq_chatwindow_text_buffer_create_tags(GtkTextBuffer *textbuf)
{
	gtk_text_buffer_create_tag(textbuf, "blue", "foreground", "blue", NULL);
	gtk_text_buffer_create_tag(textbuf, "grey", "foreground", "grey", NULL);
	gtk_text_buffer_create_tag(textbuf, "green", "foreground", "darkgreen", NULL);
	gtk_text_buffer_create_tag(textbuf, "red", "foreground", "red", NULL);
    gtk_text_buffer_create_tag(textbuf, "italic"
                                    , "style", PANGO_STYLE_ITALIC, NULL);
    gtk_text_buffer_create_tag(textbuf, "bold"
                                    , "weight", PANGO_WEIGHT_BOLD, NULL);  
    gtk_text_buffer_create_tag(textbuf, "underline"
                                    , "underline", PANGO_UNDERLINE_SINGLE
                                    , NULL);
}

//
// Add message 
// @param uin : the uin
// @param contents : QQMsgContent instance array
// @param time : the time. ms
// @param color_tag : the color tag name
//
static void qq_chat_textview_add_message(QQChatTextview *view
                                , const gchar *uin
                                , GPtrArray *contents
                                , const gchar *time
                                , const gchar *color_tag)
{
    if(uin == NULL || contents == NULL || time == NULL){
        return;
    }
    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    view, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);

    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    GtkTextIter start_iter, end_iter;
    gint64 timev = (gint64)strtoll(time, NULL, 10);
    GDateTime *t = g_date_time_new_from_unix_local(timev);
    gchar head[500];
    QQBuddy *bdy = qq_info_lookup_buddy_by_uin(info, uin);
    const gchar *name = NULL;
    if(bdy == NULL){
        name = uin;
    }else{
        name = bdy -> markname -> str;
        if(bdy -> markname -> len <= 0){
            name = bdy -> nick -> str;
        }

    }
    if(gtk_text_buffer_get_line_count(textbuf) > 1){
        // insert new line
        gtk_text_buffer_get_end_iter(textbuf, &end_iter);
        gtk_text_buffer_insert(textbuf, &end_iter, "\n", -1);
    }
    // insert head
    gint head_len;
    head_len = g_snprintf(head, 500, "%s %d-%d-%d %d:%d:%d\n", name
                                , g_date_time_get_year(t)
                                , g_date_time_get_month(t)
                                , g_date_time_get_day_of_month(t)
                                , g_date_time_get_hour(t)
                                , g_date_time_get_minute(t)
                                , g_date_time_get_second(t));
    gtk_text_buffer_get_end_iter(textbuf, &end_iter);
    gtk_text_buffer_insert_with_tags_by_name(textbuf
                                , &end_iter
                                , head, head_len
                                , color_tag, NULL);

    gint i;
    QQMsgContent *cent;
    QQMsgFont *font = NULL;
    // find the font
    for(i = 0; i < contents -> len; ++i){
        cent = g_ptr_array_index(contents, i);
        if(cent == NULL){
            continue;
        }
        if(cent -> type == 3){
            font = cent -> value.font;
            break;
        }
    }
    gchar fonttagname[100];
    gboolean gotfonttag = FALSE;
    // We already has this font tag?
    if(font != NULL){
        for(i = 0; i < priv -> msgfonts -> len; ++i){
            if(qq_msgfont_equal(font, g_ptr_array_index(priv -> msgfonts, i))){
                // Yes, we have.
                // Set font to it.
                g_snprintf(fonttagname, 100, "msgfonttag%d", i);
                gotfonttag = TRUE;
                break;
            }
        }
        if(!gotfonttag){
            g_snprintf(fonttagname, 100, "msgfonttag%d", priv -> msgfonts -> len);
            g_string_prepend(font -> color, "#");
            gtk_text_buffer_create_tag(textbuf, fonttagname
                                    , "family", font -> name -> str
                                    , "foreground", font -> color -> str
                                    , "size", font -> size * PANGO_SCALE
                                    ,  NULL);
            // erase #
            g_string_erase(font -> color, 0, 1);
            gotfonttag = TRUE;
            // save the font
            g_ptr_array_add(priv -> msgfonts, qq_msgfont_new(
                                                        font -> name -> str
                                                        , font -> size
                                                        , font -> color -> str
                                                        , font -> style.a
                                                        , font -> style.b
                                                        , font -> style.c));
        }
    }

    gchar path[500];
    GtkWidget *img;
    GtkTextChildAnchor *anchor = NULL;
    for(i = 0; i < contents -> len; ++i){
        cent = g_ptr_array_index(contents, i);
        if(cent == NULL){
            continue;
        }
        switch(cent -> type)
        {
        case 1:         // face
            if(cent -> value.face > 134){
                break;
            }
            g_snprintf(path, 500, IMGDIR"/qqfaces/%d.gif", cent -> value.face);
            img = gtk_image_new_from_file(path);
            gtk_widget_show(img);
            gtk_text_buffer_get_end_iter(textbuf, &end_iter);
            anchor = gtk_text_buffer_create_child_anchor(textbuf, &end_iter);
            gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(view)
                                                , img, anchor);
            break;
        case 2:         // string
            gtk_text_buffer_get_end_iter(textbuf, &end_iter);
            gtk_text_buffer_move_mark(textbuf, priv -> inserted_left_mark
                                                , &end_iter);
            gtk_text_buffer_insert(textbuf, &end_iter
                                        , cent -> value.str -> str
                                        , cent -> value.str -> len);
            // apply font
            if(gotfonttag){
                gtk_text_buffer_get_iter_at_mark(textbuf, &start_iter
                                                , priv -> inserted_left_mark);
                gtk_text_buffer_apply_tag_by_name(textbuf, fonttagname 
                                                , &start_iter
                                                , &end_iter);
                if(font -> style.a == 1){
                    gtk_text_buffer_apply_tag_by_name(textbuf, "bold"
                                                , &start_iter
                                                , &end_iter);
                }
                if(font -> style.b == 1){
                    gtk_text_buffer_apply_tag_by_name(textbuf, "italic"
                                                , &start_iter
                                                , &end_iter);
                }
                if(font -> style.c == 1){
                    gtk_text_buffer_apply_tag_by_name(textbuf, "underline"
                                                , &start_iter
                                                , &end_iter);
                }
            }
            break;
        case 3:         // font
            break;
        default:
            break;
        }
    }

    // insert new line
    gtk_text_buffer_get_end_iter(textbuf, &end_iter);
    gtk_text_buffer_insert(textbuf, &end_iter, "\n", -1);
    // scroll the text view to the end
    gtk_text_iter_set_line_offset(&end_iter, 0);
    gtk_text_buffer_move_mark(textbuf, priv -> end_mark, &end_iter);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(view)
                                            , priv -> end_mark);
    return;
}

static void qq_chat_textview_init(QQChatTextview *view)
{
	GtkTextBuffer *textbuf = gtk_text_view_get_buffer(
                                    GTK_TEXT_VIEW(view));
    qq_chatwindow_text_buffer_create_tags(textbuf);

    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    view, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);

    GtkTextIter iter;
	gtk_text_buffer_get_end_iter(textbuf, &iter);
	priv -> end_mark = gtk_text_buffer_create_mark(textbuf, "scrollend"
                                                    , &iter, FALSE);
	priv -> inserted_left_mark = gtk_text_buffer_create_mark(textbuf
                                                    , "insertedleft"
                                                    , &iter, TRUE);
    priv -> msgfonts = g_ptr_array_new();
}

static void qq_chat_textviewclass_init(QQChatTextviewClass *klass)
{
    g_type_class_add_private(klass, sizeof(QQChatTextviewPriv));
}

void qq_chat_textview_add_send_message(GtkWidget *widget, QQSendMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }
    gchar buf[100];
    GTimeVal now;
    g_get_current_time(&now);
    g_snprintf(buf, 100, "%ld", now.tv_sec);
    qq_chat_textview_add_message(QQ_CHAT_TEXTVIEW(widget)
                                , info -> me -> uin -> str
                                , msg -> contents, buf, "blue");
}
void qq_chat_textview_add_recv_message(GtkWidget *widget, QQRecvMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    qq_chat_textview_add_message(QQ_CHAT_TEXTVIEW(widget)
                                , msg -> from_uin -> str
                                , msg -> contents, msg -> time -> str
                                , "green");
}

void qq_chat_textview_add_face(GtkWidget *widget, gint face)
{
    if(widget == NULL || face > 134){
        return;
    }
    gchar path[500];
    g_snprintf(path, 500, IMGDIR"/qqfaces/%d.gif", face);
    GtkWidget *img = gtk_image_new_from_file(path);
    gtk_widget_show(img);
    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    GtkTextIter iter;
    GtkTextChildAnchor *anchor;
    gtk_text_buffer_get_end_iter(textbuf, &iter);
    anchor = gtk_text_buffer_create_child_anchor(textbuf, &iter);
    gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(widget), img, anchor);
}

QQSendMsg* qq_chat_textview_create_sendmsg(GtkWidget *widget)
{
    return NULL;
}

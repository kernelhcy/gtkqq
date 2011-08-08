#include <chatwindow.h>
#include <qq.h>
#include <gqqconfig.h>
#include <stdlib.h>

extern QQInfo *info;
extern GQQConfig *cfg;

static void qq_chatwindow_init(QQChatWindow *win);
static void qq_chatwindowclass_init(QQChatWindowClass *wc);

static const gchar *font_names[] = {"宋体", "黑体", "隶书", "微软雅黑"
                                    , "楷体_GB2312", "幼圆", "Arial"
                                    , "Arial Black", "Times New Roman"
                                    , "Verdana", NULL}; 

//
// Private members
//
typedef struct{
    GtkWidget *body_vbox;

    GtkWidget *faceimage;
    GtkWidget *name, *lnick;

    // The message text area
    GtkWidget *message_textview;
    GtkTextIter message_iter;

    // Font tool box
    GtkWidget *font_tool_box;
    GtkWidget *font_cb, *size_cb, *bold_btn, *italic_btn
                        , *underline_btn, *color_btn;

    // Tool bar
    GtkWidget *tool_bar;
    GtkToolItem *font_item, *face_item, *sendfile_item
                , *sendpic_item, *clear_item, *history_item;

    GtkWidget *input_textview;
    
    GtkWidget *send_btn, *close_btn;
}QQChatWindowPriv;

//
// Close button clicked handler
//
static void qq_chatwindow_on_close_clicked(GtkWidget *widget, gpointer  data)
{
    gtk_widget_destroy(data);
    return;
}

GType qq_chatwindow_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQChatWindowClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_chatwindowclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQChatWindow),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_chatwindow_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_WINDOW, "QQChatWindow"
                    , &info, 0);
    }
    return t;
}

GtkWidget* qq_chatwindow_new()
{
    return GTK_WIDGET(g_object_new(qq_chatwindow_get_type()
                        , "type", GTK_WINDOW_TOPLEVEL, NULL));
}

//
// Create a toggle button with stock
//
static GtkWidget* qq_toggle_button_new_with_stock(const gchar *stock_id)
{
    GtkWidget *btn = gtk_toggle_button_new();
    if(stock_id != NULL){
        GtkWidget *img = gtk_image_new_from_stock(stock_id
                                    , GTK_ICON_SIZE_LARGE_TOOLBAR);
        gtk_button_set_image(GTK_BUTTON(btn), img);
    }
    return btn;
}

//
// font tool button func
//
static void font_select_func(GtkToggleToolButton *btn, gpointer data)
{
    QQChatWindowPriv *priv = data;
    if(gtk_toggle_tool_button_get_active(btn)){
        gtk_box_pack_start(GTK_BOX(priv -> body_vbox), priv -> font_tool_box
                                            , FALSE, FALSE, 0); 
        gtk_box_reorder_child(GTK_BOX(priv -> body_vbox)
                                            , priv -> font_tool_box, 2);
        gtk_widget_show_all(priv -> font_tool_box);
    }else{
        gtk_container_remove(GTK_CONTAINER(priv -> body_vbox)
                                            , priv -> font_tool_box);
    }
}

//
// Create the tags for the text buffer
//
static void qq_chatwindow_text_buffer_create_tags(GtkTextBuffer *textbuf)
{
	gtk_text_buffer_create_tag(textbuf, "blue", "foreground", "#639900", NULL);
	gtk_text_buffer_create_tag(textbuf, "grey", "foreground", "#808080", NULL);
	gtk_text_buffer_create_tag(textbuf, "green", "foreground", "green", NULL);
	gtk_text_buffer_create_tag(textbuf, "red", "foreground", "#0088bf", NULL);
    gtk_text_buffer_create_tag(textbuf, "italic"
                                    , "style", PANGO_STYLE_ITALIC, NULL);
    gtk_text_buffer_create_tag(textbuf, "bold"
                                    , "weight", PANGO_WEIGHT_BOLD, NULL);  
    gtk_text_buffer_create_tag(textbuf, "underline"
                                    , "underline", PANGO_UNDERLINE_SINGLE
                                    , NULL);

}

static void qq_chatwindow_init(QQChatWindow *win)
{
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);

    GtkWidget *scrolled_win; 
    priv -> body_vbox = gtk_vbox_new(FALSE, 0);

    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 0);
    //GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

    //create header
    GdkPixbuf *pb = NULL;
    pb= gdk_pixbuf_new_from_file_at_size(IMGDIR"avatar.gif", 35, 35, NULL);
    priv -> faceimage = gtk_image_new_from_pixbuf(pb);
    g_object_unref(pb);
    priv -> name = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(priv -> name), "<b>nick</b>");
    priv -> lnick = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(priv -> lnick), "<b>long nick</b>");
    gtk_box_pack_start(GTK_BOX(header_hbox), priv -> faceimage
                                        , FALSE, FALSE, 5); 
    gtk_box_pack_start(GTK_BOX(vbox), priv -> name, FALSE, FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(vbox), priv -> lnick, FALSE, FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(header_hbox), vbox, FALSE, FALSE, 5); 
    gtk_box_pack_start(GTK_BOX(priv -> body_vbox), header_hbox
                                        , FALSE, FALSE, 5);

    // message text view
    priv -> message_textview = gtk_text_view_new(); 
    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win)
                                        , GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(scrolled_win), priv -> message_textview);
    gtk_box_pack_start(GTK_BOX(priv -> body_vbox), scrolled_win
                                                , TRUE, TRUE, 0); 
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv -> message_textview)
                                                , FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv -> message_textview)
                                                , GTK_WRAP_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(priv -> message_textview)
                                                , FALSE);

	GtkTextBuffer *textbuf = gtk_text_view_get_buffer(
                                    GTK_TEXT_VIEW(priv -> message_textview));
    qq_chatwindow_text_buffer_create_tags(textbuf);
	gtk_text_buffer_get_end_iter(textbuf, &(priv -> message_iter));
	gtk_text_buffer_create_mark(textbuf, "scrollend"
                                , &(priv -> message_iter), FALSE);

    // font tools
    priv -> font_tool_box = gtk_hbox_new(FALSE, 5);
    g_object_ref(priv -> font_tool_box);
    priv -> font_cb = gtk_combo_box_text_new();
    gint i;
    for(i = 0; font_names[i] != NULL; ++i){
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv -> font_cb)
                                        , font_names[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv -> font_cb), 1);
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> font_cb
                                        , FALSE, FALSE, 0); 
    priv -> size_cb = gtk_combo_box_text_new();
    gchar buf[10];
    for(i = 8; i < 23; ++i){
        g_snprintf(buf, 10, "%d", i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv -> size_cb)
                                        , buf);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv -> size_cb), 3);
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> size_cb
                                        , FALSE, FALSE, 0); 
    priv -> bold_btn = qq_toggle_button_new_with_stock(GTK_STOCK_BOLD);
    priv -> italic_btn = qq_toggle_button_new_with_stock(GTK_STOCK_ITALIC);
    priv -> underline_btn = qq_toggle_button_new_with_stock(
                                            GTK_STOCK_UNDERLINE);

    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> bold_btn 
                                        , FALSE, FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> italic_btn
                                        , FALSE, FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> underline_btn
                                        , FALSE, FALSE, 0); 
    priv -> color_btn = gtk_color_button_new();
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> color_btn
                                        , FALSE, FALSE, 0); 

    // tool bar
    priv -> tool_bar = gtk_toolbar_new();
    GtkWidget *img = NULL;

    img = gtk_image_new_from_file(IMGDIR"/selectfont.png");
    priv -> font_item = gtk_toggle_tool_button_new();
    g_signal_connect(G_OBJECT(priv -> font_item), "toggled",
                             G_CALLBACK(font_select_func), priv);
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(priv -> font_item), img);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> font_item, -1);

    img = gtk_image_new_from_file(IMGDIR"/selectface.png");
    priv -> face_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> face_item, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar)
                                , gtk_separator_tool_item_new() , -1);

    img = gtk_image_new_from_file(IMGDIR"/sendfile.png");
    priv -> sendfile_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> sendfile_item, -1);

    img = gtk_image_new_from_file(IMGDIR"/sendpic.png");
    priv -> sendpic_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> sendpic_item, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar)
                                , gtk_separator_tool_item_new() , -1);

    img = gtk_image_new_from_file(IMGDIR"/clearscreen.png");
    priv -> clear_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> clear_item, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar)
                                , gtk_separator_tool_item_new() , -1);

    img = gtk_image_new_from_file(IMGDIR"/showhistory.png");
    priv -> history_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> history_item, -1);
    gtk_box_pack_start(GTK_BOX(priv -> body_vbox), priv -> tool_bar
                                                , FALSE, FALSE, 0); 

    // input text view
    priv -> input_textview = gtk_text_view_new(); 
    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_win),
                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win)
                                        , GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(scrolled_win), priv -> input_textview);
    gtk_box_pack_start(GTK_BOX(priv -> body_vbox), scrolled_win, FALSE, FALSE, 0); 
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv -> input_textview)
                                                , GTK_WRAP_CHAR);

	textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv -> input_textview));
    qq_chatwindow_text_buffer_create_tags(textbuf);

    // buttons
    GtkWidget *buttonbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(buttonbox), 5);
    priv -> close_btn = gtk_button_new_with_label("Close");
    g_signal_connect(G_OBJECT(priv -> close_btn), "clicked",
                             G_CALLBACK(qq_chatwindow_on_close_clicked), win);
    priv -> send_btn = gtk_button_new_with_label("Send");
    gtk_container_add(GTK_CONTAINER(buttonbox), priv -> close_btn);
    gtk_container_add(GTK_CONTAINER(buttonbox), priv -> send_btn);
    gtk_box_pack_start(GTK_BOX(priv -> body_vbox), buttonbox, FALSE, FALSE, 3); 

    GtkWidget *w = GTK_WIDGET(win);
    //gtk_widget_set_size_request(w, 500, 500);
    gtk_window_resize(GTK_WINDOW(w), 500, 450);

    gtk_container_add(GTK_CONTAINER(win), priv -> body_vbox);
}

static void qq_chatwindowclass_init(QQChatWindowClass *wc)
{
    g_type_class_add_private(wc, sizeof(QQChatWindowPriv));
}

//
// Add message into message_textarea
// @param uin : the uin
// @param contents : QQMsgContent instance array
// @param time : the time. ms
// @param color_tag : the color tag name
//
void qq_chatwindow_add_message(GtkWidget *widget, const gchar *uin
                                , GPtrArray *contents
                                , const gchar *time
                                , const gchar *color_tag)
{
    if(uin == NULL || contents == NULL || time == NULL){
        return;
    }
    QQChatWindow *win = QQ_CHATWINDOW(widget);
    QQChatWindowPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    win, qq_chatwindow_get_type()
                                    , QQChatWindowPriv);

    GtkWidget *msg_textarea = priv -> message_textview;
    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(
                                            msg_textarea));

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
    gint head_len;
    head_len = g_snprintf(head, 500, "%s %d-%d-%d %d:%d:%d\n", name
                                , g_date_time_get_year(t)
                                , g_date_time_get_month(t)
                                , g_date_time_get_day_of_month(t)
                                , g_date_time_get_hour(t)
                                , g_date_time_get_minute(t)
                                , g_date_time_get_second(t));
    gtk_text_buffer_get_end_iter(textbuf, &(priv -> message_iter));
    gtk_text_buffer_insert_with_tags_by_name(textbuf
                                , &(priv -> message_iter)
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

    GtkTextTag *tmptag = NULL;
    GtkTextTagTable *tagtable = NULL;
    if(font != NULL){
        g_string_prepend(font -> color, "#");
        tmptag = gtk_text_buffer_create_tag(textbuf, "tmpfonttag"
                                    , "family", font -> name -> str
                                    , "foreground", font -> color -> str
                                    , "size", font -> size * PANGO_SCALE
                                    ,  NULL);
    }
    gchar path[500];
    GtkWidget *img;
    GtkTextIter start_iter, end_iter;
	GtkTextChildAnchor *anchor;
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
            gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(msg_textarea)
                                                , img, anchor);
            break;
        case 2:         // string
            gtk_text_buffer_get_end_iter(textbuf, &end_iter);
            anchor = gtk_text_buffer_create_child_anchor(textbuf
                                                , &end_iter);
            gtk_text_buffer_insert(textbuf, &end_iter
                                        , cent -> value.str -> str
                                        , cent -> value.str -> len);
            // apply font
            if(font != NULL){
                gtk_text_buffer_get_iter_at_child_anchor(textbuf, &start_iter
                                                , anchor);
                gtk_text_buffer_apply_tag_by_name(textbuf, "tmpfonttag"
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
                // remove tmp tag
                tagtable = gtk_text_buffer_get_tag_table(textbuf);
                gtk_text_tag_table_remove(tagtable, tmptag);
            }
            break;
        case 3:         // font
            break;
        default:
            break;
        }
    }
}

void qq_chatwindow_add_send_message(GtkWidget *widget, QQSendMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }
    gchar buf[100];
    GTimeVal now;
    g_get_current_time(&now);
    g_snprintf(buf, 100, "%ld", now.tv_usec);
    qq_chatwindow_add_message(widget, info -> me -> uin -> str
                                , msg -> contents, buf, "blue");
}
void qq_chatwindow_add_recv_message(GtkWidget *widget, QQRecvMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    qq_chatwindow_add_message(widget, msg -> from_uin -> str
                                , msg -> contents, msg -> time -> str
                                , "green");
}

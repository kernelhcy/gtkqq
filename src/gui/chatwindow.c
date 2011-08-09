#include <chatwindow.h>
#include <gqqconfig.h>
#include <stdlib.h>
#include <chattextview.h>
#include <facepopupwindow.h>

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

    // Font tool box
    GtkWidget *font_tool_box;
    GtkWidget *font_cb, *size_cb, *bold_btn, *italic_btn
                        , *underline_btn, *color_btn;

    // Tool bar
    GtkWidget *tool_bar;
    GtkToolItem *font_item, *face_item, *sendfile_item
                , *sendpic_item, *clear_item, *history_item;
    GtkWidget *facepopupwindow;

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
// Face tool button clicked handler
//
static void face_tool_button_clicked(GtkToolButton *btn, gpointer data)
{
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);
	int x , y , ex , ey , root_x , root_y;

	gtk_widget_translate_coordinates(GTK_WIDGET(btn), data, 0, 0, &ex, &ey);
	gtk_window_get_position(GTK_WINDOW(data), &root_x, &root_y);
	x = root_x + ex + 3;
	y = root_y + ey + 46;
    qq_face_popup_window_popup(priv -> facepopupwindow, x, y);
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
    priv -> message_textview = qq_chat_textview_new(); 
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
    g_signal_connect(G_OBJECT(priv -> face_item), "clicked",
                             G_CALLBACK(face_tool_button_clicked), win);

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
    priv -> input_textview = qq_chat_textview_new(); 
    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_win),
                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win)
                                        , GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(scrolled_win), priv -> input_textview);
    gtk_box_pack_start(GTK_BOX(priv -> body_vbox), scrolled_win, FALSE, FALSE, 0); 
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv -> input_textview)
                                                , GTK_WRAP_CHAR);

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
    g_signal_connect(G_OBJECT(w), "destroy",
                             G_CALLBACK(qq_chatwindow_on_close_clicked), win);
    gtk_container_add(GTK_CONTAINER(win), priv -> body_vbox);

    gtk_widget_show_all(w);
    gtk_widget_grab_focus(priv -> input_textview);

    priv -> facepopupwindow = qq_face_popup_window_new();
}

static void qq_chatwindowclass_init(QQChatWindowClass *wc)
{
    g_type_class_add_private(wc, sizeof(QQChatWindowPriv));
}


void qq_chatwindow_add_send_message(GtkWidget *widget, QQSendMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);

    qq_chat_textview_add_send_message(priv -> message_textview, msg);
}
void qq_chatwindow_add_recv_message(GtkWidget *widget, QQRecvMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);
    qq_chat_textview_add_recv_message(priv -> message_textview, msg);
}

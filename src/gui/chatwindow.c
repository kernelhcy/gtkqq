#include <chatwindow.h>
#include <gqqconfig.h>
#include <stdlib.h>
#include <chattextview.h>
#include <facepopupwindow.h>
#include <tray.h>
#include <msgloop.h>

extern QQInfo *info;
extern GQQConfig *cfg;
extern QQTray *tray;
extern GQQMessageLoop *send_loop;

static void qq_chatwindow_init(QQChatWindow *win);
static void qq_chatwindowclass_init(QQChatWindowClass *wc);

static const gchar *font_names[] = {"宋体", "黑体", "隶书", "微软雅黑"
                                    , "楷体_GB2312", "幼圆", "Arial"
                                    , "Arial Black", "Times New Roman"
                                    , "Verdana", NULL}; 
enum{
    QQ_CHATWINDOW_PROPERTY_UIN = 1,
    QQ_CHATWINDOW_PROPERTY_NAME,
    QQ_CHATWINDOW_PROPERTY_QQNUMBER,
    QQ_CHATWINDOW_PROPERTY_LNICK,
    QQ_CHATWINDOW_PROPERTY_STATUS,
    QQ_CHATWINDOW_PROPERTY_UNKNOWN
};

//
// Private members
//
typedef struct{
    gchar uin[100];
    gchar status[100];
    gchar qqnumber[100];
    GString *name, *lnick;

    GtkWidget *body_vbox;

    GtkWidget *faceimage;
    GtkWidget *name_label, *lnick_label;

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
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);
    gqq_config_remove_ht(cfg, "chat_window_map", priv -> uin);
    gtk_widget_destroy(data);
    return;
}

//
// Send message
// Run in the send_loop
//
static void qq_chatwindow_send_msg_cb(GtkWidget *widget, QQSendMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }
    GError *err = NULL;
    gint ret = qq_send_message(info, msg, &err);
    if(ret != 0){
        // send error
        g_warning("Send message error!! %s (%s, %d)"
                            , err == NULL ? "" : err -> message
                            , __FILE__, __LINE__);
        g_error_free(err);
    }
}

//
// Send button clicked handler
//
static void qq_chatwindow_on_send_clicked(GtkWidget *widget, gpointer  data)
{
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);
    GPtrArray *cs = g_ptr_array_new();
    qq_chat_textview_get_msg_contents(priv -> input_textview, cs); 
    qq_chat_textview_clear(priv -> input_textview);

    QQSendMsg *msg = qq_sendmsg_new(info, MSG_BUDDY_T, priv -> uin);
    gint i;
    for(i = 0; i < cs -> len; ++i){
        qq_sendmsg_add_content(msg, g_ptr_array_index(cs, i));
    }
    g_ptr_array_free(cs, TRUE);

    // font
    gchar *name = NULL, *color = NULL, *sizestr = NULL;
    gint size, a = 0, b = 0, c = 0;

    name = gtk_combo_box_text_get_active_text(
                        GTK_COMBO_BOX_TEXT(priv -> font_cb));
    if(name == NULL){
        name = "宋体";
    }
    GdkColor gc;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(priv -> color_btn), &gc);
    color = gdk_color_to_string(&gc);
    if(color == NULL){
        color = g_strdup("#ffffff");
    }

    sizestr = gtk_combo_box_text_get_active_text(
                        GTK_COMBO_BOX_TEXT(priv -> size_cb));
    if(sizestr == NULL){
        size = 11;
    }else{
        size = (gint)strtol(sizestr, NULL, 10);
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> bold_btn))){
        a = 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> italic_btn))){
        b = 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> underline_btn))){
        c = 1;
    }
    QQMsgContent *font = qq_msgcontent_new(QQ_MSG_CONTENT_FONT_T, name, size
                                                , color + 1, a, b, c);
    g_free(name);
    g_free(color);
    g_free(sizestr);
    qq_sendmsg_add_content(msg, font);

    qq_chat_textview_add_send_message(priv -> message_textview, msg);
    gqq_mainloop_attach(send_loop, qq_chatwindow_send_msg_cb
                                , 2, data, msg);
    return;
}

static gboolean qq_chatwindow_delete_event(GtkWidget *widget, GdkEvent *event
                                        , gpointer data)
{
    QQChatWindowPriv *priv = data;
    gqq_config_remove_ht(cfg, "chat_window_map", priv -> uin);
    return FALSE;
}

//
// Foucus in event
// Stop blinking the tray
//
static gboolean qq_chatwindow_focus_in_event(GtkWidget *widget, GdkEvent *event
                                                , gpointer data)
{
    QQChatWindowPriv *priv = data;
    qq_tray_stop_blinking_for(tray, priv -> uin);
    g_debug("Focus in chatwindow of %s (%s, %d)", priv -> uin
                                    , __FILE__, __LINE__);
    return FALSE;
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

GtkWidget* qq_chatwindow_new(const gchar *uin, const gchar *name
                            , const gchar *qqnumber, const gchar *status
                            , const gchar *lnick)
{
    return GTK_WIDGET(g_object_new(qq_chatwindow_get_type()
                                    , "type", GTK_WINDOW_TOPLEVEL
                                    , "qqnumber", qqnumber
                                    , "uin", uin
                                    , "status", status
                                    , "name", name
                                    , "lnick", lnick
                                    , NULL));
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
// Clear the message text view
//
static void clear_button_clicked(GtkToolButton *btn, gpointer data)
{
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);
    qq_chat_textview_clear(priv -> message_textview);
}
//
// font tool button func
//
static void qq_chat_view_font_button_clicked(GtkToggleToolButton *btn, gpointer data)
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

static void qq_chat_window_font_changed(GtkWidget *widget, gpointer data)
{
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);
    gchar *name = NULL, *color, *sizestr = NULL;
    gint size, a = 0, b = 0, c = 0;

    name = gtk_combo_box_text_get_active_text(
                        GTK_COMBO_BOX_TEXT(priv -> font_cb));
    if(name == NULL){
        return;
    }
    
    GdkColor gc;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(priv -> color_btn), &gc);
    color = gdk_color_to_string(&gc);
    g_debug("Set text view color %s (%u,%u,%u)(%s, %d)", color, gc.red
                                , gc.green, gc.blue, __FILE__, __LINE__);

    sizestr = gtk_combo_box_text_get_active_text(
                        GTK_COMBO_BOX_TEXT(priv -> size_cb));
    if(sizestr == NULL){
        goto out_label;
    }
    size = (gint)strtol(sizestr, NULL, 10);

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> bold_btn))){
        a = 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> italic_btn))){
        b = 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> underline_btn))){
        c = 1;
    }
    qq_chat_textview_set_default_font(priv -> input_textview, 
                                        name, color, size, a, b, c);

out_label:
    g_free(name);
    g_free(color);
    g_free(sizestr);
}

//
// Face clicked
//
static void qq_chatwindow_face_clicked(gpointer instance, gint face
                                            , gpointer data)
{
    QQChatWindowPriv *priv = data;
    qq_chat_textview_add_face(priv -> input_textview, face);
}

static void qq_chatwindow_init(QQChatWindow *win)
{
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);
    priv -> name = g_string_new(NULL);
    priv -> lnick = g_string_new(NULL);

    gchar buf[500];
    GtkWidget *scrolled_win; 
    priv -> body_vbox = gtk_vbox_new(FALSE, 0);

    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 0);
    //GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

    GdkPixbuf *pb = NULL;
    g_snprintf(buf, 500, IMGDIR"%s", "avatar.gif");
    pb = gdk_pixbuf_new_from_file(buf, NULL);
    gtk_window_set_icon(GTK_WINDOW(win), pb);
    g_object_unref(pb);
    g_snprintf(buf, 500, "Talking with %s", priv -> qqnumber);
    gtk_window_set_title(GTK_WINDOW(win), buf);

    //create header
    g_snprintf(buf, 500, IMGDIR"%s", "avatar.gif");
    pb= gdk_pixbuf_new_from_file_at_size(buf, 35, 35, NULL);
    priv -> faceimage = gtk_image_new_from_pixbuf(pb);
    g_object_unref(pb);
    priv -> name_label = gtk_label_new("");
    priv -> lnick_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(header_hbox), priv -> faceimage
                                        , FALSE, FALSE, 5); 
    gtk_box_pack_start(GTK_BOX(vbox), priv -> name_label, FALSE, FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(vbox), priv -> lnick_label, FALSE, FALSE, 0); 
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
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> font_cb
                                        , FALSE, FALSE, 0); 
    priv -> size_cb = gtk_combo_box_text_new();
    for(i = 8; i < 23; ++i){
        g_snprintf(buf, 10, "%d", i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv -> size_cb)
                                        , buf);
    }
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
    gtk_widget_show_all(priv -> font_tool_box);

    g_signal_connect(G_OBJECT(priv -> font_cb), "changed"
                    , G_CALLBACK(qq_chat_window_font_changed), win);
    g_signal_connect(G_OBJECT(priv -> size_cb), "changed"
                    , G_CALLBACK(qq_chat_window_font_changed), win);
    g_signal_connect(G_OBJECT(priv -> bold_btn), "toggled"
                        , G_CALLBACK(qq_chat_window_font_changed), win);
    g_signal_connect(G_OBJECT(priv -> italic_btn), "toggled"
                        , G_CALLBACK(qq_chat_window_font_changed), win);
    g_signal_connect(G_OBJECT(priv -> underline_btn), "toggled"
                        , G_CALLBACK(qq_chat_window_font_changed), win);
    g_signal_connect(G_OBJECT(priv -> color_btn), "color-set"
                            , G_CALLBACK(qq_chat_window_font_changed), win);

    // tool bar
    priv -> tool_bar = gtk_toolbar_new();
    GtkWidget *img = NULL;

    img = gtk_image_new_from_file(IMGDIR"/selectfont.png");
    priv -> font_item = gtk_toggle_tool_button_new();
    g_signal_connect(G_OBJECT(priv -> font_item), "toggled",
                             G_CALLBACK(qq_chat_view_font_button_clicked), priv);
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
    g_signal_connect(G_OBJECT(priv -> clear_item), "clicked",
                             G_CALLBACK(clear_button_clicked), win);

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

    gtk_combo_box_set_active(GTK_COMBO_BOX(priv -> font_cb), 1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv -> size_cb), 3);

    // buttons
    GtkWidget *buttonbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(buttonbox), 5);
    priv -> close_btn = gtk_button_new_with_label("Close");
    g_signal_connect(G_OBJECT(priv -> close_btn), "clicked",
                             G_CALLBACK(qq_chatwindow_on_close_clicked), win);
    priv -> send_btn = gtk_button_new_with_label("Send");
    g_signal_connect(G_OBJECT(priv -> send_btn), "clicked",
                             G_CALLBACK(qq_chatwindow_on_send_clicked), win);
    gtk_container_add(GTK_CONTAINER(buttonbox), priv -> close_btn);
    gtk_container_add(GTK_CONTAINER(buttonbox), priv -> send_btn);
    gtk_box_pack_start(GTK_BOX(priv -> body_vbox), buttonbox, FALSE, FALSE, 3); 

    GtkWidget *w = GTK_WIDGET(win);
    //gtk_widget_set_size_request(w, 500, 500);
    gtk_window_resize(GTK_WINDOW(w), 500, 450);
    g_signal_connect(G_OBJECT(w), "delete-event"
                                , G_CALLBACK(qq_chatwindow_delete_event)
                                , priv);
    gtk_container_add(GTK_CONTAINER(win), priv -> body_vbox);

    gtk_widget_show_all(priv -> body_vbox);
    gtk_widget_grab_focus(priv -> input_textview);

    priv -> facepopupwindow = qq_face_popup_window_new();
    g_signal_connect(G_OBJECT(priv -> facepopupwindow), "face-clicked"
                                , G_CALLBACK(qq_chatwindow_face_clicked)
                                , priv);
    g_signal_connect(G_OBJECT(w), "focus-in-event"
                                , G_CALLBACK(qq_chatwindow_focus_in_event)
                                , priv);
}

/*
 * The getter.
 */
static void qq_chatwindow_getter(GObject *object, guint property_id,  
                                    GValue *value, GParamSpec *pspec)
{
    if(object == NULL || value == NULL || property_id < 0){
            return;
    }
    
    QQChatWindow *obj = G_TYPE_CHECK_INSTANCE_CAST(
                                    object, qq_chatwindow_get_type()
                                    , QQChatWindow);
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    obj, qq_chatwindow_get_type()
                                    , QQChatWindowPriv);
    
    switch (property_id)
    {
    case QQ_CHATWINDOW_PROPERTY_UIN:
        g_value_set_string(value, priv -> uin);
        break;
    case QQ_CHATWINDOW_PROPERTY_QQNUMBER:
        g_value_set_string(value, priv -> qqnumber);
        break;
    case QQ_CHATWINDOW_PROPERTY_STATUS:
        g_value_set_string(value, priv -> status);
        break;
    case QQ_CHATWINDOW_PROPERTY_NAME:
        g_value_set_string(value, priv -> name -> str);
        break;
    case QQ_CHATWINDOW_PROPERTY_LNICK:
        g_value_set_string(value, priv -> lnick -> str);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

/*
 * The setter.
 */
static void qq_chatwindow_setter(GObject *object, guint property_id,  
                                 const GValue *value, GParamSpec *pspec)
{
    if(object == NULL || value == NULL || property_id < 0){
            return;
    }
    QQChatWindow *obj = G_TYPE_CHECK_INSTANCE_CAST(
                                    object, qq_chatwindow_get_type()
                                    , QQChatWindow);
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    obj, qq_chatwindow_get_type()
                                    , QQChatWindowPriv);
    gchar buf[500]; 
    GdkPixbuf *pb = NULL;
    switch (property_id)
    {
    case QQ_CHATWINDOW_PROPERTY_UIN:
        g_stpcpy(priv -> uin, g_value_get_string(value));
        break;
    case QQ_CHATWINDOW_PROPERTY_STATUS:
        g_stpcpy(priv -> status, g_value_get_string(value));
        break;
    case QQ_CHATWINDOW_PROPERTY_QQNUMBER:
        g_stpcpy(priv -> qqnumber, g_value_get_string(value));
        break;
    case QQ_CHATWINDOW_PROPERTY_NAME:
        g_string_truncate(priv -> name, 0);
        g_string_append(priv -> name, g_value_get_string(value));
        break;
    case QQ_CHATWINDOW_PROPERTY_LNICK:
        g_string_truncate(priv -> lnick, 0);
        g_string_append(priv -> lnick, g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
    // set lnick
    g_snprintf(buf, 500, "<b>%s</b>", priv -> lnick -> str);
    gtk_label_set_markup(GTK_LABEL(priv -> lnick_label), buf);
    // set face image
    g_snprintf(buf, 500, CONFIGDIR"/faces/%s", priv -> qqnumber);
    pb= gdk_pixbuf_new_from_file_at_size(buf, 35, 35, NULL);
    if(pb == NULL){
        pb= gdk_pixbuf_new_from_file_at_size(IMGDIR"/avatar.gif"
                                        , 35, 35, NULL);
    }
    gtk_image_set_from_pixbuf(GTK_IMAGE(priv -> faceimage), pb);
    // window icon
    gtk_window_set_icon(GTK_WINDOW(obj), pb);
    g_object_unref(pb);
    // set status and name
    if(g_strcmp0("online", priv -> status) == 0 
                    || g_strcmp0("away", priv -> status) == 0
                    || g_strcmp0("busy", priv -> status) == 0
                    || g_strcmp0("silent", priv -> status) == 0
                    || g_strcmp0("callme", priv -> status) == 0){
        gtk_widget_set_sensitive(priv -> faceimage, TRUE);
        g_snprintf(buf, 500, "<b>%s</b><span color='blue'>[%s]</span>"
                                            , priv -> name -> str
                                            , priv -> status);
    }else{
        gtk_widget_set_sensitive(priv -> faceimage, FALSE);
        g_snprintf(buf, 500, "<b>%s</b>", priv -> name -> str);
    }
    gtk_label_set_markup(GTK_LABEL(priv -> name_label), buf);

    // window title
    g_snprintf(buf, 500, "Talking with %s", priv -> name -> str);
    gtk_window_set_title(GTK_WINDOW(obj), buf);
}

static void qq_chatwindowclass_init(QQChatWindowClass *wc)
{
    g_type_class_add_private(wc, sizeof(QQChatWindowPriv));

    G_OBJECT_CLASS(wc) -> get_property = qq_chatwindow_getter;
    G_OBJECT_CLASS(wc) -> set_property = qq_chatwindow_setter;

    //install the uin property
    GParamSpec *pspec;
    pspec = g_param_spec_string("uin"
                                , "QQ uin"
                                , "qq uin"
                                , ""
                                , G_PARAM_READABLE | G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(wc)
                                    , QQ_CHATWINDOW_PROPERTY_UIN, pspec);
    //status
    pspec = g_param_spec_string("status"
                                , "QQ status"
                                , "qq status"
                                , "offline"
                                , G_PARAM_READABLE | G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(wc)
                                    , QQ_CHATWINDOW_PROPERTY_STATUS, pspec);
    //qq number
    pspec = g_param_spec_string("qqnumber"
                                , "QQ number"
                                , "qq number"
                                , ""
                                , G_PARAM_READABLE | G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(wc)
                                    , QQ_CHATWINDOW_PROPERTY_QQNUMBER, pspec);
    // name
    pspec = g_param_spec_string("name"
                                , "QQ mark name"
                                , "qq mark name or the nick name"
                                , ""
                                , G_PARAM_READABLE | G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(wc)
                                    , QQ_CHATWINDOW_PROPERTY_NAME, pspec);
    //long nick
    pspec = g_param_spec_string("lnick"
                                , "QQ lnick"
                                , "qq lnick"
                                , ""
                                , G_PARAM_READABLE | G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(wc)
                                    , QQ_CHATWINDOW_PROPERTY_LNICK, pspec);
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

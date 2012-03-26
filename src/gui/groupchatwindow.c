#include <groupchatwindow.h>
#include <chatwidget.h>
#include <gqqconfig.h>
#include <stdlib.h>
#include <chattextview.h>
#include <facepopupwindow.h>
#include <tray.h>
#include <msgloop.h>
#include <gdk/gdkkeysyms.h>
#include <buddylist.h>

extern QQInfo *info;
extern GQQConfig *cfg;
extern QQTray *tray;

extern GQQMessageLoop *send_loop;
extern GQQMessageLoop *get_info_loop;
//extern GQQMessageLoop *get_number_faceimg_loop;

static GQQMessageLoop gtkloop;
static void qq_group_chatwindow_init(QQGroupChatWindow *win);
static void qq_group_chatwindowclass_init(QQGroupChatWindowClass *wc);

enum{
    QQ_GROUP_CHATWINDOW_PROPERTY_CODE = 1,
    QQ_CHATWINDOW_PROPERTY_UNKNOWN
};

//
// Private members
//
typedef struct{
    gchar code[100];

    GtkWidget *faceimage;
    GtkWidget *name_label, *fingermemo_label;

    // The chat text area
    GtkWidget *chatwidget;

    // memo view
    GtkWidget *memo_textview;
    GtkWidget *memo_frame;
    GtkWidget *memo_scrollwin;
    GtkWidget *memo_load_spinner;
    // member list
    GtkWidget *member_list;
    GtkWidget *member_frame;
    GtkWidget *member_scrollwin;
    GtkWidget *member_load_spinner;

    GtkWidget *send_btn, *close_btn;
}QQGroupChatWindowPriv;

GType qq_group_chatwindow_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQGroupChatWindowClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_group_chatwindowclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQGroupChatWindow),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_group_chatwindow_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_WINDOW, "QQGroupChatWindow"
                                        , &info, 0);
    }
    return t;
}

GtkWidget* qq_group_chatwindow_new(const gchar *code)
{
    return GTK_WIDGET(g_object_new(qq_group_chatwindow_get_type()
                                    , "type", GTK_WINDOW_TOPLEVEL
                                    , "code", code
                                    , NULL));
}

//
// member list tool tip
//
static gboolean qq_member_list_on_show_tooltip(GtkWidget* widget
                                            , int x
                                            , int y
                                            , gboolean keybord_mode
                                            , GtkTooltip* tip
                                            , gpointer data)
{
    gchar *name, *num;
	GtkTreeView *tree = GTK_TREE_VIEW(widget);
    GtkTreeModel *model = gtk_tree_view_get_model(tree);
    GtkTreePath *path;
    GtkTreeIter iter;

	if(!gtk_tree_view_get_tooltip_context(tree , &x , &y , keybord_mode
						, &model , &path , &iter)){
		return FALSE;
    }
    gtk_tree_model_get(model, &iter
                        , BDY_LIST_NAME, &name
                        , BDY_LIST_NUMBER, &num
                        , -1);
    gchar buf[100];
    g_snprintf(buf, 100, "<b>%s</b> <span color='blue'>(%s)</span>"
                                    , name, num);
    gtk_tooltip_set_markup(tip, buf);
    gtk_tree_view_set_tooltip_row(tree, tip, path);

    gtk_tree_path_free(path);
    g_free(name);
    g_free(num);
    return TRUE;
}

static gboolean qq_group_chatwindow_delete_event(GtkWidget *widget, GdkEvent *event
                                        , gpointer data)
{
    QQGroupChatWindowPriv *priv = data;
    gqq_config_remove_ht(cfg, "chat_window_map", priv -> code);
    return FALSE;
}


//
// Remove the spinner and show the memo text view
//
static void qq_group_chatwindow_show_memo(QQGroupChatWindow *win)
{
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    gtk_container_remove(GTK_CONTAINER(priv -> memo_frame)
                                        , priv -> memo_load_spinner);
    gtk_container_add(GTK_CONTAINER(priv -> memo_frame)
                                        , priv -> memo_scrollwin);
}

//
// Remove the spinner and show the member list
//
static void qq_group_chatwindow_show_member(QQGroupChatWindow *win)
{
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    gtk_container_remove(GTK_CONTAINER(priv -> member_frame)
                                        , priv -> member_load_spinner);
    gtk_container_add(GTK_CONTAINER(priv -> member_frame)
                                        , priv -> member_scrollwin);
}


//
// Update the group chat window
//
static void qq_group_chatwindow_update(QQGroupChatWindow *win)
{
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    QQGroup *grp = qq_info_lookup_group_by_code(info, priv -> code);
    if(grp == NULL){
        return;
    }

    gchar buf[500];

    // update header
    g_snprintf(buf, 500, "<b>%s</b>", grp -> name -> str);
    gtk_label_set_markup(GTK_LABEL(priv -> name_label), buf);
    g_snprintf(buf, 500, "<span color='#808080'>%s</span>"
                                    , grp -> fingermemo -> str);
    gtk_label_set_markup(GTK_LABEL(priv -> fingermemo_label), buf);

    // update memo
    g_snprintf(buf, 500, "%s", grp -> memo -> str);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
                                    GTK_TEXT_VIEW(priv -> memo_textview));
    gtk_text_buffer_set_text(buffer, buf, -1);

    // update member list
    GtkListStore *store = (GtkListStore*)gtk_tree_view_get_model(
                                        GTK_TREE_VIEW(priv -> member_list));
    gtk_list_store_clear(store);
    qq_buddy_list_add_group_members(priv -> member_list, grp -> members);

    // show the memo and the group list
    qq_group_chatwindow_show_memo(win);
    qq_group_chatwindow_show_member(win);
}

//
// Remove the memo text view and add a spinner
// Tell the user, we are loading the memo.
//
static void qq_group_chatwindow_show_memo_spinner(QQGroupChatWindow *win)
{
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    gtk_container_remove(GTK_CONTAINER(priv -> memo_frame)
                                        , priv -> memo_scrollwin);
    gtk_container_add(GTK_CONTAINER(priv -> memo_frame)
                                        , priv -> memo_load_spinner);
}

//
// Remove the member list and add a spinner
// Tell the user, we are loading the member list.
//
static void qq_group_chatwindow_show_member_spinner(QQGroupChatWindow *win)
{
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    gtk_container_remove(GTK_CONTAINER(priv -> member_frame)
                                        , priv -> member_scrollwin);
    gtk_container_add(GTK_CONTAINER(priv -> member_frame)
                                        , priv -> member_load_spinner);
}

//
// Get group info
// Run in the get_info_loop loop
//
static void qq_group_chatwindow_get_info(QQGroupChatWindow *win, QQGroup *grp)
{
    gboolean update_info = TRUE;
    if(grp -> gnumber -> len <= 0){
        // get group number
        gchar gnumber[100];
        qq_get_qq_number(info, grp -> code -> str, gnumber, NULL);
        qq_group_set(grp, "gnumber", gnumber);
    }

    if(gqq_config_get_group(cfg, grp) <= 0 ){
        //
        // No recoder found. Get from the server.
        //
        update_info = FALSE;
        qq_get_group_info(info, grp, NULL);
    }

    // update the UI
    gqq_mainloop_attach(&gtkloop, qq_group_chatwindow_update
                                    , 1, win);

    // update the group info
    if(update_info){
        qq_get_group_info(info, grp, NULL);
    }
    // save group info into db
    gqq_config_save_group(cfg, grp);
}

static void qq_group_chatwindow_start_update_info(QQGroupChatWindow *win)
{
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    QQGroup *grp = qq_info_lookup_group_by_code(info, priv -> code);
    if(grp == NULL){
        return;
    }

    if(grp -> members -> len > 0){
        // We have all the group information. Just update the UI.
        qq_group_chatwindow_update(win);
        return;
    }

    // get group information in the get_info_loop loop
    gqq_mainloop_attach(get_info_loop, qq_group_chatwindow_get_info
                                        , 2, win, grp);
}
//
// Close button clicked handler
//
static void qq_group_chatwindow_on_close_clicked(GtkWidget *widget
                                                    , gpointer  data)
{
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    gqq_config_remove_ht(cfg, "chat_window_map", priv -> code);
    gtk_widget_destroy(data);
    return;
}

//
// Send message
// Run in the send_loop
//
static void qq_group_chatwindow_send_msg_cb(GtkWidget *widget
                                                , QQSendMsg *msg)
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
    qq_sendmsg_free(msg);
}

//
// Send button clicked handler
//
static void qq_group_chatwindow_on_send_clicked(GtkWidget *widget
                                                    , gpointer  data)
{
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    QQGroup *grp = qq_info_lookup_group_by_code(info, priv -> code);
    if(grp == NULL){
        g_warning("No group! code : %s (%s,%d)", priv -> code
                                                , __FILE__, __LINE__);
        return;
    }
    GPtrArray *cs = g_ptr_array_new();
    qq_chat_textview_get_msg_contents(qq_chatwidget_get_input_textview(
                                            priv -> chatwidget), cs);
    if(cs -> len <= 0){
        // empty input text view
        //
        // Show warning message...
        //
        g_ptr_array_free(cs, TRUE);
        return;
    }
    qq_chat_textview_clear(qq_chatwidget_get_input_textview(
                                            priv -> chatwidget));

    QQSendMsg *msg = qq_sendmsg_new(info, MSG_GROUP_T, grp -> gid -> str);
    gint i;
    for(i = 0; i < cs -> len; ++i){
        qq_sendmsg_add_content(msg, g_ptr_array_index(cs, i));
    }
    g_ptr_array_free(cs, TRUE);

    QQMsgContent *font = qq_chatwidget_get_font(priv -> chatwidget);
    qq_sendmsg_add_content(msg, font);

    qq_chatwidget_add_send_message(priv -> chatwidget, msg);
    gqq_mainloop_attach(send_loop, qq_group_chatwindow_send_msg_cb
                                , 2, data, msg);
    return;
}

//
// Foucus in event
// Stop blinking the tray
//
static gboolean qq_group_chatwindow_focus_in_event(GtkWidget *widget
                                                    , GdkEvent *event
                                                    , gpointer data)
{
    QQGroupChatWindowPriv *priv = data;
    qq_tray_stop_blinking_for(tray, priv -> code);
    g_debug("Focus in chatwindow of %s (%s, %d)", priv -> code
                                    , __FILE__, __LINE__);
    return FALSE;
}

//
// Input text view key press
//
static gboolean qq_input_textview_key_press(GtkWidget *widget
                                                , GdkEvent *e
                                                , gpointer data)
{
    GdkEventKey *event = (GdkEventKey*)e;
    if(event -> keyval == GDK_KEY_Return || event -> keyval == GDK_KEY_KP_Enter
                        || event -> keyval == GDK_KEY_ISO_Enter){
        if((event -> state & GDK_CONTROL_MASK) != 0
                        || (event -> state & GDK_SHIFT_MASK) != 0){
            return FALSE;
        }
        qq_group_chatwindow_on_send_clicked(NULL, data);
        return TRUE;
    }
    return FALSE;
}

//
// Chat window key press
//
static gboolean qq_group_chatwindow_key_press(GtkWidget *widget
                                                , GdkEvent *e
                                                , gpointer data)
{
    GdkEventKey *event = (GdkEventKey*)e;
    if((event -> state & GDK_CONTROL_MASK) != 0
                    && (event -> keyval == GDK_KEY_w || event -> keyval == GDK_KEY_W)){
        QQGroupChatWindowPriv *priv = data;
        gqq_config_remove_ht(cfg, "chat_window_map", priv -> code);
        gtk_widget_destroy(widget);
    }
    return FALSE;
}


static void qq_group_chatwindow_init(QQGroupChatWindow *win)
{
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    gchar buf[500];
    GtkWidget *body_vbox = gtk_vbox_new(FALSE, 0);

    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

    GdkPixbuf *pb;
    g_snprintf(buf, 500, IMGDIR"%s", "group.png");
    pb = gdk_pixbuf_new_from_file(buf, NULL);
    gtk_window_set_icon(GTK_WINDOW(win), pb);
    g_object_unref(pb);

    //create header
    pb= gdk_pixbuf_new_from_file_at_size(buf, 35, 35, NULL);
    priv -> faceimage = gtk_image_new_from_pixbuf(pb);
    g_object_unref(pb);
    priv -> name_label = gtk_label_new("");
    priv -> fingermemo_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(header_hbox), priv -> faceimage
                                        , FALSE, FALSE, 5);
    GtkWidget *name_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(name_hbox), priv -> name_label
                                            , FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), name_hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), priv -> fingermemo_label
                                            , FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header_hbox), vbox, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(body_vbox), header_hbox
                                        , FALSE, FALSE, 5);

    GtkWidget *hpaned, *vpaned;
    GtkWidget *scrolled_win;
    GtkWidget *frame;

    hpaned = gtk_hpaned_new();
    vpaned = gtk_vpaned_new();
    g_object_set(hpaned, "position", 430, NULL);
    g_object_set(vpaned, "position", 130, NULL);
    g_object_set(hpaned, "position-set", TRUE, NULL);
    g_object_set(vpaned, "position-set", TRUE, NULL);

    // message text view
    priv -> chatwidget = qq_chatwidget_new();
    gtk_paned_pack1(GTK_PANED(hpaned), priv -> chatwidget, TRUE, FALSE);

    // memo text view
    priv -> memo_textview = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv -> memo_textview)
                                            , GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(priv -> memo_textview), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv -> memo_textview)
                                                , FALSE);
    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    priv -> memo_scrollwin = scrolled_win;
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win)
                                        , GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win)
                                , GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_win), priv -> memo_textview);
    frame = gtk_frame_new("Memo:");
    priv -> memo_frame = frame;
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
    gtk_container_add(GTK_CONTAINER(frame), scrolled_win);
    gtk_paned_pack1(GTK_PANED(vpaned), frame, TRUE, FALSE);

    g_object_ref(priv -> memo_scrollwin);
    priv -> memo_load_spinner = gtk_spinner_new();
    g_object_ref(priv -> memo_load_spinner);
    gtk_widget_show(priv -> memo_load_spinner);
    gtk_spinner_start(GTK_SPINNER(priv -> memo_load_spinner));

    // member list
    priv -> member_list = qq_buddy_list_new();
    gtk_widget_set_has_tooltip(priv -> member_list, TRUE);
	g_signal_connect(priv -> member_list, "query-tooltip"
                        , G_CALLBACK(qq_member_list_on_show_tooltip) , NULL);

    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    priv -> member_scrollwin = scrolled_win;
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win)
                                        , GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win)
                                , GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_win), priv -> member_list);
    frame = gtk_frame_new("Member:");
    priv -> member_frame = frame;
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
    gtk_container_add(GTK_CONTAINER(frame), scrolled_win);
    gtk_paned_pack2(GTK_PANED(vpaned), frame, TRUE, FALSE);

    g_object_ref(priv -> member_scrollwin);
    priv -> member_load_spinner = gtk_spinner_new();
    g_object_ref(priv -> member_load_spinner);
    gtk_widget_show(priv -> member_load_spinner);
    gtk_spinner_start(GTK_SPINNER(priv -> member_load_spinner));

    gtk_paned_pack2(GTK_PANED(hpaned), vpaned, TRUE, FALSE);
    gtk_box_pack_start(GTK_BOX(body_vbox), hpaned, TRUE, TRUE, 3);

    // buttons
    GtkWidget *buttonbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(buttonbox), 5);
    priv -> close_btn = gtk_button_new_with_label("Close");
    g_signal_connect(G_OBJECT(priv -> close_btn), "clicked",
                             G_CALLBACK(qq_group_chatwindow_on_close_clicked), win);
    priv -> send_btn = gtk_button_new_with_label("Send");
    g_signal_connect(G_OBJECT(priv -> send_btn), "clicked",
                             G_CALLBACK(qq_group_chatwindow_on_send_clicked), win);
    gtk_container_add(GTK_CONTAINER(buttonbox), priv -> close_btn);
    gtk_container_add(GTK_CONTAINER(buttonbox), priv -> send_btn);
    gtk_box_pack_start(GTK_BOX(body_vbox), buttonbox, FALSE, FALSE, 3);

    GtkWidget *w = GTK_WIDGET(win);
    gtk_window_resize(GTK_WINDOW(w), 600, 490);
    gtk_container_add(GTK_CONTAINER(win), body_vbox);

    gtk_widget_show_all(body_vbox);
    gtk_widget_grab_focus(qq_chatwidget_get_input_textview(
                                            priv -> chatwidget));

    g_signal_connect(G_OBJECT(win), "delete-event"
                                , G_CALLBACK(qq_group_chatwindow_delete_event)
                                , priv);
    g_signal_connect(G_OBJECT(win), "focus-in-event"
                                , G_CALLBACK(qq_group_chatwindow_focus_in_event)
                                , priv);
    g_signal_connect(G_OBJECT(win), "key-press-event"
                            , G_CALLBACK(qq_group_chatwindow_key_press), priv);
    g_signal_connect(G_OBJECT(qq_chatwidget_get_input_textview(
                                                priv -> chatwidget))
                            , "key-press-event"
                            , G_CALLBACK(qq_input_textview_key_press), win);
    // show spinner
    qq_group_chatwindow_show_memo_spinner(win);
    qq_group_chatwindow_show_member_spinner(win);
}

/*
 * The getter.
 */
static void qq_group_chatwindow_getter(GObject *object, guint property_id,
                                    GValue *value, GParamSpec *pspec)
{
    if(object == NULL || value == NULL || property_id < 0){
            return;
    }

    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    object, qq_group_chatwindow_get_type()
                                    , QQGroupChatWindowPriv);

    switch (property_id)
    {
    case QQ_GROUP_CHATWINDOW_PROPERTY_CODE:
        g_value_set_string(value, priv -> code);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

/*
 * The setter.
 */
static void qq_group_chatwindow_setter(GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
    if(object == NULL || value == NULL || property_id < 0){
            return;
    }
    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    object, qq_group_chatwindow_get_type()
                                    , QQGroupChatWindowPriv);
    gchar buf[500];
    switch (property_id)
    {
    case QQ_GROUP_CHATWINDOW_PROPERTY_CODE:
        g_stpcpy(priv -> code, g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(
                                GTK_TEXT_VIEW(priv -> memo_textview));
    QQGroup *grp = qq_info_lookup_group_by_code(info, priv -> code);
    if(grp == NULL){
        g_snprintf(buf, 500, "<b>%s</b>", priv -> code);
        gtk_label_set_markup(GTK_LABEL(priv -> name_label), buf);
        gtk_label_set_text(GTK_LABEL(priv -> fingermemo_label), "");
        gtk_window_set_title(GTK_WINDOW(object), "GtkQQ");
        gtk_text_buffer_set_text(buffer, "No Memo.", -1);
        return;
    }
    // set fingermemo
    g_snprintf(buf, 500, "<b>%s</b>", grp -> fingermemo -> str);
    gtk_label_set_markup(GTK_LABEL(priv -> fingermemo_label), buf);
    // set name
    g_snprintf(buf, 500, "<b>%s</b>", grp -> name -> str);
    gtk_label_set_markup(GTK_LABEL(priv -> name_label), buf);
    // set memo
    gtk_text_buffer_set_text(buffer, grp -> memo -> str, -1);
    // window title
    gtk_window_set_title(GTK_WINDOW(object), grp -> name -> str);

    // start update the info
    qq_group_chatwindow_start_update_info(QQ_GROUP_CHATWINDOW(object));
}

static void qq_group_chatwindowclass_init(QQGroupChatWindowClass *wc)
{
    g_type_class_add_private(wc, sizeof(QQGroupChatWindowPriv));

    G_OBJECT_CLASS(wc) -> get_property = qq_group_chatwindow_getter;
    G_OBJECT_CLASS(wc) -> set_property = qq_group_chatwindow_setter;

    //install the code property
    GParamSpec *pspec;
    pspec = g_param_spec_string("code"
                                , "QQ group code"
                                , "qq code"
                                , ""
                                , G_PARAM_READABLE | G_PARAM_CONSTRUCT
                                        | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(wc)
                                    , QQ_GROUP_CHATWINDOW_PROPERTY_CODE
                                    , pspec);

    gtkloop.ctx = g_main_context_default();
    gtkloop.name = "MainPanel Gtk";
}

void qq_group_chatwindow_add_send_message(GtkWidget *widget, QQSendMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);

    GtkWidget *mt = qq_chatwidget_get_message_textview(priv -> chatwidget);
    qq_chat_textview_add_send_message(mt, msg);
}

void qq_group_chatwindow_add_recv_message(GtkWidget *widget, QQRecvMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    QQGroupChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_group_chatwindow_get_type()
                                        , QQGroupChatWindowPriv);
    GtkWidget *mt = qq_chatwidget_get_message_textview(priv -> chatwidget);
    qq_chat_textview_add_recv_message(mt, msg);
}

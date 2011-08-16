#include <chatwindow.h>
#include <gqqconfig.h>
#include <stdlib.h>
#include <chattextview.h>
#include <tray.h>
#include <msgloop.h>
#include <gdk/gdkkeysyms.h>
#include <chatwidget.h>

extern QQInfo *info;
extern GQQConfig *cfg;
extern QQTray *tray;
extern GQQMessageLoop *send_loop;

static void qq_chatwindow_init(QQChatWindow *win);
static void qq_chatwindowclass_init(QQChatWindowClass *wc);

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

    GtkWidget *chat_widget;
    
    GtkWidget *send_btn, *close_btn;
}QQChatWindowPriv;

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
    qq_sendmsg_free(msg);
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
    qq_chat_textview_get_msg_contents(qq_chatwidget_get_input_textview(
                                            priv -> chat_widget), cs);
    if(cs -> len <= 0){
        // empty input text view
        //
        // Show warning message...
        //
        g_ptr_array_free(cs, TRUE);
        return;
    }
    qq_chat_textview_clear(qq_chatwidget_get_input_textview(
                                            priv -> chat_widget));

    QQSendMsg *msg = qq_sendmsg_new(info, MSG_BUDDY_T, priv -> uin);
    gint i;
    for(i = 0; i < cs -> len; ++i){
        qq_sendmsg_add_content(msg, g_ptr_array_index(cs, i));
    }
    g_ptr_array_free(cs, TRUE);

    QQMsgContent *font = qq_chatwidget_get_font(priv -> chat_widget);
    qq_sendmsg_add_content(msg, font);

    qq_chatwidget_add_send_message(priv -> chat_widget, msg);
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

//
// Input text view key press
//
static gboolean qq_input_textview_key_press(GtkWidget *widget, GdkEvent *e
                                            , gpointer data)
{
    GdkEventKey *event = (GdkEventKey*)e;
    if(event -> keyval == GDK_Return || event -> keyval == GDK_KP_Enter
                        || event -> keyval == GDK_ISO_Enter){
        if((event -> state & GDK_CONTROL_MASK) != 0 
                        || (event -> state & GDK_SHIFT_MASK) != 0){
            return FALSE;
        }
        qq_chatwindow_on_send_clicked(NULL, data);
        return TRUE;
    }
    return FALSE;
}

//
// Chat window key press
//
static gboolean qq_chatwindow_key_press(GtkWidget *widget, GdkEvent *e
                                            , gpointer data)
{
    GdkEventKey *event = (GdkEventKey*)e;
    if((event -> state & GDK_CONTROL_MASK) != 0 
                    && (event -> keyval == GDK_w || event -> keyval == GDK_W)){
        QQChatWindowPriv *priv = data;
        gqq_config_remove_ht(cfg, "chat_window_map", priv -> uin);
        gtk_widget_destroy(widget);
    }
    return FALSE;
}


static void qq_chatwindow_init(QQChatWindow *win)
{
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(win
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);
    priv -> name = g_string_new(NULL);
    priv -> lnick = g_string_new(NULL);

    gchar buf[500];
    priv -> body_vbox = gtk_vbox_new(FALSE, 0);

    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 0);
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
    priv -> chat_widget = qq_chatwidget_new();
    gtk_box_pack_start(GTK_BOX(priv -> body_vbox), priv -> chat_widget
                                                , TRUE, TRUE, 0); 

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
    gtk_window_resize(GTK_WINDOW(w), 500, 450);
    gtk_container_add(GTK_CONTAINER(win), priv -> body_vbox);

    gtk_widget_show_all(priv -> body_vbox);
    gtk_widget_grab_focus(qq_chatwidget_get_input_textview(
                                priv -> chat_widget));

    g_signal_connect(G_OBJECT(win), "delete-event"
                                , G_CALLBACK(qq_chatwindow_delete_event)
                                , priv);
    g_signal_connect(G_OBJECT(win), "focus-in-event"
                                , G_CALLBACK(qq_chatwindow_focus_in_event)
                                , priv);
    g_signal_connect(G_OBJECT(win), "key-press-event"
                            , G_CALLBACK(qq_chatwindow_key_press), priv);

    g_signal_connect(G_OBJECT(qq_chatwidget_get_input_textview(
                                                priv -> chat_widget))
                            , "key-press-event"
                            , G_CALLBACK(qq_input_textview_key_press), win);
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
    
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    object, qq_chatwindow_get_type()
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
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    object, qq_chatwindow_get_type()
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
    gtk_window_set_icon(GTK_WINDOW(object), pb);
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
    gtk_window_set_title(GTK_WINDOW(object), buf);
}

//
// Finalize
// Free all the allocated memory
//
static void qq_chatwindow_finalize(GObject *obj)
{
    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    obj, qq_chatwindow_get_type()
                                    , QQChatWindowPriv);
    g_string_free(priv -> name, TRUE);
    g_string_free(priv -> lnick, TRUE);
    
    // chain up
    GObjectClass *klass = (GObjectClass*)g_type_class_peek_parent(
                                g_type_class_peek(qq_chatwindow_get_type()));
    klass -> finalize(obj);
}

static void qq_chatwindowclass_init(QQChatWindowClass *wc)
{
    g_type_class_add_private(wc, sizeof(QQChatWindowPriv));

    G_OBJECT_CLASS(wc) -> get_property = qq_chatwindow_getter;
    G_OBJECT_CLASS(wc) -> set_property = qq_chatwindow_setter;
    G_OBJECT_CLASS(wc) -> finalize = qq_chatwindow_finalize;

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

    GtkWidget *mt = qq_chatwidget_get_message_textview(priv -> chat_widget);
    qq_chat_textview_add_send_message(mt, msg);
}

void qq_chatwindow_add_recv_message(GtkWidget *widget, QQRecvMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    QQChatWindowPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwindow_get_type()
                                        , QQChatWindowPriv);
    GtkWidget *mt = qq_chatwidget_get_message_textview(priv -> chat_widget);
    qq_chat_textview_add_recv_message(mt, msg);
}

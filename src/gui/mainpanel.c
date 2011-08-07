#include <mainpanel.h>
#include <statusbutton.h>
#include <msgloop.h>
#include <qq.h>
#include <buddytree.h>

/*
 * The main event loop context of Gtk.
 */
extern QQInfo *info;

extern GQQMessageLoop *get_info_loop;
extern GQQMessageLoop *send_loop;
/*
 * The main event loop context of Gtk.
 */
static GQQMessageLoop gtkloop;

static QQMainPanelClass *this_class = NULL;

static void qq_mainpanel_init(QQMainPanel *panel);
static void qq_mainpanelclass_init(QQMainPanelClass *c);

static gboolean lnick_release_cb(GtkWidget *w, GdkEvent *event, gpointer data);
static gboolean lnick_focus_out_cb(GtkWidget *w, GdkEvent *event, gpointer data);
static gboolean lnick_enter_cb(GtkWidget *w, GdkEvent *event, gpointer data);
static gboolean lnick_leave_cb(GtkWidget *w, GdkEvent *event, gpointer data);

//button group callback.
static void btn_group_release_cb(GtkWidget *btn, GdkEvent *event, gpointer data);
static void btn_group_enter_cb(GtkWidget *btn, GdkEvent *event, gpointer data);
static void btn_group_leave_cb(GtkWidget *btn, GdkEvent *event, gpointer data);

static void update_my_face_image(QQMainPanel *panel);

GType qq_mainpanel_get_type()
{
    static GType t = 0;
    if(!t){
        static const GTypeInfo info =
            {
                sizeof(QQMainPanelClass),
                NULL,
                NULL,
                (GClassInitFunc)qq_mainpanelclass_init,
                NULL,
                NULL,
                sizeof(QQMainPanel),
                0,
                (GInstanceInitFunc)qq_mainpanel_init,
                NULL
            };
        t = g_type_register_static(GTK_TYPE_VBOX, "QQMainPanel"
                        , &info, 0);
    }
    return t;
}
GtkWidget* qq_mainpanel_new(GtkWidget *container)
{
    QQMainPanel *panel = g_object_new(qq_mainpanel_get_type(), NULL);
    panel -> container = container;

    return GTK_WIDGET(panel);
}

static void qq_mainpanel_init(QQMainPanel *panel)
{

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *box = NULL;
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

    panel -> faceimgframe = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(hbox), panel -> faceimgframe
                , FALSE, FALSE, 15);

    panel -> status_btn = qq_statusbutton_new();
    panel -> nick = gtk_label_new("");
    box = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), panel -> status_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), panel -> nick, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), box, FALSE, FALSE, 2);

    panel -> longnick = gtk_label_new("");
    panel -> longnick_entry = gtk_entry_new();    
    gtk_widget_set_size_request(GTK_WIDGET(panel -> longnick_entry), 0, 0);
    panel -> longnick_box = gtk_hbox_new(FALSE, 0);
    panel -> longnick_eventbox = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(panel -> longnick_eventbox)
                        , panel -> longnick);
    gtk_box_pack_start(GTK_BOX(panel -> longnick_box)
                            , panel -> longnick_eventbox
                            , FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(panel -> longnick_box)
                            , panel -> longnick_entry
                            , FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), panel -> longnick_box, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(panel), hbox, FALSE, FALSE, 10);

    g_signal_connect(GTK_WIDGET(panel -> longnick_eventbox)
                        , "button-release-event"
                        , G_CALLBACK(lnick_release_cb), panel);
    g_signal_connect(GTK_WIDGET(panel -> longnick_eventbox)
                        , "enter-notify-event"
                        , G_CALLBACK(lnick_enter_cb), panel);
    g_signal_connect(GTK_WIDGET(panel -> longnick_eventbox)
                        , "leave-notify-event"
                        , G_CALLBACK(lnick_leave_cb), panel);
    g_signal_connect(GTK_WIDGET(panel -> longnick_entry)
                        , "focus-out-event"
                        , G_CALLBACK(lnick_focus_out_cb), panel);

    panel -> contact_btn = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(panel -> contact_btn)
                        , this_class -> contact_img[0]);
    panel -> grp_btn = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(panel -> grp_btn)
                        , this_class -> grp_img[1]);
    panel -> recent_btn = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(panel -> recent_btn)
                        , this_class -> recent_img[1]);

    g_signal_connect(GTK_WIDGET(panel -> contact_btn), "button-release-event"
                                , G_CALLBACK(btn_group_release_cb), panel);
    g_signal_connect(GTK_WIDGET(panel -> grp_btn), "button-release-event"
                                , G_CALLBACK(btn_group_release_cb), panel);
    g_signal_connect(GTK_WIDGET(panel -> recent_btn), "button-release-event"
                                , G_CALLBACK(btn_group_release_cb), panel);

    g_signal_connect(GTK_WIDGET(panel -> contact_btn), "enter-notify-event"
                                , G_CALLBACK(btn_group_enter_cb), panel);
    g_signal_connect(GTK_WIDGET(panel -> grp_btn), "enter-notify-event"
                                , G_CALLBACK(btn_group_enter_cb), panel);
    g_signal_connect(GTK_WIDGET(panel -> recent_btn), "enter-notify-event"
                                , G_CALLBACK(btn_group_enter_cb), panel);

    g_signal_connect(GTK_WIDGET(panel -> contact_btn), "leave-notify-event"
                                , G_CALLBACK(btn_group_leave_cb), panel);
    g_signal_connect(GTK_WIDGET(panel -> grp_btn), "leave-notify-event"
                                , G_CALLBACK(btn_group_leave_cb), panel);
    g_signal_connect(GTK_WIDGET(panel -> recent_btn), "leave-notify-event"
                                , G_CALLBACK(btn_group_leave_cb), panel);

    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), panel -> contact_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), panel -> grp_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), panel -> recent_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(panel), hbox, FALSE, FALSE, 0);


    panel -> notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(panel -> notebook), FALSE);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(panel -> notebook), TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(panel -> notebook), TRUE);
    gtk_box_pack_start(GTK_BOX(panel), panel -> notebook, TRUE, TRUE, 3);

    panel -> buddy_tree= qq_buddy_tree_new();
    panel -> group_list = gtk_tree_view_new();
    panel -> recent_list = gtk_tree_view_new();

    GtkWidget *scrolled_win; 
    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_win),
                GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_win), panel -> buddy_tree);
    gtk_notebook_append_page(GTK_NOTEBOOK(panel -> notebook)
                            , scrolled_win, NULL);

    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_win),
                GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_win), panel -> group_list);
    gtk_notebook_append_page(GTK_NOTEBOOK(panel -> notebook)
                            , scrolled_win, NULL);

    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_win),
                GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_win), panel -> recent_list);
    gtk_notebook_append_page(GTK_NOTEBOOK(panel -> notebook)
                            , scrolled_win, NULL);

}
static void qq_mainpanelclass_init(QQMainPanelClass *c)
{
    this_class = c;
    GdkPixbuf * pb;
    pb= gdk_pixbuf_new_from_file(IMGDIR"ContactMainTabButton1.png", NULL);
    c -> contact_img[0] = gtk_image_new_from_pixbuf(pb);
    pb = gdk_pixbuf_new_from_file(IMGDIR"ContactMainTabButton2.png", NULL);
    c -> contact_img[1] = gtk_image_new_from_pixbuf(pb);
    pb = gdk_pixbuf_new_from_file(IMGDIR"GroupMainTabButton1.png", NULL);
    c -> grp_img[0] = gtk_image_new_from_pixbuf(pb);
    pb = gdk_pixbuf_new_from_file(IMGDIR"GroupMainTabButton2.png", NULL);
    c -> grp_img[1] = gtk_image_new_from_pixbuf(pb);
    pb = gdk_pixbuf_new_from_file(IMGDIR"RecentMainTabButton1.png", NULL);
    c -> recent_img[0] = gtk_image_new_from_pixbuf(pb);
    pb = gdk_pixbuf_new_from_file(IMGDIR"RecentMainTabButton2.png", NULL);
    c -> recent_img[1] = gtk_image_new_from_pixbuf(pb);

    //increase the reference count
    g_object_ref(G_OBJECT(c -> contact_img[0]));
    g_object_ref(G_OBJECT(c -> contact_img[1]));
    g_object_ref(G_OBJECT(c -> grp_img[0]));
    g_object_ref(G_OBJECT(c -> grp_img[1]));
    g_object_ref(G_OBJECT(c -> recent_img[0]));
    g_object_ref(G_OBJECT(c -> recent_img[1]));

    //show the image
    gtk_widget_show(c -> contact_img[0]);
    gtk_widget_show(c -> contact_img[1]);
    gtk_widget_show(c -> grp_img[0]);
    gtk_widget_show(c -> grp_img[1]);
    gtk_widget_show(c -> recent_img[0]);
    gtk_widget_show(c -> recent_img[1]);

    c -> hand = gdk_cursor_new(GDK_HAND1);

    gtkloop.ctx = g_main_context_default();
    gtkloop.name = "MainPanel Gtk";
}

//
// Update the information of all the widgets.
//
void qq_mainpanel_update(QQMainPanel *panel)
{
    update_my_face_image(panel);
    gtk_label_set_text(GTK_LABEL(panel -> nick)
                            , info -> me -> nick -> str);
    gtk_label_set_text(GTK_LABEL(panel -> longnick)
                            , info -> me -> lnick -> str);

    g_debug("Update status: %s (%s, %d)", info -> me -> status -> str
                                , __FILE__, __LINE__);
    qq_statusbutton_set_status_string(panel -> status_btn
                            , info -> me -> status -> str);
    
    // Update buddy tree
    qq_buddy_tree_update(panel -> buddy_tree, info);
}

//
// click the long nick label, replace it with the gtkentry.
//
static gboolean lnick_release_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    QQMainPanel *panel = QQ_MAINPANEL(data);

    gtk_widget_hide(GTK_WIDGET(panel -> longnick_eventbox));
    gtk_widget_show(GTK_WIDGET(panel -> longnick_entry));

    gtk_entry_set_text(GTK_ENTRY(panel -> longnick_entry), 
                        gtk_label_get_text(GTK_LABEL(panel -> longnick)));
    gtk_widget_show(GTK_WIDGET(panel -> longnick_entry));
    gtk_widget_set_size_request(GTK_WIDGET(panel -> longnick_entry)
                                , -1, 22);
    gtk_widget_grab_focus(GTK_WIDGET(panel -> longnick_entry));
    return FALSE;
}

//
//  longnick entry lose focus, replace it with label
//
static gboolean lnick_focus_out_cb(GtkWidget *w, GdkEvent *event
                                        , gpointer data)
{
    QQMainPanel *panel = QQ_MAINPANEL(data);
    gtk_widget_show(panel -> longnick_eventbox);
    gtk_widget_hide(panel -> longnick_entry);
    gtk_label_set_text(GTK_LABEL(panel -> longnick)
                , gtk_entry_get_text(GTK_ENTRY(panel -> longnick_entry)));
    return FALSE;
}

//
//Button group callbutton
//
//The buttons which connect to this callback function will
//only one button actived in any time.
//
static void btn_group_release_cb(GtkWidget *btn, GdkEvent *event, gpointer data)
{
    //grab the focus
    gtk_widget_grab_focus(btn);

    //clear
    QQMainPanel *panel = QQ_MAINPANEL(data);
    gtk_container_remove(GTK_CONTAINER(panel -> contact_btn)
                        , gtk_bin_get_child(GTK_BIN(panel -> contact_btn)));
    gtk_container_remove(GTK_CONTAINER(panel -> grp_btn)
                        , gtk_bin_get_child(GTK_BIN(panel -> grp_btn)));
    gtk_container_remove(GTK_CONTAINER(panel -> recent_btn)
                        , gtk_bin_get_child(GTK_BIN(panel -> recent_btn)));

    if(btn == panel -> contact_btn){
        gtk_container_add(GTK_CONTAINER(panel -> contact_btn)
                            , this_class -> contact_img[0]);
        gtk_container_add(GTK_CONTAINER(panel -> grp_btn)
                            , this_class -> grp_img[1]);
        gtk_container_add(GTK_CONTAINER(panel -> recent_btn)
                            , this_class -> recent_img[1]);
    }else if(btn == panel -> grp_btn){
        gtk_container_add(GTK_CONTAINER(panel -> contact_btn)
                            , this_class -> contact_img[1]);
        gtk_container_add(GTK_CONTAINER(panel -> grp_btn)
                            , this_class -> grp_img[0]);
        gtk_container_add(GTK_CONTAINER(panel -> recent_btn)
                            , this_class -> recent_img[1]);
    }else if(btn == panel -> recent_btn){
        gtk_container_add(GTK_CONTAINER(panel -> contact_btn)
                            , this_class -> contact_img[1]);
        gtk_container_add(GTK_CONTAINER(panel -> grp_btn)
                            , this_class -> grp_img[1]);
        gtk_container_add(GTK_CONTAINER(panel -> recent_btn)
                            , this_class -> recent_img[0]);
    }

}
static void btn_group_enter_cb(GtkWidget *btn, GdkEvent *event, gpointer data)
{
    GdkWindow *win = gtk_widget_get_window(btn);    
    gdk_window_set_cursor(win, this_class -> hand);
    GdkColor color;
    gdk_color_parse("#C4E6FF", &color);
    gtk_widget_modify_bg(btn, GTK_STATE_NORMAL, &color);
}
static void btn_group_leave_cb(GtkWidget *btn, GdkEvent *event, gpointer data)
{
    GdkWindow *win = gtk_widget_get_window(btn);    
    gdk_window_set_cursor(win, NULL);
    gtk_widget_modify_bg(btn, GTK_STATE_NORMAL, NULL);
}

//
// The long nick label enter-notify-event and leave-notify-event call back.
// Change the background and cursor.
//
static gboolean lnick_enter_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    QQMainPanel *panel = QQ_MAINPANEL(data);
    GdkWindow *win = gtk_widget_get_window(w);    
    gdk_window_set_cursor(win, this_class -> hand);
    GdkColor color;
    gdk_color_parse("#C4E6FF", &color);
    gtk_widget_modify_bg(GTK_WIDGET(panel -> longnick_eventbox)
                        , GTK_STATE_NORMAL, &color);
    return TRUE;
}
static gboolean lnick_leave_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    QQMainPanel *panel = QQ_MAINPANEL(data);
    GdkWindow *win = gtk_widget_get_window(w);    
    gdk_window_set_cursor(win, NULL);
    gtk_widget_modify_bg(GTK_WIDGET(panel -> longnick_eventbox)
                        , GTK_STATE_NORMAL, NULL);
    return TRUE;
}

static void update_my_face_image(QQMainPanel *panel)
{
    GtkBin *faceimgframe = GTK_BIN(panel -> faceimgframe);

    //free the old image.
    GtkWidget *faceimg = gtk_bin_get_child(faceimgframe);
    if(faceimg != NULL){
        gtk_container_remove(GTK_CONTAINER(faceimgframe), faceimg);
    }
    
    GError *err = NULL;
    gchar buf[500];
    g_snprintf(buf, 500, CONFIGDIR"/faces/%s", info -> me -> qqnumber -> str);
    GdkPixbuf *pb = gdk_pixbuf_new_from_file_at_size(buf, 48, 48, &err);
    if(pb == NULL){
        g_debug("Load %s's face image error. use default. %s (%s, %d)"
                                    , info -> me -> qqnumber -> str
                                    , err -> message, __FILE__, __LINE__);
        g_error_free(err);
        err = NULL;
        pb = gdk_pixbuf_new_from_file_at_size(
                                    IMGDIR"/avatar.gif", 48, 48, &err);
        if(pb == NULL){
            g_warning("Load default face image error. %s (%s, %d)"
                                    , err -> message, __FILE__, __LINE__);
            g_error_free(err);
        }
    }
    GtkWidget *img = gtk_image_new_from_pixbuf(pb);
    g_object_unref(pb);
    //we MUST show it!
    gtk_widget_show(img);
    gtk_container_add(GTK_CONTAINER(faceimgframe), img);
}

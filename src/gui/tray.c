#include <qq.h>
#include "tray.h"
#include "gqqconfig.h"
#include "mainwindow.h"

/*
 * The main event loop context of Gtk.
 */
extern QQInfo *info;
extern GQQConfig *cfg;
extern GtkWidget *main_win;

//
// Private members
//
typedef struct {
	GQueue *blinking_queue;     // blinking queue
	GQueue *tmp_queue;          // tmp queue

	GtkWidget *popupmenu;       // popup menu
	GtkWidget *mute_item;		/**< mute item. */
} QQTrayPriv;

static void qq_tray_init(QQTray *tray);
static void qq_trayclass_init(QQTrayClass *tc, gpointer data);

GType qq_tray_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQTrayClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_trayclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQTray),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_tray_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_STATUS_ICON, "QQTray", &info, 0);
    }
    return t;
}

QQTray* qq_tray_new()
{
    return QQ_TRAY(g_object_new(qq_tray_get_type()
                                , "file", IMGDIR"/webqq_icon.png"
                                , NULL));
}

/*
 * Blinking uin's face image
 */
static void qq_tray_blinking(QQTray *tray, const gchar *uin)
{
    gchar buf[500];
    GdkPixbuf *pb;
    QQBuddy *bdy = qq_info_lookup_buddy_by_uin(info, uin);

    // blinking
    if(bdy == NULL){
        g_snprintf(buf, 500, IMGDIR"/webqq_icon.png%s", "");
    }else{
		g_snprintf(buf, 500, "%s/%s", QQ_FACEDIR, bdy -> qqnumber -> str);
    }
    pb = gdk_pixbuf_new_from_file(buf, NULL);
    if(pb == NULL){
        pb = gdk_pixbuf_new_from_file(IMGDIR"/webqq_icon.png", NULL);
    }
    gtk_status_icon_set_from_pixbuf(GTK_STATUS_ICON(tray), pb);
    g_object_unref(pb);
    gtk_status_icon_set_blinking(GTK_STATUS_ICON(tray), TRUE);
}

//
// popup-menu event
// Popup the menu
//
static void qq_tray_popup_menu(GtkStatusIcon *tray, guint button
                                    , guint active_time
                                    , gpointer data)
{
    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, qq_tray_get_type()
                                                    , QQTrayPriv);
    gtk_menu_popup(GTK_MENU(priv -> popupmenu),
                   NULL, NULL,
                   gtk_status_icon_position_menu, tray, 
                   button, active_time);
}

static gboolean qq_tray_button_press(GtkStatusIcon *tray, GdkEvent *event
                                    , gpointer data)
{
    GdkEventButton *buttonevent = (GdkEventButton*)event;

	/* Only handle left clicked. */
	if(buttonevent -> button != 1 || buttonevent -> type != GDK_BUTTON_PRESS){
		return FALSE;
	}
    
    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, qq_tray_get_type()
                                                    , QQTrayPriv);
    gchar *uin = g_queue_pop_tail(priv -> blinking_queue);
    if(uin == NULL){
		/* If there is no new msg, show or hide the main window. */
		qq_mainwindow_show_hide(main_win);
        return FALSE;
    }
    GtkWidget *cw = gqq_config_lookup_ht(cfg, "chat_window_map", uin);
    if(cw != NULL){
        gtk_widget_show(cw);
    }
    g_free(uin);

    if(g_queue_is_empty(priv -> blinking_queue)){
	    /**
	     * WARNING:
	     * 		gtk_status_icon_set_blinking()
	     * 		has been deprecated since version GTK 2.22
	     * 		and will be removed in GTK+ 3
	     * 		maybe we should try libnotify
	     */
        gtk_status_icon_set_blinking(tray, FALSE);
        GdkPixbuf *pb = gdk_pixbuf_new_from_file(IMGDIR"/webqq_icon.png"
                                                    , NULL);
        gtk_status_icon_set_from_pixbuf(GTK_STATUS_ICON(tray), pb);
        g_object_unref(pb);
        return FALSE;
    }
    qq_tray_blinking(QQ_TRAY(tray), g_queue_peek_tail(priv -> blinking_queue));
    return FALSE;
}

//
// Custom the tooltip
//
static gboolean qq_tray_on_show_tooltip(GtkWidget* widget
                                            , int x
                                            , int y
                                            , gboolean keybord_mode
                                            , GtkTooltip* tip
                                            , gpointer data)
{
    GdkPixbuf *pb;
    if(info -> me -> qqnumber == NULL || info -> me -> qqnumber -> len <=0){
        // Not login. 
        pb = gdk_pixbuf_new_from_file_at_size(IMGDIR"/webqq_icon.png"
                                                , 35, 35, NULL);
        gtk_tooltip_set_markup(tip, "<b>GtkQQ</b>"); 
        gtk_tooltip_set_icon(tip, pb);
        g_object_unref(pb);
        return TRUE;
    }
    gchar buf[500];
	g_snprintf(buf, 500, "%s/%s", QQ_FACEDIR, info -> me -> qqnumber -> str);
    pb = gdk_pixbuf_new_from_file_at_size(buf, 35, 35, NULL);
    gtk_tooltip_set_icon(tip, pb);
    g_object_unref(pb);
    g_snprintf(buf, 500, "<b>%s</b><span color='blue'>(%s)</span>"
                                    , info -> me -> nick -> str
                                    , info -> me -> qqnumber -> str);
    gtk_tooltip_set_markup(tip, buf); 
    return TRUE;
}

/**
 * Status menu item signal handler
 */
static void qq_tray_mute_menu_item_activate(GtkMenuItem *item, gpointer data)
{
	gint mute = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));

	if (mute)
		g_print("Mute (%s, %d)\n", __FILE__, __LINE__);
	
	gqq_config_set_mute(cfg, mute);
}

/** 
 * Set mute item status, it usually called when user login.
 * 
 * @param tray 
 * @param mute 
 */
void qq_tray_set_mute_item(QQTray *tray, gboolean mute)
{
	QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray,
			qq_tray_get_type(), QQTrayPriv);
	if (!priv)
		return ;
	gtk_check_menu_item_set_active(
			GTK_CHECK_MENU_ITEM(priv->mute_item), mute);
}

static void qq_tray_status_menu_item_activate(GtkMenuItem *item, gpointer data)
{
    const gchar *status = data;
    g_debug("Change status to %s (%s, %d)", status, __FILE__, __LINE__);
}

static void qq_tray_personal_setting_menu_item_activate(GtkMenuItem *item,
								gpointer data)
{
    g_debug("Tray personal setting (%s, %d)", __FILE__, __LINE__);
}
static void qq_tray_system_setting_menu_item_activate(GtkMenuItem *item,
								gpointer data)
{
    g_debug("Tray system setting (%s, %d)", __FILE__, __LINE__);

}

static void qq_tray_about_menu_item_activate(GtkMenuItem *item, gpointer data)
{
    g_debug("Tray about(%s, %d)", __FILE__, __LINE__);

}

static void qq_tray_quit_menu_item_activate(GtkMenuItem *item, gpointer data)
{
    g_debug("Tray quit(%s, %d)", __FILE__, __LINE__);
    gtk_main_quit();
}

static void qq_tray_init(QQTray *tray)
{
    gtk_status_icon_set_tooltip_markup(GTK_STATUS_ICON(tray), "<b>GtkQQ</b>");

    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, qq_tray_get_type()
                                                    , QQTrayPriv);

    priv -> blinking_queue = g_queue_new();
    priv -> tmp_queue = g_queue_new();
    priv -> popupmenu = gtk_menu_new();

    GtkWidget *menuitem;

    menuitem = gtk_check_menu_item_new_with_label("Mute");
	priv -> mute_item = menuitem;
	g_signal_connect(G_OBJECT(menuitem), "activate"
					 , G_CALLBACK(qq_tray_mute_menu_item_activate)
					 , NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv -> popupmenu), menuitem);

    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(priv -> popupmenu), menuitem);

    GtkWidget *img;
    GdkPixbuf *pb;
#define STATUS_ITEM(x,y) \
    menuitem = gtk_image_menu_item_new();\
    gtk_menu_shell_append(GTK_MENU_SHELL(priv -> popupmenu), menuitem);\
    pb = gdk_pixbuf_new_from_file_at_size(IMGDIR"/status/"x".png"\
                                                , 12, 12, NULL);\
    img = gtk_image_new_from_pixbuf(pb);\
    g_object_unref(pb);\
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), img);\
    gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), y);\
    g_signal_connect(G_OBJECT(menuitem), "activate"\
                    , G_CALLBACK(qq_tray_status_menu_item_activate), x);

    STATUS_ITEM("online", "Online");
    STATUS_ITEM("hidden", "Hidden");
    STATUS_ITEM("away", "Away");
    STATUS_ITEM("busy", "Busy");
    STATUS_ITEM("callme", "Call Me");
    STATUS_ITEM("silent", "Silent");
#undef STATUS_ITEM

    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(priv -> popupmenu), menuitem);

    menuitem = gtk_menu_item_new_with_label("Personal Setting");
    g_signal_connect(G_OBJECT(menuitem), "activate"
                    , G_CALLBACK(qq_tray_personal_setting_menu_item_activate)
                    , NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv -> popupmenu), menuitem);

    menuitem = gtk_menu_item_new_with_label("System Setting");
    g_signal_connect(G_OBJECT(menuitem), "activate"
                    , G_CALLBACK(qq_tray_system_setting_menu_item_activate)
                    , NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv -> popupmenu), menuitem);

    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(priv -> popupmenu), menuitem);

    menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
    g_signal_connect(G_OBJECT(menuitem), "activate"
                    , G_CALLBACK(qq_tray_about_menu_item_activate)
                    , NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv -> popupmenu), menuitem);
    gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), "About GtkQQ");

    menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
    g_signal_connect(G_OBJECT(menuitem), "activate"
                    , G_CALLBACK(qq_tray_quit_menu_item_activate)
                    , NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv -> popupmenu), menuitem);
    gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), "Quit");

    gtk_widget_show_all(priv -> popupmenu);

    g_signal_connect(G_OBJECT(tray), "popup-menu"
                            , G_CALLBACK(qq_tray_popup_menu), tray);
    g_signal_connect(G_OBJECT(tray), "button-press-event"
                            , G_CALLBACK(qq_tray_button_press), tray);
    g_signal_connect(G_OBJECT(tray), "query-tooltip"
                            , G_CALLBACK(qq_tray_on_show_tooltip), tray);
}

static void qq_trayclass_init(QQTrayClass *tc, gpointer data)
{
    g_type_class_add_private(tc, sizeof(QQTrayPriv));
}

void qq_tray_blinking_for(QQTray *tray, const gchar *uin)
{
    if(tray == NULL || uin == NULL){
        return;
    }

    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, qq_tray_get_type()
                                                    , QQTrayPriv);

    if(NULL != g_queue_find_custom(priv -> blinking_queue, uin
                                    , (GCompareFunc)g_strcmp0)){
        // already blinking
        return;
    }
    g_queue_push_head(priv -> blinking_queue, g_strdup(uin));
    qq_tray_blinking(tray, g_queue_peek_tail(priv -> blinking_queue));
}

void qq_tray_stop_blinking_for(QQTray *tray, const gchar *uin)
{
    if(tray == NULL || uin == NULL){
        return;
    }

    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, qq_tray_get_type()
                                                    , QQTrayPriv);

    gchar *tmpuin = NULL;
    g_queue_clear(priv -> tmp_queue);
    while(!g_queue_is_empty(priv -> blinking_queue)){
        tmpuin = g_queue_pop_tail(priv -> blinking_queue);
        if(g_strcmp0(tmpuin, uin) == 0){
            //remove it
            g_free(tmpuin);
            break;
        }
        g_queue_push_head(priv -> tmp_queue, tmpuin);
    }
    while(!g_queue_is_empty(priv -> tmp_queue)){
        g_queue_push_tail(priv -> blinking_queue
                            , g_queue_pop_head(priv -> tmp_queue));
    }

    GdkPixbuf *pb;
    if(g_queue_is_empty(priv -> blinking_queue)){
        // no more blinking
        gtk_status_icon_set_blinking(GTK_STATUS_ICON(tray), FALSE);
        pb = gdk_pixbuf_new_from_file(IMGDIR"/webqq_icon.png", NULL);
        gtk_status_icon_set_from_pixbuf(GTK_STATUS_ICON(tray), pb);
        g_object_unref(pb);
    }else{
        qq_tray_blinking(tray, g_queue_peek_tail(priv -> blinking_queue));
    }
}

#include <splashpanel.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static void qq_splashpanelclass_init(QQSplashPanelClass *c);
static void qq_splashpanel_init(QQSplashPanel *obj);
#ifndef USE_GTK3
static void qq_splashpanel_destroy(GtkObject *obj);
#else
static void qq_splashpanel_destroy(GtkWidget *obj);
#endif /* USE_GTK3 */

GtkWidget *qq_splashpanel_new()
{
	QQSplashPanel *panel = g_object_new(qq_splashpanel_get_type(), NULL);
	return GTK_WIDGET(panel);
}

GType qq_splashpanel_get_type()
{
	static GType t = 0;
	if(!t){
		static const GTypeInfo info =
			{
				sizeof(QQSplashPanelClass),
				NULL,
				NULL,
				(GClassInitFunc)qq_splashpanelclass_init,
				NULL,
				NULL,
				sizeof(QQSplashPanel),
				0,
				(GInstanceInitFunc)qq_splashpanel_init,
				NULL
			};
		t = g_type_register_static(GTK_TYPE_VBOX, "QQSplashPanel"
						, &info, 0);
	}
	return t;
	
}

static void qq_splashpanelclass_init(QQSplashPanelClass *c)
{
#ifndef USE_GTK3
	GtkObjectClass *cl = GTK_OBJECT_CLASS(c);
#else
	GtkWidgetClass *cl = GTK_WIDGET_CLASS(c);
#endif /* USE_GTK3 */
	cl -> destroy = qq_splashpanel_destroy;
}

/*
 * Let the progress bar shake.
 */
static gboolean progress_bar_timeout_func(gpointer data)
{
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(data));
	return TRUE;
}

static void qq_splashpanel_init(QQSplashPanel *obj)
{	
	GtkWidget *logo = gtk_image_new_from_file(IMGDIR"webqq_icon.png");
	gtk_widget_set_size_request(logo, -1, 250);
	gtk_box_pack_start(GTK_BOX(obj), logo, FALSE, FALSE, 0);

	GtkWidget *probar = gtk_progress_bar_new();
	gtk_widget_set_size_request(probar, 200, 50);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(probar), "Login...");
#ifdef USE_GTK3
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(probar), TRUE);
#endif /* USE_GTK3 */

	g_timeout_add(100, progress_bar_timeout_func, probar);
	
	GtkWidget *box = NULL;
	box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), probar, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(obj), box, FALSE, FALSE, 50);
}
#ifndef USE_GTK3
static void qq_splashpanel_destroy(GtkObject *obj)
#else
static void qq_splashpanel_destroy(GtkWidget *obj)
#endif /* USE_GTK3 */
{

}

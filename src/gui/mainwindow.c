#include <mainwindow.h>

static void qq_mainwindow_init(QQMainWindow *win);
static void qq_mainwindowclass_init(QQMainWindowClass *wc);
static void qq_mainwindow_destroy(GObject *obj);
static void destroy_handle(GtkWidget *widget, gpointer  data);
/*
 * The handler of "destroy" singal
 */
static void destroy_handler(GtkWidget *widget, gpointer  data)
{
	    gtk_main_quit ();
}

GType qq_mainwindow_get_type()
{
	static GType t = 0;
	if(!t){
		const GTypeInfo info =
		{
			sizeof(QQMainWindowClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc)qq_mainwindowclass_init,
			NULL,	/* class finalize*/
			NULL,	/* class data */
			sizeof(QQMainWindow),
			0,	/* n pre allocs */
			(GInstanceInitFunc)qq_mainwindow_init,
			0
		};

		t = g_type_register_static(GTK_TYPE_WINDOW, "QQMainWindow"
					, &info, 0);
	}
	return t;
}

GtkWidget* qq_mainwindow_new()
{
	return GTK_WIDGET(g_object_new(qq_mainwindow_get_type(), NULL));
}


static void qq_mainwindow_init(QQMainWindow *win)
{
	GtkWidget *w = GTK_WIDGET(win);
	gtk_widget_set_size_request(w, 260, 520);
	gtk_window_set_resizable(GTK_WINDOW(w), FALSE);
	g_signal_connect(G_OBJECT(w), "destroy",
			                 G_CALLBACK(destroy_handler), NULL);

}
static void qq_mainwindowclass_init(QQMainWindowClass *wc)
{

}
static void qq_mainwindow_destroy(GObject *obj)
{

}

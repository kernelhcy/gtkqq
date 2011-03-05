#include <mainwindow.h>
#include <loginpanel.h>
#include <splashpanel.h>

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
	
	win -> login_panel = qq_loginpanel_new(w);
	win -> splash_panel = qq_splashpanel_new();
	win -> main_panel = NULL;

	win -> notebook = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(win -> notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(win -> notebook), FALSE);

	gtk_widget_show_all(win -> login_panel);
	gtk_widget_show_all(win -> splash_panel);
	gtk_widget_show_all(win -> main_panel);
	
	gtk_notebook_append_page(GTK_NOTEBOOK(win -> notebook)
				, win -> login_panel, NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(win -> notebook)
				, win -> splash_panel, NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(win -> notebook)
				, win -> main_panel, NULL);

	gtk_container_add(GTK_CONTAINER(win), win -> notebook);

	GtkWidget *img = gtk_image_new_from_file(IMGDIR"webqq_icon.png");
	gtk_window_set_icon(GTK_WINDOW(win)
			, gtk_image_get_pixbuf(GTK_IMAGE(img)));

}
static void qq_mainwindowclass_init(QQMainWindowClass *wc)
{

}
static void qq_mainwindow_destroy(GObject *obj)
{

}

void qq_mainwindow_show_loginpanel(GtkWidget *win)
{
	if(!QQ_IS_MAINWINDOW(win)){
		g_warning("Not a mainwindow!!(%s, %d)", __FILE__, __LINE__);
		return;
	}
	gtk_notebook_set_current_page(GTK_NOTEBOOK(
				QQ_MAINWINDOW(win) -> notebook), 0);
}
void qq_mainwindow_show_splashpanel(GtkWidget *win)
{
	if(!QQ_IS_MAINWINDOW(win)){
		g_warning("Not a mainwindow!!(%s, %d)", __FILE__, __LINE__);
		return;
	}
	gtk_notebook_set_current_page(GTK_NOTEBOOK(
				QQ_MAINWINDOW(win) -> notebook), 1);
}
void qq_mainwindow_show_mainpanel(GtkWidget *win)
{
	if(!QQ_IS_MAINWINDOW(win)){
		g_warning("Not a mainwindow!!(%s, %d)", __FILE__, __LINE__);
		return;
	}
	gtk_notebook_set_current_page(GTK_NOTEBOOK(
				QQ_MAINWINDOW(win) -> notebook), 2);
}

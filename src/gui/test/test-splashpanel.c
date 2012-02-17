#include <gtk/gtk.h>
#include "splashpanel.h"

gint main(gint argc, gchar **argv)
{
	GtkWidget *window;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	GtkWidget *splashpanel = qq_splashpanel_new();

	gtk_container_add(GTK_CONTAINER(window), splashpanel);

	g_signal_connect(G_OBJECT(window), "destroy", 
			G_CALLBACK(gtk_main_quit), NULL);

	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}

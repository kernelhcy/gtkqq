#include <gtk/gtk.h>
#include <stdlib.h>
#include <loginpanel.h>
#include <mainpanel.h>

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	GtkWidget *win = qq_mainwindow_new();
	GtkWidget *panel = qq_loginpanel_new();
	gtk_container_add(GTK_CONTAINER(win), panel);
	gtk_widget_show_all(win);
	
	gtk_main();
	return 0;
}

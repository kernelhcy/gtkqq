#include <gtk/gtk.h>
#include <stdlib.h>
#include <loginpanel.h>
#include <mainpanel.h>
#include <mainwindow.h>
#include <qq.h>
#include <log.h>

/*
 * Global
 */
QQInfo *info = NULL;
int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);

	log_init();
	info = qq_init(NULL, NULL);
	GtkWidget *win = qq_mainwindow_new();
	gtk_widget_show_all(win);
	
	gtk_main();
	return 0;
}

#include <gtk/gtk.h>
#include <stdlib.h>
#include <loginpanel.h>
#include <mainpanel.h>
#include <mainwindow.h>
#include <qq.h>
#include <log.h>
#include <config.h>

/*
 * Global
 */
QQInfo *info = NULL;
QQConfig *cfg = NULL;

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);

	log_init();
	cfg = qq_config_init();
	qq_config_read(cfg);
	if(cfg == NULL){
		return -1;
	}
	info = qq_init(NULL, NULL);
	if(info == NULL){
		return -1;
	}

	GtkWidget *win = qq_mainwindow_new();
	gtk_widget_show_all(win);
	
	gtk_main();
	return 0;
}

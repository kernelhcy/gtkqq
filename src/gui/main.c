#include <gtk/gtk.h>
#include <stdlib.h>
#include <loginpanel.h>
#include <mainpanel.h>
#include <mainwindow.h>
#include <qq.h>
#include <log.h>
#include <gqqconfig.h>

/*
 * Global
 */
QQInfo *info = NULL;
GQQConfig *cfg = NULL;

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);

	log_init();
	info = qq_init(NULL);
	if(info == NULL){
		return -1;
	}
    cfg = qq_config_new(info);

	GtkWidget *win = qq_mainwindow_new();
	gtk_widget_show_all(win);
	
	gtk_main();
    qq_finalize(info, NULL);
	return 0;
}

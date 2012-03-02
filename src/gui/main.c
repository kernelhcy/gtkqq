#include <gtk/gtk.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <loginpanel.h>
#include <mainpanel.h>
#include <mainwindow.h>
#include <qq.h>
#include <log.h>
#include <gqqconfig.h>
#include <msgloop.h>
#include <groupchatwindow.h>
#include <tray.h>
#include <config.h>

#ifdef USE_GSTREAMER
#include <gst/gst.h>
#endif //USE_GSTREAMER

/*
 * Global
 */
QQInfo *info = NULL;
GQQConfig *cfg = NULL;
QQTray *tray = NULL;
GtkWidget *main_win = NULL;

//
// The main loop used to get information from the server.
// Such as face images, buddy information.
//
GQQMessageLoop *get_info_loop = NULL;
//
// The main loop used to get qq number and face images from the server.
// Such as face images, buddy information.
//
GQQMessageLoop *get_number_faceimg_loop = NULL;
//
// The main loop used to send messages.
// Such as group messages and buddy messages.
//
GQQMessageLoop *send_loop = NULL;



gboolean debug = FALSE;		/* Whether handle debug message */

static gboolean  _print_version_and_exit(const char *option_name, 
					const char *value, gpointer data, 
					GError **error)
{
	g_print("%s %s\n", PACKAGE, VERSION);
	exit(EXIT_SUCCESS);
	return TRUE;
}

static GOptionEntry entries[] = 
{
	{"debug", 'd', 0, G_OPTION_ARG_NONE, &debug, "Open debug mode", NULL},
	{"version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, 
	 _print_version_and_exit, "Show the application's version", NULL},
	{NULL}
};

int main(int argc, char **argv)
{
	GError *error = NULL;
	GOptionContext *context;
	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("option parsing failed: %s\n", error->message);
		exit(1);
	}
	g_option_context_free(context);

#ifdef USE_GSTREAMER
	gst_init(NULL, NULL);
#endif
	gtk_init(&argc, &argv);

    log_init(debug);
    info = qq_init(NULL);
    if(info == NULL){
        return -1;
    }
    cfg = gqq_config_new(info);

    send_loop = gqq_msgloop_start("Send");
    if(send_loop == NULL){
        return -1;
    }
    get_info_loop = gqq_msgloop_start("Get informain");
    if(get_info_loop == NULL){
        return -1;
    }
    get_number_faceimg_loop = gqq_msgloop_start("Get Number and face images");
    if(get_number_faceimg_loop == NULL){
        return -1;
    }

    /* GtkWidget *win = qq_mainwindow_new(); */
	main_win = qq_mainwindow_new();
    tray = qq_tray_new();
    gtk_widget_show_all(main_win);

    gtk_main();

    g_object_unref(G_OBJECT(tray));

    qq_logout(info, NULL);
    gqq_msgloop_stop(get_info_loop);
    gqq_msgloop_stop(send_loop);

    //Save config
    gqq_config_save(cfg);
    qq_finalize(info, NULL);
    return 0;
}

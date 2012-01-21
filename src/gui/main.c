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

static void usage()
{
	fprintf(stdout, "Usage: gtkqq [options]...\n"
			"gtkqq: A qq client based on gtk+ uses webqq protocol\n"
			"  -v, --version\n"
			"      Show version of program\n"
			"  -d, --debug\n"
			"      Open debug mode\n"
			"  -h, --help\n"
			"      Print help and exit\n"
		);
}

int main(int argc, char **argv)
{
	gboolean debug = FALSE;		/* Wheather handle debug message */
	int c, err = 0;
	static const struct option long_options[] = {
		{ "version", 0, 0, 'v' },
		{ "debug", 0, 0, 'd' },
		{ "help", 0, 0, 'h' },
		{ 0, 0, 0, 0 }
	};
		
#ifdef USE_GSTREAMER
	gst_init(NULL, NULL);
#endif
    gtk_init(&argc, &argv);

	while ((c = getopt_long(argc, argv, "vdh",
							long_options, NULL)) != EOF) {
		switch (c) {
		case 'v':
			printf("" PACKAGE " " VERSION "\n");
			exit(0);

		case 'd':
			debug = TRUE;
			break;

		case 'h':
			usage();
			exit(0);

		default:
			err++;
			break;
		}
	}
	if (err || argc > optind) {
		usage();
		exit(1);
	}
	
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

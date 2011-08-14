#include <gtk/gtk.h>
#include <stdlib.h>
#include <loginpanel.h>
#include <mainpanel.h>
#include <mainwindow.h>
#include <qq.h>
#include <log.h>
#include <gqqconfig.h>
#include <msgloop.h>
#include <chatwindow.h>
#include <tray.h>

/*
 * Global
 */
QQInfo *info = NULL;
GQQConfig *cfg = NULL;
QQTray *tray = NULL;

//
// The main loop used to get information from the server.
// Such as face images, buddy information.
//
GQQMessageLoop *get_info_loop = NULL;
//
// The main loop used to send messages.
// Such as group messages and buddy messages.
//
GQQMessageLoop *send_loop = NULL;

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    log_init();
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

    GtkWidget *win = qq_mainwindow_new();
    tray = qq_tray_new();
    gtk_widget_show_all(win);

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

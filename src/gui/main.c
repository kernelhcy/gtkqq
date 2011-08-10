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
/*
 * Global
 */
QQInfo *info = NULL;
GQQConfig *cfg = NULL;
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
    gtk_widget_show_all(win);

    //test chat window
    GtkWidget *cw = qq_chatwindow_new("1234567890");

    QQSendMsg *msg = qq_sendmsg_new(info, 0, "1234567890");
    QQMsgContent *ctent = NULL;
    ctent = qq_msgcontent_new(2, "hello你好");
    qq_sendmsg_add_content(msg, ctent);
    ctent = qq_msgcontent_new(2, "hello你好");
    qq_sendmsg_add_content(msg, ctent);
    ctent = qq_msgcontent_new(2, "hello你好");
    qq_sendmsg_add_content(msg, ctent);
    ctent = qq_msgcontent_new(1, 62);
    qq_sendmsg_add_content(msg, ctent);
    ctent = qq_msgcontent_new(1, 51);
    qq_sendmsg_add_content(msg, ctent);
    ctent = qq_msgcontent_new(3, "宋体", 20, "808000", 0, 0, 0);
    qq_sendmsg_add_content(msg, ctent);
    qq_chatwindow_add_send_message(cw, msg);
    qq_chatwindow_add_send_message(cw, msg);
    qq_sendmsg_free(msg);

    
    QQRecvMsg *rmsg = qq_recvmsg_new(info, "buddy_message");
    ctent = qq_msgcontent_new(2, "hello你好");
    qq_recvmsg_add_content(rmsg, ctent);
    ctent = qq_msgcontent_new(2, "hello你好");
    qq_recvmsg_add_content(rmsg, ctent);
    ctent = qq_msgcontent_new(2, "hello你好");
    qq_recvmsg_add_content(rmsg, ctent);
    ctent = qq_msgcontent_new(1, 62);
    qq_recvmsg_add_content(rmsg, ctent);
    ctent = qq_msgcontent_new(1, 51);
    qq_recvmsg_add_content(rmsg, ctent);
    ctent = qq_msgcontent_new(3, "黑体", 15, "808080", 1, 1, 0);
    qq_recvmsg_add_content(rmsg, ctent);
    qq_chatwindow_add_recv_message(cw, rmsg);
    qq_chatwindow_add_recv_message(cw, rmsg);
    qq_recvmsg_free(rmsg);

    gtk_main();

    qq_logout(info, NULL);
    gqq_msgloop_stop(get_info_loop);
    gqq_msgloop_stop(send_loop);

    //Save config
    gqq_config_save(cfg);
    qq_finalize(info, NULL);
    return 0;
}

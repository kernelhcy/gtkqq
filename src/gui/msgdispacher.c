#include <msgdispacher.h>
#include <gqqconfig.h>
#include <chatwindow.h>
#include <groupchatwindow.h>
#include <tray.h>
#include <msgloop.h>
#include <config.h>
#include <sound.h>

//
// Global 
//
extern QQTray *tray;
extern QQInfo *info;
extern GQQConfig *cfg;

#ifdef USE_GSTREAMER
static void qq_sound_notify(QQRecvMsg *msg)
{
	char filename[512] = {0};
	
	if (!msg)
		return ;
	
	switch (msg->msg_type) {
    case MSG_BUDDY_T:
    case MSG_GROUP_T:
		/* When new message is coming, play a msg audio. */ 
		g_snprintf(filename, sizeof(filename), SOUNDDIR"/Classic/msg.wav");
        break;
    case MSG_STATUS_CHANGED_T:
		if (!g_strcmp0(msg->status->str, "online")) {
			g_snprintf(filename, sizeof(filename), SOUNDDIR"/Classic/Global.wav");
			break;
		} else {
			return;
		}
    case MSG_KICK_T:
		/* No implement now */
		return;
    default:
        g_warning("Unknonw poll message type! %d (%s, %d)", msg->msg_type
				  , __FILE__, __LINE__);
		return;
    }
	
	qq_play_wavfile(filename);
}
#endif //USE_GSTREAMER

static void qq_poll_dispatch_buddy_msg(QQRecvMsg *msg)
{
    GtkWidget *cw = gqq_config_lookup_ht(cfg, "chat_window_map"
                                            , msg -> from_uin -> str);
	
    if(cw == NULL){
        cw = qq_chatwindow_new(msg -> from_uin -> str); 
        // not show it
        gtk_widget_hide(cw);
        gqq_config_insert_ht(cfg, "chat_window_map"
                                , msg -> from_uin -> str, cw);
    }
    qq_chatwindow_add_recv_message(cw, msg);
    qq_tray_blinking_for(tray, msg -> from_uin -> str);
    qq_recvmsg_free(msg);
}

static void qq_poll_dispatch_group_msg(QQRecvMsg *msg)
{
    GtkWidget *cw = gqq_config_lookup_ht(cfg, "chat_window_map"
                                            , msg -> group_code -> str);
    if(cw == NULL){
        cw = qq_group_chatwindow_new(msg -> group_code -> str); 
        // not show it
        gtk_widget_hide(cw);
        gqq_config_insert_ht(cfg, "chat_window_map"
                                , msg -> group_code -> str, cw);
    }
    qq_group_chatwindow_add_recv_message(cw, msg);
    qq_tray_blinking_for(tray, msg -> group_code -> str);
    qq_recvmsg_free(msg);

}

static void qq_poll_dispatch_status_changed_msg(QQRecvMsg *msg)
{

}

static void qq_poll_dispatch_kick_msg(QQRecvMsg *msg)
{

}

gint qq_poll_message_callback(QQRecvMsg *msg, gpointer data)
{
    if(msg == NULL || data == NULL){
        return 0;
    }
   
    GQQMessageLoop *loop = data;

#ifdef USE_GSTREAMER
	/* Play a audio to notify that a new msg is coming. */
	qq_sound_notify(msg);
#endif // USE_GSTREAMER
	
    switch(msg -> msg_type)
    {
    case MSG_BUDDY_T:
        gqq_mainloop_attach(loop, qq_poll_dispatch_buddy_msg, 1, msg);
        break;
    case MSG_GROUP_T:
        gqq_mainloop_attach(loop, qq_poll_dispatch_group_msg, 1, msg);
        break;
    case MSG_STATUS_CHANGED_T:
        gqq_mainloop_attach(loop, qq_poll_dispatch_status_changed_msg, 1, msg);
        break;
    case MSG_KICK_T:
        gqq_mainloop_attach(loop, qq_poll_dispatch_kick_msg, 1, msg);
        break;
    default:
        g_warning("Unknonw poll message type! %d (%s, %d)", msg -> msg_type
                                    , __FILE__, __LINE__);
        break;
    }
    return 0;
}

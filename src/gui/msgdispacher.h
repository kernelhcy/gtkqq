#ifndef _GTKQQ_MSG_DISPACHER_H_
#define _GTKQQ_MSG_DISPACHER_H_
#include <qq.h>
#include <glib.h>

//
// The callback of the message poll
// For every received message, this function will be called.
// This function will be called out the gtk main event loop.
//
gint qq_poll_message_callback(QQRecvMsg *msg, gpointer data);

#endif

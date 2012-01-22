/**
 * notify.h
 *
 * send a message to the system bus by libnotify
 */

#ifndef GTKQQ_NOTIFY_H
#define GTKQQ_NOTIFY_H

/** 
 * Send a notify event.
 * 
 * @param title Msg title
 * @param body Msg body
 * @param icon 
 */
void qq_notify_send(const gchar *title, const gchar *body, const gchar *icon);

#endif

#include <glib.h>
#include <libnotify/notify.h>  

#ifndef PACKAGE_NAME
#define PACKAGE_NAME "GtkQQ"
#endif	/* PACKAGE_NAME */

#define NOTIFY_TIMEOUT 3 * 1000	/* timeout: 3S  */

/** 
 * Send a notify event.
 * 
 * @param title Msg title
 * @param body Msg body
 * @param icon 
 */
void qq_notify_send(const gchar *title, const gchar *body, const gchar *icon)
{  
	NotifyNotification *notify = NULL;
	GError *error = NULL;

	if (!notify_is_initted()) {
		g_debug("initialy notify. (%s, %d)\n", __FILE__, __LINE__);
		notify_init(PACKAGE_NAME);
	}
	
	notify = notify_notification_new(title, body, icon,NULL);
	if (!notify) {
		g_warning("notify new failed. (%s, %d)\n", __FILE__, __LINE__);
		goto notify_exit;
	}
	
	notify_notification_set_timeout (notify, NOTIFY_TIMEOUT);
	
	if (!notify_notification_show(notify, &error)) {
		g_warning("failed to send notification: %s. (%s, %d)\n",
				  error->message, __FILE__, __LINE__);
		g_error_free(error);
	}

notify_exit:
	if (notify)
		g_object_unref(G_OBJECT(notify));
}


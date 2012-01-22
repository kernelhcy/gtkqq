#ifndef __GTKQQ_TRAY_H
#define __GTKQQ_TRAY_H
#include <gtk/gtk.h>
#include <qq.h>

#define QQ_TRAY(obj)	G_TYPE_CHECK_INSTANCE_CAST(obj, qq_tray_get_type(), QQTray)
#define QQ_TRAYCLASS(c)	G_TYPE_CHECK_CLASS_CAST(c, qq_tray_get_type()\
				      	, QQTrayClass)
#define QQ_IS_TRAY(obj)	G_TYPE_CHECK_INSTANCE_TYPE(obj, qq_tray_get_type())

typedef struct __QQTray 		QQTray;
typedef struct __QQTrayClass		QQTrayClass;

struct __QQTray{
    GtkStatusIcon parent;
};

struct __QQTrayClass{
    GtkStatusIconClass parent;
};

QQTray* qq_tray_new();
GType qq_tray_get_type();


/**
 * Blink the tray icon for the uin
 * If the tray icon is blinking, add the uin to the queue.
 */
void qq_tray_blinking_for(QQTray *tray, const gchar *uin);

/**
 * Stop blinking for the uin.
 * If some uin is waiting for blinking, blink it.
 */
void qq_tray_stop_blinking_for(QQTray *tray, const gchar *uin);

/** 
 * Set mute item status, it usually called when user login.
 * 
 * @param tray 
 * @param mute 
 */
void qq_tray_set_mute_item(QQTray *tray, gboolean mute);

#endif /* __GTKQQ_TRAY_H */

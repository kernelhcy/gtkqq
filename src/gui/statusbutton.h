#ifndef __GTKQQ_STATUS_BUTTON
#define __GTKQQ_STATUS_BUTTON
#include <gtk/gtk.h>
#include <qq.h>

/*
 * QQ status button.
 * Show and change the status.
 */
#define QQ_STATUSBUTTON(obj) 		GTK_CHECK_CAST(obj\
						, qq_statusbutton_get_type()\
						, QQStatusButton)
#define QQ_STATUSBUTTONCLASS(c)		GTK_CHECK_CLASS_CAST(c\
						, qq_statusbutton_get_type()\
						, QQStatusButtonClass)
#define QQ_IS_STATUSBUTTON(obj) 	GTK_CHECK_TYPE(obj\
						, qq_statusbutton_get_type())
#define QQ_TYPE_STATUSBUTTON 		qq_statusbutton_get_type()

typedef struct _QQStatusButton 		QQStatusButton;
typedef struct _QQStatusButtonClass 	QQStatusButtonClass;

typedef enum _QQStatusButtonStatus 	QQStatusButtonStatus;

enum _QQStatusButtonStatus{
	STATUS_ONLINE = 0,
	STATUS_HIDDEN,
	STATUS_AWAY,
	STATUS_OFFLINE,
	STATUS_UNKNOWN
};

struct _QQStatusButton{
	GtkComboBox parent;
	QQStatusButtonStatus status;

	/*< private >*/
	GtkWidget *popmenu;
};

struct _QQStatusButtonClass{
	GtkComboBoxClass parent;

	/*< private >*/
	GdkPixbuf *pb[4];
	GdkCursor *hand_cursor;
};

GType qq_statusbutton_get_type();
GtkWidget* qq_statusbutton_new();

/*
 * Get the status
 * string or int
 */
const gchar* qq_statusbutton_get_status_string(QQStatusButton *btn);
QQStatusButtonStatus qq_statusbutton_get_status(QQStatusButton *btn);

#endif

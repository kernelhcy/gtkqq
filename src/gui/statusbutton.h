#ifndef __GTKQQ_STATUS_BUTTON
#define __GTKQQ_STATUS_BUTTON
#include <gtk/gtk.h>
#include <qq.h>

/*
 * QQ status button.
 * Show and change the status.
 */
#define QQ_STATUSBUTTON(obj) 		G_TYPE_CHECK_INSTANCE_CAST(obj\
						, qq_statusbutton_get_type()\
						, QQStatusButton)
#define QQ_STATUSBUTTONCLASS(c)		G_TYPE_CHECK_CLASS_CAST(c\
						, qq_statusbutton_get_type()\
						, QQStatusButtonClass)
#define QQ_IS_STATUSBUTTON(obj) 	G_TYPE_CHECK_INSTANCE_TYPE(obj\
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
	STATUS_CALLME,
	STATUS_BUSY,
	STATUS_SILENT,
	STATUS_NUM
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
	GdkPixbuf *pb[STATUS_NUM];
	GdkPixbuf *arrow;
	GdkCursor *hand_cursor;
};

GType qq_statusbutton_get_type();
GtkWidget* qq_statusbutton_new();

//
// Get the status
// string or int
//
const gchar* qq_statusbutton_get_status_string(GtkWidget *btn);
QQStatusButtonStatus qq_statusbutton_get_status(GtkWidget *btn);

//
// Set the status
//
void qq_statusbutton_set_status_string(GtkWidget *btn
                                        , const gchar *status);
void qq_statusbutton_set_status(GtkWidget *btn
                                        , QQStatusButtonStatus status);
#endif

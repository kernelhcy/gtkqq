#ifndef __GTKQQ_LOGINWIN_H
#define __GTKQQ_LOGINWIN_H
#include <gtk/gtk.h>

#define QQ_LOGINPANEL(obj)	GTK_CHECK_CAST(obj, qq_loginpanel_get_type()\
						, QQLoginPanel)
#define QQ_LOGINPANEL_CLASS(c)	GTK_CHECK_CLASS_CAST(c\
					, qq_loginpanel_get_type()\
					, QQLoginPanelClass)
#define QQ_IS_LOGINPANEL(obj)	GTK_CHECK_TYPE(obj, qq_loginpanel_get_type())

typedef struct _QQLoginPanel 		QQLoginPanel;
typedef struct _QQLoginPanelClass 	QQLoginPanelClass;

struct _QQLoginPanel{
	GtkVBox parent;

	GtkWidget *uin_label, *uin_entry;
	GtkWidget *passwd_label, *passwd_entry;

	GtkWidget *login_btn;
};

struct _QQLoginPanelClass{
	GtkVBoxClass parent;
};

GtkWidget* qq_loginpanel_new();
GtkType qq_loginpanel_get_type();

#endif

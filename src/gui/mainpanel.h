#ifndef __GTKQQ_MAINPANEL_H
#define __GTKQQ_MAINPANEL_H
#include <gtk/gtk.h>

#define QQ_MAINPANEL(obj)	GTK_CHECK_CAST(obj, qq_mainpanel_get_type()\
						, QQMainPanel)
#define QQ_MAINPANELCLASS(c)	GTK_CHECK_CLASS_CAST(c\
						, qq_mainpanel_get_type()\
						, QQMainPanelClass)
#define QQ_IS_MAINPANEL(obj)	GTK_CHECK_TYPE(obj, qq_mainpanel_get_type())

typedef struct _QQMainPanel 		QQMainPanel;
typedef struct _QQMainPanelClass	QQMainPanelClass;

struct _QQMainPanel{
	GtkVBox parent;

	GtkWidget *container;
};

struct _QQMainPanelClass{
	GtkVBoxClass parent;
};

GType qq_mainpanel_get_type();
GtkWidget* qq_mainpanel_new(GtkWidget *container);
#endif

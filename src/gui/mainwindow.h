#ifndef __GTKQQ_MAINWINDOW_H
#define __GTKQQ_MAINWINDOW_H
#include <gtk/gtk.h>

#define QQ_MAINWINDOW(obj)	GTK_CHEKC_CAST(obj, qq_mainwindow_get_type()\
						, QQMainWindow)
#define QQ_MAINWINDOWCLASS(c)	GTK_CHECK_CLASS_CAST(c\
						, qq_mainwindow_get_type()\
						, QQMainWindowClass)
#define QQ_IS_MAINWINDOW(obj)	GTK_CHECK_TYPE(obj, qq_main_window_get_type())

typedef struct __QQMainWindow 		QQMainWindow;
typedef struct __QQMainWindowClass	QQMainWindowClass;

struct __QQMainWindow{
	GtkWindow parent;
};

struct __QQMainWindowClass{
	GtkWindowClass parent;
};

GtkWidget* qq_mainwindow_new();
GType qq_mainwindow_get_type();

#endif

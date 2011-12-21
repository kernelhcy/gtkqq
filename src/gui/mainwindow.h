#ifndef __GTKQQ_MAINWINDOW_H
#define __GTKQQ_MAINWINDOW_H
#include <gtk/gtk.h>

#define QQ_MAINWINDOW(obj)	GTK_CHECK_CAST(obj, qq_mainwindow_get_type()\
						, QQMainWindow)
#define QQ_MAINWINDOWCLASS(c)	GTK_CHECK_CLASS_CAST(c\
						, qq_mainwindow_get_type()\
						, QQMainWindowClass)
#define QQ_IS_MAINWINDOW(obj)	GTK_CHECK_TYPE(obj, qq_mainwindow_get_type())

typedef struct __QQMainWindow 		QQMainWindow;
typedef struct __QQMainWindowClass	QQMainWindowClass;

struct __QQMainWindow{
	GtkWindow parent;

	GtkWidget *notebook;

	GtkWidget *login_panel;
	GtkWidget *main_panel;
	GtkWidget *splash_panel;

	gboolean showed;
};

struct __QQMainWindowClass{
	GtkWindowClass parent;
};

GtkWidget* qq_mainwindow_new();
GType qq_mainwindow_get_type();

/* Main window show or hide */
void qq_mainwindow_show(GtkWidget *win);
void qq_mainwindow_hide(GtkWidget *win);

/* If the window now is shown, hide it,
   else show it. */
void qq_mainwindow_show_hide(GtkWidget *win);
/*
 * Show different panels
 */
void qq_mainwindow_show_loginpanel(GtkWidget *win);
void qq_mainwindow_show_splashpanel(GtkWidget *win);
void qq_mainwindow_show_mainpanel(GtkWidget *win);

/** 
 * Get the main panel accoss main window.
 * 
 * @param win The main window.
 * 
 * @return 
 */
GtkWidget *qq_mainwindow_get_mainpanel(GtkWidget *win);
#endif

#ifndef __GTKQQ_QQWINDOW_H
#define __GTKQQ_QQWINDOW_H
#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>

#define QQ_WINDOW_TYPE			(qq_window_get_type())
#define QQ_WINDOW(obj) 			(G_TYPE_CHECK_INSTANCE_CAST((obj)\
						, QQ_WINDOW_TYPE\
						, QQWindow))
#define QQ_WINDOW_CLASS(klass) 		(G_TYPE_CHECK_CLASS_CAST((klass)\
						, QQ_WINDOW_TYPE\
						, QQWindowClass))
#define QQ_IS_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj)\
						, QQ_WINDOW_TYPE))
#define QQ_IS_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass)\
						, QQ_WINDOW_TYPE))
#define QQ_WINDOW_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj)\
						, QQ_WINDOW_TYPE\
						, QQWindowClass))
typedef struct _QQWindow 	QQWindow;
typedef struct _QQWindowClass 	QQWindowClass;


struct _QQWindow{
	GtkWindow parent;
	
	/* private */
	gboolean is_maxsize;
	gint pre_w, pre_h;	//the pre width and height
};

struct _QQWindowClass{
	GtkWindowClass parentclass;

	/* private */
	/*
	 * the cursor used when resize the window
	 */
	GdkCursor *ls, *rs, *bs, *ts;
	GdkCursor *tlc, *trc, *blc, *brc;
	GdkCursor *nc;	//the normal cursor

	/*
	 * The width of the border which is used to resize the
	 * window
	 */
	gint resize_border;

	/*
	 * Use to get a rounded rectangle
	 */
	GdkColormap 	*cm;
	GdkColor 	fg, bg;
};

GtkWidget* 	qq_window_new();
GtkType 	qq_window_get_type();

/*
 * Maximizes the windows
 * If the window is already maximized, then this function
 * does nothing.
 */
void		qq_window_maximize(QQWindow *qwin);
/*
 * Unmaximizes the windows
 * If the window is already unmaximized, then this function
 * does nothing.
 */
void		qq_window_unmaximize(QQWindow *qwin);

#endif


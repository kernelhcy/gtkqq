#include "qqwindow.h"
#include <gtk/gtkmain.h>
#include <cairo.h>
#include <math.h>

static void qq_window_init		(QQWindow *qwin, gpointer data);
static void qq_window_class_init	(QQWindowClass *qclass, gpointer data);
static gboolean delete_event_cb		(GtkWidget *widget, gpointer data);
static gboolean button_press_event_cb	(GtkWidget *widget, GdkEventButton *event
					, gpointer data);
static gboolean button_release_event_cb	(GtkWidget *widget, GdkEventButton *event
					, gpointer data);
static gboolean motion_event_cb		(GtkWidget *widget, GdkEventMotion *event
					, gpointer data);
static gboolean expose_event_cb		(GtkWidget *widget, GdkEventExpose *event
					, gpointer data);
static gboolean configure_event_cb	(GtkWidget *widget, GdkEventConfigure *event
					, gpointer data);

static void qq_window_set_shape_mask	(QQWindow *qwin);
static void qq_window_remove_shape_mask	(QQWindow *qw);



GtkWidget* qq_window_new()
{
	GtkWidget *widget;
	widget = g_object_new(qq_window_get_type(), NULL);

	return GTK_WIDGET(widget);
}

void qq_window_unmaximize(QQWindow *qwin)
{
	g_return_if_fail(qwin != NULL);
	g_return_if_fail(QQ_IS_WINDOW(qwin));

	gtk_window_unmaximize(GTK_WINDOW(qwin));
	qwin -> is_maxsize = FALSE;
	return;
}

void qq_window_maximize(QQWindow *qwin)
{
	g_return_if_fail(qwin != NULL);
	g_return_if_fail(QQ_IS_WINDOW(qwin));

	gtk_window_maximize(GTK_WINDOW(qwin));
	qwin -> is_maxsize = TRUE;

	return;
}
GtkType qq_window_get_type()
{
	static GType type = 0;
	if(type == 0){
		static const GTypeInfo type_info = 
		{
			sizeof(QQWindowClass),
			NULL,
			NULL,
			(GClassInitFunc)qq_window_class_init,
			NULL,
			NULL,
			sizeof(QQWindow),
			0,
			(GInstanceInitFunc)qq_window_init
		};

		type = g_type_register_static(GTK_TYPE_WINDOW
					, "QQWindow"
					, &type_info
					, (GTypeFlags)0);
	}
	return type;
}

/*
 * Instance initial function
 *
 * Most of the works are here.
 */
void qq_window_init(QQWindow *qwin, gpointer data)
{
	g_return_if_fail(qwin != NULL);
	g_return_if_fail(QQ_IS_WINDOW(qwin));
	
	GtkWidget *widget = GTK_WIDGET(qwin);
	GtkWindow *win = GTK_WINDOW(qwin);

	qwin -> is_maxsize = FALSE;
	gtk_window_resize(win, 500, 600);
	gtk_widget_set_size_request(widget, 50, 50);

	gtk_window_set_decorated(win, FALSE);

	g_signal_connect(widget, "delete-event"
			, G_CALLBACK(delete_event_cb), NULL);

	g_signal_connect(widget, "button-press-event"
			, G_CALLBACK(button_press_event_cb), NULL);
	g_signal_connect(widget, "button-release-event"
			, G_CALLBACK(button_release_event_cb), NULL);
	g_signal_connect(widget, "motion-notify-event"
			, G_CALLBACK(motion_event_cb), NULL);
	g_signal_connect(widget, "configure-event"
			, G_CALLBACK(configure_event_cb), NULL);

	g_signal_connect(widget, "expose-event"
			, G_CALLBACK(expose_event_cb), NULL);

	/*
	 * set the events mask.
	 * At default, the window does not recieve the button-press event.
	 * We must show the window first, which means the gdk window is 
	 * created, before we can set the events mask.
	 */
	gtk_widget_show(widget);
	gdk_window_set_events(widget -> window, GDK_ALL_EVENTS_MASK);
	/*
	 * We do NOT want to show the window at the creation.
	 * So we hide it.
	 */
	gtk_widget_hide(widget);
	// set the shape of the window
	qq_window_set_shape_mask(QQ_WINDOW(widget));

}

void qq_window_class_init(QQWindowClass *qclass, gpointer data)
{
	qclass -> rs = gdk_cursor_new(GDK_RIGHT_SIDE);
	qclass -> ls = gdk_cursor_new(GDK_LEFT_SIDE);
	qclass -> ts = gdk_cursor_new(GDK_TOP_SIDE);
	qclass -> bs = gdk_cursor_new(GDK_BOTTOM_SIDE);
	qclass -> trc = gdk_cursor_new(GDK_TOP_RIGHT_CORNER);
	qclass -> tlc = gdk_cursor_new(GDK_TOP_LEFT_CORNER);
	qclass -> brc = gdk_cursor_new(GDK_BOTTOM_RIGHT_CORNER);
	qclass -> blc = gdk_cursor_new(GDK_BOTTOM_LEFT_CORNER);

	qclass -> resize_border = 10;

	qclass -> cm = gdk_colormap_get_system();
	gdk_color_black(qclass -> cm, &qclass -> bg);
	gdk_color_white(qclass -> cm, &qclass -> fg);
}

void qq_window_dispose(GObject *obj)
{

}

void qq_window_finalize(GObject *obj)
{

}

gboolean button_press_event_cb(GtkWidget *widget, GdkEventButton *event
				, gpointer data)
{
	if(event -> button != 1){
		//Not the left button
		return TRUE;
	}

	if(event -> type == GDK_2BUTTON_PRESS){
		qq_window_remove_shape_mask(QQ_WINDOW(widget));
		if(QQ_WINDOW(widget) -> is_maxsize){
			qq_window_unmaximize(QQ_WINDOW(widget));
		}else{
			qq_window_maximize(QQ_WINDOW(widget));
		}
		return FALSE;
	}

	QQWindowClass *c = QQ_WINDOW_GET_CLASS(QQ_WINDOW(widget));
	gint root_x, root_y;
	gint x, y;
	root_x = (gint)event -> x_root;
	root_y = (gint)event -> y_root;
	x = (gint)event -> x;
	y = (gint)event -> y;
	gint width, height;
	gtk_window_get_size(GTK_WINDOW(widget), &width, &height);

	if(x >= 10 && y >= 10 && x <= width - 10 && y <= height - 10
			&& ! QQ_WINDOW(widget) -> is_maxsize){
		/*
		 * In the center area, we want to move the window
		 */
		gtk_window_begin_move_drag(GTK_WINDOW(widget)
					, 1 /* the left button */
					, root_x, root_y, 0);
		return FALSE;
	}

	/*
	 * In the border area, we want to resize the window.
	 */
	GdkWindowEdge edge = -1;
	if(x < c -> resize_border && y < c -> resize_border){
		edge = GDK_WINDOW_EDGE_NORTH_WEST;
	}else if(x > width - c -> resize_border && y > height - c -> resize_border){
		edge = GDK_WINDOW_EDGE_SOUTH_EAST;
	}else if(x < c -> resize_border && y > height - c -> resize_border){
		edge = GDK_WINDOW_EDGE_SOUTH_WEST;
	}else if(x > width - c -> resize_border && y < c -> resize_border){
		edge = GDK_WINDOW_EDGE_NORTH_EAST;
	}else if(x < c -> resize_border && y > c -> resize_border 
				&& y < height - c -> resize_border){
		edge = GDK_WINDOW_EDGE_WEST;
	}else if(x > width - c -> resize_border && y > c -> resize_border 
				&& y < height - c -> resize_border){
		edge = GDK_WINDOW_EDGE_EAST;
	}else if(y < c -> resize_border && x > c -> resize_border 
				&& x < width - c -> resize_border){
		edge = GDK_WINDOW_EDGE_NORTH;
	}else if(y > height - c -> resize_border && x > c -> resize_border 
				&& x < width - c -> resize_border){
		edge = GDK_WINDOW_EDGE_SOUTH;
	}
	if(edge != -1 && !QQ_WINDOW(widget) -> is_maxsize){
		qq_window_remove_shape_mask(QQ_WINDOW(widget));
		gtk_window_begin_resize_drag(GTK_WINDOW(widget)
					,edge, 1 /* left button */
					, root_x, root_y, 0);
	}
	
	return FALSE;
}

gboolean button_release_event_cb(GtkWidget *widget, GdkEventButton *event
				, gpointer data)
{
	return FALSE;
}
gboolean motion_event_cb(GtkWidget *widget, GdkEventMotion *event
					, gpointer data)
{
	gint x,y;
	x = (gint)event -> x;
	y = (gint)event -> y;
	QQWindowClass *c = QQ_WINDOW_GET_CLASS(QQ_WINDOW(widget));

	gint width, height;
	gtk_window_get_size(GTK_WINDOW(widget), &width, &height);

	GdkCursor *cur, *nc;
	nc = gdk_window_get_cursor(widget -> window);

	if(x < c -> resize_border && y < c -> resize_border){
		cur = c -> tlc;
	}else if(x > width - c -> resize_border && y > height - c -> resize_border){
		cur = c -> brc;
	}else if(x < c -> resize_border && y > height - c -> resize_border){
		cur = c -> blc;
	}else if(x > width - c -> resize_border && y < c -> resize_border){
		cur = c -> trc;
	}else if(x < c -> resize_border && y > c -> resize_border 
				&& y < height - c -> resize_border){
		cur = c -> ls;
	}else if(x > width - c -> resize_border && y > c -> resize_border 
				&& y < height - c -> resize_border){
		cur = c -> rs;
	}else if(y < c -> resize_border && x > c -> resize_border 
				&& x < width - c -> resize_border){
		cur = c -> ts;
	}else if(y > height - c -> resize_border && x > c -> resize_border 
				&& x < width - c -> resize_border){
		cur = c -> bs;
	}else{
		cur = NULL;
	}
	if(cur != nc && ! QQ_WINDOW(widget) -> is_maxsize){
		gdk_window_set_cursor(widget -> window, cur);
	}

	return FALSE;
}

gboolean delete_event_cb(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

/*
 * Remove the shape mask
 */
void qq_window_remove_shape_mask(QQWindow *qw)
{
	gtk_widget_shape_combine_mask(GTK_WIDGET(qw), NULL, 0, 0);
	gtk_widget_input_shape_combine_mask(GTK_WIDGET(qw), NULL, 0, 0);
}

/*
 * Set the window shape mask
 */
void qq_window_set_shape_mask(QQWindow *qwin)
{
	g_return_if_fail(qwin != NULL);

	QQWindowClass *c = QQ_WINDOW_GET_CLASS(qwin);
	gint width, height;
	gtk_window_get_size(GTK_WINDOW(qwin), &width, &height);

	if(qwin -> pre_w == width 
			&& qwin -> pre_h == height){
		/*
		 * The shape of the window does change.
		 * We just do nothing. 
		 */
		return;
	}

	GdkBitmap 	*bm = NULL;
	gdouble 	radius = 10;

	bm = (GdkBitmap*)gdk_pixmap_new(NULL, width, height, 1);	
	/*
	cairo_t *ct = gdk_cairo_create(bm);
	
	cairo_set_source_rgb(ct, 0, 0, 0);
	cairo_rectangle(ct, 0, 0, width, height);
	cairo_fill(ct);

	cairo_set_source_rgb(ct, 255, 255, 255);
	cairo_arc(ct, radius, radius, radius, 0, M_PI * 2);
	cairo_arc(ct, width - radius, radius, radius, 0, M_PI * 2);
	cairo_arc(ct, radius, height - radius, radius, 0, M_PI * 2);
	cairo_arc(ct, width - radius, height - radius, radius, 0, M_PI * 2);
	cairo_rectangle(ct, radius, 0, width - radius * 2, height);
	cairo_rectangle(ct, 0, radius, width, height - radius * 2);
	cairo_fill(ct);
	cairo_destroy(ct);
	*/
	GdkGC *gc = gdk_gc_new(bm);

	gdk_gc_set_foreground(gc, & c -> bg);
	gdk_gc_set_background(gc, & c -> fg);
	gdk_draw_rectangle(bm, gc, TRUE, 0, 0, width, height);

	gdk_gc_set_foreground(gc, & c -> fg);
	gdk_gc_set_background(gc, & c -> bg);
	gdk_draw_arc(bm, gc, TRUE, 0, 0, radius * 2, radius * 2, 0, 360 * 64);
	gdk_draw_arc(bm, gc, TRUE, width - radius * 2, 0, radius * 2
				, radius * 2, 0, 360 * 64);
	gdk_draw_arc(bm, gc, TRUE, 0, height - radius * 2, radius * 2
				, radius * 2, 0, 360 * 64);
	gdk_draw_arc(bm, gc, TRUE, width - radius * 2, height - radius * 2
				, radius * 2, radius * 2, 0, 360 * 64);
	gdk_draw_rectangle(bm, gc, TRUE, radius, 0, width - radius * 2, height);
	gdk_draw_rectangle(bm, gc, TRUE, 0, radius, width, height - radius * 2);

	gtk_widget_shape_combine_mask(GTK_WIDGET(qwin), bm, 0, 0);
	gtk_widget_input_shape_combine_mask(GTK_WIDGET(qwin), bm, 0, 0);

	qwin -> pre_w = width;
	qwin -> pre_h = height;
}

gboolean expose_event_cb(GtkWidget *widget, GdkEventExpose *event
			, gpointer data)
{
	return FALSE;
}

/*
 * The window's size is changed.
 * Reset the shape mask.
 */
gboolean configure_event_cb(GtkWidget *widget, GdkEventConfigure *event
				, gpointer data)
{
	qq_window_set_shape_mask(QQ_WINDOW(widget));
	return FALSE;
}

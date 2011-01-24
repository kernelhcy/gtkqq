#include <gtk/gtk.h>

static gboolean delete_event_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

static gboolean button_press_cb(GtkWidget *w, GdkEventButton *event
		, gpointer data){
	
	g_print("button press.\n");
	gint width,height;
	gtk_window_get_size(GTK_WINDOW(w), &width, &height);
	gint x,y;
	x = (gint)event -> x;
	y = (gint)event -> y;
	if(x > width - 10 && y > height - 10){
		gtk_window_begin_resize_drag(GTK_WINDOW(w), GDK_WINDOW_EDGE_SOUTH_EAST
				, 1, (gint)event -> x_root, (gint)event -> y_root
				, 0);
	}else{
		gtk_window_begin_move_drag(GTK_WINDOW(w), 1, (gint)event -> x_root
					, (gint)event -> y_root
					, 0);

	}
	return FALSE;
}

int main(int argc, char **argv)
{

	gtk_init(&argc, &argv);
	
	GtkWidget *window;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "test");
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 600);

	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event_cb)
			, NULL);
	g_signal_connect(window, "button-press-event", G_CALLBACK(button_press_cb)
			, NULL);

	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_widget_show(window);

	//Must be called _after_ the widget is realized
	gdk_window_set_events(window -> window, GDK_ALL_EVENTS_MASK);
	gtk_main();
	return 0;
}

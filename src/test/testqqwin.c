#include <gtk/gtk.h>
#include <qqwindow.h>

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	
	GtkWidget *qwin = qq_window_new();
	gtk_widget_show(qwin);

	gtk_main();
	return 0;
}

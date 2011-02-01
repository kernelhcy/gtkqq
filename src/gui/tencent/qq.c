#include <gtk/gtk.h>
#include <qqwindow.h>
#include <qqbutton.h>

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	
	GtkWidget *qwin = qq_window_new();

	QQButton *btn1 = qq_button_new(QQ_BUTTON_CLOSE);
	QQButton *btn2 = qq_button_new(QQ_BUTTON_MIN);
	QQButton *btn3 = qq_button_new(QQ_BUTTON_MAX);
	QQButton *btn4 = qq_button_new(QQ_BUTTON_RESTORE);
	gtk_widget_show(GTK_WIDGET(btn1));
	gtk_widget_show(GTK_WIDGET(btn2));
	gtk_widget_show(GTK_WIDGET(btn3));
	gtk_widget_show(GTK_WIDGET(btn4));

	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(btn4), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(btn2), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(btn3), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(btn1), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hbox), FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(qwin), vbox);
	gtk_widget_show_all(qwin);
	gtk_main();
	return 0;
}

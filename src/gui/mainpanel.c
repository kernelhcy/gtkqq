#include <mainpanel.h>

static void qq_mainpanel_init(QQMainPanel *panel);
static void qq_mainpanelclass_init(QQMainPanelClass *c);
static void qq_mainpanel_destory(QQMainPanel *panel);

GType qq_mainpanel_get_type()
{
	static GType t = 0;
	if(!t){
		static const GTypeInfo info =
			{
				sizeof(QQMainPanelClass),
				NULL,
				NULL,
				(GClassInitFunc)qq_mainpanelclass_init,
				NULL,
				NULL,
				sizeof(QQMainPanel),
				0,
				(GInstanceInitFunc)qq_mainpanel_init,
				NULL
			};
		t = g_type_register_static(GTK_TYPE_VBOX, "QQMainPanel"
						, &info, 0);
	}
	return t;
}
GtkWidget* qq_mainpanel_new(GtkWidget *container)
{
	QQMainPanel *panel = g_object_new(qq_mainpanel_get_type(), NULL);
	panel -> container = container;

	panel -> faceimg = gtk_image_new();
	panel -> nick = gtk_label_new("kernel");

	panel -> longnick = gtk_label_new("不抛弃，不放弃。");
	panel -> longnick_entry = gtk_entry_new();	

	return GTK_WIDGET(panel);
}

static void qq_mainpanel_init(QQMainPanel *panel)
{

}
static void qq_mainpanelclass_init(QQMainPanelClass *c)
{

}
static void qq_mainpanel_destory(QQMainPanel *panel)
{

}

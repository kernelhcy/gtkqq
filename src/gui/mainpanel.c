#include <mainpanel.h>
#include <statusbutton.h>
#include <qq.h>

extern QQInfo *info;

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

	return GTK_WIDGET(panel);
}

static void qq_mainpanel_init(QQMainPanel *panel)
{
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	GtkWidget *box = NULL;
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

	panel -> faceimgframe = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), panel -> faceimgframe
				, FALSE, FALSE, 15);

	panel -> status_btn = qq_statusbutton_new();
	panel -> nick = gtk_label_new("kernel");
	box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), panel -> status_btn, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), panel -> nick, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), box, FALSE, FALSE, 0);

	panel -> longnick = gtk_label_new("不抛弃，不放弃。");
	panel -> longnick_entry = gtk_entry_new();	
	gtk_box_pack_start(GTK_BOX(vbox), panel -> longnick
				, FALSE, FALSE, 5);

	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(panel), hbox, FALSE, FALSE, 15);

}
static void qq_mainpanelclass_init(QQMainPanelClass *c)
{

}
static void qq_mainpanel_destory(QQMainPanel *panel)
{

}

void qq_mainpanel_update(QQMainPanel *panel)
{
	GtkBin *faceimgframe = GTK_BIN(panel -> faceimgframe);

	//free the old image.
	GtkWidget *faceimg = gtk_bin_get_child(faceimgframe);
	if(faceimg != NULL){
		gtk_container_remove(GTK_CONTAINER(faceimgframe)
						, faceimg);
	}

	GdkPixbuf *pb = gdk_pixbuf_new_from_file_at_scale(
					info -> me -> faceimgfile -> str
					, 48, 48, TRUE, NULL);
	if(pb != NULL){
		GtkWidget *img = gtk_image_new_from_pixbuf(pb);
		//we MUST show it!
		gtk_widget_show(img);
		gtk_container_add(GTK_CONTAINER(faceimgframe), img);
	}else{
		g_warning("Create face image for %s error!(%s, %d)"
				, info -> me -> faceimgfile -> str
				, __FILE__, __LINE__ );
	}
}

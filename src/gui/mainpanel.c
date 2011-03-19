#include <mainpanel.h>
#include <statusbutton.h>
#include <qq.h>

extern QQInfo *info;
static QQMainPanelClass *this_class = NULL;

static void qq_mainpanel_init(QQMainPanel *panel);
static void qq_mainpanelclass_init(QQMainPanelClass *c);
static void qq_mainpanel_destory(QQMainPanel *panel);
static gboolean lnick_release_cb(GtkWidget *w, GdkEvent *event, gpointer data);
static gboolean lnick_focus_out_cb(GtkWidget *w, GdkEvent *event, gpointer data);

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
	panel -> nick = gtk_label_new("");
	box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), panel -> status_btn, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), panel -> nick, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), box, FALSE, FALSE, 2);

	panel -> longnick = gtk_label_new("");
	panel -> longnick_entry = gtk_entry_new();	
	gtk_widget_set_size_request(GTK_WIDGET(panel -> longnick_entry), 0, 0);
	panel -> longnick_box = gtk_hbox_new(FALSE, 0);
	panel -> longnick_eventbox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(panel -> longnick_eventbox)
						, panel -> longnick);
	gtk_box_pack_start(GTK_BOX(panel -> longnick_box)
							, panel -> longnick_eventbox
							, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(panel -> longnick_box)
							, panel -> longnick_entry
							, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), panel -> longnick_box, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(panel), hbox, FALSE, FALSE, 10);

	g_signal_connect(GTK_WIDGET(panel -> longnick_eventbox)
						, "button-release-event"
						, G_CALLBACK(lnick_release_cb), panel);
	g_signal_connect(GTK_WIDGET(panel -> longnick_entry)
						, "focus-out-event"
						, G_CALLBACK(lnick_focus_out_cb), panel);

	panel -> contact_btn = gtk_toggle_button_new();
	gtk_button_set_image(GTK_BUTTON(panel -> contact_btn)
						, this_class -> contact_img[0]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(panel -> contact_btn)
								, TRUE);

	panel -> grp_btn = gtk_toggle_button_new();
	gtk_button_set_image(GTK_BUTTON(panel -> grp_btn)
						, this_class -> grp_img[1]);
	
	panel -> recent_btn = gtk_toggle_button_new();
	gtk_button_set_image(GTK_BUTTON(panel -> recent_btn)
						, this_class -> recent_img[1]);
	hbox = gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), panel -> contact_btn, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), panel -> grp_btn, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), panel -> recent_btn, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(panel), hbox, FALSE, FALSE, 0);

}
static void qq_mainpanelclass_init(QQMainPanelClass *c)
{
	this_class = c;
	GdkPixbuf * pb;
	pb= gdk_pixbuf_new_from_file(IMGDIR"ContactMainTabButton1.png", NULL);
	c -> contact_img[0] = gtk_image_new_from_pixbuf(pb);
	pb = gdk_pixbuf_new_from_file(IMGDIR"ContactMainTabButton2.png", NULL);
	c -> contact_img[1] = gtk_image_new_from_pixbuf(pb);
	pb = gdk_pixbuf_new_from_file(IMGDIR"GroupMainTabButton1.png", NULL);
	c -> grp_img[0] = gtk_image_new_from_pixbuf(pb);
	pb = gdk_pixbuf_new_from_file(IMGDIR"GroupMainTabButton2.png", NULL);
	c -> grp_img[1] = gtk_image_new_from_pixbuf(pb);
	pb = gdk_pixbuf_new_from_file(IMGDIR"RecentMainTabButton1.png", NULL);
	c -> recent_img[0] = gtk_image_new_from_pixbuf(pb);
	pb = gdk_pixbuf_new_from_file(IMGDIR"RecentMainTabButton2.png", NULL);
	c -> recent_img[1] = gtk_image_new_from_pixbuf(pb);
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

	GString *nick, *lnick;
	nick = g_string_new(info -> me -> nick -> str);
	lnick = g_string_new(info -> me -> lnick -> str);
	gtk_label_set_text(GTK_LABEL(panel -> nick), nick -> str);
	gtk_label_set_text(GTK_LABEL(panel -> longnick), lnick -> str);
}

/*
 * click the long nick label, replace it with the gtkentry.
 */
static gboolean lnick_release_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
	QQMainPanel *panel = QQ_MAINPANEL(data);

	gtk_widget_hide(GTK_WIDGET(panel -> longnick_eventbox));
	gtk_widget_show(GTK_WIDGET(panel -> longnick_entry));

	gtk_entry_set_text(GTK_ENTRY(panel -> longnick_entry), 
						gtk_label_get_text(GTK_LABEL(panel -> longnick)));
	gtk_widget_show(GTK_WIDGET(panel -> longnick_entry));
	gtk_widget_set_size_request(GTK_WIDGET(panel -> longnick_entry)
								, -1, 22);
	gtk_widget_grab_focus(GTK_WIDGET(panel -> longnick_entry));
	return FALSE;
}

/*
 * longnick entry lose focus, replace it with label
 */
static gboolean lnick_focus_out_cb(GtkWidget *w, GdkEvent *event
										, gpointer data)
{
	QQMainPanel *panel = QQ_MAINPANEL(data);
	gtk_widget_show(GTK_WIDGET(panel -> longnick_eventbox));
	gtk_widget_hide(GTK_WIDGET(panel -> longnick_entry));

	return FALSE;
}

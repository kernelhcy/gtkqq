#include <loginpanel.h>

static void qq_loginpanelclass_init(QQLoginPanelClass *c);
static void qq_loginpanel_init(QQLoginPanel *obj);
static void qq_loginpanel_destroy(GtkObject *obj);

GtkType qq_loginpanel_get_type()
{
	static GType t = 0;
	if(!t){
		static const GTypeInfo info =
			{
				sizeof(QQLoginPanelClass),
				NULL,
				NULL,
				(GClassInitFunc)qq_loginpanelclass_init,
				NULL,
				NULL,
				sizeof(QQLoginPanel),
				0,
				(GInstanceInitFunc)qq_loginpanel_init,
				NULL
			};
		t = g_type_register_static(GTK_TYPE_VBOX, "QQLoginPanel"
						, &info, 0);
	}
	return t;
}

GtkWidget* qq_loginpanel_new()
{
	QQLoginPanel *panel = g_object_new(qq_loginpanel_get_type(), NULL);

	return GTK_WIDGET(panel);
}

static void qq_loginpanelclass_init(QQLoginPanelClass *c)
{
	GtkObjectClass *object_class = NULL;

	object_class = GTK_OBJECT_CLASS(c);
	object_class -> destroy = qq_loginpanel_destroy;

}

static void qq_loginpanel_init(QQLoginPanel *obj)
{
	obj -> uin_label = gtk_label_new("账号：");
	obj -> uin_entry = gtk_entry_new();

	obj -> passwd_label = gtk_label_new("密码：");
	obj -> passwd_entry = gtk_entry_new();
	//not visibily 
	gtk_entry_set_visibility(GTK_ENTRY(obj -> passwd_entry), FALSE);
	
	GtkWidget *vbox = gtk_vbox_new(FALSE, 10);

	GtkWidget *uin_hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(uin_hbox), obj -> uin_label, FALSE, FALSE, 0);
	GtkWidget *uin_vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(uin_vbox), uin_hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(uin_vbox), obj -> uin_entry, FALSE, FALSE, 0);

	GtkWidget *passwd_hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(passwd_hbox), obj -> passwd_label
					, FALSE, FALSE, 0);
	GtkWidget *passwd_vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(passwd_vbox), passwd_hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(passwd_vbox), obj -> passwd_entry, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), uin_vbox, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), passwd_vbox, FALSE, FALSE, 2);

	obj -> login_btn = gtk_button_new_with_label("登录");

	GtkWidget *hbox1 = gtk_hbox_new(FALSE, 0);
	GtkWidget *hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox1), vbox, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox2), obj -> login_btn, TRUE, FALSE, 0);

	gtk_box_set_homogeneous(GTK_BOX(obj), FALSE);
	gtk_box_pack_start(GTK_BOX(obj), hbox1, TRUE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(obj), hbox2, TRUE, FALSE, 5);
}

/*
 * Destroy the instance of QQLoginPanel
 */
static void qq_loginpanel_destroy(GtkObject *obj)
{
	/*
	 * Child widgets will be destroied by their parents.
	 * So, we should not try to unref the Child widgets here.
	 */
}


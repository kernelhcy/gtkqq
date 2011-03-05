#include <loginpanel.h>
#include <mainwindow.h>
#include <qq.h>

static void qq_loginpanelclass_init(QQLoginPanelClass *c);
static void qq_loginpanel_init(QQLoginPanel *obj);
static void qq_loginpanel_destroy(GtkObject *obj);

/*
 * The main event loop context of Gtk.
 */
static GMainContext *gtkctx;

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

GtkWidget* qq_loginpanel_new(GtkWidget *container)
{
	QQLoginPanel *panel = g_object_new(qq_loginpanel_get_type(), NULL);
	panel -> container = container;

	return GTK_WIDGET(panel);
}

static void qq_loginpanelclass_init(QQLoginPanelClass *c)
{
	GtkObjectClass *object_class = NULL;

	object_class = GTK_OBJECT_CLASS(c);
	object_class -> destroy = qq_loginpanel_destroy;

	/*
	 * get the default main evet loop context.
	 * 
	 * I think this will work...
	 */
	gtkctx = g_main_context_default();
}

/*
 * The global value
 */
extern QQInfo *info;

/*
 * After login, this function is attached to the gtk main event loop to
 * manage the widgets' status.
 */
static gboolean login_idle(gpointer data)
{

	return FALSE;
}


/*
 * Callback of qq_login function.
 * NOTE:
 * 	This function runs in the different main event loop.
 * 	So, we can NOT change the status of any widgets.
 */
static void login_cb(CallBackResult cbr, gpointer redata, gpointer usrdata)
{
	if(cbr == CB_SUCCESS){
		g_printf("login success!");
		GSource *src = g_idle_source_new();
		g_source_set_callback(src, (GSourceFunc)login_idle
				, usrdata, NULL);

	}
}

/*
 * Callback of login_btn button
 */
static void login_btn_cb(GtkButton *btn, gpointer data)
{
	GtkWidget *panel = GTK_WIDGET(data);
	const gchar *uin, *passwd, *status;
	
	uin = qq_loginpanel_get_uin(panel);
	passwd = qq_loginpanel_get_passwd(panel);
	status = qq_loginpanel_get_status(panel);
	
	qq_login(info, uin, passwd, status, login_cb, data);
}

static void qq_loginpanel_init(QQLoginPanel *obj)
{
	obj -> uin_label = gtk_label_new("账号：");
	obj -> uin_entry = gtk_combo_box_entry_new();

	obj -> passwd_label = gtk_label_new("密码：");
	obj -> passwd_entry = gtk_entry_new();
	//not visibily 
	gtk_entry_set_visibility(GTK_ENTRY(obj -> passwd_entry), FALSE);
	gtk_widget_set_size_request(GTK_WIDGET(obj -> uin_entry), 220, 30);
	gtk_widget_set_size_request(GTK_WIDGET(obj -> passwd_entry), 220, 30);
	
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
	g_signal_connect(G_OBJECT(obj -> login_btn), "clicked"
			, G_CALLBACK(login_btn_cb), (gpointer)obj);

	GtkWidget *hbox1 = gtk_hbox_new(FALSE, 0);
	GtkWidget *hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox1), vbox, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox2), obj -> login_btn, TRUE, FALSE, 0);

	gtk_box_set_homogeneous(GTK_BOX(obj), FALSE);

	GtkWidget *logo = gtk_image_new_from_file(IMGDIR"webqq_icon.png");
	
	gtk_box_pack_start(GTK_BOX(obj), logo, TRUE, FALSE, 5);
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

const gchar* qq_loginpanel_get_uin(GtkWidget *loginpanel)
{
	if(!QQ_IS_LOGINPANEL(loginpanel)){
		return NULL;
	}

	QQLoginPanel *panel = QQ_LOGINPANEL(loginpanel);
	return gtk_combo_box_get_active_text(
				GTK_COMBO_BOX(panel -> uin_entry));	

}
const gchar* qq_loginpanel_get_passwd(GtkWidget *loginpanel)
{
	if(!QQ_IS_LOGINPANEL(loginpanel)){
		return NULL;
	}
	QQLoginPanel *panel = QQ_LOGINPANEL(loginpanel);
	return gtk_entry_get_text(
				GTK_ENTRY(panel -> passwd_entry));
}
const gchar* qq_loginpanel_get_status(GtkWidget *loginpanel)
{
	if(!QQ_IS_LOGINPANEL(loginpanel)){
		return NULL;
	}
	QQLoginPanel *panel = QQ_LOGINPANEL(loginpanel);
	return "hidden";
}

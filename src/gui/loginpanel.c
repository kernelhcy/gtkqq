#include <loginpanel.h>
#include <mainpanel.h>
#include <mainwindow.h>
#include <qq.h>
#include <consts.h>
#include <config.h>
#include <stdlib.h>
#include <statusbutton.h>

/*
 * The global value
 */
extern QQInfo *info;
extern QQConfig *cfg;

/*
 * This is the parameter struct of the XXX_idle function.
 */
typedef struct{
	CallBackResult cbr;
	gpointer redata;
	gpointer usrdata;
}IdlePar;

static void qq_loginpanelclass_init(QQLoginPanelClass *c);
static void qq_loginpanel_init(QQLoginPanel *obj);
static void qq_loginpanel_destroy(GtkObject *obj);
static void login_cb(CallBackResult cbr, gpointer redata, gpointer usrdata);

//
//At the end of login.
//
static void done_login()
{
	gchar icondir[200];
	g_sprintf(icondir, CONFIGDIR"%s/icons/", cfg -> me -> uin -> str);

	if(!g_file_test(icondir, G_FILE_TEST_EXISTS)){
		if(-1 == g_mkdir(icondir, 0777)){
			g_warning("create icons dir %s error!(%s, %s)"
					, icondir, __FILE__, __LINE__);
			exit(1);
		}
	}

	//save the face image to file
	qq_save_face_img(info -> me, icondir);
}
//
// Just a test.
//
//
static void callback(CallBackResult re, gpointer redata
				, gpointer usrdata)
{
	g_printf("Call back: %s\n", (gchar *)redata);
	return;
}
//
// A state mechine
// Used to login.
//
// check verify code -> login -> get my info -> get friends
// 		-> get group list name mask -> done.
//
static void qq_loginpanel_login_sm(gpointer data)
{

	IdlePar *par = (IdlePar*)data;
	CallBackResult cbr = par -> cbr;
	gpointer redata = par -> redata;
	gpointer usrdata = par -> usrdata;
	g_slice_free(IdlePar, par);

	GtkWidget *w;
	QQLoginPanel *p = QQ_LOGINPANEL(usrdata);
	w = p -> container;
	GtkWidget *dialog, *vc_entry, *img;
	GtkWidget *vbox, *hbox;
	gchar fn[200];
	switch(p -> login_state)
	{
	case LS_CHECK_VC:
		p -> login_state = LS_LOGIN;
		qq_check_verifycode(info, p -> uin, login_cb, usrdata);
		if(cbr == CB_ERROR){
			//try again.
			g_warning("Check verify code faile. Try again."
					"(%s, %d)", __FILE__, __LINE__);
			p -> login_state = LS_CHECK_VC;
			break;
		}
		break;
	case LS_LOGIN:
		if(info -> need_vcimage){
			if(info -> vc_image_data == NULL
					|| info -> vc_image_type == NULL){
				g_warning("No vc image data or type!(%s, %d)"
						, __FILE__, __LINE__);
				gtk_label_set_text(GTK_LABEL(p -> err_label)
						, "登录失败，请重新登录。");
				qq_mainwindow_show_loginpanel(w);
				break;
			}
			sprintf(fn, CONFIGDIR"verifycode.%s"
					, info -> vc_image_type -> str);
			save_img_to_file(info -> vc_image_data -> str
					, info -> vc_image_data -> len
					, info -> vc_image_type -> str
					, CONFIGDIR, "verifycode");
			dialog = gtk_dialog_new_with_buttons("Information"
					, GTK_WINDOW(w), GTK_DIALOG_MODAL
					, GTK_STOCK_OK, GTK_RESPONSE_OK
					, NULL);
			vbox = GTK_DIALOG(dialog) -> vbox;
			img = gtk_image_new_from_file(fn);
			gtk_box_pack_start(GTK_BOX(vbox)
					, gtk_label_new("请输入验证码：")
					, FALSE, FALSE, 20);	
			gtk_box_pack_start(GTK_BOX(vbox), img
					, FALSE, FALSE, 0);	

			vc_entry = gtk_entry_new();
			gtk_widget_set_size_request(vc_entry, 200, -1);
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), vc_entry
					, TRUE, FALSE, 0);
			gtk_box_pack_start(GTK_BOX(vbox), hbox
					, FALSE, FALSE, 10);	

			gtk_widget_set_size_request(dialog, 300, 220);
			gtk_widget_show_all(dialog);
		       	gtk_dialog_run(GTK_DIALOG(dialog));
			//got the verify code
			info -> verify_code = g_string_new(
					gtk_entry_get_text(
						GTK_ENTRY(vc_entry)));
	       		gtk_widget_destroy(dialog);
		}
		p -> login_state = LS_GET_MY_INFO;
		qq_login(info , p -> uin, p -> passwd, p -> status
				, login_cb, usrdata);
		break;
	case LS_GET_MY_INFO:
		switch(cbr)
		{
		case CB_SUCCESS:
			//go on and get my information.
			p -> login_state = LS_GET_FRIENDS;
			//
			// Create the user configure.
			//
			cfg -> me = info -> me;
			qq_get_my_info(info, login_cb, usrdata);
			break;
		case CB_WRONGPASSWD:
			//wrong passwd.
			gtk_label_set_text(GTK_LABEL(p -> err_label)
					, "密码错误！");
			qq_mainwindow_show_loginpanel(w);
			break;
		case CB_WRONGVC:
			//wrong verify code.
			//Get a new verify code and try again.
			p -> login_state = LS_CHECK_VC;
			break;
		case CB_ERROR:
		default:
			gtk_label_set_text(GTK_LABEL(p -> err_label)
					, "登录失败，请重新登录。");
			qq_mainwindow_show_loginpanel(w);
			break;
		}
		break;
	case LS_GET_FRIENDS:
		p -> login_state = LS_GET_GROUP_LIST;
		qq_get_my_friends(info, login_cb, usrdata);
		break;
	case LS_GET_GROUP_LIST:
		p -> login_state = LS_ONLINE;
		qq_get_group_name_list_mask(info, login_cb, usrdata);
		break;
	case LS_ONLINE:
		p -> login_state = LS_RECENT;
		qq_get_online_buddies(info, login_cb, usrdata);
		break;
	case LS_RECENT:
		p -> login_state = LS_SLNICK;
		qq_get_recent_contact(info, login_cb, usrdata);
		break;
	case LS_SLNICK:
		p -> login_state = LS_GET_FACEIMG;
		qq_get_single_long_nick(info, info -> me, login_cb, usrdata);
		break;
	case LS_GET_FACEIMG:
		p -> login_state = LS_DONE;
		qq_get_face_img(info, info -> me -> uin -> str
					, login_cb, usrdata);
		break;
	case LS_DONE:
		//save the face image
		info -> me -> faceimg = (QQFaceImg*)redata;
		done_login();
		//update the mainpanel
		qq_mainpanel_update(QQ_MAINPANEL(QQ_MAINWINDOW(w) 
						-> main_panel));
		g_debug("Login done! Go to main panel.(%s, %d)"
				, __FILE__, __LINE__);
		qq_mainwindow_show_mainpanel(w);
		g_debug("Start poll........(%s, %d)", __FILE__, __LINE__);
		qq_start_poll(info, callback, NULL);
		break;
	case LS_ERROR:
	case LS_UNKNOWN:
	default:
		gtk_label_set_text(GTK_LABEL(p -> err_label)
				, "登录失败，请重新登录。");
		qq_mainwindow_show_loginpanel(w);
		break;
	}
}

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
 * This function is run in the gtk main event loop.
 */
static gboolean gtk_idle(gpointer data)
{
	qq_loginpanel_login_sm(data);
	return FALSE;
	
}

/*
 * Callback of login function.
 * NOTE:
 * 	This function runs in the different main event loop.
 * 	So, we can NOT change the status of any widgets.
 */
static void login_cb(CallBackResult cbr, gpointer redata, gpointer usrdata)
{
	IdlePar *par = g_slice_new0(IdlePar);
	par -> cbr = cbr;
	par -> redata = redata;
	par -> usrdata = usrdata;
	GSource *src = g_idle_source_new();
	g_source_set_callback(src, (GSourceFunc)gtk_idle, par, NULL);
	g_source_attach(src, gtkctx);
	g_source_unref(src);
}

/*
 * Callback of login_btn button
 */
static void login_btn_cb(GtkButton *btn, gpointer data)
{
	QQLoginPanel *panel = QQ_LOGINPANEL(data);
	GtkWidget *win = QQ_LOGINPANEL(panel) -> container;
	qq_mainwindow_show_splashpanel(win);

	panel -> uin = qq_loginpanel_get_uin(panel);
	panel -> passwd = qq_loginpanel_get_passwd(panel);
	panel -> status = qq_loginpanel_get_status(panel);
	//create the user config
	//qq_userconfig_new(cfg, panel -> uin);

	//start the login state mechine
	panel -> login_state = LS_CHECK_VC;
	IdlePar *par = g_slice_new0(IdlePar);
	par -> cbr = CB_SUCCESS;
	par -> redata = NULL;
	par -> usrdata = data;
	qq_loginpanel_login_sm(par);

	//clear the error message.
	gtk_label_set_text(GTK_LABEL(panel -> err_label), "");
}

static void qq_loginpanel_init(QQLoginPanel *obj)
{
	obj -> uin_label = gtk_label_new("账  号：");
	obj -> uin_entry = gtk_combo_box_entry_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(obj -> uin_entry)
			, "1421032531");

	obj -> passwd_label = gtk_label_new("密  码：");
	obj -> passwd_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(obj -> passwd_entry), "1234567890");
	//not visibily 
	gtk_entry_set_visibility(GTK_ENTRY(obj -> passwd_entry), FALSE);
	gtk_widget_set_size_request(obj -> uin_entry, 200, -1);
	gtk_widget_set_size_request(obj -> passwd_entry, 220, -1);
	
	GtkWidget *vbox = gtk_vbox_new(FALSE, 10);

	//uin label and entry
	GtkWidget *uin_hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(uin_hbox), obj -> uin_label, FALSE, FALSE, 0);
	GtkWidget *uin_vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(uin_vbox), uin_hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(uin_vbox), obj -> uin_entry, FALSE, FALSE, 0);

	//password label and entry
	GtkWidget *passwd_hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(passwd_hbox), obj -> passwd_label
					, FALSE, FALSE, 0);
	GtkWidget *passwd_vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(passwd_vbox), passwd_hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(passwd_vbox), obj -> passwd_entry, FALSE, FALSE, 0);
	
	//put uin and password in a vbox
	gtk_box_pack_start(GTK_BOX(vbox), uin_vbox, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), passwd_vbox, FALSE, FALSE, 2);

	//rember password check box
	obj -> rempwcb = gtk_check_button_new_with_label("记住密码");
	GtkWidget *hbox4 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox4), obj -> rempwcb, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox4, FALSE, TRUE, 2);

	//login button
	obj -> login_btn = gtk_button_new_with_label("登  录");
	gtk_widget_set_size_request(obj -> login_btn, 90, -1);
	g_signal_connect(G_OBJECT(obj -> login_btn), "clicked"
			, G_CALLBACK(login_btn_cb), (gpointer)obj);

	//status combo box
	obj -> status_comb = qq_statusbutton_new();
	gtk_combo_box_set_active(GTK_COMBO_BOX(obj -> status_comb), 0);

	GtkWidget *hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox1), vbox, TRUE, FALSE, 0);

	GtkWidget *hbox2 = gtk_hbox_new(FALSE, 0);
	GtkWidget *hbox3 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox2), obj -> status_comb, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox2), obj -> login_btn, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox3), hbox2, TRUE, FALSE, 0);

	//error informatin label
	obj -> err_label = gtk_label_new("");
	GdkColor color;
	GdkColormap *cmap = gdk_colormap_get_system();
	gdk_colormap_alloc_color(cmap, &color, TRUE, TRUE);
	gdk_color_parse("#fff000000", &color);	//red
	//change text color to red
	//MUST modify fb, not text
	gtk_widget_modify_fg(obj -> err_label, GTK_STATE_NORMAL, &color);
	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox2), obj -> err_label, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, TRUE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox3, FALSE, FALSE, 0);

	gtk_box_set_homogeneous(GTK_BOX(obj), FALSE);
	GtkWidget *logo = gtk_image_new_from_file(IMGDIR"webqq_icon.png");
	gtk_widget_set_size_request(logo, -1, 150);	
	gtk_box_pack_start(GTK_BOX(obj), logo, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(obj), hbox1, FALSE, FALSE, 15);
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

const gchar* qq_loginpanel_get_uin(QQLoginPanel *loginpanel)
{
	QQLoginPanel *panel = QQ_LOGINPANEL(loginpanel);
	return gtk_combo_box_get_active_text(
				GTK_COMBO_BOX(panel -> uin_entry));	

}
const gchar* qq_loginpanel_get_passwd(QQLoginPanel *loginpanel)
{
	QQLoginPanel *panel = QQ_LOGINPANEL(loginpanel);
	return gtk_entry_get_text(
				GTK_ENTRY(panel -> passwd_entry));
}
const gchar* qq_loginpanel_get_status(QQLoginPanel *loginpanel)
{
	return qq_statusbutton_get_status_string(QQ_STATUSBUTTON(
						loginpanel -> status_comb));
}


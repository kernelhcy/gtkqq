#include <loginpanel.h>
#include <mainpanel.h>
#include <mainwindow.h>
#include <qq.h>
#include <gqqconfig.h>
#include <stdlib.h>
#include <statusbutton.h>
#include <msgloop.h>

/*
 * The global value
 * in main.c
 */
extern QQInfo *info;
extern GQQConfig *cfg;

extern GQQMessageLoop *get_info_loop;
extern GQQMessageLoop *send_loop;

static void qq_loginpanelclass_init(QQLoginPanelClass *c);
static void qq_loginpanel_init(QQLoginPanel *obj);
static void qq_loginpanel_destroy(GtkObject *obj);

static void qqnumber_combox_changed(GtkComboBox *widget, gpointer data);
static void update_face_image(QQInfo *info, QQMainPanel *panel);
/*
 * The main event loop context of Gtk.
 */
static GQQMessageLoop gtkloop;

static GPtrArray* login_users = NULL;

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
    gtkloop.ctx = g_main_context_default();
    gtkloop.name = "LoginPanel Gtk";
}

//
// Run in message loop
//
static gint do_login(QQLoginPanel *panel)
{
    const gchar *uin = panel -> uin;
    const gchar *passwd = panel -> passwd;
    const gchar *status = panel -> status;

    GError *err = NULL;
    gint ret = qq_login(info, uin, passwd, status, &err);
    const gchar *msg;
    switch(ret)
    {
    case NO_ERR:
        // going on
        return 0;
    case NETWORK_ERR:
        msg = "Network error. Please try again.";
        break;
    case WRONGPWD_ERR:
        msg = "Wrong Password.";
        break;
    case WRONGUIN_ERR:
        msg = "Wrong QQ Number.";
        break;
    case WRONGVC_ERR:
        msg = "Wrong Verify Code.";
        break;
    default:
        msg = "Error. Please try again.";
        break;
    }
    g_warning("Login error! %s (%s, %d)", err -> message, __FILE__, __LINE__);
    g_error_free(err);
    //show err message
    gqq_mainloop_attach(&gtkloop, gtk_label_set_text, 2, panel -> err_label, msg);
    return -1;
}

//login state machine state.
enum{
    LOGIN_SM_CHECKVC,
    LOGIN_SM_LOGIN,
    LOGIN_SM_GET_DATA,
    LOGIN_SM_DONE,
    LOGIN_SM_ERR
};

static void read_verifycode(gpointer p);
static gint state = LOGIN_SM_ERR;
static void login_state_machine(gpointer data)
{
    QQLoginPanel *panel = (QQLoginPanel*)data;
    while(TRUE){
        switch(state)
        {
        case LOGIN_SM_CHECKVC:
            if(qq_check_verifycode(info, panel -> uin, NULL) != 0){
                state = LOGIN_SM_ERR;
                break;
            }
            state = LOGIN_SM_LOGIN;
            if(info -> need_vcimage){
                gqq_mainloop_attach(&gtkloop, read_verifycode, 1, panel);
                // Quit the state machine.
                // The state machine will restart in the read verify code
                // dialog.
                return;
            }
        case LOGIN_SM_LOGIN:
            if(do_login(panel) != 0){
                state = LOGIN_SM_ERR;
            }else{
                state = LOGIN_SM_GET_DATA;
            }
            break;
        case LOGIN_SM_GET_DATA:
            //Read cached data from db
            gqq_config_load(cfg, panel -> uin);
            qq_get_buddies_and_categories(info, NULL);
            qq_get_groups(info, NULL);
            state = LOGIN_SM_DONE;
            break;
        case LOGIN_SM_DONE:
            g_debug("Login done. show main panel!(%s, %d)", __FILE__, __LINE__);
            // update main panel
            gqq_mainloop_attach(&gtkloop, qq_mainpanel_update
                                    , 1, QQ_MAINWINDOW(panel -> container) 
                                                    -> main_panel);
            // show main panel
            gqq_mainloop_attach(&gtkloop, qq_mainwindow_show_mainpanel
                                    , 1, panel -> container);
            // update my information
            qq_get_buddy_info(info, info -> me, NULL);
            gqq_mainloop_attach(&gtkloop, qq_mainpanel_update
                                    , 1, QQ_MAINWINDOW(panel -> container) 
                                                    -> main_panel);
            update_face_image(info, (QQMainPanel*)QQ_MAINWINDOW(
                                                    panel -> container) 
                                                        -> main_panel);
            return;
        case LOGIN_SM_ERR:
            g_debug("Login error... (%s, %d)", __FILE__, __LINE__);
            gqq_mainloop_attach(&gtkloop, qq_mainwindow_show_loginpanel
                                    , 1, panel -> container);
            g_debug("Show login panel.(%s, %d)", __FILE__, __LINE__);
            return;
        default:
            break;
        }
    }
    return;
}

//
// The login state machine is run in the message loop.
//
static void run_login_state_machine(QQLoginPanel *panel)
{
    gqq_mainloop_attach(get_info_loop, login_state_machine, 1, panel); 
}

// In libqq/qqutils.c
extern gint save_img_to_file(const gchar *data, gint len, const gchar *ext, 
                const gchar *path, const gchar *fname);
//
// Show the verify code input dialog
// run in gtk main event loop.
//
static void read_verifycode(gpointer p)
{
    QQLoginPanel *panel = (QQLoginPanel*)p;
    GtkWidget *w = panel -> container;
    gchar fn[300];
    if(info -> vc_image_data == NULL || info -> vc_image_type == NULL){
        g_warning("No vc image data or type!(%s, %d)" , __FILE__, __LINE__);
        gtk_label_set_text(GTK_LABEL(panel -> err_label)
                                , "Login failed. Please retry.");
        qq_mainwindow_show_loginpanel(w);
        return;
    }
    sprintf(fn, CONFIGDIR"verifycode.%s", info -> vc_image_type -> str);
    save_img_to_file(info -> vc_image_data -> str
                        , info -> vc_image_data -> len
                        , info -> vc_image_type -> str
                        , CONFIGDIR, "verifycode");
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Information"
                            , GTK_WINDOW(w), GTK_DIALOG_MODAL
                            , GTK_STOCK_OK, GTK_RESPONSE_OK
                            , NULL);
    GtkWidget *vbox = GTK_DIALOG(dialog) -> vbox;
    GtkWidget *img = gtk_image_new_from_file(fn);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("VerifyCode：")
                            , FALSE, FALSE, 20);    
    gtk_box_pack_start(GTK_BOX(vbox), img, FALSE, FALSE, 0); 

    GtkWidget *vc_entry = gtk_entry_new();
    gtk_widget_set_size_request(vc_entry, 200, -1);
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vc_entry, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);    

    gtk_widget_set_size_request(dialog, 300, 220);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    
    //got the verify code
    info -> verify_code = g_string_new(
                gtk_entry_get_text(GTK_ENTRY(vc_entry)));
    gtk_widget_destroy(dialog); 
    
    //restart the state machine
    run_login_state_machine(panel);
}

/*
 * Callback of login_btn button
 */
static void login_btn_cb(GtkButton *btn, gpointer data)
{
    QQLoginPanel *panel = QQ_LOGINPANEL(data);
    GtkWidget *win = panel -> container;
    qq_mainwindow_show_splashpanel(win);

    panel -> uin = qq_loginpanel_get_uin(panel);
    panel -> passwd = qq_loginpanel_get_passwd(panel);
    panel -> status = qq_loginpanel_get_status(panel);

    // run the login state machine
    g_debug("Run login state machine...(%s, %d)", __FILE__, __LINE__);
    state = LOGIN_SM_CHECKVC;
    run_login_state_machine(panel);

    g_object_set(cfg, "qqnum", panel -> uin, NULL);
    g_object_set(cfg, "passwd", panel -> passwd, NULL);
    g_object_set(cfg, "status", panel -> status, NULL);

    qq_buddy_set(info -> me, "qqnumber", panel -> uin);
    qq_buddy_set(info -> me, "uin", panel -> uin);

    //clear the error message.
    gtk_label_set_text(GTK_LABEL(panel -> err_label), "");
    gqq_config_save_last_login_user(cfg);
}

static void qq_loginpanel_init(QQLoginPanel *obj)
{
    login_users = gqq_config_get_all_login_user(cfg);
    //Put the last login user at the first of the array
    gint i;
    GQQLoginUser *usr, *tmp;
    for(i = 0; i < login_users -> len; ++i){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, i);
        if(usr == NULL){
            continue;
        }
        if(usr -> last == 1){
            break;
        }
    }
    if(i < login_users -> len){
        tmp = login_users -> pdata[0];
        login_users -> pdata[0] = login_users -> pdata[i];
        login_users -> pdata[i] = tmp;
    }

    obj -> uin_label = gtk_label_new("QQ Number:");
    obj -> uin_entry = gtk_combo_box_entry_new_text();

    for(i = 0; i < login_users -> len; ++i){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, i);
        gtk_combo_box_append_text(GTK_COMBO_BOX(obj -> uin_entry)
                            , usr -> qqnumber);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(obj -> uin_entry), 0);

    obj -> passwd_label = gtk_label_new("Password:");
    obj -> passwd_entry = gtk_entry_new();
    if(login_users -> len > 0){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, 0);
        gtk_entry_set_text(GTK_ENTRY(obj -> passwd_entry), usr -> passwd);
    }
    g_signal_connect(G_OBJECT(obj -> uin_entry), "changed"
                        , G_CALLBACK(qqnumber_combox_changed), obj);
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
    obj -> rempwcb = gtk_check_button_new_with_label("Remeber Password");
    GtkWidget *hbox4 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox4), obj -> rempwcb, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox4, FALSE, TRUE, 2);

    //login button
    obj -> login_btn = gtk_button_new_with_label("Login");
    gtk_widget_set_size_request(obj -> login_btn, 90, -1);
    g_signal_connect(G_OBJECT(obj -> login_btn), "clicked"
                                , G_CALLBACK(login_btn_cb), (gpointer)obj);

    //status combo box
    obj -> status_comb = qq_statusbutton_new();
    if(login_users -> len > 0){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, 0);
        qq_statusbutton_set_status_string(obj -> status_comb, usr -> status);
    }

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
    gdk_color_parse("#fff000000", &color);    //red
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

static void qqnumber_combox_changed(GtkComboBox *widget, gpointer data)
{
    QQLoginPanel *obj = QQ_LOGINPANEL(data);
    gint idx = gtk_combo_box_get_active(widget);
    if(idx < 0 || idx >= login_users -> len){
        return;
    }
    GQQLoginUser *usr = (GQQLoginUser*)g_ptr_array_index(login_users, idx); 
    gtk_entry_set_text(GTK_ENTRY(obj -> passwd_entry), usr -> passwd);
    qq_statusbutton_set_status_string(obj -> status_comb, usr -> status);
    return;
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
    return qq_statusbutton_get_status_string(loginpanel -> status_comb);
}

typedef struct{
    QQInfo *info;
    GPtrArray *imgs;
    gint t_num;
    gint id;
}GetFaceThreadPar;

static gpointer get_face_thread_func(gpointer data)
{
    GetFaceThreadPar *par = data;
    GPtrArray *imgs = par -> imgs;
    gint t_num = par -> t_num;
    gint id = par -> id;
    QQInfo *info = par -> info;
    g_slice_free(GetFaceThreadPar, par);
    gint i;
    QQFaceImg *img;
    gchar path[500];

    for(i = id; i < imgs -> len; i += t_num){
        img = g_ptr_array_index(imgs, i);
        qq_get_face_img(info, img, NULL);
        g_snprintf(path, 500, CONFIGDIR"/faces/%s", img -> num -> str);
        qq_save_face_img(img, path, NULL);
    }
    return NULL;
}

//
// Get all the face images
// Run in get_info_loop.
//
static void update_face_image(QQInfo *info, QQMainPanel *panel)
{
    if(info == NULL || panel == NULL){
        return;
    }

    GPtrArray *fimgs = g_ptr_array_new();
    gint i, j;
    QQBuddy *bdy = NULL;
    QQGroup *grp = NULL;
    QQGMember *gmem = NULL;
    QQFaceImg *img = NULL;

    img = qq_faceimg_new(); 
    qq_faceimg_set(img, "uin", info -> me -> uin);
    qq_faceimg_set(img, "num", info -> me  -> qqnumber);
    g_ptr_array_add(fimgs, img);
    
    gchar qqnum [100];
    for(i = 0; i < info -> buddies -> len; ++i){
        bdy = g_ptr_array_index(info -> buddies, i);
        if(bdy == NULL){
            continue;
        }
        img = qq_faceimg_new(); 
        qq_faceimg_set(img, "uin", bdy -> uin);

        //Get qq number
        qq_get_qq_number(info, bdy -> uin -> str, qqnum, NULL);
        qq_buddy_set(bdy, "qqnumber", qqnum);
        qq_faceimg_set(img, "num", bdy -> qqnumber);

        g_ptr_array_add(fimgs, img);
    }

    for(i = 0; i < info -> groups -> len; ++i){
        grp = g_ptr_array_index(info -> groups, i);
        img = qq_faceimg_new();
        qq_faceimg_set(img, "uin", grp -> code);
        
        qq_get_qq_number(info, grp -> code -> str, qqnum, NULL);
        qq_group_set(grp, "gnumber", qqnum);
        qq_faceimg_set(img, "num", grp -> gnumber);

        g_ptr_array_add(fimgs, img);
        for(j = 0; j < grp -> members -> len; ++j){
            gmem = g_ptr_array_index(grp -> members, j);
            img = qq_faceimg_new();
            qq_faceimg_set(img, "uin", gmem -> uin);
            
            qq_get_qq_number(info, gmem -> uin -> str, qqnum, NULL);
            qq_gmember_set(gmem, "qqnumber", qqnum);
            qq_faceimg_set(img, "num", gmem -> qqnumber);

            g_ptr_array_add(fimgs, img);
        }
    }

    gint t_num = 10;
    GThread **threads = g_malloc(sizeof(GThread*) * t_num);
    GError *err = NULL;
    GetFaceThreadPar *par = NULL;
    for(i = 0; i < t_num; ++i){
        par = g_slice_new0(GetFaceThreadPar);
        par -> imgs = fimgs;
        par -> t_num = t_num;
        par -> id = i;
        par -> info = info;
        threads[i] = g_thread_create(get_face_thread_func, par, TRUE, &err);
        if(threads[i] == NULL){
            g_warning("Create thread to get face image error. %s (%s, %d)"
                                    , err -> message, __FILE__, __LINE__);
            g_error_free(err);
        }
    }
    for(i = 0; i < t_num; ++i){
        g_thread_join(threads[i]);
    }

    for(i = 0; i < fimgs -> len; ++i){
        img = g_ptr_array_index(fimgs, i);
        qq_faceimg_free(img);
    }
    g_free(threads);

    //update the panel
    gqq_mainloop_attach(&gtkloop, qq_mainpanel_update, 1, panel);
}


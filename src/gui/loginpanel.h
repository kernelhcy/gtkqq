#ifndef __GTKQQ_LOGINWIN_H
#define __GTKQQ_LOGINWIN_H
#include <gtk/gtk.h>

#define QQ_LOGINPANEL(obj)    G_TYPE_CHECK_INSTANCE_CAST(obj, qq_loginpanel_get_type()\
                                        , QQLoginPanel)
#define QQ_LOGINPANEL_CLASS(c)    G_TYPE_CHECK_CLASS_CAST(c\
                                        , qq_loginpanel_get_type()\
                                        , QQLoginPanelClass)
#define QQ_IS_LOGINPANEL(obj)    G_TYPE_CHECK_INSTANCE_TYPE(obj, qq_loginpanel_get_type())

typedef struct _QQLoginPanel            QQLoginPanel;
typedef struct _QQLoginPanelClass       QQLoginPanelClass;
typedef enum _QQLoginPanelLoginState    QQLoginPanelLoginState;

//the login status
enum _QQLoginPanelLoginState{
    LS_CHECK_VC,        //check verify code
    LS_LOGIN,           //login, get psessionid
    LS_GET_MY_INFO,     //get information of myself
    LS_GET_FRIENDS,     //get my friends list.
    LS_GET_GROUP_LIST,  //get group list.
    LS_ONLINE,          //get online buddies
    LS_RECENT,          //get recent connected buddies
    LS_SLNICK,          //get single long nick
    LS_GET_FACEIMG,     //get face image
    LS_DONE,            //finish the login.
    LS_ERROR,           //error
    LS_UNKNOWN          //unknown status
};

struct _QQLoginPanel{
    GtkVBox parent;

    /*< private >*/
    GtkWidget *uin_label, *uin_entry;
    GtkWidget *passwd_label, *passwd_entry;
    GtkWidget *rempwcb;             //rember password check button
    GtkWidget *err_label;           //show error infomation.
    GtkWidget *login_btn, *status_comb;
#ifdef USE_PROXY
	GtkWidget *set_proxy_btn;
#endif	/* USE_PROXY */

    const gchar *uin, *passwd, *status;
    gint rempw;

    GtkWidget *container;

    //used to mark the login state.
    QQLoginPanelLoginState login_state;
};

struct _QQLoginPanelClass{
    GtkVBoxClass parent;
};

/*
 * Create a new instance of QQLoginPanel.
 *
 * @param container the container which contains this instance. Can be
 *     set to NULL.
 */
GtkWidget* qq_loginpanel_new(GtkWidget *container);
GType qq_loginpanel_get_type();

/*
 * Get the inputs
 */
const gchar* qq_loginpanel_get_uin(QQLoginPanel *loginpanel);
const gchar* qq_loginpanel_get_passwd(QQLoginPanel  *loginpanel);
const gchar* qq_loginpanel_get_status(QQLoginPanel *loginpanel);
gint qq_loginpanel_get_rempw(QQLoginPanel *loginpanel);
#endif

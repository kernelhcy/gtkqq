#ifndef __GTKQQ_SENDMSG_H
#define __GTKQQ_SENDMSG_H
#include <glib.h>
#include <qqtypes.h>

/*
 * Define the result of the function
 */
typedef enum{
	CB_SUCCESS,	//success
	CB_FAILED,	//failed
	CB_WRONGPASSWD,	//wrong password
	CB_WRONGVC,	//wrong verify code
	CB_NETWORDERR,	//network error
	CB_UNKNOWN	//unknown result
}CallBackResult;

/*
 * Callback function of the functions below.
 */
typedef void (*QQCallBack)(CallBackResult re, gpointer data);

/*
 * Login
 * @param info store the information used by this program
 * @param uin the QQ number
 * @param passwd the QQ password
 */
void qq_login(QQInfo *info, const gchar *uin, const gchar *passwd
		, QQCallBack cb);

/*
 * Logout
 */
void qq_logout(QQUser *usr, QQCallBack cb);

void qq_get_user_info(QQUser *usr, QQCallBack cb);
void qq_get_group_info();
void qq_get_catogery_info();

/*
 * Send message
 * This function will return immediately after called. After the 
 * message has been send, cb will be called.
 */
void qq_sendmsg(QQMsg *msg, QQCallBack cb);
GString* get_pwvc_md5(const gchar *pwd, const gchar *vc);

#endif

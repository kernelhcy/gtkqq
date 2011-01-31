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
 * @param verifycode the verify code
 */
void QQ_login(QQInfo *info, const gchar *uin, const gchar *passwd
		, const gchar *verifycode, QQCallBack cb);

/*
 * Logout
 */
void QQ_logout(QQUser *usr, QQCallBack cb);

void QQ_get_user_info(QQUser *usr, QQCallBack cb);
void QQ_get_group_info();
void QQ_get_catogery_info();

/*
 * Send message
 * This function will return immediately after called. After the 
 * message has been send, cb will be called.
 */
void QQ_sendmsg(QQMsg *msg, QQCallBack cb);

#endif

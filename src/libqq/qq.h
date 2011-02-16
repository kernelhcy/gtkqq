#ifndef __GTKQQ_SENDMSG_H
#define __GTKQQ_SENDMSG_H
#include <glib.h>
#include <qqtypes.h>


/*
 * Initialize the libqq.
 * NOTE:
 * 	**This function MUST be called before any other function!**
 * return 0 if success or -1 if failed
 */
gint qq_init();


/*
 * Login
 * @param info store the information used by this program
 * @param uin the QQ number
 * @param passwd the QQ password
 * @param status the status of the user. If NULL, this func
 * 	will not change the status.
 */
void qq_login(QQInfo *info, const gchar *uin, const gchar *passwd
		, const gchar *status , QQCallBack cb);

/*
 * Logout
 */
void qq_logout(QQInfo *info, QQCallBack cb);

void qq_get_group_info();
void qq_get_catogery_info();

/*
 * Send message
 * This function will return immediately after called. After the 
 * message has been send, cb will be called.
 */
void qq_sendmsg(QQMsg *msg, QQCallBack cb);
GString* get_pwvc_md5(const gchar *pwd, const gchar *vc);

/*
 * Get information of myself
 */
void qq_get_my_info(QQInfo *info, QQCallBack cb);
/*
 * Get all my friends' information.
 * Just simple information.
 */
void qq_get_my_friends(QQInfo *info, QQCallBack cb);
/*
 * Get the group name list mask.
 */
void qq_get_group_name_list_mask(QQInfo *info, QQCallBack cb);

#endif

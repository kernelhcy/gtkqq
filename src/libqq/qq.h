#ifndef __GTKQQ_SENDMSG_H
#define __GTKQQ_SENDMSG_H
#include <glib.h>
#include <qqtypes.h>


/*
 * Initialize the libqq.
 * NOTE:
 * 	**This function MUST be called before any other function!**
 * return QQInfo instance if success or NULL if failed
 */
QQInfo* qq_init(QQCallBack cb);


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
/*
 * Get the online buddies
 */
void qq_get_online_buddies(QQInfo *info, QQCallBack cb);
/*
 * Get the recent contact.
 */
void qq_get_recent_contact(QQInfo *info, QQCallBack cb);

/*
 * Get the long nick of bdy.
 */
void qq_get_single_long_nick(QQInfo *info, QQBuddy *bdy, QQCallBack cb);

/*
 * Start the poll thread to poll the message form the server.
 */
void qq_start_poll(QQInfo *info, QQCallBack cb);

/*
 * Send message to friends and group
 */
void qq_sendmsg_to_friend(QQInfo *info, QQBuddy *to
			, const gchar *msg, QQCallBack cb);
void qq_sendmsg_to_group(QQInfo *info, QQGroup *to
			, const gchar *msg, QQCallBack cb);
#endif

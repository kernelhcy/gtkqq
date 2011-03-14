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
QQInfo* qq_init(QQCallBack cb, gpointer usrdata);

/*
 * Check if we need the verify code.
 * We need call this function before qq_login.
 * If we need the verify code, after this function, the verify code image
 * is stored in info -> vc_image_data.
 */
void qq_check_verifycode(QQInfo *info, const gchar *uin, QQCallBack cb
						, gpointer usrdata);

/*
 * Login
 * @param info store the information used by this program
 * @param uin the QQ number
 * @param passwd the QQ password
 * @param status the status of the user. If NULL, this func
 * 	will not change the status.
 *
 * If success, the callback's redata is "LOGIN"
 * or, it is the failed information string.
 */
void qq_login(QQInfo *info, const gchar *uin, const gchar *passwd
		, const gchar *status , QQCallBack cb, gpointer usrdata);

/*
 * Logout
 * If success, the callback's redata is "LOGOUT"
 * or, it is the failed information string.
 */
void qq_logout(QQInfo *info, QQCallBack cb, gpointer usrdata);

/*
 * Send message
 * This function will return immediately after called. After the 
 * message has been send, cb will be called.
 */
void qq_sendmsg(QQMsg *msg, QQCallBack cb, gpointer usrdata);
GString* get_pwvc_md5(const gchar *pwd, const gchar *vc);

/*
 * Get information of myself
 * If success, the callback's redata is "GET_MY_INFO"
 * or, it is the failed information string.
 */
void qq_get_my_info(QQInfo *info, QQCallBack cb, gpointer usrdata);
/*
 * Get all my friends' information.
 * Just simple information.
 * If success, the callback's redata is "GET_MY_FRIENDS"
 * or, it is the failed information string.
 */
void qq_get_my_friends(QQInfo *info, QQCallBack cb, gpointer usrdata);
/*
 * Get the group name list mask.
 * If success, the callback's redata is "GET_GROUP_NAME_LIST_MASK"
 * or, it is the failed information string.
 */
void qq_get_group_name_list_mask(QQInfo *info, QQCallBack cb
						, gpointer usrdata);
/*
 * Get the online buddies
 * If success, the callback's redata is "GET_ONLINE_BUDDIES"
 * or, it is the failed information string.
 */
void qq_get_online_buddies(QQInfo *info, QQCallBack cb, gpointer usrdata);
/*
 * Get the recent contact.
 * If success, the callback's redata is "GET_RECENT_CONTACT"
 */
void qq_get_recent_contact(QQInfo *info, QQCallBack cb, gpointer usrdata);

/*
 * Get the long nick of bdy.
 * If success, the callback's redata is "GET_SINGLE_LONG_NICK".
 * or, it is the failed information string.
 */
void qq_get_single_long_nick(QQInfo *info, QQBuddy *bdy, QQCallBack cb
						, gpointer usrdata);

/*
 * Start the poll thread to poll the message form the server.
 */
void qq_start_poll(QQInfo *info, QQCallBack cb, gpointer usrdata);

/*
 * Send message to friends and group
 */
void qq_sendmsg_to_friend(QQInfo *info, QQMsg *msg, QQCallBack cb
						, gpointer usrdata);
void qq_sendmsg_to_group(QQInfo *info, QQMsg *msg, QQCallBack cb
						, gpointer usrdata);

//
//Get the face image of uin
//
void qq_get_face_img(QQInfo *info, const gchar *uin, QQCallBack cb
				, gpointer usrdata);
//
//Save face image to file path/uin.type
//
gint qq_save_face_img(QQBuddy *bdy, const gchar *path);
#endif

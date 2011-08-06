#ifndef __GTKQQ_SENDMSG_H
#define __GTKQQ_SENDMSG_H
#include <glib.h>
#include <qqtypes.h>


//
// Initialize the libqq.
// NOTE:
// 	**This function MUST be called before any other function!**
// return QQInfo instance if success or NULL if failed
//
QQInfo* qq_init(GError **err);
//
// Finalze the event loops.
// Called before the program exits.
//
void qq_finalize(QQInfo *info, GError **err);

//
// Check if we need the verify code.
// We need call this function before qq_login.
// If we need the verify code, after this function, the verify code image
// is stored in info -> vc_image_data.
//
gint qq_check_verifycode(QQInfo *info, const gchar *uin, GError **err);

//
// Login
// @param info store the information used by this program
// @param uin the QQ number
// @param passwd the QQ password
// @param status the status of the user. If NULL, this func
// 	will not change the status.
//
// If success, return 0. or error code and set *err.
//
gint qq_login(QQInfo *info, const gchar *uin, const gchar *passwd
		                    , const gchar *status, GError **err);

//
// Logout
// If success, return 0. or error code and set *err.
//
gint qq_logout(QQInfo *info, GError **err);

//
// Send message
// If success, return 0. or error code and set *err.
//
gint qq_sendmsg(QQSendMsg *msg, GError **err);
GString* get_pwvc_md5(const gchar *pwd, const gchar *vc, GError **err);

//
// Get all the buddies and categories
// If success, return 0. or error code and set *err.
//
gint qq_get_buddies_and_categories(QQInfo *info, GError **err);
//
// Get all groups.
// If success, return 0. or error code and set *err.
//
gint qq_get_groups(QQInfo *info, GError **err);
//
// Update the details of buddies and groups.
// Buddies, groups etc
//
gint qq_update_details(QQInfo *info, GError **err);

//
// Get buddy information 
// If success, return 0. or error code and set *err.
//
gint qq_get_buddy_info(QQInfo *info, QQBuddy *bdy, GError **err);
//
// Get the group information 
// If success, return 0. or error code and set *err.
//
gint qq_get_group_info(QQInfo *info, QQGroup *grp, GError **err);

//
// Get the online buddies
// If success, return 0. or error code and set *err.
//
gint qq_get_online_buddies(QQInfo *info, GError **err);
//
// Get the recent contact.
// If success, the callback's redata is "GET_RECENT_CONTACT"
//
gint qq_get_recent_contact(QQInfo *info, GError **err);
//
// Get the long nick of bdy.
// If success, return 0. or error code and set *err.
//
gint qq_get_single_long_nick(QQInfo *info, QQBuddy *bdy, GError **err);
//
// Get the qq (group) number of uin
//
gint qq_get_qq_number(QQInfo *info, const gchar *uin
                                    , gchar *num, GError **err);

//
// The callback of the poll fuction.
// @param msg : the message received
// @param data : the user data
//
typedef gint (*QQPollCallBack)(QQRecvMsg *msg, gpointer data);
//
// Start the poll thread to poll the message form the server.
// When a new message received, the QQPollCallBack is called.
// data is passed to cb.
//
// NOTE:
//  This function will create a new thread to poll message.
//  This function will return immeditally.
//  The callback function will called in the new created thread.
//
gint qq_start_poll(QQInfo *info, QQPollCallBack cb, gpointer data
                                                , GError **err);
//
// Stop poll message.
//
void qq_stop_poll(QQInfo *info);

//
// Send message to friends and group
//
gint qq_send_message(QQInfo *info, QQSendMsg *msg, GError **err);

//
// Get the face image of uin
// The redata of the cb function is QQFaceImg struct which contains
// the information of the face image.
//
gint qq_get_face_img(QQInfo *info, QQFaceImg *img, GError **err);
//
// Save face image to file path
//
gint qq_save_face_img(QQFaceImg *img, const gchar *path, GError **err);
#endif

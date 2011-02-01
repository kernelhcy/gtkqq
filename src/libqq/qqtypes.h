#ifndef __GTKQQ_QQTYPES_H
#define __GTKQQ_QQTYPES_H
#include <glib.h>
/*
 * The data structures of gtkqq
 */

typedef struct _QQInfo 		QQInfo;
typedef struct _QQMsg 		QQMsg;
typedef struct _QQUser 		QQUser;
typedef struct _QQGroup 	QQGroup;
typedef struct _QQCategory 	QQCategory;

/*
 * The main information
 */
struct _QQInfo{
	GThread *mainloopthread;
	GMainLoop *mainloop;
	GMainContext *mainctx;	

	GString *uin;
	GString *status;

	GString *verify_code;		//the verify code return from server
	GString *vc_type;		//vc_type
	gboolean need_vcimage;		//if we need get the verify code image
	GString *vc_image_data;		//store the verify code image data
	GString *ptvfsession;		
};

QQInfo* qq_info_new();
void qq_info_free(QQInfo *);

struct _QQMsg{
	QQUser *to, *from;
	GString *content;	
};
QQMsg* qq_msg_new();
void qq_msg_free(QQMsg *);

struct _QQUser{
	GString *uin;

};
QQUser* qq_user_new();
void qq_user_free(QQUser *);

struct _QQGroup{
	GString *name;
};
QQGroup* qq_group_new();
void qq_group_free(QQGroup *);

struct _QQCategory{
	GString *name;
};
QQCategory* qq_category_new();
void qq_category_free(QQCategory *);

#endif

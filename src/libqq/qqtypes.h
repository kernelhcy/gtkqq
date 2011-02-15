#ifndef __GTKQQ_QQTYPES_H
#define __GTKQQ_QQTYPES_H
#include <glib.h>

#define SL g_debug("(%s, %d)", __FILE__, __LINE__);

/*
 * Define the result of the function
 */
typedef enum{
	CB_SUCCESS,	//success
	CB_ERROR,	//error occured
	CB_WRONGPASSWD,	//wrong password
	CB_WRONGVC,	//wrong verify code
	CB_NETWORKERR,	//network error
	CB_UNKNOWN	//unknown result
}CallBackResult;
/*
 * Callback function of the functions below.
 */
typedef void (*QQCallBack)(CallBackResult re, gpointer data);

/*
 * The data structures of gtkqq
 */

typedef struct _QQInfo 		QQInfo;
typedef struct _QQMsg 		QQMsg;
typedef struct _QQBuddy		QQBuddy;
typedef struct _QQGroup 	QQGroup;
typedef struct _QQGMember	QQGMember;
typedef struct _QQCategory 	QQCategory;

/*
 * The main information
 */
struct _QQInfo{
	GThread *mainloopthread;
	GMainLoop *mainloop;
	GMainContext *mainctx;	

	QQBuddy *me;			//myself

	GPtrArray *buddies;		//all friends;
	GPtrArray *groups;		//all groups;
	GPtrArray *recentcon;		//the recenet contacts
	GPtrArray *categories;		//all categories

	GString *verify_code;		//the verify code return from server
	GString *vc_type;		//vc_type
	gboolean need_vcimage;		//if we need get the verify code image
	GString *vc_image_data;		//store the verify code image data
	GString *vc_image_type;		//the verify code image file type
	GString *ptvfsession;		

	GString *version;
	GString *ptwebqq;

	GString *ptuserinfo;
	GString *ptcz;
	GString *skey;

	GString *psessionid;
	GString *seskey;
	GString *cip;
	GString *index;
	GString *port;
	GString *vfwebqq;

	GString *clientid;		
	GString *cookie;

	gchar errmsg[500];		//store error message.

	/*
	 * Maybe we need a lock...
	 */
	GMutex *lock;			
};

QQInfo* qq_info_new();
void qq_info_free(QQInfo *);
void qq_append_cookie(QQInfo *, const gchar *);

struct _QQMsg{
	GString *to, *from;
	GString *content;	
};
QQMsg* qq_msg_new();
void qq_msg_free(QQMsg *);

/*
 * The inforamtion of the buddies and myself.
 */
struct _QQBuddy{
	GString *uin;
	GString *status;
	gint vip_info;
	GString *nick;				//
	GString *markname;

	GString *country;
	GString *province;
	GString *city;

	GString *gender;			//male or female
	GString *face;

	struct Birthday{
		gint year, month, day;
	}birthday;
	gint blood;				//A, B, AB or O
	gint shengxiao;			

	gint constel;
	GString *phone;
	GString *mobile;
	GString *email;
	GString *occupation;
	GString *college;
	
	GString *homepage;
	GString *personal;
	GString *lnick;

	gint allow;

	/*
	 * 1 : 桌面客户端
	 * 21: 手机客户端
	 *   : web QQ
	 */
	gint client_type;

	gint category_idx;			//the index of the category 
						//contains this buddy
};
QQBuddy* qq_buddy_new();
void qq_buddy_free(QQBuddy *);

/*
 * The member of the group.
 */
struct _QQGMember{
	GString *uin;
	GString *nick;
	GString *flag;
	GString *status;
	GString *card;
};
QQGMember* qq_gmember_new();
void qq_gmember_free(QQGMember *m);
/*
 * QQ group
 */
struct _QQGroup{
	GString *name;
	GString *gid;
	GString *code;
	GString *flag;
	GString *owner;
	gint option;
	glong createTime;
	gint gclass;
	gint level;
	gint face;

	GString *memo;
	GString *fingermemo;

	GPtrArray *members;
};
QQGroup* qq_group_new();
void qq_group_free(QQGroup *);

struct _QQCategory{
	GString *name;
	gint index;
};
QQCategory* qq_category_new();
void qq_category_free(QQCategory *);


/**
 * Used in the lib. Not public. 
 */

/*
 * As the parameter of the do_xxxx functions.
 */
typedef struct _DoFuncParam{
	QQInfo *info;
	QQCallBack cb;
}DoFuncParam;

/*
 * Get the milliseconds of now.
 */
glong get_now_millisecond();

#endif

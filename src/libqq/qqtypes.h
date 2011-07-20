#ifndef __GTKQQ_QQTYPES_H
#define __GTKQQ_QQTYPES_H
#include <glib.h>

#define SL g_debug("(%s, %d)", __FILE__, __LINE__);

/*
 * The data structures of gtkqq
 */

typedef struct _QQInfo          QQInfo;
typedef struct _QQSendMsg       QQSendMsg;
typedef struct _QQRecvMsg       QQRecvMsg;
typedef struct _QQMsgContent    QQMsgContent;
typedef struct _QQMsgFont       QQMsgFont;
typedef struct _QQBuddy         QQBuddy;
typedef struct _QQGroup         QQGroup;
typedef struct _QQGMember       QQGMember;
typedef struct _QQCategory      QQCategory;
typedef struct _QQRecentCon     QQRecentCon;
typedef struct _QQFaceImg       QQFaceImg;

/*
 * Store the face image data and type
 */
struct _QQFaceImg{
    GString *uin;
    GString *data;
    GString *type;
};
QQFaceImg* qq_faceimg_new();
void qq_faceimg_free(QQFaceImg *img);

/*
 * The main information
 */
struct _QQInfo{
    /*
     * The main event loop
     */
    GThread *mainloopthread;
    GMainLoop *mainloop;
    GMainContext *mainctx;    

    /*
     * The poll event loop
     */
    GThread *pollloopthread;
    GMainLoop *pollloop;
    GMainContext *pollctx;    

    QQBuddy *me;                    //myself

    GPtrArray *buddies;                //all friends;
    GHashTable *buddies_image_ht;    //face image file name 
    GHashTable *buddies_ht;         //buddies hash table by uin
    GPtrArray *groups;                //all groups;
    GHashTable *groups_ht;            //goups hash table by gid
    GPtrArray *recentcons;            //the recenet contacts
    GPtrArray *categories;            //all categories

    GString *verify_code;            //the verify code return from server
    GString *vc_type;                //vc_type
    gboolean need_vcimage;            //if we need get the verify code image
    GString *vc_image_data;            //store the verify code image data
    GString *vc_image_type;            //the verify code image file type
    gint vc_image_size;             //verify code image size
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
    GString *uin;
    GString *ptisp;
    GString *pt2gguin;

    GString *clientid;        
    GString *cookie;

    /*
     * Maybe we need a lock...
     */
    GMutex *lock;            
    glong msg_id;            //used to send message
};

QQInfo* qq_info_new();
void qq_info_free(QQInfo *);
void qq_append_cookie(QQInfo *, const gchar *);
//
// Look up the buddy by the uin
//
QQBuddy* qq_info_lookup_buddy(QQInfo *, const gchar *uin);
//
// Look up the group by the gid
//
QQGroup* qq_info_lookup_group(QQInfo *, const gchar *gid);

//
// The font of the messages
//
struct _QQMsgFont{
    GString *name;
    gint size;
    GString *color;
    struct{
        gint a, b, c;
    }style;
};
QQMsgFont* qq_msgfont_new(const gchar *name, gint size, const gchar *color
                            , gint sa, gint sb, gint sc);
void qq_msgfont_free(QQMsgFont *font);


//
// The message content.
//
// type :
//      1, face.    eg: ["face", 21]
//      2, string.  eg: "some string"
//
struct _QQMsgContent{
    gint type;              //the type of the content.
    union{
        gint face;
        GString *str;
    }value;
};
//
// Create a new instance of QQMsgContent.
//
// Example:
//      qq_msgcontent_new(1, 20);               // face
//      qq_msgcontent_new(2, "somg string");    // string
//
QQMsgContent *qq_msgcontent_new(gint type, ...);
void qq_msgcontent_free(QQMsgContent *cnt);
//
// Convert to string
//
GString* qq_msgcontent_tostring(QQMsgContent *cnt);

//
// The send message.
//
struct _QQSendMsg{
    QQInfo *info;
    gint type;              // 0 this is buddy message, or 1 is group message.
    GString *to_uin;        // If buddy msg, this is "to"
                            // If group msg, this is "group_uin"
    GString *face;          // Only used when this is buddy message.

    GPtrArray *contents;    // Message contents. An array of QQMsgContent.
    QQMsgFont *font;

    GString *msg_id;
    GString *clientid;
    GString *psessionid;
};
QQSendMsg* qq_sendmsg_new(QQInfo *info, gint type, const gchar *to_uin);
void qq_sendmsg_free(QQSendMsg *msg);
void qq_sendmsg_set_font(QQSendMsg *msg, const gchar *name, gint size, const gchar *color
                                , gint sa, gint sb, gint sc);
void qq_sendmsg_add_context(QQSendMsg *msg, QQMsgContent *content);
//
// Convert contents to string
//
GString * qq_sendmsg_contents_tostring(QQSendMsg *msg);

//
// The received message
//
struct _QQRecvMsg{
    QQInfo *info;
    
    GString *poll_type;
    GString *msg_id, *msg_id2;
    GString *from_uin, *to_uin;
    gint msg_type;
    GString *reply_ip;
    GString *group_code;        // only group message
    GString *send_uin;          // only group message
    gint seq;                   // only group message
    GString *time;
    gint info_seq;              // only group message

    GPtrArray *contents;
    GString *raw_content;       // the raw content.
};
QQRecvMsg* qq_recvmsg_new();
void qq_recvmsg_free(QQRecvMsg *);

/*
 * The inforamtion of the buddies and myself.
 */
struct _QQBuddy{
    GString *uin;
    GString *status;
    gint vip_info;
    GString *nick;                //
    GString *markname;
    QQFaceImg *faceimg;             //the face image data
    GString *faceimgfile;             //the face image file name

    GString *country;
    GString *province;
    GString *city;

    GString *gender;            //male or female
    GString *face;
    GString *flag;

    struct Birthday{
        gint year, month, day;
    }birthday;
    gint blood;                //A, B, AB or O
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
     * 41: web QQ
     */
    gint client_type;
    QQCategory *cate;            // The category of this buddy

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
    QQFaceImg *faceimg;             //the face image data
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
    GString *mark;
    GString *mask;
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

    /*
     * The pointers of the members
     * QQBuddy instance.
     */
    GPtrArray *members;
};
QQCategory* qq_category_new();
void qq_category_free(QQCategory *);


struct _QQRecentCon{
    GString *uin;
    /*
     * 0: friend
     * 1: group
     */
    gint type;
};
QQRecentCon* qq_recentcon_new();
void qq_recentcon_free(QQRecentCon *rc);


/*
 * Get the milliseconds of now.
 */
glong get_now_millisecond();

#endif

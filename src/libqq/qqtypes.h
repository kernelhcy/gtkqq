#ifndef __GTKQQ_QQTYPES_H
#define __GTKQQ_QQTYPES_H
#include <glib.h>

#define SL g_debug("(%s, %d)", __FILE__, __LINE__);

/**
 * The data structures of gtkqq
 */
typedef struct _QQInfo              QQInfo;

typedef enum   _QQMsgType           QQMsgType;
typedef struct _QQSendMsg           QQSendMsg;
typedef struct _QQRecvMsg           QQRecvMsg;
typedef struct _QQMsgContent        QQMsgContent;
typedef enum   _QQMsgContentType    QQMsgContentType;
typedef struct _QQMsgFont           QQMsgFont;

typedef struct _QQBuddy             QQBuddy;
typedef struct _QQGroup             QQGroup;
typedef struct _QQGMember           QQGMember;
typedef struct _QQCategory          QQCategory;

typedef struct _QQRecentCon         QQRecentCon;
typedef struct _QQFaceImg           QQFaceImg;

typedef enum  _QQErrorCode          QQErrorCode;

enum _QQReturnCode{
    NO_ERR          = 0,    // no error
    PARAMETER_ERR   = -1,   // the parameter(s) is(are) not suitable.
    NETWORK_ERR     = -2,   // networking error.
    WRONGPWD_ERR    = -3,   // wrong password
    WRONGVC_ERR     = -4,   // wrong verify code
    WRONGUIN_ERR    = -5,   // wrong uin
    OTHER_ERR       = -100  // others wrong.
};
/*
 * Store the face image data and type
 */
struct _QQFaceImg{
    GString *uin;
    GString *num;
    GString *data;
    GString *type;
};
QQFaceImg* qq_faceimg_new();
void qq_faceimg_free(QQFaceImg *img);
void qq_faceimg_copy(QQFaceImg *from, QQFaceImg *to);
void qq_faceimg_set(QQFaceImg *img, const gchar *key, GString *val);

/*
 * The main information
 */
struct _QQInfo{
    /*
     * The poll thread
     */
    GThread *pollthread;

    QQBuddy *me;                    //myself

    GPtrArray *buddies;             //all friends;
    GHashTable *buddies_ht;         //buddies hash table by uin
    GHashTable *buddies_number_ht;  //buddies hash table by qq number

    GPtrArray *groups;              //all groups;
    GHashTable *groups_ht;          //goups hash table by code
    GHashTable *groups_number_ht;   //goups hash table by group number 

    GPtrArray *recentcons;          //the recenet contacts
    GPtrArray *categories;          //all categories

    GString *verify_code;           //the verify code return from server
    GString *vc_type;               //vc_type
    gboolean need_vcimage;          //if we need get the verify code image
    GString *vc_image_data;         //store the verify code image data
    GString *vc_image_type;         //the verify code image file type
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
// Look up the buddy by the uin and qq number
//
QQBuddy* qq_info_lookup_buddy_by_uin(QQInfo *, const gchar *uin);
QQBuddy* qq_info_lookup_buddy_by_number(QQInfo *, const gchar *number);
//
// Look up the group by the code and group number
//
QQGroup* qq_info_lookup_group_by_code(QQInfo *, const gchar *code);
QQGroup* qq_info_lookup_group_by_number(QQInfo *, const gchar *number);

//
// The font of the messages
//
struct _QQMsgFont{
    GString *name;
    gint size;
    GString *color;
    struct{
        gint a, b, c; // bold , italic , underline
    }style;
};
QQMsgFont* qq_msgfont_new(const gchar *name, gint size, const gchar *color
                            , gint sa, gint sb, gint sc);
void qq_msgfont_free(QQMsgFont *font);
//
// If two font is the same
//
gboolean qq_msgfont_equal(QQMsgFont *a, QQMsgFont *b);

//
// Type of the message content
// type :
//      face.    eg: ["face", 21]
//      string.  eg: "some string"
//      font.    eg: ["font", {"name":"Arial", "size":10
//                          ,"style":[1, 0, 0], "color":"000000"}]
//
enum  _QQMsgContentType
{
    QQ_MSG_CONTENT_FACE_T = 256,        // face
    QQ_MSG_CONTENT_STRING_T,            // string
    QQ_MSG_CONTENT_FONT_T,              // font
    QQ_MSG_CONTENT_UNKNOWN_T            // unknown
};
//
// The message content.
//
struct _QQMsgContent{
    gint type;              //the type of the content.
    union{
        gint face;
        GString *str;
        QQMsgFont *font;
    }value;
};
//
// Create a new instance of QQMsgContent.
//
// Example:
//      qq_msgcontent_new(1, 20);               // face
//      qq_msgcontent_new(2, "somg string");    // string
//      qq_msgcontent_new(3,                    // string
//                         "name",      //name
//                         10,          //size
//                         "000000",    //color
//                         1,0,0);      //style
//
QQMsgContent *qq_msgcontent_new(gint type, ...);
void qq_msgcontent_free(QQMsgContent *cnt);
//
// Convert to string
//
GString* qq_msgcontent_tostring(QQMsgContent *cnt);

//
// Message type
//
enum _QQMsgType{
    MSG_BUDDY_T = 128,      /* buddy message */
    MSG_GROUP_T,            // group message
    MSG_STATUS_CHANGED_T,   // buddy status changed
    MSG_KICK_T,             // kick message. In other place logined
    MSG_UNKNOWN_T
};
//
// The send message.
//
struct _QQSendMsg{
    QQInfo *info;
    QQMsgType type;         // 0 this is buddy message, or 1 is group message.
    GString *to_uin;        // If buddy msg, this is "to"
                            // If group msg, this is "group_uin"
    GString *face;          // Only used when this is buddy message.

    GPtrArray *contents;    // Message contents. An array of QQMsgContent.

    GString *msg_id;
    GString *clientid;
    GString *psessionid;
};
QQSendMsg* qq_sendmsg_new(QQInfo *info, QQMsgType type
                                , const gchar *to_uin);
void qq_sendmsg_free(QQSendMsg *msg);
void qq_sendmsg_add_content(QQSendMsg *msg, QQMsgContent *content);
//
// Convert contents to string
//
GString * qq_sendmsg_contents_tostring(QQSendMsg *msg);

/**
 * message type
 * The received message
 */
struct _QQRecvMsg {
    QQInfo *info;
    QQMsgType msg_type;
    
    GString *msg_id, *msg_id2;
    GString *from_uin, *to_uin;
    GString *reply_ip;

    GString *group_code;        // only group message
    GString *send_uin;          // only group message
    gint seq;                   // only group message
    GString *time;
    gint info_seq;              // only group message
    gint type;

    GString *uin, *status, *client_type;       //only buddy status change

    GPtrArray *contents;
    GString *raw_content;       // the raw content.
};

QQRecvMsg* qq_recvmsg_new(QQInfo *info, QQMsgType type);
void qq_recvmsg_set(QQRecvMsg *msg, const gchar *name, const gchar *value);
void qq_recvmsg_add_content(QQRecvMsg *msg, QQMsgContent *content);
void qq_recvmsg_free(QQRecvMsg *);

/*
 * The inforamtion of the buddies and myself.
 * represent a QQ user.
 */
struct _QQBuddy{
    GString *uin;               //the uin. Change every login
    GString *qqnumber;          //the qq number
    GString *status;
    gint vip_info;
    GString *nick;              //
    GString *markname;

    GString *country;
    GString *province;
    GString *city;

    GString *gender;            //male or female
    GString *face;
    GString *flag;

    struct Birthday {
        gint year, month, day;
    } birthday;
    gint blood;                 //A, B, AB or O
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
    gint cate_index;            //The index of the category

    /*
     * 1 : 桌面客户端
     * 21: 手机客户端
     * 41: web QQ
     */
    gint client_type;

};

QQBuddy* qq_buddy_new();
void qq_buddy_free(QQBuddy *bdy);
//Set the value of the member named `name`
void qq_buddy_set(QQBuddy *bdy, const gchar *name, ...);
//
// Copy `from` to `to`
// Deep copy
//
void qq_buddy_copy(QQBuddy *from, QQBuddy *to);


/**
 * The member of the group.
 */
struct _QQGMember{
    GString *uin;
    GString *qqnumber;
    GString *nick;
    GString *flag;
    GString *status;
    GString *card;
    GString *client_type;
};
QQGMember* qq_gmember_new();
void qq_gmember_free(QQGMember *m);
void qq_gmember_set(QQGMember *m, const gchar *name, ...);

/*
 * QQ group
 */
struct _QQGroup{
    GString *name;
    GString *gid;
    GString *gnumber;
    GString *code;
    GString *flag;
    GString *owner;
    GString *mark;
    GString *mask;
    GString *option;
    GString *createtime;
    GString *gclass;
    GString *level;
    GString *face;

    GString *memo;
    GString *fingermemo;

    GPtrArray *members;
};
QQGroup* qq_group_new();
void qq_group_free(QQGroup *);
void qq_group_set(QQGroup *grp, const gchar *name, ...);
//
// Add group member
// Return the index or -1 for failed
//
gint qq_group_add(QQGroup *grp, QQGMember *m);
//
// Lookup the group member by the uin
//
QQGMember* qq_group_lookup_member_by_uin(QQGroup *grp, const gchar *uin);

struct _QQCategory{
    GString *name;
    gint index;

    /*
     * The pointers of the members QQBuddy instance.
     */
    GPtrArray *members;
};
QQCategory* qq_category_new();
void qq_category_free(QQCategory *cate);
void qq_category_set(QQCategory *cate, const gchar *key, ...);

//
// QQRecentCon
//
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


/**
 * Get the milliseconds of now.
 */
glong get_now_millisecond();

/**
 * Create an error message.
 * return form err.
 */
void create_error_msg(GError **err, gint code, const gchar *fmt, ...);
#endif

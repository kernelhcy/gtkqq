#ifndef __QQ_H_
#define	__QQ_H_ 
#include <glib.h>

#define QQ_(obj) 							GTK_CHECK_CAST(obj,qq_get_type(),QQ)
#define QQ_CLASS(klass) 					(G_TYPE_CHECK_CLASS_CAST((klass),QQ,QQClass))
#define IS_QQ_(obj) 						(G_TYPE_CHECK_INSTANCE_TYPE((obj),QQ))
#define IS_QQ_(klass) 						(G_TYPE_CHECK_CLASS_TYPE ((klass),QQ))
#define QQ_GET_CLASS(obj) 					(G_TYPE_INSTANCE_GET_CLASS((obj),QQ,QQClass))
typedef struct _QQ			QQ;
typedef struct _QQClass		QQClass;
typedef	struct _QQConfig	QQconfig;
struct _QQ
{
	GObject object;
}
struct _QQClass
{
	GObjectClass parent_class;
}
struct	_QQConfig
{

}
GType qq_get_type(void);
//配置文件相关
gint 		qq_set_config						(QQ* qq,QQConfig* config);
gint 		qq_set_config_from_file				(QQ* qq,gchar* url);
gint 		qq_set_config_from_string			(QQ* qq,gchar* buf);

gpointer 	(*QQSaveConfigHandler) 				(QQ* qq,QQConfig* config,gpointer user_data);
gpointer 	qq_set_save_config_handler			(QQSaveConfigHandler handler);

// 登录
// 
// qq_login(qq,1234,2345,
// 				"check-verifycode",
// 				login_check_verifycode_callback,
// 				user_data,
// 				"success",
// 				login_success_callback,
// 				user_data,
// 				"fail",
// 				login_fail_callback,
// 				user_data,
// 				NULL)
void		(*QQLoginCallback)					(QQ* qq,gint status,gpointer user_data);
void		qq_login							(QQ* qq,
												 gchar* uin,
												 gchar* passwd,
												 ...);
gpointer	(*QQLogoutHandler)					(QQ* qq,gpointer user_data);
void		qq_logout(QQ* qq,QQLogoutHandler *handler,gpointer user_data);

//获取好友QQ头像,为了兼容以后保存的方法不同，所以设计的麻烦了点。
//比如保存在数据库里，或者以图片文件的形式保存，或者保存在其它主机上。
gpointer	(*QQSaveFaceImageHandler)			(QQ* qq,pointer image,gpointer user_data);
gpointer 	(*QQGetFaceImageHandler) 			(QQ* qq,QQBuddy* buddy,gpointer user_data);

gint 		qq_buddy_set_get_face_image_handler	(QQ* qq,
												 QQGetFaceImageHandler handler,gpointer user_data);
gint		qq_buddy_set_save_face_image_handler(QQ* qq,
												 QQSaveFaceImageHandler handler,gpointer user_data);
gpointer 	qq_buddy_get_face_image				(QQBuddy*);
#endif

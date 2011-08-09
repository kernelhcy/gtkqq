#ifndef __QQ_CHAT_TEXT_VIEW_H_
#define __QQ_CHAT_TEXT_VIEW_H_
#include <gtk/gtk.h>
#include <qq.h>

#define QQ_CHAT_TEXTVIEW(obj)	    GTK_CHECK_CAST(obj\
                                                , qq_chat_textview_get_type()\
						                        , QQChatTextview)
#define QQ_CHAT_TEXTVIEWCLASS(c)	GTK_CHECK_CLASS_CAST(c\
						                        , qq_chat_textview_get_type()\
						                        , QQChatTextviewClass)
#define QQ_IS_CHAT_TEXTVIEW(obj)	GTK_CHECK_TYPE(obj\
                                                , qq_chat_textview_get_type())

typedef struct __QQChatTextview 		QQChatTextview;
typedef struct __QQChatTextviewClass	QQChatTextviewClass;

struct __QQChatTextview{
	GtkTextView parent;
};

struct __QQChatTextviewClass{
	GtkTextViewClass parent;
};

GtkWidget* qq_chat_textview_new();
GType qq_chat_textview_get_type();

//
// Add messages
//
void qq_chat_textview_add_recv_message(GtkWidget *widget, QQRecvMsg *msg);
void qq_chat_textview_add_send_message(GtkWidget *widget, QQSendMsg *msg);
#endif

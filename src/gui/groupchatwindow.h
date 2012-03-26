#ifndef __GTKQQ_GROUP_CHATWINDOW_H
#define __GTKQQ_GROUP_CHATWINDOW_H
#include <gtk/gtk.h>
#include <qq.h>

#define QQ_TYPE_GROUP_CHATWINDOW        qq_group_chatwindow_get_type()
#define QQ_GROUP_CHATWINDOW(obj)        G_TYPE_CHECK_INSTANCE_CAST(obj\
                                                , QQ_TYPE_GROUP_CHATWINDOW\
                                                , QQGroupChatWindow)
#define QQ_GROUP_CHATWINDOWCLASS(c)     G_TYPE_CHECK_CLASS_CAST(c\
                                                , QQ_TYPE_GROUP_CHATWINDOW()\
                                                , QQGroupChatWindowClass)
#define QQ_IS_GROUP_CHATWINDOW(obj)     G_TYPE_CHECK_INSTANCE_TYPE(obj\
                                                , QQ_TYPE_GROUP_CHATWINDOW())

typedef struct __QQGroupChatWindow         QQGroupChatWindow;
typedef struct __QQGroupChatWindowClass    QQGroupChatWindowClass;

struct __QQGroupChatWindow{
    GtkWindow parent;

};

struct __QQGroupChatWindowClass{
    GtkWindowClass parent;
};

GtkWidget* qq_group_chatwindow_new(const gchar *code);
GType qq_group_chatwindow_get_type();

//
// Add messages
//
void qq_group_chatwindow_add_recv_message(GtkWidget *widget, QQRecvMsg *msg);
void qq_group_chatwindow_add_send_message(GtkWidget *widget, QQSendMsg *msg);
#endif

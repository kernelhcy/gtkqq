#ifndef __GTKQQ_CHATWINDOW_H
#define __GTKQQ_CHATWINDOW_H
#include <gtk/gtk.h>
#include <qq.h>

#define QQ_CHATWINDOW(obj)      G_TYPE_CHECK_INSTANCE_CAST(obj, qq_chatwindow_get_type()\
                                                , QQChatWindow)
#define QQ_CHATWINDOWCLASS(c)   G_TYPE_CHECK_CLASS_CAST(c\
                                                , qq_chatwindow_get_type()\
                                                , QQChatWindowClass)
#define QQ_IS_CHATWINDOW(obj)   G_TYPE_CHECK_INSTANCE_TYPE(obj, qq_chatwindow_get_type())

typedef struct __QQChatWindow         QQChatWindow;
typedef struct __QQChatWindowClass    QQChatWindowClass;

struct __QQChatWindow{
    GtkWindow parent;

};

struct __QQChatWindowClass{
    GtkWindowClass parent;
};

GtkWidget* qq_chatwindow_new(const gchar *uin);
GType qq_chatwindow_get_type();

//
// Add messages
//
void qq_chatwindow_add_recv_message(GtkWidget *widget, QQRecvMsg *msg);
void qq_chatwindow_add_send_message(GtkWidget *widget, QQSendMsg *msg);

#endif /* __GTKQQ_CHATWINDOW_H */

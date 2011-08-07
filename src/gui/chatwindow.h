#ifndef __GTKQQ_CHATWINDOW_H
#define __GTKQQ_CHATWINDOW_H
#include <gtk/gtk.h>

#define QQ_CHATWINDOW(obj)	GTK_CHECK_CAST(obj, qq_chatwindow_get_type()\
						, QQChatWindow)
#define QQ_CHATWINDOWCLASS(c)	GTK_CHECK_CLASS_CAST(c\
						, qq_chatwindow_get_type()\
						, QQChatWindowClass)
#define QQ_IS_CHATWINDOW(obj)	GTK_CHECK_TYPE(obj, qq_chatwindow_get_type())

typedef struct __QQChatWindow 		QQChatWindow;
typedef struct __QQChatWindowClass	QQChatWindowClass;

struct __QQChatWindow{
	GtkWindow parent;
};

struct __QQChatWindowClass{
	GtkWindowClass parent;
};

GtkWidget* qq_chatwindow_new();
GType qq_chatwindow_get_type();
#endif

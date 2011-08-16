#ifndef __GTKQQ_CHATWIDGET_H
#define __GTKQQ_CHATWIDGET_H
#include <gtk/gtk.h>
#include <qq.h>

//
// Chat widget
// Contians:
//      message text view, font tool bar, tool bar
// Both the buddy chat window and the group chat window use this widget.
//
// ----------------------------------------------
// |                                            |
// |                                            |
// |          message text view                 | <-- message text view
// |                                            |
// |                                            |
// ----------------------------------------------
// | font|v| size|v| |a| |a| |a| |clor|         | <-- font tool bar (may hide)
// ----------------------------------------------
// | |font| |face| ....... |clrscr|             | <-- tool bar
// ----------------------------------------------
// |                                            |
// |          input text view                   | <--  input text view
// |                                            |
// ----------------------------------------------
//
#define QQ_CHATWIDGET(obj)      GTK_CHECK_CAST(obj, qq_chatwidget_get_type()\
                                                , QQChatWidget)
#define QQ_CHATWIDGETCLASS(c)   GTK_CHECK_CLASS_CAST(c\
                                                , qq_chatwidget_get_type()\
                                                , QQChatWidgetClass)
#define QQ_IS_CHATWIDGET(obj)   GTK_CHECK_TYPE(obj, qq_chatwidget_get_type())

typedef struct __QQChatWidget         QQChatWidget;
typedef struct __QQChatWidgetClass    QQChatWidgetClass;

struct __QQChatWidget{
    GtkVBox parent;

};

struct __QQChatWidgetClass{
    GtkVBoxClass parent;
};

GtkWidget* qq_chatwidget_new();
GType qq_chatwidget_get_type();

//
// Add messages
//
void qq_chatwidget_add_recv_message(GtkWidget *widget, QQRecvMsg *msg);
void qq_chatwidget_add_send_message(GtkWidget *widget, QQSendMsg *msg);

//
// Get the text view
//
GtkWidget* qq_chatwidget_get_message_textview(GtkWidget *widget);
GtkWidget* qq_chatwidget_get_input_textview(GtkWidget *widget);

//
// Get the font of font tool bar
//
QQMsgContent* qq_chatwidget_get_font(GtkWidget *widget);

#endif

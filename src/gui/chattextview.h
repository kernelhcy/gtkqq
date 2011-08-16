#ifndef __QQ_CHAT_TEXT_VIEW_H_
#define __QQ_CHAT_TEXT_VIEW_H_
#include <gtk/gtk.h>
#include <qq.h>

//
// The chat text view
// The chat text view need to show the qq face images and show
// the message like this:
//
// ------------------------------------------
// |  kernel 2010-9-10 12:20:10             | <-- blue
// |     hello!                             | <-- custom color
// |                                        |
// |  memory 2010-9-10 12:20:30             | <-- green
// |     hello. Need help?                  | <-- custom color
// |                                        |
// |  kernel 2010-9-10 12:21:10             |
// |     Yes. Give me more memory. Thanks.  |
// |                                        |
// |  memory 2010-9-10 12:22:30             |
// |     Ok.                                |
// |                                        |
// |  ...  ...                              |
// ------------------------------------------
//

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
// Clear the view
//
void qq_chat_textview_clear(GtkWidget *widget);

//
// Add messages
//
void qq_chat_textview_add_recv_message(GtkWidget *widget, QQRecvMsg *msg);
void qq_chat_textview_add_send_message(GtkWidget *widget, QQSendMsg *msg);

//
// Add face or string
//
void qq_chat_textview_add_face(GtkWidget *widget, gint face);
void qq_chat_textview_add_string(GtkWidget *widget, const gchar *str, gint len);

//
// Set font
// This function will create a new font tag and the new inserted string is setted
// to the new font. Old strings will not change.
// @param name      : font name
// @param color     : foreground color
// @param size      : font size
// @param a, b, c   : bold? italic? underline?
//
void qq_chat_textview_set_font(GtkWidget *widget, const gchar *name
                                                , const gchar *color
                                                , gint size
                                                , gint a, gint b, gint c);

//
// Set the default font the the text view widget
//
void qq_chat_textview_set_default_font(GtkWidget *widget, const gchar *name
                                                , const gchar *color
                                                , gint size
                                                , gint a, gint b, gint c);

//
// Get the QQMsgContent of the text view.
// Save them in the contents.
// Return the number of the contents
//
gint qq_chat_textview_get_msg_contents(GtkWidget *widget, GPtrArray *contents);
#endif

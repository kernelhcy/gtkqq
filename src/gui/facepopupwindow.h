#ifndef __QQ_FACE_POPUP_WINDOW_H_
#define __QQ_FACE_POPUP_WINDOW_H_

#include <gtk/gtk.h>
#include <qq.h>

#define QQ_TYPE_FACE_POPUP_WINDOW       qq_face_popup_window_get_type()
#define QQ_FACE_POPUP_WINDOW(obj)	    G_TYPE_CHECK_INSTANCE_CAST(obj\
                                                , QQ_TYPE_FACE_POPUP_WINDOW\
						                        , QQFacePopupWindow)
#define QQ_FACE_POPUP_WINDOWCLASS(c)	G_TYPE_CHECK_CLASS_CAST(c\
						                        , QQ_TYPE_FACE_POPUP_WINDOW\
						                        , QQFacePopupWindowClass)
#define QQ_IS_FACE_POPUP_WINDOW(obj)	G_TYPE_CHECK_INSTANCE_TYPE(obj\
                                                , QQ_TYPE_FACE_POPUP_WINDOW)

typedef struct __QQFacePopupWindow 		QQFacePopupWindow;
typedef struct __QQFacePopupWindowClass	QQFacePopupWindowClass;

struct __QQFacePopupWindow{
	GtkTextView parent;
};

struct __QQFacePopupWindowClass{
	GtkTextViewClass parent;

    void (*face_clicked_default_handler)(gpointer instance
                                        , gint face
                                        , gpointer usrdata);
    guint face_clicked_signal_id;
};

GtkWidget* qq_face_popup_window_new();
GType qq_face_popup_window_get_type();

void qq_face_popup_window_popup(GtkWidget *win, gint x, gint y);
#endif



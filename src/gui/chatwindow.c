#include <chatwindow.h>
#include <qq.h>
#include <gqqconfig.h>
#include <stdlib.h>

extern QQInfo *info;
extern GQQConfig *cfg;

static void qq_chatwindow_init(QQChatWindow *win);
static void qq_chatwindowclass_init(QQChatWindowClass *wc);

/*
 * The handler of "destroy" singal
 */
static void destroy_handler(GtkWidget *widget, gpointer  data)
{
    g_debug("Destroy chat window.(%s, %d)", __FILE__, __LINE__);
    return;
}

GType qq_chatwindow_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQChatWindowClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_chatwindowclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQChatWindow),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_chatwindow_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_WINDOW, "QQChatWindow"
                    , &info, 0);
    }
    return t;
}

GtkWidget* qq_chatwindow_new()
{
    return GTK_WIDGET(g_object_new(qq_chatwindow_get_type()
                        , "type", GTK_WINDOW_TOPLEVEL, NULL));
}


static void qq_chatwindow_init(QQChatWindow *win)
{
    GtkWidget *w = GTK_WIDGET(win);
    gtk_widget_set_size_request(w, 500, 500);
    gtk_window_resize(GTK_WINDOW(w), 550, 550);

    g_signal_connect(G_OBJECT(w), "destroy",
                             G_CALLBACK(destroy_handler), NULL);

}
static void qq_chatwindowclass_init(QQChatWindowClass *wc)
{

}


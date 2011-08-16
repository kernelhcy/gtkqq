#include <chatwindow.h>
#include <gqqconfig.h>
#include <stdlib.h>
#include <chattextview.h>
#include <facepopupwindow.h>
#include <tray.h>
#include <msgloop.h>
#include <gdk/gdkkeysyms.h>

extern QQInfo *info;
extern GQQConfig *cfg;
extern QQTray *tray;
extern GQQMessageLoop *send_loop;

static void qq_group_chatwindow_init(QQGroupChatWindow *win);
static void qq_group_chatwindowclass_init(QQGroupChatWindowClass *wc);

enum{
    QQ_GROUP_CHATWINDOW_PROPERTY_UIN = 1,
    QQ_GROUP_CHATWINDOW_PROPERTY_NAME,
    QQ_GROUP_CHATWINDOW_PROPERTY_GRPNUMBER,
    QQ_CHATWINDOW_PROPERTY_UNKNOWN
};

static guint scale_255(guint v)
{
    guint re = (guint)(v / 65535.0 * 255.0 + 0.5);
    re = re < 0 ? 0 : re;
    re = re > 255 ? 255 : re;
    return re;
}

//
// Private members
//
typedef struct{
    gchar uin[100];
    gchar grpnumber[100];
    GString *name;

    GtkWidget *body_vbox;

    GtkWidget *faceimage;
    GtkWidget *name_label, *lnick_label;

    // The message text area
    GtkWidget *message_textview;

    // Font tool box
    GtkWidget *font_tool_box;
    GtkWidget *font_cb, *size_cb, *bold_btn, *italic_btn
                        , *underline_btn, *color_btn;

    // Tool bar
    GtkWidget *tool_bar;
    GtkToolItem *font_item, *face_item, *sendfile_item
                , *sendpic_item, *clear_item, *history_item;
    GtkWidget *facepopupwindow;

    GtkWidget *input_textview;
    
    GtkWidget *send_btn, *close_btn;
}QQGroupChatWindowPriv;

GType qq_group_chatwindow_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQGroupChatWindowClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_group_chatwindowclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQGroupChatWindow),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_group_chatwindow_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_WINDOW, "QQGroupChatWindow"
                                        , &info, 0);
    }
    return t;
}

GtkWidget* qq_group_chatwindow_new(const gchar *uin, const gchar *name
                            , const gchar *grpnumber)
{
    return GTK_WIDGET(g_object_new(qq_chatwindow_get_type()
                                    , "type", GTK_WINDOW_TOPLEVEL
                                    , "grpnumber", grpnumber
                                    , "uin", uin
                                    , "name", name
                                    , NULL));
}


#include <facepopupwindow.h>
#include <gqqconfig.h>

extern QQInfo *info;
extern GQQConfig *cfg;

static void qq_face_popup_window_init(QQFacePopupWindow *win);
static void qq_face_popup_windowclass_init(QQFacePopupWindowClass *klass);

typedef struct{
}QQFacePopupWindowPriv;

GType qq_face_popup_window_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQFacePopupWindowClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc)qq_face_popup_windowclass_init,
            NULL,   /* class finalize*/
            NULL,   /* class data */
            sizeof(QQFacePopupWindow),
            0,      /* n pre allocs */
            (GInstanceInitFunc)qq_face_popup_window_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_WINDOW, "QQFacePopupWindow"
                    , &info, 0);
    }
    return t;
}

GtkWidget* qq_face_popup_window_new()
{
    return GTK_WIDGET(g_object_new(qq_face_popup_window_get_type()
                        , "type", GTK_WINDOW_TOPLEVEL, NULL));
}
//
// Focus out event handler
//
static gboolean qq_face_popup_window_focus_out(GtkWidget *widget
		                                        , GdkEvent* event
                                                , gpointer data)
{
    gtk_widget_hide(GTK_WIDGET(data));
	return TRUE;
}

//
// Face clicked event handler parameter
//
struct FaceClickedPar
{
    gint face;
    GtkWidget *win;
};

//
// Face clicked event handler
//
static gboolean face_popup_window_clicked(GtkWidget *widget
                                            , GdkEvent *event
                                            , gpointer data)
{
    struct FaceClickedPar *par = data;
    g_debug("Clicked face %d.gif (%s, %d)", par -> face, __FILE__, __LINE__);
    gtk_widget_hide(par -> win);
    return FALSE;
}

static gboolean face_enter_notify_event(GtkWidget *widget
                                            , GdkEvent *event
                                            , gpointer data)
{
    GtkFrame *frame = data;
    gtk_frame_set_shadow_type(frame, GTK_SHADOW_OUT);
    GdkWindow *gdkwin = gtk_widget_get_window(widget);
    gdk_window_set_cursor(gdkwin, gdk_cursor_new(GDK_HAND1));
    return FALSE;
}
static gboolean face_leave_notify_event(GtkWidget *widget
                                            , GdkEvent *event
                                            , gpointer data)
{
    GtkFrame *frame = data;
    gtk_frame_set_shadow_type(frame, GTK_SHADOW_ETCHED_IN);
    GdkWindow *gdkwin = gtk_widget_get_window(widget);
    gdk_window_set_cursor(gdkwin, NULL);
    return FALSE;
}
static void qq_face_popup_window_init(QQFacePopupWindow *win)
{
	gtk_window_set_decorated(GTK_WINDOW(win) , FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(win), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_default_size(GTK_WINDOW(win) , 450 , 250);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(win), TRUE);

	g_signal_connect(win, "focus-out-event"
			        , GTK_SIGNAL_FUNC(qq_face_popup_window_focus_out), win);

    GtkWidget *table, *img, *eventbox, *frame;
    gint i, j, k;
    table = gtk_table_new(7, 15, TRUE);
    gchar path[500];
    struct FaceClickedPar *par;

    k = 0;
    for(i = 0; i < 7; ++i){
        for(j = 0; j < 15; ++j){
            g_snprintf(path, 500, IMGDIR"/qqfaces/%d.gif", k);
            ++k;
            img = gtk_image_new_from_file(path);
            eventbox = gtk_event_box_new();
            gtk_widget_set_events(eventbox, GDK_ALL_EVENTS_MASK);
            gtk_container_add(GTK_CONTAINER(eventbox), img);
            par = g_slice_new0(struct FaceClickedPar);
            par -> face = k;
            par -> win = GTK_WIDGET(win);
            g_signal_connect(eventbox , "button-release-event" 
                                , GTK_SIGNAL_FUNC(face_popup_window_clicked)
                                , par);
            frame = gtk_frame_new(NULL);
            gtk_container_add(GTK_CONTAINER(frame), eventbox);
            g_signal_connect(eventbox , "enter-notify-event" 
                                , GTK_SIGNAL_FUNC(face_enter_notify_event)
                                , frame);
            g_signal_connect(eventbox , "leave-notify-event" 
                                , GTK_SIGNAL_FUNC(face_leave_notify_event)
                                , frame);
			gtk_table_attach_defaults(GTK_TABLE(table), frame
                                            , j, j + 1, i, i + 1);
        }
    }

    frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(frame) , table);
	gtk_container_add(GTK_CONTAINER(win) , frame);
}

static void qq_face_popup_windowclass_init(QQFacePopupWindowClass *klass)
{

}

void qq_face_popup_window_popup(GtkWidget *win, gint x, gint y)
{
    gtk_window_move(GTK_WINDOW(win), x - 30, y - 280);
    gtk_widget_show_all(GTK_WIDGET(win));
}

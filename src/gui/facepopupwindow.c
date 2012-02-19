#include <facepopupwindow.h>
#include <gqqconfig.h>

static void qq_face_popup_window_init(QQFacePopupWindow *win);
static void qq_face_popup_windowclass_init(QQFacePopupWindowClass *klass);

static gint face_transfer_table[] = {14, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
                                    , 12, 13, 0, 50, 51, 96, 53, 54, 73
                                    , 74, 75, 76, 77, 78, 55, 56, 57, 58
                                    , 79, 80, 81, 82, 83, 84, 85, 86, 87
                                    , 88, 97, 98, 99, 100, 101, 102, 103
                                    , 104, 105, 106, 107, 108, 109, 110, 111
                                    , 112, 32, 113, 114, 115, 63, 64, 59, 33
                                    , 34, 116, 36, 37, 38, 91, 92, 93, 29, 117
                                    , 72, 45, 42, 39, 62, 46, 47, 71, 95, 118
                                    , 119, 120, 121, 122, 123, 124, 27, 21, 23
                                    , 25, 26, 125, 126, 127, 128, 129, 130
                                    , 131, 132, 133, 134, 52, 24, 22, 20, 60
                                    , 61, 89, 90, 31, 94, 65, 35, 66, 67, 68
                                    , 69, 70, 15, 16, 17, 18, 19, 28, 30, 40
                                    , 41, 43, 44, 48, 49};
typedef struct {
} QQFacePopupWindowPriv;

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
    g_signal_emit_by_name(par -> win, "face-clicked", par -> face);
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
	/*gtk_window_set_decorated(GTK_WINDOW(win) , FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(win), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_default_size(GTK_WINDOW(win) , 450 , 250);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(win), TRUE);

    */
	g_signal_connect(G_OBJECT(win), "focus-out-event", 
			G_CALLBACK(qq_face_popup_window_focus_out), win);

    GtkWidget *table, *img, *eventbox, *frame;
    gint i, j, k;
    table = gtk_table_new(7, 15, TRUE);
    gchar path[500];
    struct FaceClickedPar *par;

    k = 0;
    for(i = 0; i < 7; ++i){
        for(j = 0; j < 15; ++j){
            g_snprintf(path, 500, IMGDIR"/qqfaces/%d.gif", k);
            img = gtk_image_new_from_file(path);
            eventbox = gtk_event_box_new();
            gtk_widget_set_events(eventbox, GDK_ALL_EVENTS_MASK);
            gtk_container_add(GTK_CONTAINER(eventbox), img);
            par = g_slice_new0(struct FaceClickedPar);
            par -> face = face_transfer_table[k];
            par -> win = GTK_WIDGET(win);
            ++k;
            g_signal_connect(eventbox , "button-release-event", 
			    G_CALLBACK(face_popup_window_clicked), par);
            frame = gtk_frame_new(NULL);
            gtk_container_add(GTK_CONTAINER(frame), eventbox);
            g_signal_connect(eventbox , "enter-notify-event",
			    G_CALLBACK(face_enter_notify_event), frame);
            g_signal_connect(eventbox , "leave-notify-event",
			    G_CALLBACK(face_leave_notify_event), frame);
			gtk_table_attach_defaults(GTK_TABLE(table), frame
                                            , j, j + 1, i, i + 1);
        }
    }

    frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(frame) , table);
	gtk_container_add(GTK_CONTAINER(win) , frame);
}

//
// Default signal handler
//
static void default_handler(gpointer instance, gint face
                                    , gpointer usr_data)
{
    //do nothing.
    return;
}
static void qq_face_popup_windowclass_init(QQFacePopupWindowClass *klass)
{
    //install the 'face-clicked' signal
    klass -> face_clicked_default_handler = default_handler;
    //void signal_handler(gpointer instance, gint face, gpointer data)
    klass -> face_clicked_signal_id = 
            g_signal_new("face-clicked"
                , G_TYPE_FROM_CLASS(klass) 
                , G_SIGNAL_RUN_LAST     //run after the default handler
                , G_STRUCT_OFFSET(QQFacePopupWindowClass
                                , face_clicked_default_handler)
                , NULL, NULL            //no used
                , g_cclosure_marshal_VOID__INT
                , G_TYPE_NONE
                , 1, G_TYPE_INT);
}

void qq_face_popup_window_popup(GtkWidget *win, gint x, gint y)
{
    gtk_window_move(GTK_WINDOW(win), x - 30, y - 280);
    gtk_widget_show_all(GTK_WIDGET(win));
}

#include <statusbutton.h>
#include <consts.h>

static QQStatusButtonClass *this_class = NULL;

static void qq_statusbutton_init(QQStatusButton *btn);
static void qq_statusbuttonclass_init(QQStatusButtonClass *c);

static gboolean expose_event_cb(GtkWidget *widget
			, GdkEventExpose *event, gpointer data);
static void change_img_cb(GtkComboBox *w, gpointer data);

static gboolean enter_event_cb(GtkWidget *w, GdkEventCrossing *event
		, gpointer data);
static gboolean leave_event_cb(GtkWidget *w, GdkEventCrossing *event
		, gpointer data);

GType qq_statusbutton_get_type()
{
	static GType t = 0;
	if(!t){
		static const GTypeInfo info={
			sizeof(QQStatusButtonClass),
			NULL,
			NULL,
			(GClassInitFunc)qq_statusbuttonclass_init,
			NULL,
			NULL,
			sizeof(QQStatusButton),
			0,
			(GInstanceInitFunc)qq_statusbutton_init,
			NULL
		};
		t = g_type_register_static(GTK_TYPE_COMBO_BOX, "QQStatusButton"
					, &info, 0);
	}
	return t;
}

/*
 * Create the treemode for the status combo box.
 */
static GtkTreeModel* create_model()
{
	GtkTreeIter iter;
	GtkListStore *store;
	gint i;
        
	store = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	for(i = 0; i < 4; i++){
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, this_class -> pb[i]
					, 1, status_label[i], -1);
	}
	return GTK_TREE_MODEL(store);
}

GtkWidget* qq_statusbutton_new()
{
	gpointer btn = g_object_new(QQ_TYPE_STATUSBUTTON, NULL);
	return GTK_WIDGET(btn);	
}

static void qq_statusbutton_init(QQStatusButton *btn)
{
	gtk_widget_set_size_request(GTK_WIDGET(btn), 20, 20);
	gtk_combo_box_set_model(GTK_COMBO_BOX(btn), create_model());
	GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(btn)
					, renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(btn), renderer
					,"pixbuf", 0, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(btn)
						, renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(btn), renderer 
					,"text", 1, NULL); 

	GdkEventMask event_mask = 0;
	event_mask |= GDK_ENTER_NOTIFY_MASK;
	event_mask |= GDK_LEAVE_NOTIFY_MASK;
	gtk_widget_add_events(GTK_WIDGET(btn), event_mask);

	g_signal_connect(GTK_WIDGET(btn), "expose-event"
			, G_CALLBACK(expose_event_cb), NULL);
	g_signal_connect(GTK_WIDGET(btn), "changed"
			, G_CALLBACK(change_img_cb), NULL);
	g_signal_connect(GTK_WIDGET(btn), "enter-notify-event"
			, G_CALLBACK(enter_event_cb), NULL);
	g_signal_connect(GTK_WIDGET(btn), "leave-notify-event"
			, G_CALLBACK(leave_event_cb), NULL);

	btn -> status = STATUS_ONLINE;
	gtk_combo_box_set_active(GTK_COMBO_BOX(btn), STATUS_ONLINE);
}

static void qq_statusbuttonclass_init(QQStatusButtonClass *c)
{
	c -> pb[STATUS_ONLINE] = gdk_pixbuf_new_from_file_at_scale(
					IMGDIR"/status/online.png"
					, 10, 10, TRUE, NULL);
	c -> pb[STATUS_HIDDEN] = gdk_pixbuf_new_from_file_at_scale(
					IMGDIR"/status/hidden.png"
					, 10, 10, TRUE, NULL);
	c -> pb[STATUS_OFFLINE] = gdk_pixbuf_new_from_file_at_scale(
					IMGDIR"/status/offline.png"
					, 10, 10, TRUE, NULL);
	c -> pb[STATUS_AWAY] = gdk_pixbuf_new_from_file_at_scale(
					IMGDIR"/status/away.png"
					, 10, 10, TRUE, NULL);
	c -> hand_cursor = gdk_cursor_new(GDK_HAND1);	
	this_class = c;
}

/*
 * 
 */
static gboolean expose_event_cb(GtkWidget *widget
			, GdkEventExpose *event, gpointer data)
{
	cairo_t *ct = gdk_cairo_create(widget -> window);
	GdkPixbuf *pb = this_class -> pb[QQ_STATUSBUTTON(widget) -> status];
	gint pbw, pbh;
	pbw = gdk_pixbuf_get_width(pb);
	pbh = gdk_pixbuf_get_height(pb);
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	gint alignx, aligny;
	alignx = (alloc.width - pbw) / 2;
	aligny = (alloc.height - pbh) / 2;
	
	//maybe they are -
	alignx = (alignx > 0 ? alignx : 0);
	aligny = (aligny > 0 ? aligny : 0);

	gdk_cairo_set_source_pixbuf(ct, pb
			, alloc.x + alignx 
			, alloc.y + aligny);
	cairo_paint(ct);
	cairo_destroy(ct);

	/*
	 * Stop the signal transferring.
	 * Or not, the normal UI will cover the image.
	 */
	return TRUE;
}

const gchar* qq_statusbutton_get_status_string(QQStatusButton *btn)
{
	if(btn == NULL){
		return NULL;
	}

	gint idx = gtk_combo_box_get_active(GTK_COMBO_BOX(btn));
	return status_label_en[idx];
}

QQStatusButtonStatus qq_statusbutton_get_status(QQStatusButton *btn)
{
	if(btn == NULL){
		return STATUS_UNKNOWN;
	}

	gint idx = gtk_combo_box_get_active(GTK_COMBO_BOX(btn));
	return idx;
}

/*
 * `changed` signal handler
 * Change the shown image 
 */
static void change_img_cb(GtkComboBox *w, gpointer data)
{
	QQStatusButton *btn = QQ_STATUSBUTTON(w);
	btn -> status = gtk_combo_box_get_active(w);

	//Maybe this combo box has not been realized.
	//So, we must test. If not be realized, just return.
	if(gtk_widget_get_window(GTK_WIDGET(w)) == NULL){
		return;
	}

	expose_event_cb(GTK_WIDGET(w), NULL, NULL);
}

static gboolean enter_event_cb(GtkWidget *w, GdkEventCrossing *event
		, gpointer data)
{
	GdkWindow *win = gtk_widget_get_window(w);
	if(win != NULL){
		//Set to hand1 cursor.
		gdk_window_set_cursor(win, this_class -> hand_cursor);
	}
	return FALSE;
}
static gboolean leave_event_cb(GtkWidget *w, GdkEventCrossing *event
		, gpointer data)
{
	GdkWindow *win = gtk_widget_get_window(w);
	if(win != NULL){
		//Set to default cursor.
		gdk_window_set_cursor(win, NULL);
	}
	return FALSE;
}

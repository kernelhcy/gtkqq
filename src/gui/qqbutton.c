#include <qqbutton.h>
#include <constval.h>

static void qq_button_init		(QQButton *btn, gpointer data);
static void qq_button_class_init	(QQButtonClass *bclass, gpointer data);
static void qq_button_set_type		(QQButton *btn, QQButtonType type);

static gboolean enter_notify_event_cb	(GtkWidget *widget
					, GdkEventCrossing *event
					, gpointer data);
static gboolean leave_notify_event_cb	(GtkWidget *widget
					, GdkEventCrossing *event
					, gpointer data);

static gboolean expose_event_cb		(GtkWidget *widget
					, GdkEventExpose *event
					, gpointer data);

static gboolean button_press_event_cb	(GtkWidget *widget, GdkEventButton *event
					, gpointer data);
static gboolean button_release_event_cb	(GtkWidget *widget, GdkEventButton *event
					, gpointer data);

QQButton* qq_button_new(QQButtonType type)
{
	QQButton *btn;
	btn = g_object_new(qq_button_get_type(), NULL);

	/*
	 * Set the type.
	 * Maybe this should be done in the ini function.
	 * But I don't want to use the property.
	 */
	qq_button_set_type(btn, type);
	
	return btn;
}

GtkType qq_button_get_type()
{
	static GType type = 0;
	if(type == 0){
		static const GTypeInfo type_info = 
		{
			sizeof(QQButtonClass),
			NULL,
			NULL,
			(GClassInitFunc)qq_button_class_init,
			NULL,
			NULL,
			sizeof(QQButton),
			0,
			(GInstanceInitFunc)qq_button_init
		};

		type = g_type_register_static(GTK_TYPE_BUTTON
					, "QQButton"
					, &type_info
					, (GTypeFlags)0);
	}
	return type;
}

void qq_button_set_type(QQButton *btn, QQButtonType type)
{
	QQButtonClass *c = QQ_BUTTON_GET_CLASS(btn);
	switch(type)
	{
	case QQ_BUTTON_NORMAL:
		/* normal button, do nothing */
		break;
	case QQ_BUTTON_CLOSE:
		btn -> nor_img = c -> closebtn_nor_img;
		btn -> hl_img = c -> closebtn_hl_img;
		btn -> push_img = c -> closebtn_push_img;
		
		btn -> curr_img = btn -> nor_img;
		btn -> type = type;
		break;
	case QQ_BUTTON_MIN:
		btn -> nor_img = c -> minbtn_nor_img;
		btn -> hl_img = c -> minbtn_hl_img;
		btn -> push_img = c -> minbtn_push_img;
		
		btn -> curr_img = btn -> nor_img;
		btn -> type = type;
		break;
	case QQ_BUTTON_MAX:
		btn -> nor_img = c -> maxbtn_nor_img;
		btn -> hl_img = c -> maxbtn_hl_img;
		btn -> push_img = c -> maxbtn_push_img;
		
		btn -> curr_img = btn -> nor_img;
		btn -> type = type;
		break;
	case QQ_BUTTON_RESTORE:
		btn -> nor_img = c -> restorebtn_nor_img;
		btn -> hl_img = c -> restorebtn_hl_img;
		btn -> push_img = c -> restorebtn_push_img;
		btn -> curr_img = btn -> nor_img;
		btn -> type = type;
		break;
	case QQ_BUTTON_UNKNOWN:
	default:
		break;
	}
	GtkImage *img = GTK_IMAGE(btn -> curr_img);
	GdkPixbuf *pb = gtk_image_get_pixbuf(img);

	gint w,h;
	w = gdk_pixbuf_get_width(pb);
	h = gdk_pixbuf_get_height(pb);
	gtk_widget_set_size_request(GTK_WIDGET(btn), w, h);
}
void qq_button_init(QQButton *btn, gpointer data)
{
	g_signal_connect(GTK_WIDGET(btn), "enter-notify-event"
			, G_CALLBACK(enter_notify_event_cb), NULL);
	g_signal_connect(GTK_WIDGET(btn), "leave-notify-event"
			, G_CALLBACK(leave_notify_event_cb), NULL);
	g_signal_connect(GTK_WIDGET(btn), "button-press-event"
			, G_CALLBACK(button_press_event_cb), NULL);
	g_signal_connect(GTK_WIDGET(btn), "button-release-event"
			, G_CALLBACK(button_release_event_cb), NULL);

	g_signal_connect(GTK_WIDGET(btn), "expose-event"
			, G_CALLBACK(expose_event_cb), NULL);

	btn -> curr_img = NULL;
	btn -> nor_img = NULL;
	btn -> hl_img = NULL;
	btn -> push_img = NULL;

	btn -> alloc = g_slice_new(GtkAllocation);
	
	return;
}

void qq_button_class_init(QQButtonClass *bclass, gpointer data)
{
	bclass -> closebtn_nor_img = gtk_image_new_from_file(
			IMGDIR""CLOSEBTN_NOR_IMG);
	bclass -> closebtn_hl_img = gtk_image_new_from_file(
			IMGDIR""CLOSEBTN_HL_IMG);
	bclass -> closebtn_push_img = gtk_image_new_from_file(
			IMGDIR""CLOSEBTN_PUSH_IMG);
	bclass -> minbtn_nor_img = gtk_image_new_from_file(
			IMGDIR""MINBTN_NOR_IMG);
	bclass -> minbtn_hl_img = gtk_image_new_from_file(
			IMGDIR""MINBTN_HL_IMG);
	bclass -> minbtn_push_img = gtk_image_new_from_file(
			IMGDIR""MINBTN_PUSH_IMG);
	bclass -> maxbtn_nor_img = gtk_image_new_from_file(
			IMGDIR""MAXBTN_NOR_IMG);
	bclass -> maxbtn_hl_img = gtk_image_new_from_file(
			IMGDIR""MAXBTN_HL_IMG);
	bclass -> maxbtn_push_img = gtk_image_new_from_file(
			IMGDIR""MAXBTN_PUSH_IMG);
	bclass -> restorebtn_nor_img = gtk_image_new_from_file(
			IMGDIR""RESTOREBTN_NOR_IMG);
	bclass -> restorebtn_hl_img = gtk_image_new_from_file(
			IMGDIR""RESTOREBTN_HL_IMG);
	bclass -> restorebtn_push_img = gtk_image_new_from_file(
			IMGDIR""RESTOREBTN_PUSH_IMG);
}


gboolean enter_notify_event_cb(GtkWidget *widget
				, GdkEventCrossing *event, gpointer data)
{
	QQButton *btn = QQ_BUTTON(widget);
	if(btn -> curr_img == btn -> hl_img){
		return FALSE;
	}

	btn -> curr_img = btn -> hl_img;
	expose_event_cb(widget, NULL, NULL);
	return FALSE;
}

gboolean leave_notify_event_cb(GtkWidget *widget
				, GdkEventCrossing *event, gpointer data)
{
	QQButton *btn = QQ_BUTTON(widget);
	if(btn -> curr_img == btn -> nor_img){
		return FALSE;
	}

	btn -> curr_img = btn -> nor_img;
	expose_event_cb(widget, NULL, NULL);
	return FALSE;
}

gboolean button_press_event_cb(GtkWidget *widget, GdkEventButton *event
				, gpointer data)
{
	if(event -> button != 1){
		return FALSE;
	}
	
	QQButton *btn = QQ_BUTTON(widget);
	btn -> curr_img = btn -> push_img;
	expose_event_cb(widget, NULL, NULL);
	return FALSE;
}
gboolean button_release_event_cb(GtkWidget *widget, GdkEventButton *event
				, gpointer data)
{
	if(event -> button != 1){
		return FALSE;
	}

	QQButton *btn = QQ_BUTTON(widget);
	btn -> curr_img = btn -> hl_img;
	expose_event_cb(widget, NULL, NULL);
	return FALSE;
}
gboolean expose_event_cb(GtkWidget *widget
			, GdkEventExpose *event, gpointer data)
{
	QQButton *btn = QQ_BUTTON(widget);
	GtkImage *img = btn -> curr_img;
	cairo_t *ct = gdk_cairo_create(widget -> window);
	GdkPixbuf *pb = gtk_image_get_pixbuf(img);

	gtk_widget_get_allocation(widget, btn -> alloc);
	gdk_cairo_set_source_pixbuf(ct, pb, btn -> alloc -> x
					, btn -> alloc -> y);
	cairo_paint(ct);
	cairo_destroy(ct);

	/*
	 * Stop the signal transferring.
	 * Or not, the normal UI will cover the image.
	 */
	return TRUE;
}


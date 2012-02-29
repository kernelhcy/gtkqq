#include <chatwidget.h>
#include <gqqconfig.h>
#include <stdlib.h>
#include <chattextview.h>
#include <facepopupwindow.h>

extern QQInfo *info;
extern GQQConfig *cfg;

static void qq_chatwidget_init(QQChatWidget *win);
static void qq_chatwidgetclass_init(QQChatWidgetClass *wc);

static const gchar *font_names[] = {"宋体", "黑体", "隶书", "微软雅黑"
                                    , "楷体_GB2312", "幼圆", "Arial"
                                    , "Arial Black", "Times New Roman"
                                    , "Verdana", NULL}; 
static guint scale_255(guint v)
{
    guint re = (guint)(v / 65535.0 * 255.0 + 0.5);
	//re = re < 0 ? 0 : re;  clang output error: comparison of unsigned expression < 0 is always false
    re = re > 255 ? 255 : re;
    return re;
}

//
// Private members
//
typedef struct{
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
}QQChatWidgetPriv;

GType qq_chatwidget_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQChatWidgetClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_chatwidgetclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQChatWidget),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_chatwidget_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_VBOX, "QQChatWidget"
                                        , &info, 0);
    }
    return t;
}

GtkWidget* qq_chatwidget_new()
{
    return GTK_WIDGET(g_object_new(qq_chatwidget_get_type()
                                    , "spacing", 0
                                    , "homogeneous", FALSE
                                    , NULL));
}

//
// Create a toggle button with stock
//
static GtkWidget* qq_toggle_button_new_with_stock(const gchar *stock_id)
{
    GtkWidget *btn = gtk_toggle_button_new();
    if(stock_id != NULL){
        GtkWidget *img = gtk_image_new_from_stock(stock_id
                                    , GTK_ICON_SIZE_LARGE_TOOLBAR);
        gtk_button_set_image(GTK_BUTTON(btn), img);
    }
    return btn;
}

//
// Face tool button clicked handler
//
static void face_tool_button_clicked(GtkToolButton *btn, gpointer data)
{
    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    int x , y , ex , ey , root_x , root_y;

    // Get the top level window
    GtkWidget *parent = GTK_WIDGET(btn), *tmp;
    while((tmp = gtk_widget_get_parent(parent)) != NULL){
        parent = tmp;
    }

    gtk_widget_translate_coordinates(GTK_WIDGET(btn), parent, 0, 0, &ex, &ey);
    gtk_window_get_position(GTK_WINDOW(parent), &root_x, &root_y);
    x = root_x + ex + 2;
    y = root_y + ey + 45;
    qq_face_popup_window_popup(priv -> facepopupwindow, x, y);
}

//
// Clear the message text view
//
static void clear_button_clicked(GtkToolButton *btn, gpointer data)
{
    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    qq_chat_textview_clear(priv -> message_textview);
}
//
// font tool button func
//
static void qq_chat_view_font_button_clicked(GtkToggleToolButton *btn, gpointer data)
{
    QQChatWidget *widget = data;
    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    if(gtk_toggle_tool_button_get_active(btn)){
        gtk_box_pack_start(GTK_BOX(widget), priv -> font_tool_box
                                            , FALSE, FALSE, 0); 
        gtk_box_reorder_child(GTK_BOX(widget), priv -> font_tool_box, 1);
    }else{
        gtk_container_remove(GTK_CONTAINER(widget)
                                            , priv -> font_tool_box);
    }
}

static void qq_chat_widget_font_changed(GtkWidget *widget, gpointer data)
{
    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(data
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    gchar *name = NULL, color[20], *sizestr = NULL;
    gint size, a = 0, b = 0, c = 0;

    name = gtk_combo_box_text_get_active_text(
                        GTK_COMBO_BOX_TEXT(priv -> font_cb));
    if(name == NULL){
        return;
    }
    
    GdkColor gc;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(priv -> color_btn), &gc);
    g_snprintf(color, 20, "#%02X%02X%02X", scale_255(gc.red), scale_255(gc.green)
                                    , scale_255(gc.blue));
    g_debug("Set text view color %s (%u,%u,%u)(%s, %d)", color, gc.red
                                , gc.green, gc.blue, __FILE__, __LINE__);

    sizestr = gtk_combo_box_text_get_active_text(
                        GTK_COMBO_BOX_TEXT(priv -> size_cb));
    if(sizestr == NULL){
        goto out_label;
    }
    size = (gint)strtol(sizestr, NULL, 10);

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> bold_btn))){
        a = 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> italic_btn))){
        b = 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> underline_btn))){
        c = 1;
    }
    qq_chat_textview_set_default_font(priv -> input_textview, 
                                        name, color, size, a, b, c);

out_label:
    g_free(name);
    g_free(sizestr);
}

//
// Face clicked
//
static void qq_chatwidget_face_clicked(gpointer instance, gint face
                                            , gpointer data)
{
    QQChatWidgetPriv *priv = data;
    qq_chat_textview_add_face(priv -> input_textview, face);
}

static void qq_chatwidget_init(QQChatWidget *widget)
{
    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    GtkWidget *scrolled_win; 
    gchar buf[100];

	GtkWidget *paned=gtk_vpaned_new();
	GtkWidget* vbox=gtk_vbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(widget),paned, TRUE,TRUE,0); 
	gtk_paned_set_position(GTK_PANED(paned),250);
	gtk_paned_pack2(GTK_PANED(paned),vbox,TRUE,TRUE);
    // message text view
    priv -> message_textview = qq_chat_textview_new(); 
    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win)
                                        , GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(scrolled_win), priv -> message_textview);
	gtk_paned_pack1(GTK_PANED(paned),scrolled_win,TRUE,FALSE);

    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv -> message_textview)
                                                , FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv -> message_textview)
                                                , GTK_WRAP_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(priv -> message_textview)
                                                , FALSE);

    // font tools
    priv -> font_tool_box = gtk_hbox_new(FALSE, 5);
    g_object_ref(priv -> font_tool_box);
    priv -> font_cb = gtk_combo_box_text_new();
    gint i;
    for(i = 0; font_names[i] != NULL; ++i){
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv -> font_cb)
                                        , font_names[i]);
    }
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> font_cb
                                        , FALSE, FALSE, 0); 
    priv -> size_cb = gtk_combo_box_text_new();
    for(i = 8; i < 23; ++i){
        g_snprintf(buf, 10, "%d", i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv -> size_cb)
                                        , buf);
    }
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> size_cb
                                        , FALSE, FALSE, 0); 
    priv -> bold_btn = qq_toggle_button_new_with_stock(GTK_STOCK_BOLD);
    priv -> italic_btn = qq_toggle_button_new_with_stock(GTK_STOCK_ITALIC);
    priv -> underline_btn = qq_toggle_button_new_with_stock(
                                            GTK_STOCK_UNDERLINE);
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> bold_btn 
                                        , FALSE, FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> italic_btn
                                        , FALSE, FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> underline_btn
                                        , FALSE, FALSE, 0); 
    priv -> color_btn = gtk_color_button_new();
    gtk_box_pack_start(GTK_BOX(priv -> font_tool_box), priv -> color_btn
                                        , FALSE, FALSE, 0); 
    gtk_widget_show_all(priv -> font_tool_box);

    g_signal_connect(G_OBJECT(priv -> font_cb), "changed"
                    , G_CALLBACK(qq_chat_widget_font_changed), widget);
    g_signal_connect(G_OBJECT(priv -> size_cb), "changed"
                    , G_CALLBACK(qq_chat_widget_font_changed), widget);
    g_signal_connect(G_OBJECT(priv -> bold_btn), "toggled"
                        , G_CALLBACK(qq_chat_widget_font_changed), widget);
    g_signal_connect(G_OBJECT(priv -> italic_btn), "toggled"
                        , G_CALLBACK(qq_chat_widget_font_changed), widget);
    g_signal_connect(G_OBJECT(priv -> underline_btn), "toggled"
                        , G_CALLBACK(qq_chat_widget_font_changed), widget);
    g_signal_connect(G_OBJECT(priv -> color_btn), "color-set"
                            , G_CALLBACK(qq_chat_widget_font_changed), widget);


    // tool bar
    priv -> tool_bar = gtk_toolbar_new();
    GtkWidget *img = NULL;

    img = gtk_image_new_from_file(IMGDIR"/selectfont.png");
    priv -> font_item = gtk_toggle_tool_button_new();
    g_signal_connect(G_OBJECT(priv -> font_item), "toggled"
                             , G_CALLBACK(qq_chat_view_font_button_clicked)
                             , widget);
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(priv -> font_item), img);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> font_item, -1);

    img = gtk_image_new_from_file(IMGDIR"/selectface.png");
    priv -> face_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> face_item, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar)
                                , gtk_separator_tool_item_new() , -1);
    g_signal_connect(G_OBJECT(priv -> face_item), "clicked",
                             G_CALLBACK(face_tool_button_clicked), widget);

    img = gtk_image_new_from_file(IMGDIR"/sendfile.png");
    priv -> sendfile_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> sendfile_item, -1);

    img = gtk_image_new_from_file(IMGDIR"/sendpic.png");
    priv -> sendpic_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> sendpic_item, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar)
                                , gtk_separator_tool_item_new() , -1);

    img = gtk_image_new_from_file(IMGDIR"/clearscreen.png");
    priv -> clear_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> clear_item, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar)
                                , gtk_separator_tool_item_new() , -1);
    g_signal_connect(G_OBJECT(priv -> clear_item), "clicked",
                             G_CALLBACK(clear_button_clicked), widget);

    img = gtk_image_new_from_file(IMGDIR"/showhistory.png");
    priv -> history_item = gtk_tool_button_new(img, NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(priv -> tool_bar), priv -> history_item, -1);
    gtk_box_pack_start(GTK_BOX(vbox), priv -> tool_bar
                                                , FALSE, FALSE, 0); 

    // input text view
    priv -> input_textview = qq_chat_textview_new(); 
    gtk_text_view_set_indent(GTK_TEXT_VIEW(priv -> input_textview), 1);
    scrolled_win= gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_win),
                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win)
                                        , GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(scrolled_win), priv -> input_textview);
//#ifndef USE_UNITY
//    gtk_box_pack_start(GTK_BOX(vbox), scrolled_win, FALSE, FALSE, 0);
//#else
	/* On ubuntu unity, there is a fuck bug if we pack the textview
	 with the argument FALSE, FALSE, it cant be shown, how fuck it is. */
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_win, TRUE, TRUE, 0);
//#endif	/* USE_UNITY */
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv -> input_textview)
                                                , GTK_WRAP_CHAR);

    gtk_combo_box_set_active(GTK_COMBO_BOX(priv -> font_cb), 1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv -> size_cb), 3);

    gtk_widget_grab_focus(priv -> input_textview);

    priv -> facepopupwindow = qq_face_popup_window_new();
    g_signal_connect(G_OBJECT(priv -> facepopupwindow), "face-clicked"
                                , G_CALLBACK(qq_chatwidget_face_clicked)
                                , priv);

}

static void qq_chatwidget_finalize(GObject *obj)
{
    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(obj
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    gtk_widget_destroy(priv -> facepopupwindow);
    // chain up
    GObjectClass *klass = (GObjectClass*)g_type_class_peek_parent(
                                g_type_class_peek(qq_chatwidget_get_type()));
    klass -> finalize(obj);
}

static void qq_chatwidgetclass_init(QQChatWidgetClass *wc)
{
    g_type_class_add_private(wc, sizeof(QQChatWidgetPriv));
    G_OBJECT_CLASS(wc) -> finalize = qq_chatwidget_finalize;
}


void qq_chatwidget_add_send_message(GtkWidget *widget, QQSendMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);

    qq_chat_textview_add_send_message(priv -> message_textview, msg);
}
void qq_chatwidget_add_recv_message(GtkWidget *widget, QQRecvMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    qq_chat_textview_add_recv_message(priv -> message_textview, msg);
}

GtkWidget * qq_chatwidget_get_message_textview(GtkWidget *widget)
{
    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    return priv -> message_textview;
}

GtkWidget * qq_chatwidget_get_input_textview(GtkWidget *widget)
{
    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    return priv -> input_textview;
}

QQMsgContent* qq_chatwidget_get_font(GtkWidget *widget)
{
    QQChatWidgetPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget
                                        , qq_chatwidget_get_type()
                                        , QQChatWidgetPriv);
    // font
    gchar *name = NULL, color[20] , *sizestr = NULL;
    gint size, a = 0, b = 0, c = 0;

    name = gtk_combo_box_text_get_active_text(
                        GTK_COMBO_BOX_TEXT(priv -> font_cb));
    if(name == NULL){
        name = "宋体";
    }
    GdkColor gc;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(priv -> color_btn), &gc);
    g_snprintf(color, 20, "%02X%02X%02X", scale_255(gc.red), scale_255(gc.green)
                                    , scale_255(gc.blue));

    sizestr = gtk_combo_box_text_get_active_text(
                        GTK_COMBO_BOX_TEXT(priv -> size_cb));
    if(sizestr == NULL){
        size = 11;
    }else{
        size = (gint)strtol(sizestr, NULL, 10);
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> bold_btn))){
        a = 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> italic_btn))){
        b = 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv -> underline_btn))){
        c = 1;
    }
    QQMsgContent *font = qq_msgcontent_new(QQ_MSG_CONTENT_FONT_T, name, size
                                                , color, a, b, c);
    g_free(name);
    g_free(sizestr);
    return font;
}

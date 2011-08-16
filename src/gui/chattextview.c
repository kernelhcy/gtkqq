#include <chattextview.h>
#include <gqqconfig.h>
#include <stdlib.h>

extern QQInfo *info;
extern GQQConfig *cfg;

static void qq_chat_textview_init(QQChatTextview *view);
static void qq_chat_textviewclass_init(QQChatTextviewClass *klass);

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
//
// Widget type.
//
typedef enum{
    QQ_WIDGET_FACE_T,
    QQ_WIDGET_UNKNOWN_T
}QQWidgetType;
//
// The marks of the widget in the text buffer.
//
typedef struct{
    GtkTextMark *begin, *end;
    QQWidgetType type;
    union{
        gint face;
        gpointer p;
    }data;
}QQWidgetMark;

typedef struct{
    //QQMsgFont array
    GPtrArray *msgfonts;
    gint cur_font_index;        // current font's index of msgfonts
    //
    // Default under line flag
    //
    gboolean default_underline;

    GPtrArray *widget_marks;;

    GtkTextMark *end_mark, *inserted_left_mark;
}QQChatTextviewPriv;

GType qq_chat_textview_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQChatTextviewClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc)qq_chat_textviewclass_init,
            NULL,       /* class finalize*/
            NULL,       /* class data */
            sizeof(QQChatTextview),
            0,          /* n pre allocs */
            (GInstanceInitFunc)qq_chat_textview_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_TEXT_VIEW, "QQChatTextview"
                    , &info, 0);
    }
    return t;
}

GtkWidget* qq_chat_textview_new()
{
    return GTK_WIDGET(g_object_new(qq_chat_textview_get_type(), NULL));
}

//
// Create the tags for the text buffer
//
static void qq_chatwindow_text_buffer_create_tags(GtkTextBuffer *textbuf)
{
	gtk_text_buffer_create_tag(textbuf, "blue", "foreground", "blue", NULL);
	gtk_text_buffer_create_tag(textbuf, "grey", "foreground", "grey", NULL);
	gtk_text_buffer_create_tag(textbuf, "green", "foreground", "darkgreen", NULL);
	gtk_text_buffer_create_tag(textbuf, "red", "foreground", "red", NULL);
	gtk_text_buffer_create_tag(textbuf, "underline", "underline"
                                    , PANGO_UNDERLINE_SINGLE, NULL);
    gtk_text_buffer_create_tag(textbuf, "left_margin1", "left_margin", 5, NULL);
    gtk_text_buffer_create_tag(textbuf, "left_margin2", "left_margin", 20, NULL);
}

//
// Applay the default under line
//
static void qq_chat_buffer_insert_text(GtkTextBuffer *textbuf
                                        , GtkTextIter *pos
                                        , gchar *text
                                        , gint len
                                        , gpointer data)
{
    QQChatTextviewPriv *priv = data;
    if(priv -> default_underline){
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(textbuf, &start);
        gtk_text_buffer_get_end_iter(textbuf, &end);
        gtk_text_buffer_apply_tag_by_name(textbuf, "underline", &start, &end);
    }
}
//
// Add message 
// @param name : the name of the sender
// @param contents : QQMsgContent instance array
// @param time : the time. ms
// @param color_tag : the color tag name
//
static void qq_chat_textview_add_message(QQChatTextview *view
                                , const gchar *name
                                , GPtrArray *contents
                                , const gchar *time
                                , const gchar *color_tag)
{
    if(name == NULL || contents == NULL || time == NULL){
        return;
    }
    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    view, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);

    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    GtkTextIter end_iter;
    gint64 timev = (gint64)strtoll(time, NULL, 10);
    GDateTime *t = g_date_time_new_from_unix_local(timev);
    gchar head[500];
    if(gtk_text_buffer_get_line_count(textbuf) > 1){
        // insert new line
        gtk_text_buffer_get_end_iter(textbuf, &end_iter);
        gtk_text_buffer_insert(textbuf, &end_iter, "\n", -1);
    }
    // insert head
    gint head_len;
    head_len = g_snprintf(head, 500, "%s %d-%d-%d %d:%d:%d\n", name
                                , g_date_time_get_year(t)
                                , g_date_time_get_month(t)
                                , g_date_time_get_day_of_month(t)
                                , g_date_time_get_hour(t)
                                , g_date_time_get_minute(t)
                                , g_date_time_get_second(t));
    gtk_text_buffer_get_end_iter(textbuf, &end_iter);
    gtk_text_buffer_insert_with_tags_by_name(textbuf
                                , &end_iter
                                , head, head_len
                                , color_tag, "left_margin1",  NULL);

    gint i;
    QQMsgContent *cent;
    QQMsgFont *font = NULL;
    // find the font
    for(i = 0; i < contents -> len; ++i){
        cent = g_ptr_array_index(contents, i);
        if(cent == NULL){
            continue;
        }
        if(cent -> type == QQ_MSG_CONTENT_FONT_T){
            font = cent -> value.font;
            break;
        }
    }
    if(font != NULL){
        qq_chat_textview_set_font(GTK_WIDGET(view)
                                , font -> name -> str
                                , font -> color -> str
                                , font -> size
                                , font -> style.a
                                , font -> style.b
                                , font -> style.c);
    }
    for(i = 0; i < contents -> len; ++i){
        cent = g_ptr_array_index(contents, i);
        if(cent == NULL){
            continue;
        }
        switch(cent -> type)
        {
        case QQ_MSG_CONTENT_FACE_T:         // face
            if(cent -> value.face > 134){
                break;
            }
            qq_chat_textview_add_face(GTK_WIDGET(view), cent -> value.face);
            break;
        case QQ_MSG_CONTENT_STRING_T:         // string
            qq_chat_textview_add_string(GTK_WIDGET(view)
                                        , cent -> value.str -> str
                                        , cent -> value.str -> len);
            break;
        default:
            break;
        }
    }

    // insert new line
    gtk_text_buffer_get_end_iter(textbuf, &end_iter);
    gtk_text_buffer_insert(textbuf, &end_iter, "\n", -1);
    // scroll the text view to the end
    gtk_text_iter_set_line_offset(&end_iter, 0);
    gtk_text_buffer_move_mark(textbuf, priv -> end_mark, &end_iter);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(view)
                                            , priv -> end_mark);
    return;
}

static void qq_chat_textview_init(QQChatTextview *view)
{
	GtkTextBuffer *textbuf = gtk_text_view_get_buffer(
                                    GTK_TEXT_VIEW(view));
    qq_chatwindow_text_buffer_create_tags(textbuf);

    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    view, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);
    GtkTextIter iter;
	gtk_text_buffer_get_end_iter(textbuf, &iter);
	priv -> end_mark = gtk_text_buffer_create_mark(textbuf, "scrollend"
                                                    , &iter, FALSE);
	priv -> inserted_left_mark = gtk_text_buffer_create_mark(textbuf
                                                    , "insertedleft"
                                                    , &iter, TRUE);
    priv -> msgfonts = g_ptr_array_new();
    priv -> cur_font_index = -1;
    priv -> widget_marks = g_ptr_array_new();

    //
    // Connect the insert-text signal to apply the underline tag
    //
    g_signal_connect_after(G_OBJECT(textbuf), "insert-text"
                            , G_CALLBACK(qq_chat_buffer_insert_text), priv);
}

static void qq_chat_textviewclass_init(QQChatTextviewClass *klass)
{
    g_type_class_add_private(klass, sizeof(QQChatTextviewPriv));
}

void qq_chat_textview_add_send_message(GtkWidget *widget, QQSendMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }
    gchar buf[100];
    GTimeVal now;
    g_get_current_time(&now);
    g_snprintf(buf, 100, "%ld", now.tv_sec);
    qq_chat_textview_add_message(QQ_CHAT_TEXTVIEW(widget)
                                , info -> me -> nick -> str
                                , msg -> contents, buf, "blue");
}
void qq_chat_textview_add_recv_message(GtkWidget *widget, QQRecvMsg *msg)
{
    if(widget == NULL || msg == NULL){
        return;
    }

    QQBuddy *bdy = qq_info_lookup_buddy_by_uin(info, msg -> from_uin -> str);
    const gchar *name = NULL;
    if(bdy == NULL){
        name = msg -> from_uin -> str;
    }else{
        name = bdy -> markname -> str;
        if(bdy -> markname -> len <= 0){
            name = bdy -> nick -> str;
        }
    }
    qq_chat_textview_add_message(QQ_CHAT_TEXTVIEW(widget)
                                , name
                                , msg -> contents, msg -> time -> str
                                , "green");
}

void qq_chat_textview_add_face(GtkWidget *widget, gint face)
{
    if(widget == NULL || face > 134){
        return;
    }
    gint i;
    gint real_face = face;
    for(i = 0; i < 135; ++i){
        if(face_transfer_table[i] == face){
            real_face = i;
            break;
        }
    }

    QQWidgetMark *mark = g_slice_new0(QQWidgetMark);
    gchar path[500];
    g_snprintf(path, 500, IMGDIR"/qqfaces/%d.gif", real_face);
    GtkWidget *img = gtk_image_new_from_file(path);
    gtk_widget_show(img);
    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    GtkTextIter iter;
    GtkTextChildAnchor *anchor;
    gtk_text_buffer_get_end_iter(textbuf, &iter);
    mark -> begin =  gtk_text_buffer_create_mark(textbuf, NULL, &iter, TRUE);
    anchor = gtk_text_buffer_create_child_anchor(textbuf, &iter);
    gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(widget), img, anchor);
    gtk_text_buffer_get_end_iter(textbuf, &iter);
    mark -> end =  gtk_text_buffer_create_mark(textbuf, NULL, &iter, TRUE);

    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_iter_at_mark(textbuf, &start_iter, mark -> begin);
    gtk_text_buffer_get_iter_at_mark(textbuf, &end_iter, mark -> end);
    gtk_text_buffer_apply_tag_by_name(textbuf, "left_margin2"
                                    , &start_iter, &end_iter);

    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    widget, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);
    mark -> data.face = face;
    mark -> type = QQ_WIDGET_FACE_T;
    g_ptr_array_add(priv -> widget_marks, mark);
}

void qq_chat_textview_add_string(GtkWidget *widget, const gchar *str, gint len)
{
    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    widget, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    gtk_text_buffer_get_end_iter(textbuf, &end_iter);
    gtk_text_buffer_move_mark(textbuf, priv -> inserted_left_mark
                                        , &end_iter);
    gtk_text_buffer_insert_with_tags_by_name(textbuf, &end_iter, str, len
                                                , "left_margin2", NULL);
    // apply font
    gchar tagname[100];
    if(priv -> cur_font_index >= 0){
        g_snprintf(tagname, 100, "msgfonttag%d", priv -> cur_font_index);
        gtk_text_buffer_get_iter_at_mark(textbuf, &start_iter
                                        , priv -> inserted_left_mark);
        gtk_text_buffer_apply_tag_by_name(textbuf, tagname 
                                                , &start_iter
                                                , &end_iter);
    }
}

void qq_chat_textview_set_font(GtkWidget *widget, const gchar *name
                                                , const gchar *color
                                                , gint size
                                                , gint a, gint b, gint c)
{
    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    widget, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);
    QQMsgFont *font = NULL;
    gint i;
    for(i = 0; i < priv -> msgfonts -> len; ++i){
        font = g_ptr_array_index(priv -> msgfonts, i);
        if(g_strcmp0(name, font -> name -> str) == 0
                    && g_strcmp0(color, font -> color -> str) == 0
                    && size == font -> size
                    && a == font -> style.a
                    && b == font -> style.b
                    && c == font -> style.c){
            break;
        }
    }
    if(i < priv -> msgfonts -> len){
        // We already have this font tag. Just use it.
        priv -> cur_font_index = i;
        return;
    }

    // Create the font tag
    gchar fonttagname[100];
    gchar colorstr[100];
    g_snprintf(fonttagname, 100, "msgfonttag%d", priv -> msgfonts -> len);
    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    g_snprintf(colorstr, 100, "#%s", color);
    GtkTextTag *tag = gtk_text_buffer_create_tag(textbuf, fonttagname
                                                , "family", name
                                                , "foreground", colorstr
                                                , "size", size * PANGO_SCALE
                                                , NULL);
    if(a == 1){
        g_object_set(tag, "weight", PANGO_WEIGHT_BOLD, NULL);
    }
    if(b == 1){
        g_object_set(tag, "style", PANGO_STYLE_ITALIC, NULL);
    }
    if(c == 1){
        g_object_set(tag, "underline", PANGO_UNDERLINE_SINGLE, NULL);
    }
    // save the font
    g_ptr_array_add(priv -> msgfonts, qq_msgfont_new(name, size, color
                                                    , a, b, c));
    priv -> cur_font_index = priv -> msgfonts -> len - 1;
    g_debug("Create font tag %s : %s %s %d %d %d %d(%s, %d)"
                        , fonttagname, name, color, size, a, b, c
                        , __FILE__, __LINE__);
}

void qq_chat_textview_set_default_font(GtkWidget *widget, const gchar *name
                                                , const gchar *color
                                                , gint size
                                                , gint a, gint b, gint c)
{
    if(widget == NULL){
        return;
    }

    gchar buf[100];
    PangoFontDescription *desc = NULL;

    if(color != NULL){
        g_snprintf(buf, 100, color[0] == '#' ? "%s":"#%s", color);
        GdkColor cc;
        if(gdk_color_parse(buf, &cc)){
            gtk_widget_modify_text(widget, GTK_STATE_NORMAL, &cc);
        }else{
            g_warning("Wrong color: %s (%s, %d)", buf, __FILE__, __LINE__);
        }
    }

    g_snprintf(buf, 100, "%s %s %s %d", name == NULL ? "" : name
                                        , a == 1 ? "bold" : ""
                                        , b == 1 ? "italic" : ""
                                        , size);
    desc = pango_font_description_from_string(buf);
    gtk_widget_modify_font(widget, desc);
    pango_font_description_free(desc);

    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    widget, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);
    priv -> default_underline = (c == 1 ? TRUE : FALSE); 
    GtkTextIter start, end;
    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    gtk_text_buffer_get_start_iter(textbuf, &start);
    gtk_text_buffer_get_end_iter(textbuf, &end);
    if(priv -> default_underline){
        gtk_text_buffer_apply_tag_by_name(textbuf, "underline", &start, &end);
    }else{
        gtk_text_buffer_remove_tag_by_name(textbuf, "underline", &start, &end);
    }
    return;
}
void qq_chat_textview_clear(GtkWidget *widget)
{
    if(widget == NULL){
        return;
    }
    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    widget, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);
    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    gtk_text_buffer_set_text(textbuf, "", -1);

    gint i;
    QQWidgetMark *mark;
    for(i = 0; i < priv -> widget_marks -> len; ++i){
        mark = g_ptr_array_index(priv -> widget_marks, i);
        gtk_text_buffer_delete_mark(textbuf, mark -> begin);
        gtk_text_buffer_delete_mark(textbuf, mark -> end);
        g_slice_free(QQWidgetMark, mark);
    }
    if(priv -> widget_marks -> len > 0){
        g_ptr_array_remove_range(priv -> widget_marks, 0
                                , priv -> widget_marks -> len);
    }
    return;
}

gint qq_chat_textview_get_msg_contents(GtkWidget *widget, GPtrArray *contents)
{
    if(widget == NULL || contents == NULL){
        return 0;
    }

    QQChatTextviewPriv *priv  = G_TYPE_INSTANCE_GET_PRIVATE(
                                    widget, qq_chat_textview_get_type()
                                    , QQChatTextviewPriv);
    GtkTextBuffer *textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    if(gtk_text_buffer_get_char_count(textbuf) <= 0){
        // no input message
        return 0;
    }
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(textbuf, &start_iter);
    gint i;
    QQWidgetMark *mark;
    gchar *text;
    QQMsgContent *cent;
    for(i = 0; i < priv -> widget_marks -> len; ++i){
        mark = g_ptr_array_index(priv -> widget_marks, i);
        if(mark == NULL){
            continue;
        }
        gtk_text_buffer_get_iter_at_mark(textbuf, &end_iter, mark -> begin);
        // get text between two widgets
        text = gtk_text_buffer_get_text(textbuf, &start_iter, &end_iter, FALSE);
        cent = qq_msgcontent_new(QQ_MSG_CONTENT_STRING_T, text);
        g_ptr_array_add(contents, cent);
        g_free(text);
        switch(mark -> type)
        {
        case QQ_WIDGET_FACE_T:
            cent = qq_msgcontent_new(QQ_MSG_CONTENT_FACE_T, mark -> data.face);
            g_ptr_array_add(contents, cent);
            break;
        default:
            g_warning("Unknown chat text view widget type! %d (%s, %d)"
                            , mark -> type, __FILE__, __LINE__);
            break;
        }
        gtk_text_buffer_get_iter_at_mark(textbuf, &start_iter, mark -> end);
    }

    // add the last text
    gtk_text_buffer_get_end_iter(textbuf, &end_iter);
    text = gtk_text_buffer_get_text(textbuf, &start_iter, &end_iter, FALSE);
    cent = qq_msgcontent_new(QQ_MSG_CONTENT_STRING_T, text);
    g_ptr_array_add(contents, cent);
    g_free(text);
    return contents -> len;
}

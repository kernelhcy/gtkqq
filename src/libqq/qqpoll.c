/*
 * Poll to receive message.
 */
#include <qqtypes.h>
#include <qq.h>
#include <glib.h>
#include <url.h>
#include <http.h>
#include <qqhosts.h>
#include <json.h>
#include <string.h>
#include <unicode.h>
#include <glib/gprintf.h>
#include <glib.h>
#include <stdlib.h>

static const gchar* find_value(json_t *json, const gchar *name)
{
    json_t *tmp = NULL;
    tmp = json_find_first_label_all(json, name);
    if(tmp == NULL){
        g_warning("No such name %s!(%s, %d)", name, __FILE__, __LINE__);
        return "";
    }

    return tmp -> child -> text;
}

static void parase_content(QQRecvMsg *msg, json_t *json)
{
    json_t *tmp = NULL;
    GString *utf8 = g_string_new("");
    GString *tmpstring = g_string_new("");
    gint i;
    tmp = json_find_first_label_all(json, "content");
    if(tmp == NULL){
        g_warning("No conent found!(%s, %d)", __FILE__, __LINE__);
        return;
    }

    tmp = tmp -> child;
    json_t *ctent;
    QQMsgContent *ct = NULL;
    for(ctent = tmp -> child; ctent != NULL; ctent = ctent -> next){
        if(ctent -> type == JSON_STRING){
            //String
            g_string_truncate(utf8, 0);
            g_string_truncate(tmpstring, 0);
            ucs4toutf8(utf8, ctent -> text);
            for(i = 0; i < utf8 -> len; ++i){
                if(utf8 -> str[i] == '\\' && i + 1 < utf8 -> len){
                    switch(utf8 -> str[i + 1])
                    {
                    case '\\':
                        g_string_append_c(tmpstring, '\\');
                        break;
                    case 'n':
                        g_string_append_c(tmpstring, '\n');
                        break;
                    case 'r':
                        g_string_append_c(tmpstring, '\r');
                        break;
                    case 't':
                        g_string_append_c(tmpstring, '\t');
                        break;
                    case '"':
                        g_string_append_c(tmpstring, '"');
                        break;
                    default:
                        break;
                    }
                    ++i;
                }else{
                    g_string_append_c(tmpstring, utf8 -> str[i]);
                }
            }
            ct = qq_msgcontent_new(QQ_MSG_CONTENT_STRING_T, tmpstring -> str); 
            qq_recvmsg_add_content(msg, ct);
            g_debug("Msg Content: string %s(%s, %d)", utf8 -> str
                            , __FILE__, __LINE__);
        }else if(ctent -> type == JSON_ARRAY){
            if(g_strcmp0(ctent -> child -> text, "font") == 0){
                const gchar *name, *color;
                gint size, sa, sb, sc;
                //font name
                tmp = json_find_first_label_all(ctent, "name");
                if(tmp == NULL){
                    g_warning("No font name found!(%s, %d)", __FILE__, __LINE__);
                    name = "Arial";
                }else{
                    name = tmp -> child -> text;
                }
                g_string_truncate(utf8, 0);
                ucs4toutf8(utf8, name);
                name = utf8 -> str;
                //font color
                tmp = json_find_first_label_all(ctent, "color");
                if(tmp == NULL){
                    g_warning("No font color found!(%s, %d)", __FILE__, __LINE__);
                    color = "000000";
                }else{
                    color = tmp -> child -> text;
                }
                //font size
                tmp = json_find_first_label_all(ctent, "size");
                if(tmp == NULL){
                    g_warning("No font size found!(%s, %d)", __FILE__, __LINE__);
                    size = 12;
                }else{
                    size = (gint)strtol(tmp -> child -> text, NULL, 10);
                }
                //font style
                tmp = json_find_first_label_all(ctent, "style");
                if(tmp == NULL){
                    g_warning("No font style found!(%s, %d)", __FILE__, __LINE__);
                    sa = 0;
                    sb = 0;
                    sc = 0;
                }else{
                    json_t *style = tmp -> child -> child;
                    const gchar *stylestr = style -> text;
                    sa = (gint)strtol(stylestr, NULL, 10);
                    style = style -> next;
                    stylestr = style -> text;
                    sb = (gint)strtol(stylestr, NULL, 10);
                    style = style -> next;
                    stylestr = style -> text;
                    sc = (gint)strtol(stylestr, NULL, 10);
                }
                ct = qq_msgcontent_new(QQ_MSG_CONTENT_FONT_T, name, size
                                                    , color, sa, sb, sc);
                qq_recvmsg_add_content(msg, ct);
                g_debug("Msg Content: font %s %d %s %d%d%d(%s, %d)", name
                        , size, color, sa, sb, sc, __FILE__, __LINE__);
            }else if(g_strcmp0(ctent -> child -> text, "face") == 0){
                gint facenum = (gint)strtol(ctent -> child -> next -> text
                                                            , NULL, 10);
                ct = qq_msgcontent_new(QQ_MSG_CONTENT_FACE_T, facenum);
                qq_recvmsg_add_content(msg, ct);
                g_debug("Msg Content: face %d(%s, %d)", facenum
                                        , __FILE__, __LINE__);
            }
        }else{
            g_warning("Unknown content!(%s, %d)", __FILE__, __LINE__);
        }
    }
    g_string_free(utf8, TRUE);
    g_string_free(tmpstring, TRUE);
}


static QQRecvMsg* create_buddy_message(QQInfo *info, json_t *json)
{
    QQRecvMsg *msg = qq_recvmsg_new(info, MSG_BUDDY_T);
#define SET_VALUE(x)    qq_recvmsg_set(msg, x, find_value(json, x))
    SET_VALUE("msg_id");
    SET_VALUE("from_uin");
    SET_VALUE("to_uin");
    SET_VALUE("msg_id2");
    SET_VALUE("reply_ip");
    SET_VALUE("time");
//    SET_VALUE("raw_content");
#undef SET_VALUE
    const gchar *msg_type = find_value(json, "msg_type");
    gint mt = (gint)strtol(msg_type, NULL, 10);
    msg -> type = mt;

    parase_content(msg, json);
    return msg;
}

static QQRecvMsg* create_group_message(QQInfo *info, json_t *json)
{

    QQRecvMsg *msg = qq_recvmsg_new(info, MSG_GROUP_T);
#define SET_VALUE(x)    qq_recvmsg_set(msg, x, find_value(json, x))
    SET_VALUE("msg_id");
    SET_VALUE("from_uin");
    SET_VALUE("to_uin");
    SET_VALUE("msg_id2");
    SET_VALUE("reply_ip");
    SET_VALUE("time");
//    SET_VALUE("raw_content");
    SET_VALUE("group_code");
    SET_VALUE("send_uin");
#undef SET_VALUE
    GString *utf8 = g_string_new("");
    ucs4toutf8(utf8, msg -> raw_content -> str);
    qq_recvmsg_set(msg, "raw_content", utf8 -> str);
    g_string_free(utf8, TRUE);

    const gchar *msg_type = find_value(json, "msg_type");
    gint mt = (gint)strtol(msg_type, NULL, 10);
    msg -> type = mt;

    const gchar *info_seq = find_value(json, "info_seq");
    gint seq = (gint)strtol(info_seq, NULL, 10);
    msg -> info_seq = seq;

    parase_content(msg, json);
    return msg;
}

static QQRecvMsg* create_status_change_message(QQInfo *info, json_t *json)
{

    QQRecvMsg *msg = qq_recvmsg_new(info, MSG_STATUS_CHANGED_T);
#define SET_VALUE(x)    qq_recvmsg_set(msg, x, find_value(json, x))
    SET_VALUE("uin");
    SET_VALUE("status");
    SET_VALUE("client_type");
#undef SET_VALUE

    g_debug("Status change: %s %s %s(%s, %d)", msg -> uin -> str
                        , msg -> status -> str
                        , msg -> client_type -> str
                        , __FILE__, __LINE__);
    return msg;
}

static QQRecvMsg* create_kick_message(QQInfo *info, json_t *json)
{

    QQRecvMsg *msg = qq_recvmsg_new(info, MSG_KICK_T);
    GString *reason = g_string_new(NULL);
    const gchar *rn = find_value(json, "reason");
    ucs4toutf8(reason, rn);
    QQMsgContent *cent = qq_msgcontent_new(QQ_MSG_CONTENT_STRING_T
                            , reason -> str);
    g_string_free(reason, TRUE);
    qq_recvmsg_add_content(msg, cent);
    return msg;
}

static QQRecvMsg* create_recvmsg(QQInfo *info, json_t *msg)
{
    if(msg == NULL){
        return NULL;
    }

    json_t *tmp = json_find_first_label_all(msg, "poll_type");
    if(tmp == NULL){
        return NULL;
    }

    const gchar *type = tmp -> child -> text;
    if(g_strcmp0(type, "message") == 0){
        //buddy message
        return create_buddy_message(info, msg);
    }else if(g_strcmp0(type, "buddies_status_change") == 0){
        //buddy status change
        return create_status_change_message(info, msg);
    }else if(g_strcmp0(type, "group_message") == 0){
        //group message
        return create_group_message(info, msg);
    }else if(g_strcmp0(type, "kick_message") == 0){
        //kick message
        return create_kick_message(info, msg);
    }
    return NULL;
}

struct Par{
    QQInfo *info;
    QQPollCallBack cb;
    gpointer data;
};

#if GLIB_CHECK_VERSION(2,32,0)
static GMutex  lock_impl;
static GMutex *lock = &lock_impl;
#else
static GMutex *lock = NULL;
#endif
static gboolean run = TRUE;
static gpointer do_poll(gpointer data)
{
    struct Par *par = (struct Par*)data;
    QQInfo *info = par -> info;
    QQPollCallBack cb = par -> cb;
    gpointer usrdata = par -> data;
    g_slice_free(struct Par, par);

    gchar params[300];
	gint res = 0;
again:
    g_mutex_lock(lock);
    if(!run){
        g_mutex_unlock(lock);
        return NULL;
    }
    g_mutex_unlock(lock);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "GET");
    request_set_version(req, "HTTP/1.1");
    g_sprintf(params, POLLPATH"?clientid=%s&psessionid=%s&t=%ld"
            , info -> clientid -> str 
            , info -> psessionid -> str, get_now_millisecond());
    request_set_uri(req, params);
    request_set_default_headers(req);
    request_add_header(req, "Host", POLLHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Content-Type", "utf-8");
    request_add_header(req, "Referer"
            , "http://"POLLHOST"/proxy.html?v=20101025002");

    rps = NULL;    //the again label needs this.
    g_debug("poll.(%s, %d)", __FILE__, __LINE__);
    Connection *con = connect_to_host(POLLHOST, 80);
    if(con == NULL){
        g_warning("Can NOT connect to server!(%s, %d)"
                , __FILE__, __LINE__);
        request_del(req);
        goto again;
    }

    send_request(con, req);
    res = rcv_response(con, &rps);
    close_con(con);
    connection_free(con);

	if (-1 == res || !rps) {
		g_warning("Null point access (%s, %d)\n", __FILE__, __LINE__);
		response_del(rps);
        request_del(req);
		goto again;
	}
    gchar *retstatus = rps -> status -> str;
    if(g_strstr_len(retstatus, -1, "200") == NULL){
        /*
         * Maybe some error occured.
         */
        g_warning("Resoponse status is NOT 200, but %s (%s, %d)"
                , retstatus, __FILE__, __LINE__);
        response_del(rps);
        request_del(req);
        goto again;
    }

    json_t *json = NULL;
    switch(json_parse_document(&json, rps -> msg -> str))
    {
    case JSON_OK:
        break;
    default:
        g_warning("json_parser_document: syntax error. (%s, %d)"
                , __FILE__, __LINE__);
        json_free_value(&json);
        response_del(rps);
        request_del(req);
        goto again;
    }
    
    g_debug("MSG: %s", rps -> msg -> str);

    json_t *msg_array = json_find_first_label_all(json, "result");
    if(msg_array == NULL){
        json_free_value(&json);
        response_del(rps);
        request_del(req);
        goto again;
    }
    json_t *msg = msg_array -> child -> child;
    if(msg == NULL){
        json_free_value(&json);
        response_del(rps);
        request_del(req);
        goto again;
    }
    for(; msg != NULL; msg = msg -> next){
        QQRecvMsg *rmsg = create_recvmsg(info, msg);
        if(cb != NULL){
            cb(rmsg, usrdata);
        }
    }

    response_del(rps);
    request_del(req);
    goto again;
}


gint qq_start_poll(QQInfo *info, QQPollCallBack cb, gpointer data, GError **err)
{
    if(info == NULL){
        if(err != NULL){
            *err = g_error_new(g_quark_from_string("GtkQQ"), PARAMETER_ERR
                            , "info == NULL");
        }
        return -1;
    }
#if !GLIB_CHECK_VERSION(2,32,0)
    if(!g_thread_supported()){
        g_thread_init(NULL);
    }

    lock = g_mutex_new();
#else
    g_mutex_init(&lock_impl);
#endif
    run = TRUE;

    struct Par *par= g_slice_new0(struct Par);
    par -> info = info;
    par -> cb = cb;
    par -> data = data;

#if GLIB_CHECK_VERSION(2,32,0)
    GThread *thread = g_thread_new("", do_poll, NULL);
#else
    GThread *thread = g_thread_create(do_poll, par, FALSE, err);
#endif
    if(thread == NULL){
        g_error("Create poll thread failed... (%s, %d)", __FILE__, __LINE__);
        return -1;
    }
    info -> pollthread = thread;
    return 0;
}
void qq_stop_poll(QQInfo *info)
{
    if(info == NULL){
        return;
    }
   
    g_mutex_lock(lock);
    run = FALSE;
    g_mutex_unlock(lock);
#if GLIB_CHECK_VERSION(2,32,0)
    g_mutex_clear(&lock_impl);
#else
    g_mutex_free(lock);
#endif
    info -> pollthread = NULL;
}

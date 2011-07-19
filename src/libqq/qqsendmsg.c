#include <qqtypes.h>
#include <qq.h>
#include <glib.h>
#include <url.h>
#include <http.h>
#include <qqhosts.h>
#include <json.h>
#include <string.h>

static gint do_send_msg(QQInfo *info, QQSendMsg *msg, GError **err)
{
    GString *uin = msg -> to_uin;

    gint ret_code = 0;
    gchar params[3000];
    g_debug("Send msg to %s!(%s, %d)", uin -> str, __FILE__, __LINE__);
    const gchar *path;
    path = msg -> type == 0 ? MSGFRIPATH : MSGGRPPATH;

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "POST");
    request_set_version(req, "HTTP/1.1");
    request_set_uri(req, path);
    request_set_default_headers(req);
    request_add_header(req, "Host", MSGHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Origin", "http://d.web2.qq.com");
    request_add_header(req, "Content-Type", 
            "application/x-www-form-urlencoded");
    request_add_header(req, "Referer"
            , "http://"MSGHOST"/proxy.html?v=20101025002");
    GString *content;
    if(msg -> type == 0){
        content = qq_sendmsg_contents_tostring(msg);
        g_snprintf(params, 3000, "r={\"to\":%s,\"face\":%s,"
                "%s,\"msg_id\":%d,"
                "\"clientid\":\"%s\",\"psessionid\":\"%s\"}"
                , uin -> str, msg -> face -> str
                , content -> str
                , (info -> msg_id)++    
                , info -> clientid -> str
                , info -> psessionid -> str);
        g_string_free(content, TRUE);
    }else{
        content = qq_sendmsg_contents_tostring(msg);
        g_snprintf(params, 3000, "r={\"group_uin\":\"%s\","
                "%s,\"msg_id\":%d,"
                "\"clientid\":\"%s\",\"psessionid\":\"%s\"}"
                , uin -> str, content -> str
                , (info -> msg_id)++    
                , info -> clientid -> str
                , info -> psessionid -> str);
        g_string_free(content, TRUE);
    }
    gchar *euri = g_uri_escape_string(params, "=", FALSE);
    request_append_msg(req, euri, strlen(euri));
    g_free(euri);
    g_snprintf(params, 3000, "&clientid=%s&psessionid=%s"
                    , info -> clientid -> str, info -> psessionid -> str);
    request_append_msg(req, params, strlen(params));
    g_snprintf(params, 3000, "%d", strlen(req -> msg -> str));
    request_add_header(req, "Content-Length", params);

    Connection *con = connect_to_host(MSGHOST, 80);
    if(con == NULL){
        g_warning("Can NOT connect to server!(%s, %d)"
                , __FILE__, __LINE__);
        request_del(req);
        return -1;
    }

    send_request(con, req);
    rcv_response(con, &rps);
    close_con(con);
    connection_free(con);

    const gchar *retstatus = rps -> status -> str;
    json_t *json = NULL;
    if(g_strstr_len(retstatus, -1, "200") == NULL){
        /*
         * Maybe some error occured.
         */
        g_warning("Resoponse status is NOT 200, but %s (%s, %d)"
                , retstatus, __FILE__, __LINE__);
        ret_code = -1;
        goto error;
    }

    switch(json_parse_document(&json, rps -> msg -> str))
    {
    case JSON_OK:
        break;
    default:
        g_warning("json_parser_document: syntax error. (%s, %d)"
                , __FILE__, __LINE__);
    }

    json_t *val = json_find_first_label_all(json, "result");
    if(val != NULL){
        val = val -> child;
        if(g_strstr_len(val -> text, -1, "ok") == NULL){
            g_warning("Server return error. %s (%s, %d)"
                    , val -> text, __FILE__, __LINE__);
        }
    }else{
        g_warning("Server return: (%s, %d)%s", __FILE__, __LINE__
                , rps -> msg -> str);
    }
    json_free_value(&json);
error:
    request_del(req);
    response_del(rps);
    return ret_code;
}

gint qq_send_message(QQInfo *info, QQSendMsg *msg, GError **err)
{
    if(info == NULL || msg == NULL){
        g_warning("info or msg == NULL. (%s, %d)", __FILE__
                , __LINE__);
        return -1;
    }
    
    return do_send_msg(info, msg, err);  
}

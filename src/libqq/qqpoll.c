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

static gint do_poll(QQInfo *info, GError **err)
{
    gint ret_code = 0;
    gchar params[300];

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
        return -1;
    }

    send_request(con, req);
    rcv_response(con, &rps);
    close_con(con);
    connection_free(con);

    gchar *retstatus = rps -> status -> str;
    if(g_strstr_len(retstatus, -1, "200") == NULL){
        /*
         * Maybe some error occured.
         */
        g_warning("Resoponse status is NOT 200, but %s (%s, %d)"
                , retstatus, __FILE__, __LINE__);
        ret_code = -1;
        goto error;
    }

    json_t *json = NULL;
    switch(json_parse_document(&json, rps -> msg -> str))
    {
    case JSON_OK:
        break;
    default:
        g_warning("json_parser_document: syntax error. (%s, %d)"
                , __FILE__, __LINE__);
        ret_code = -1;
        goto error;
    }
    
    /*
     * temporarily like this.
     */
    json_find_first_label_all(json, "raw_content");

error:
    json_free_value(&json);
    response_del(rps);
    request_del(req);
    return ret_code;
}


gint qq_start_poll(QQInfo *info, GError **err)
{
    return do_poll(info, err);
}

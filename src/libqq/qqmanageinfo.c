/*
 * Manage the information of the user
 *
 *     1.Get friends' information.
 *     2.Get friends' pictures.
 *     3.Get groups' informatin.
 *     4.Get self information.
 *     5.Get online buddies
 *     6.Get recent connected buddies.
 *     7.Get face image.
 *     8.Get level.
 *
 *     ***Next version***
 *     1.Manage all above.
 */

#include <qq.h>
#include <qqtypes.h>
#include <url.h>
#include <http.h>
#include <json.h>
#include <qqhosts.h>
#include <unicode.h>
#include <string.h>
#include <glib/gprintf.h>
#include <stdlib.h>

//
// Get the qq (group) number.
// The result is put into num.
//
gint qq_get_qq_number(QQInfo *info, const gchar *uin, gchar *num, GError **err)
{
    if(info -> vfwebqq == NULL || info -> vfwebqq -> len <= 0){
        g_warning("Need vfwebqq!!(%s, %d)", __FILE__, __LINE__);
        return -1;
    }

    gint ret_code = 0;
	gint res = 0;
    gchar params[300];
    g_debug("Get qq number.(%s, %d)", __FILE__, __LINE__);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "GET");
    request_set_version(req, "HTTP/1.1");
    g_sprintf(params, GETQQNUM"?tuin=%s&verifysession=&type=1&code="
                        "&vfwebqq=%s&t=%ld", uin
                        , info -> vfwebqq -> str, get_now_millisecond());
    request_set_uri(req, params);
    request_set_default_headers(req);
    request_add_header(req, "Host", SWQQHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Content-Type", "utf-8");
    request_add_header(req, "Referer"
            , "http://s.web2.qq.com/proxy.html?v=20101025002");


    Connection *con = connect_to_host(SWQQHOST, 80);
    if(con == NULL){
        g_warning("Can NOT connect to server!(%s, %d)"
                , __FILE__, __LINE__);
        request_del(req);
        return -1;
    }

    send_request(con, req);
    response_del(rps);
    res = rcv_response(con, &rps);
    close_con(con);
    connection_free(con);

	if (-1 == res || !rps) {
		g_warning("Null point access (%s, %d)\n", __FILE__, __LINE__);
		ret_code = -1;
		goto error;
	}
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
    json_t *val = json_find_first_label_all(json, "account");
    if(val != NULL){
        if(num != NULL){
            g_stpcpy(num, val -> child -> text);
        }
        g_debug("qq number: %s (%s, %d)", val -> child -> text
                                                , __FILE__, __LINE__);
    }else{
        g_warning("%s (%s, %d)", rps -> msg -> str, __FILE__, __LINE__);
    }
    json_free_value(&json);

error:
    request_del(req);
    response_del(rps);
    return ret_code;
}

static gint do_get_single_long_nick(QQInfo *info, QQBuddy *bdy, GError **err)
{
    if(info -> vfwebqq == NULL || info -> vfwebqq -> len <= 0){
        g_warning("Need vfwebqq!!(%s, %d)", __FILE__, __LINE__);
        return -1;
    }

    gint ret_code = 0;
    gchar params[300];
    g_debug("Get single long nick.(%s, %d)", __FILE__, __LINE__);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "GET");
    request_set_version(req, "HTTP/1.1");
    g_sprintf(params, LNICKPATH"?tuin=%s&vfwebqq=%s&t=%ld"
            , bdy -> uin -> str
            , info -> vfwebqq -> str, get_now_millisecond());
    request_set_uri(req, params);
    request_set_default_headers(req);
    request_add_header(req, "Host", SWQQHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Content-Type", "utf-8");
    request_add_header(req, "Referer"
            , "http://s.web2.qq.com/proxy.html?v=20101025002");


    Connection *con = connect_to_host(SWQQHOST, 80);
    if(con == NULL){
        g_warning("Can NOT connect to server!(%s, %d)"
                , __FILE__, __LINE__);
        request_del(req);
        return -1;
    }

    send_request(con, req);
    response_del(rps);
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
    json_t *val = json_find_first_label_all(json, "lnick");
    if(val != NULL){
        GString *vs = g_string_new(NULL);
        ucs4toutf8(vs, val -> child -> text);
        qq_buddy_set(bdy, "lnick", vs -> str);
        g_debug("lnick: %s (%s, %d)", vs -> str , __FILE__, __LINE__);
        g_string_free(vs, TRUE);
    }
    /*
     * Just to check error.
     */
    val = json_find_first_label_all(json, "uin");
    if(val == NULL){
        g_debug("(%s, %d) %s", __FILE__, __LINE__, rps -> msg -> str);
    }

error:
    json_free_value(&json);
    request_del(req);
    response_del(rps);
    return ret_code;
}

static gint do_get_online_buddies(QQInfo *info, GError **err)
{
    if(info -> psessionid == NULL || info -> psessionid -> len <= 0){
        g_warning("Need psessionid!!(%s, %d)", __FILE__, __LINE__);
        return -1;
    }

    gint ret_code = 0;
    gchar params[300];
    g_debug("Get online buddies!(%s, %d)", __FILE__, __LINE__);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "GET");
    request_set_version(req, "HTTP/1.1");
    g_sprintf(params, ONLINEPATH"?clientid=%s&psessionid=%s&t=%ld"
            , info -> clientid -> str
            , info -> psessionid -> str, get_now_millisecond());
    request_set_uri(req, params);
    request_set_default_headers(req);
    request_add_header(req, "Host", ONLINEHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Content-Type", "utf-8");
    request_add_header(req, "Referer"
            , "http://d.web2.qq.com/proxy.html?v=20101025002");

    Connection *con = connect_to_host(ONLINEHOST, 80);
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

    json_t *val;
    val = json_find_first_label_all(json, "result");
    if(val != NULL){
        val = val -> child;
        gint i;
        json_t *cur = NULL, *tmp = NULL;
        gchar *uin = NULL, *status = NULL, *client_type = NULL;
        for(cur = val -> child; cur != NULL; cur = cur -> next){
            tmp = json_find_first_label(cur, "uin");    
            if(tmp != NULL){
                uin = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "status");    
            if(tmp != NULL){
                status = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "client_type");    
            if(tmp != NULL){
                client_type= tmp -> child -> text;
            }
            QQBuddy *bdy;
            gint ct;
            char *endptr;
            for(i = 0; i < info -> buddies -> len; ++i){
                bdy = (QQBuddy *)info -> buddies -> pdata[i];
                if(g_strstr_len(bdy -> uin -> str, -1, uin) != NULL){
                    qq_buddy_set(bdy, "status", status);
                    ct = strtol(client_type, &endptr, 10);
                    if(endptr == client_type){
                        g_warning("strtol error(%s,%d)", __FILE__, __LINE__);
                        continue;
                    }
                    qq_buddy_set(bdy, "client_type", ct);
                    g_debug("Online buddies: %s %s %d(%s, %d)", uin, status, ct
                                        , __FILE__, __LINE__);
                }
            }
        }
    }

error:
    json_free_value(&json);
    request_del(req);
    response_del(rps);
    return ret_code;
}

static gint do_get_recent_contact(QQInfo *info, GError **err)
{
    if(info -> vfwebqq == NULL || info -> vfwebqq -> len <= 0){
        g_warning("Need vfwebqq!!(%s, %d)", __FILE__, __LINE__);
        return -1;
    }

    gint ret_code = 0;
    gchar params[300];
    g_debug("Get recent contacts!(%s, %d)", __FILE__, __LINE__);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "POST");
    request_set_version(req, "HTTP/1.1");
    request_set_uri(req, RECENTPATH);
    request_set_default_headers(req);
    request_add_header(req, "Host", RECENTHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Content-Type"
            , "application/x-www-form-urlencoded");
    request_add_header(req, "Content-Transfer-Encoding", "binary");
    request_add_header(req, "Referer"
            , "http://s.web2.qq.com/proxy.html?v=20101025002");
    g_snprintf(params, 300, "r={\"vfwebqq\":\"%s\"}"
            , info -> vfwebqq -> str);
    gchar *euri = g_uri_escape_string(params, "=", FALSE);
    request_append_msg(req, euri, strlen(euri));
    g_snprintf(params, 300, "%u", (unsigned int)strlen(euri));
    request_add_header(req, "Content-Length", params);
    g_free(euri);

    Connection *con = connect_to_host(RECENTHOST, 80);
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

    json_t *val;
    val = json_find_first_label_all(json, "contents");
    if(val != NULL){
        val = val -> child;
        gint ti;
        char *endptr;
        json_t *cur, *tmp;
        gchar *uin= NULL, *type = NULL;
        for(cur = val -> child; cur != NULL; cur = cur -> next){
            tmp = json_find_first_label(cur, "uin");    
            if(tmp != NULL){
                uin = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "type");    
            if(tmp != NULL){
                type = tmp -> child -> text;
            }
//            g_debug("recent con, uin: %s, type: %s (%s, %d)"
//                    , uin, type, __FILE__, __LINE__);
            
            QQRecentCon *rc = qq_recentcon_new();
            rc -> uin = g_string_new(uin);
            ti = strtol(type, &endptr, 10);
            if(endptr == type){
                g_warning("strtol error.(%s, %d)", __FILE__
                        , __LINE__);
                continue;
            }    
            rc -> type = ti;
            g_ptr_array_add(info -> recentcons, rc);
        }
    }

error:
    json_free_value(&json);
    request_del(req);
    response_del(rps);
    return ret_code;
}


static gint do_get_buddy_info(QQInfo *info, QQBuddy *bdy, GError **err)
{
    gint ret_code = 0;
    gchar params[500];
    g_debug("Get %s information!(%s, %d)", bdy -> uin -> str
                            ,  __FILE__, __LINE__);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "GET");
    request_set_version(req, "HTTP/1.1");

    g_sprintf(params, GETINFO"?tuin=%s&verifysession=&code=&"
                "vfwebqq=%s&t=%ld", bdy -> uin -> str
                , info -> vfwebqq -> str, get_now_millisecond());
    request_set_uri(req, params);
    request_set_default_headers(req);
    request_add_header(req, "Host", SWQQHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Content-Type", "utf-8");
    request_add_header(req, "Referer"
            , "http://s.web2.qq.com/proxy.html?v=20101025002");

    Connection *con = connect_to_host(SWQQHOST, 80);
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

    json_t *val;
    GString *vs;

#define  SET_STR_VALUE(x)   \
    val = json_find_first_label_all(json, x);\
    if(val != NULL){\
        vs = g_string_new(NULL);\
        ucs4toutf8(vs, val -> child -> text);\
        qq_buddy_set(bdy, x, vs -> str);\
        g_debug(x": %s (%s, %d)", vs -> str, __FILE__, __LINE__);\
     }
    SET_STR_VALUE("nick");
    SET_STR_VALUE("uin");
    SET_STR_VALUE("country");
    SET_STR_VALUE("province");
    SET_STR_VALUE("city");
    SET_STR_VALUE("gender");
    SET_STR_VALUE("phone");
    SET_STR_VALUE("mobile");
    SET_STR_VALUE("face");
    SET_STR_VALUE("email");
    SET_STR_VALUE("occupation");
    SET_STR_VALUE("college");
    SET_STR_VALUE("homepage");
    SET_STR_VALUE("personal");
#undef    SET_STR_VALUE

    json_t *year, *month, *day;
    year = json_find_first_label_all(json, "year");
    month = json_find_first_label_all(json, "month");
    day = json_find_first_label_all(json, "day");
    if(year != NULL && month != NULL && day != NULL){
        gint y, m, d;
        gchar *endptr;
        y = strtol(year -> child -> text, &endptr, 10);
        if(endptr == year -> child -> text){
            g_warning("strtol error. input: %s (%s, %d)"
                    , year -> child -> text, __FILE__, __LINE__);
        }else{
            y = 1900;
        }

        m = strtol(month -> child -> text, &endptr, 10);
        if(endptr == month -> child -> text){
            g_warning("strtol error. input: %s (%s, %d)"
                    , month -> child -> text, __FILE__, __LINE__);
        }else{
            m = 1;
        }

        d = strtol(day -> child -> text, &endptr, 10);
        if(endptr == day -> child -> text){
            g_warning("strtol error. input: %s (%s, %d)"
                    , day -> child -> text, __FILE__, __LINE__);
        }else{
            d = 1;
        }

        qq_buddy_set(bdy, "birthday", y, m, d);

        g_debug("Birthday:%d/%d/%d (%s, %d)", bdy -> birthday.year
                    , bdy -> birthday.month
                    , bdy -> birthday.day
                    , __FILE__, __LINE__);
    }

    val = json_find_first_label_all(json, "blood");
    if(val != NULL){
        gint tmpi;
        gchar *endptr;
        tmpi = strtol(val -> child -> text, &endptr, 10);
        if(endptr == val -> child -> text){
            g_warning("strtol error. input: %s (%s, %d)"
                    , val -> child -> text, __FILE__, __LINE__);
        }else{
            qq_buddy_set(bdy, "blood", tmpi);
            g_debug("blood: %d (%s, %d)", tmpi, __FILE__, __LINE__);
        }
    }
    val = json_find_first_label_all(json, "shengxiao");
    if(val != NULL){
        gint tmpi;
        gchar *endptr;
        tmpi = strtol(val -> child -> text, &endptr, 10);
        if(endptr == val -> child -> text){
            g_warning("strtol error. input: %s (%s, %d)"
                    , val -> child -> text, __FILE__, __LINE__);
        }else{
            qq_buddy_set(bdy, "shengxiao", tmpi);
            g_debug("shengxiao: %d (%s, %d)", tmpi, __FILE__, __LINE__);
        }
    }
    val = json_find_first_label_all(json, "constel");
    if(val != NULL){
        gint tmpi;
        gchar *endptr;
        tmpi = strtol(val -> child -> text, &endptr, 10);
        if(endptr == val -> child -> text){
            g_warning("strtol error. input: %s (%s, %d)"
                    , val -> child -> text, __FILE__, __LINE__);
        }else{
            qq_buddy_set(bdy, "constel", tmpi);
            g_debug("constel: %d (%s, %d)", tmpi, __FILE__, __LINE__);
        }
    }
    val = json_find_first_label_all(json, "allow");
    if(val != NULL){
        gint tmpi;
        gchar *endptr;
        tmpi = strtol(val -> child -> text, &endptr, 10);
        if(endptr == val -> child -> text){
            g_warning("strtol error. input: %s (%s, %d)"
                    , val -> child -> text, __FILE__, __LINE__);
        }else{
            qq_buddy_set(bdy, "allow", tmpi);
            g_debug("allow: %d (%s, %d)", tmpi, __FILE__, __LINE__);
        }
    }
    

    ret_code = do_get_single_long_nick(info, bdy, err);
    gchar qqnum[100];
    ret_code = qq_get_qq_number(info, bdy -> uin -> str, qqnum, err);
    qq_buddy_set(bdy, "qqnumber", qqnum);

error:
    json_free_value(&json);
    request_del(req);
    response_del(rps);
    return ret_code;
}

gint qq_get_buddies_and_categories(QQInfo *info, GError **err)
{
    if(info -> vfwebqq == NULL || info -> vfwebqq -> len <= 0){
        g_warning("Need vfwebqq!!(%s, %d)", __FILE__, __LINE__);
        return -1;
    }


    gint ret_code = 0;
    gint i;
    gchar params[300];
    g_debug("Get all buddies.(%s, %d)", __FILE__, __LINE__);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "POST");
    request_set_version(req, "HTTP/1.1");
    request_set_uri(req, FRIENDSPATH);
    request_set_default_headers(req);
    request_add_header(req, "Host", SWQQHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Content-Type"
            , "application/x-www-form-urlencoded");
    request_add_header(req, "Content-Transfer-Encoding", "binary");
    request_add_header(req, "Referer"
            , "http://s.web2.qq.com/proxy.html?v=20101025002");
    g_snprintf(params, 300, "r={\"h\":\"hello\",\"vfwebqq\":\"%s\"}"
            , info -> vfwebqq -> str);
    gchar *euri = g_uri_escape_string(params, "=", FALSE);
    request_append_msg(req, euri, strlen(euri));
    g_snprintf(params, 300, "%u", (unsigned int)strlen(euri));
    request_add_header(req, "Content-Length", params);
    g_free(euri);

    Connection *con = connect_to_host(SWQQHOST, 80);
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
    if(g_strstr_len(retstatus, -1, "200") == NULL){
        /*
         * Maybe some error occured.
         */
        g_warning("Resoponse status is NOT 200, but %s (%s, %d)"
                , retstatus, __FILE__, __LINE__);
        ret_code = -1;
        goto error;
    }

//    g_printf("(%s, %d) %s \n", __FILE__, __LINE__, rps -> msg -> str);

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
    
    QQBuddy *bdy = NULL;
    QQCategory *cate = NULL;

    //Free the old ones
    for(i = 0; i < info -> categories -> len; ++i){
        qq_category_free(g_ptr_array_index(info -> categories, i));
    }
    if(info -> categories -> len > 0){
        g_ptr_array_remove_range(info -> categories, 0, info -> categories -> len);
    }

    json_t *val;
    val = json_find_first_label_all(json, "result");
    if(val != NULL){
        val = val -> child -> child;
        while(val -> next != NULL){
            if(g_strcmp0("categories", val -> text) == 0){
                break;
            }
            val = val -> next;
        }
        val = val -> child;    //point to the array.[]    
        json_t *cur, *index, *name;
        GString *sn;
        gint ii;
        gchar *endptr;
        for(cur = val -> child; cur != NULL; cur = cur -> next){
            index = json_find_first_label_all(cur, "index");
            name = json_find_first_label_all(cur, "name");
            index = index -> child;
            name = name -> child;
            sn = g_string_new(NULL);
            ucs4toutf8(sn, name -> text);
            ii = strtol(index -> text, &endptr, 10);
            if(endptr == index -> text){
                g_warning("strtol error! s:%s (%s, %d)"
                        , index -> text
                        , __FILE__, __LINE__);
            }
            cate = qq_category_new();
            cate -> name = sn;
            cate -> index = ii;
            g_ptr_array_add(info -> categories, (gpointer)cate);
            g_debug("category: %d %s (%s, %d)", ii, sn -> str
                                                , __FILE__, __LINE__);
        }
        
        //add the default category
        cate = qq_category_new();
        cate -> name = g_string_new("My Friends");
        cate -> index = 0;
        g_ptr_array_add(info -> categories, (gpointer)cate);
    }

    /*
     * qq buddies' info
     */
    for(i = 0; i < info -> buddies -> len; ++i){
        qq_buddy_free(g_ptr_array_index(info -> buddies, i));
    }
    if(info -> buddies -> len > 0){
        g_ptr_array_remove_range(info -> buddies, 0, info -> buddies -> len);
    }

    val = json_find_first_label_all(json, "info");
    if(val != NULL){
        val = val -> child;
        const gchar *uin = NULL, *nick = NULL, *face = NULL, *flag = NULL;
        json_t *cur = NULL, *tmp = NULL;
        GString *ns  = g_string_new(NULL);
        for(cur = val -> child; cur != NULL; cur = cur -> next){
            tmp = json_find_first_label(cur, "uin");
            if(tmp != NULL){
                uin = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "nick");
            if(tmp != NULL){
                nick = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "face");
            if(tmp != NULL){
                face = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "flag");
            if(tmp != NULL){
                flag = tmp -> child -> text;
            }
            g_string_truncate(ns, 0);
            ucs4toutf8(ns, nick);
            g_debug("uin:%s nick:%s face:%s flag:%s (%s, %d)"
                                    , uin, ns -> str, face, flag
                                    , __FILE__, __LINE__);

            bdy = qq_buddy_new();    
            qq_buddy_set(bdy, "uin", uin);
            qq_buddy_set(bdy, "nick", ns -> str);
            qq_buddy_set(bdy, "face", face);
            qq_buddy_set(bdy, "flag", flag);

            g_ptr_array_add(info -> buddies, bdy);
        }
        g_string_free(ns, TRUE);
    }
    /*
     * qq buddies' marknames
     */
    val = json_find_first_label_all(json, "marknames");
    if(val != NULL){
        val = val -> child;
        const gchar *uin = NULL, *markname = NULL;
        json_t *cur = NULL, *tmp = NULL;
        for(cur = val -> child; cur != NULL; cur = cur -> next){
            tmp = json_find_first_label(cur, "uin");
            if(tmp != NULL){
                uin = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "markname");
            if(tmp != NULL){
                markname = tmp -> child -> text;
            }
            for(i = 0; i < info -> buddies -> len; ++i){
                bdy = (QQBuddy*)g_ptr_array_index(info -> buddies, i);
                if(bdy == NULL){
                    continue;
                }
                if(g_strcmp0(bdy -> uin -> str, uin) == 0){
                    break;
                }
                bdy = NULL;
            }
            if(bdy != NULL){
                /*
                 * Find the buddy
                 */
                g_string_truncate(bdy -> markname, 0);
                ucs4toutf8(bdy -> markname, markname);
                g_debug("uin:%s markname:%s (%s, %d)", uin
                            , bdy -> markname -> str, __FILE__, __LINE__);
            }
        }
    }
    /*
     * qq buddies' categories
     */
    val = json_find_first_label_all(json, "friends");
    if(val != NULL){
        val = val -> child;
        const gchar *uin = NULL, *cateidx = NULL;
        json_t *cur = NULL, *tmp = NULL;
        for(cur = val -> child; cur != NULL; cur = cur -> next){
            tmp = json_find_first_label(cur, "uin");
            if(tmp != NULL){
                uin = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "categories");
            if(tmp != NULL){
                cateidx = tmp -> child -> text;
                g_debug("uin %s Category %s(%s, %d)", uin, cateidx
                                    , __FILE__, __LINE__);
            }

            gint idx = 0;
            char *endptr = NULL;
            bdy = qq_info_lookup_buddy_by_uin(info, uin);
            if(bdy != NULL){
                idx = strtol(cateidx, &endptr, 10);
                if(endptr == cateidx){
                    g_warning("strtol error. %s:%d (%s, %d)"
                                , cateidx, idx, __FILE__, __LINE__);
                }
                for(i = 0; i < info -> categories -> len; ++i){
                    cate = (QQCategory*)g_ptr_array_index(
                                                info -> categories, i); 
                    if(cate != NULL && cate -> index == idx){
                        //find the category
                        g_debug("Buddy %s in category %d(%s, %d)", uin
                                            , idx, __FILE__, __LINE__);
                        g_ptr_array_add(cate -> members, bdy);
                        qq_buddy_set(bdy, "cate_index", idx);
                        break;
                    }
                }
            }else{
                g_warning("Can not find buddy %s!(%s, %d)", uin
                            , __FILE__, __LINE__);
            }
        }
    }

error:
    json_free_value(&json);
    request_del(req);
    response_del(rps);
    return ret_code;
}

gint qq_get_groups(QQInfo *info, GError **err)
{
    if(info -> vfwebqq == NULL || info -> vfwebqq -> len <= 0){
        g_warning("Need vfwebqq!!(%s, %d)", __FILE__, __LINE__);
        return -1;
    }

    gint ret_code = 0;
    gchar params[300];
    g_debug("Get my groups!(%s, %d)", __FILE__, __LINE__);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "POST");
    request_set_version(req, "HTTP/1.1");
    request_set_uri(req, GNAMEPATH);
    request_set_default_headers(req);
    request_add_header(req, "Host", SWQQHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Content-Type"
            , "application/x-www-form-urlencoded");
    request_add_header(req, "Content-Transfer-Encoding", "binary");
    request_add_header(req, "Referer"
            , "http://s.web2.qq.com/proxy.html?v=20101025002");
    g_snprintf(params, 300, "r={\"vfwebqq\":\"%s\"}"
            , info -> vfwebqq -> str);
    gchar *euri = g_uri_escape_string(params, "=", FALSE);
    request_append_msg(req, euri, strlen(euri));
    g_snprintf(params, 300, "%u", (unsigned)strlen(euri));
    request_add_header(req, "Content-Length", params);
    g_free(euri);

    Connection *con = connect_to_host(SWQQHOST, 80);
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
    
    QQGroup *grp = NULL;
    gint i;
    for(i = 0; i < info -> groups -> len; ++i){
        qq_group_free(g_ptr_array_index(info -> groups, i));
    }
    if(info -> groups -> len > 0){
        g_ptr_array_remove_range(info -> groups, 0, info -> groups -> len);
    }

    /*
     * gnamelist
     */
    json_t *val;
    val = json_find_first_label_all(json, "gnamelist");
    if(val != NULL){
        val = val -> child;
        json_t *cur = NULL, *tmp = NULL;
        gchar *gid = NULL, *code = NULL, *flag = NULL, *name = NULL;
        GString *tmps = g_string_new(NULL);
        for(cur = val -> child; cur != NULL; cur = cur -> next){
            tmp = json_find_first_label(cur, "gid");
            if(tmp != NULL){
                gid = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "code");
            if(tmp != NULL){
                code = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "flag");
            if(tmp != NULL){
                flag = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "name");
            if(tmp != NULL){
                name = tmp -> child -> text;
            }
            grp = qq_group_new();
            qq_group_set(grp, "gid", gid);
            qq_group_set(grp, "code", code);
            qq_group_set(grp, "flag", flag);
            g_string_truncate(tmps, 0);
            ucs4toutf8(tmps, name);
            g_debug("gid: %s, code %s, flag %s, name %s (%s, %d)"
                                        , gid, code, flag, tmps -> str
                                        , __FILE__, __LINE__);
            qq_group_set(grp, "name", tmps -> str);

            g_ptr_array_add(info -> groups, grp);
        }
        g_string_free(tmps, TRUE);
    }else{
        g_warning("No gnamelist find. (%s, %d)", __FILE__, __LINE__);
    }

    /*
     * gmasklist
     */
    val = json_find_first_label_all(json, "gmasklist");
    if(val != NULL){
        val = val -> child;
        json_t *cur = NULL, *tmp = NULL;
        gchar *gid = NULL, *mask = NULL;
        for(cur = val -> child; cur != NULL; cur = cur -> next){
            tmp = json_find_first_label(cur, "gid");
            if(tmp != NULL){
                gid = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "mask");
            if(tmp != NULL){
                mask = tmp -> child -> text;
            }
            for(i = 0; i < info -> groups -> len; ++i){
                grp = (QQGroup *)g_ptr_array_index(info -> groups, i);
                if(g_strstr_len(grp -> gid -> str, -1, gid) != NULL){
                    g_debug("Find group %s for mask %s(%s, %d)", gid, mask
                                            , __FILE__, __LINE__);
                    qq_group_set(grp, "mask", mask);
                }
            }
        }
    }else{
        g_warning("No gmasklist find. (%s, %d)", __FILE__, __LINE__);
    }

    /*
     * gmarklist
     */
    val = json_find_first_label_all(json, "gmarklist");
    if(val != NULL){
        val = val -> child;
        json_t *cur = NULL, *tmp = NULL;
        gchar *gid = NULL, *mark = NULL;
        GString *tmps = g_string_new(NULL);
        for(cur = val -> child; cur != NULL; cur = cur -> next){
            tmp = json_find_first_label(cur, "uin");
            if(tmp != NULL){
                gid = tmp -> child -> text;
            }
            tmp = json_find_first_label(cur, "mark");
            if(tmp != NULL){
                mark = tmp -> child -> text;
            }
            for(i = 0; i < info -> groups -> len; ++i){
                grp = (QQGroup *)g_ptr_array_index(info -> groups, i);
                if(g_strstr_len(grp -> gid -> str, -1, gid)
                        != NULL){
                    g_string_truncate(tmps, 0);
                    ucs4toutf8(tmps, mark);
                    g_debug("Find group %s for mark %s(%s, %d)", gid, tmps -> str
                            , __FILE__, __LINE__);
                    qq_group_set(grp, "mark", tmps -> str);
                }
            }
        }
        g_string_free(tmps, TRUE);
    }else{
        g_warning("No gmarklist find. (%s, %d)", __FILE__, __LINE__);
    }

error:
    json_free_value(&json);
    request_del(req);
    response_del(rps);
    return ret_code;
}

gint qq_get_buddy_info(QQInfo *info, QQBuddy *bdy, GError **err)
{
    if(info == NULL){
        return -1;
    }
    if(bdy == NULL){
        return NO_ERR;
    }
    if(info -> vfwebqq == NULL || info -> vfwebqq -> len <= 0){
        g_warning("Need vfwebqq!!(%s, %d)", __FILE__, __LINE__);
        return -1;
    }

    return do_get_buddy_info(info, bdy, err);
}

gint qq_get_online_buddies(QQInfo *info, GError **err)
{
    if(info == NULL){
        return -1;
    }

    return do_get_online_buddies(info, err);
}

gint qq_get_recent_contact(QQInfo *info, GError **err)
{
    if(info == NULL){
        return -1;
    }

    return do_get_recent_contact(info, err);
}

gint qq_get_single_long_nick(QQInfo *info, QQBuddy *bdy, GError **err)
{
    if(info == NULL){
        return -1;
    }

    if(bdy == NULL){
        return -1;
    }

    return do_get_single_long_nick(info, bdy, err);
}


gint qq_get_group_info(QQInfo *info, QQGroup *grp, GError **err)
{
    if(grp == NULL){
        return NO_ERR;
    }
    if(info -> vfwebqq == NULL || info -> vfwebqq -> len <= 0){
        g_warning("Need vfwebqq!!(%s, %d)", __FILE__, __LINE__);
        return -1;
    }
    gchar grpnum[100];
    if(qq_get_qq_number(info, grp -> code -> str, grpnum, NULL) == NO_ERR){
        qq_group_set(grp, "gnumber", grpnum);
    }else{
        g_warning("Get group number failed... (%s, %d)", __FILE__, __LINE__);
    }
    

    gint ret_code = NO_ERR;
    gchar params[300];
    g_debug("Get group information.(%s, %d)", __FILE__, __LINE__);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "GET");
    request_set_version(req, "HTTP/1.1");
    g_sprintf(params, GETGRPINFO"?gcode=%s&vfwebqq=%s&t=%ld"
                        , grp -> code -> str 
                        , info -> vfwebqq -> str, get_now_millisecond());
    request_set_uri(req, params);
    request_set_default_headers(req);
    request_add_header(req, "Host", SWQQHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Content-Type", "utf-8");
    request_add_header(req, "Referer"
            , "http://s.web2.qq.com/proxy.html?v=20101025002");


    Connection *con = connect_to_host(SWQQHOST, 80);
    if(con == NULL){
        g_warning("Can NOT connect to server!(%s, %d)"
                , __FILE__, __LINE__);
        request_del(req);
        return NETWORK_ERR;
    }

    send_request(con, req);
    response_del(rps);
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
        ret_code = NETWORK_ERR;
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
        ret_code = NETWORK_ERR;
        goto error;
    }

    GString *utf8 = g_string_new(NULL);

    json_t *val = json_find_first_label_all(json, "ginfo");
    json_t *tmp;
    if(val != NULL){
        val = val -> child;
#define SET_VAL(x) \
        tmp = json_find_first_label_all(val, x);\
        if(tmp != NULL){\
            tmp = tmp -> child;\
            g_string_truncate(utf8, 0);\
            ucs4toutf8(utf8, tmp -> text);\
            qq_group_set(grp, x, utf8 -> str);\
            g_debug(x": %s(%s, %d)", utf8 -> str, __FILE__, __LINE__);\
        }
    
        SET_VAL("gid");
        SET_VAL("flag");
        SET_VAL("owner");
        SET_VAL("code");
        SET_VAL("option");
        SET_VAL("createtime");
        SET_VAL("class");
        SET_VAL("name");
        SET_VAL("level");
        SET_VAL("face");
        SET_VAL("memo");
        SET_VAL("fingermemo");
#undef SET_VAL
    }else{
        g_warning("No ginfo found. %s (%s, %d)", rps -> msg -> str
                                                , __FILE__, __LINE__);
    }

#define FIND_VAL(x,y,z)\
    tmp = json_find_first_label_all(x, y);\
    if(tmp != NULL){\
        z = tmp -> child -> text;\
    }else{\
        z = "";\
        g_warning("No member named %s found!!(%s, %d)"\
                                    , y, __FILE__, __LINE__);\
    }\

    val = json_find_first_label_all(json, "members");
    if(val != NULL){
        val = val -> child -> child;
        gchar *uin, *flag, num[100];
        QQGMember *gmem;
        for(;val != NULL; val = val -> next){
            FIND_VAL(val, "muin", uin);
            FIND_VAL(val, "mflag", flag);
            // get qq number
            qq_get_qq_number(info, uin, num, NULL);
            gmem = qq_gmember_new();
            qq_gmember_set(gmem, "uin", uin);
            qq_gmember_set(gmem, "flag", flag);
            qq_gmember_set(gmem, "qqnumber", num);
            g_ptr_array_add(grp -> members, gmem);
            g_debug("Group memeber. uin %s, flag %s , num %s(%s, %d)"
                            , uin, flag, num, __FILE__, __LINE__);
        }
    }else{
        g_warning("No group member found. %s (%s, %d)", rps -> msg -> str
                                                , __FILE__, __LINE__);
    }

    val = json_find_first_label_all(json, "minfo");
    if(val != NULL){
        val = val -> child -> child;
        gchar *uin, *nick;
        QQGMember *gmem = NULL;
        gint i;
        for(;val != NULL; val = val -> next){
            FIND_VAL(val, "uin", uin);
            FIND_VAL(val, "nick", nick);
            g_string_truncate(utf8, 0);
            ucs4toutf8(utf8, nick);
            for(i = 0; i < grp -> members -> len; ++i){
                gmem = (QQGMember*)g_ptr_array_index(grp -> members, i);
                if(gmem == NULL){
                    continue;
                }
                if(g_strcmp0(gmem -> uin -> str, uin) == 0){
                    qq_gmember_set(gmem, "nick", utf8 -> str);
                    g_debug("Group memeber. uin %s, nick %s (%s, %d)"
                            , uin, utf8 -> str, __FILE__, __LINE__);
                    break;
                }
            }
        } 
    }else{
        g_warning("No minfo found. %s (%s, %d)", rps -> msg -> str
                                                , __FILE__, __LINE__);
    }

    val = json_find_first_label_all(json, "stats");
    if(val != NULL){
        val = val -> child -> child;
        gchar *uin, *status, *client_type;
        QQGMember *gmem = NULL;
        gint i;
        for(;val != NULL; val = val -> next){
            FIND_VAL(val, "uin", uin);
            FIND_VAL(val, "stat", status);
            FIND_VAL(val, "client_type", client_type);
            for(i = 0; i < grp -> members -> len; ++i){
                gmem = (QQGMember*)g_ptr_array_index(grp -> members, i);
                if(gmem == NULL){
                    continue;
                }
                if(g_strcmp0(gmem -> uin -> str, uin) == 0){
                    qq_gmember_set(gmem, "status", status);
                    qq_gmember_set(gmem, "client_type", client_type);
                    g_debug("Group memeber. uin %s, status %s (%s, %d)"
                            , uin, status, __FILE__, __LINE__);
                    break;
                }
            }
        } 

    }else{
        g_warning("No stats found. %s (%s, %d)", rps -> msg -> str
                                                , __FILE__, __LINE__);
    }

    val = json_find_first_label_all(json, "cards");
    if(val != NULL){
        val = val -> child -> child;
        gchar *uin, *card;
        QQGMember *gmem = NULL;
        gint i;
        for(;val != NULL; val = val -> next){
            FIND_VAL(val, "muin", uin);
            FIND_VAL(val, "card", card);
            g_string_truncate(utf8, 0);
            ucs4toutf8(utf8, card);
            g_debug("Find %s %s (%s, %d)", uin, utf8 -> str
                                    , __FILE__, __LINE__);
            for(i = 0; i < grp -> members -> len; ++i){
                gmem = (QQGMember*)g_ptr_array_index(grp -> members, i);
                if(gmem == NULL){
                    continue;
                }
                if(g_strcmp0(gmem -> uin -> str, uin) == 0){
                    qq_gmember_set(gmem, "card", utf8 -> str);
                    g_debug("Group memeber. uin %s, card %s (%s, %d)"
                            , uin, utf8 -> str, __FILE__, __LINE__);
                    break;
                }
            }
        } 
    }else{
        g_warning("No cards found. %s (%s, %d)", rps -> msg -> str
                                                , __FILE__, __LINE__);
    }
#undef FIND_VAL
    g_string_free(utf8, TRUE);

error:
    json_free_value(&json);
    request_del(req);
    response_del(rps);
    return ret_code;
}
gint qq_update_details(QQInfo *info, GError **err)
{
    if(info == NULL){
        create_error_msg(err, PARAMETER_ERR, "info == NULL");
        return PARAMETER_ERR;
    }
    
    qq_get_buddy_info(info, info -> me, NULL);
    qq_buddy_set(info -> me, "qqnumber", info -> me -> uin -> str);
    
    gint i;
    QQBuddy *bdy = NULL;
    QQGroup *grp = NULL;

    for(i = 0; i < info -> buddies -> len; ++i){
        bdy = (QQBuddy*)g_ptr_array_index(info -> buddies, i);
        if(bdy == NULL){
            continue;
        }
        qq_get_buddy_info(info, bdy, NULL);
    }

    for(i = 0; i < info -> groups -> len; ++i){
        grp = (QQGroup*)g_ptr_array_index(info -> groups, i);
        if(grp == NULL){
            continue;
        }
        qq_get_group_info(info, grp, NULL);
    }
    return NO_ERR;
}

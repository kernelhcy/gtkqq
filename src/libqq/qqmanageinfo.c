#ifndef __LIBQQ_QQ_MANAGEINFO_H
#define __LIBQQ_QQ_MANAGEINFO_H
/*
 * Manage the information of the user
 *
 * 	1.Get friends' information.
 * 	2.Get friends' pictures.
 * 	3.Get groups' informatin.
 * 	4.Get self information.
 * 	5.Get online buddies
 * 	6.Get recent connected buddies.
 * 	7.Get face image.
 * 	8.Get level.
 *
 * 	***Next version***
 * 	1.Manage all above.
 */

#include <qq.h>
#include <qqtypes.h>
#include <url.h>
#include <http.h>
#include <json.h>
#include <qqhosts.h>
#include <unicode.h>
#include <string.h>

struct GetLNickPar{
	QQInfo *info;
	QQCallBack cb;
	gpointer usrdata;
	QQBuddy *bdy;
};
static gboolean do_get_single_long_nick(gpointer data)
{
	struct GetLNickPar *par = (struct GetLNickPar*)data;
	if(par == NULL){
		g_warning("par == NULL in do_get_single_long_nick.(%s, %d)"
				, __FILE__, __LINE__);
		return FALSE;
	}
	QQInfo *info = par -> info;
	QQCallBack cb = par -> cb;
	gpointer usrdata = par -> usrdata;
	QQBuddy *bdy = par -> bdy;
	g_slice_free(struct GetLNickPar, par);

	gchar params[300];
	g_debug("Get long nick.(%s, %d)", __FILE__, __LINE__);

	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, LNICKPATH"?tuin=%s&vfwebqq=%s&t=%lld"
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
		if(cb != NULL){
			cb(CB_NETWORKERR, "Can not connect to server!", usrdata);
		}
		request_del(req);
		g_free(par);
		return FALSE;
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
		if(cb != NULL){
			cb(CB_ERROR, "Response error!", usrdata);
		}
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
		goto error;
	}
	json_t *val = json_find_first_label_all(json, "lnick");
	if(val != NULL){
		GString *vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		bdy -> lnick = vs;
		g_debug("lnick: %s (%s, %d)", vs -> str , __FILE__, __LINE__);
	}
	/*
	 * Just to check error.
	 */
	val = json_find_first_label_all(json, "uin");
	if(val == NULL){
		g_debug("(%s, %d) %s", __FILE__, __LINE__, rps -> msg -> str);
	}

	if(cb != NULL){
		cb(CB_SUCCESS, "GET_SINGLE_LONG_NICK", usrdata);
	}

error:
	json_free_value(&json);
	request_del(req);
	response_del(rps);
	return FALSE;
}

static gboolean do_get_online_buddies(gpointer data)
{
	DoFuncParam *par = (DoFuncParam*)data;
	if(par == NULL){
		g_warning("par == NULL in do_get_online_buddies.(%s, %d)"
				, __FILE__, __LINE__);
		return FALSE;
	}
	QQInfo *info = par -> info;
	QQCallBack cb = par -> cb;
	gpointer usrdata = par -> usrdata;
	g_free(par);

	gchar params[300];
	g_debug("Get online buddies!(%s, %d)", __FILE__, __LINE__);

	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, ONLINEPATH"?clientid=%s&psessionid=%s&t=%lld"
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
		if(cb != NULL){
			cb(CB_NETWORKERR, "Can not connect to server!"
					, usrdata);
		}
		request_del(req);
		g_free(par);
		return FALSE;
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
		if(cb != NULL){
			cb(CB_ERROR, "Response error!", usrdata);
		}
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
		goto error;
	}

	g_printf("%s", rps -> msg -> str);
	json_t *val;
	val = json_find_first_label_all(json, "result");
	if(val != NULL){
		val = val -> child;
		gint i;
		json_t *cur, *tmp;
		gchar *uin, *status, *client_type;
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
			g_debug("uin: %s, status: %s, client_type: %s (%s, %d)"
					, uin, status, client_type
					, __FILE__, __LINE__);
			QQBuddy *bdy;
			gint ct;
			char *endptr;
			for(i = 0; i < info -> buddies -> len; ++i){
				bdy = (QQBuddy *)info -> buddies -> pdata[i];
				if(g_strstr_len(bdy -> uin -> str, -1, uin)
						!= NULL){
					g_string_free(bdy -> status, TRUE);
					bdy -> status = g_string_new(status);
					ct = strtol(client_type, &endptr, 10);
					if(endptr == client_type){
						g_warning("strtol error(%s,%d)"
								, __FILE__
								, __LINE__);
						continue;
					}
					bdy -> client_type = ct;
				}
			}
		}
	}

	if(cb != NULL){
		cb(CB_SUCCESS, "GET_ONLINE_BUDDIES", usrdata);
	}
error:
	json_free_value(&json);
	request_del(req);
	response_del(rps);
	return FALSE;
}

static gboolean do_get_recent_contact(gpointer data)
{
	DoFuncParam *par = (DoFuncParam*)data;
	if(par == NULL){
		g_warning("par == NULL in do_recent_contact.(%s, %d)"
				, __FILE__, __LINE__);
		return FALSE;
	}
	QQInfo *info = par -> info;
	QQCallBack cb = par -> cb;
	gpointer usrdata = par -> usrdata;
	g_free(par);

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
	g_snprintf(params, 300, "%d", strlen(euri));
	request_add_header(req, "Content-Length", params);
	g_free(euri);

	Connection *con = connect_to_host(RECENTHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_NETWORKERR, "Can not connect to server!"
						, usrdata);
		}
		request_del(req);
		g_free(par);
		return FALSE;
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
		if(cb != NULL){
			cb(CB_ERROR, "Response error!", usrdata);
		}
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
		goto error;
	}

	g_printf("%s", rps -> msg -> str);
	json_t *val;
	val = json_find_first_label_all(json, "contents");
	if(val != NULL){
		val = val -> child;
		gint ti;
		char *endptr;
		json_t *cur, *tmp;
		gchar *uin, *type;
		for(cur = val -> child; cur != NULL; cur = cur -> next){
			tmp = json_find_first_label(cur, "uin");	
			if(tmp != NULL){
				uin = tmp -> child -> text;
			}
			tmp = json_find_first_label(cur, "type");	
			if(tmp != NULL){
				type = tmp -> child -> text;
			}
			g_debug("recent con, uin: %s, type: %s (%s, %d)"
					, uin, type, __FILE__, __LINE__);
			
			QQRecentCon *rc = qq_recentcon_new();
			rc -> uin = g_string_new(uin);
			ti = strtol(type, &endptr, 10);
			if(endptr == type){
				g_warning("strtol error.(%s, %s)", __FILE__
						, __LINE__);
				continue;
			}	
			rc -> type = ti;
			g_ptr_array_add(info -> recentcons, rc);
		}
	}

	if(cb != NULL){
		cb(CB_SUCCESS, "GET_RECENT_CONTACE", usrdata);
	}
error:
	json_free_value(&json);
	request_del(req);
	response_del(rps);
	return FALSE;
}


static gboolean do_get_my_info(gpointer data)
{
	DoFuncParam *par = (DoFuncParam*)data;
	if(par == NULL){
		g_warning("par == NULL in do_get_my_info.(%s, %d)", __FILE__
				, __LINE__);
		return FALSE;
	}
	QQInfo *info = par -> info;
	QQCallBack cb = par -> cb;
	gpointer usrdata = par -> usrdata;
	g_free(par);

	gchar params[500];
	g_debug("Get my information!(%s, %d)", __FILE__, __LINE__);

	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");

	g_sprintf(params, GETMYINFO"?tuin=%s&verifysession=&code=&"
			"vfwebqq=%s&t=%lld"
			, info -> me -> uin -> str
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
		if(cb != NULL){
			cb(CB_NETWORKERR, "Can not connect to server!"
					, usrdata);
		}
		request_del(req);
		g_free(par);
		return FALSE;
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
		if(cb != NULL){
			cb(CB_ERROR, "Response error!", usrdata);
		}
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
		goto error;
	}

	json_t *val;
	GString *vs;
	val = json_find_first_label_all(json, "nick");
	if(val != NULL){
		vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		info -> me -> nick = vs;
		g_debug("nick: %s (%s, %d)", vs -> str, __FILE__, __LINE__);
	}
	val = json_find_first_label_all(json, "country");
	if(val != NULL){
		vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		info -> me -> country = vs;
		g_debug("country: %s (%s, %d)", vs -> str
				, __FILE__, __LINE__);
	}
	val = json_find_first_label_all(json, "province");
	if(val != NULL){
		vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		info -> me -> province = vs;
		g_debug("province: %s (%s, %d)", vs -> str
				, __FILE__, __LINE__);
	}
	val = json_find_first_label_all(json, "city");
	if(val != NULL){
		vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		info -> me -> city = vs;
		g_debug("city: %s (%s, %d)", vs -> str, __FILE__, __LINE__);
	}
	val = json_find_first_label_all(json, "gender");
	if(val != NULL){
		vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		info -> me -> gender = vs;
		g_debug("gender: %s (%s, %d)", vs -> str, __FILE__, __LINE__);
	}
	val = json_find_first_label_all(json, "phone");
	if(val != NULL){
		g_debug("phone: %s (%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
		info -> me -> phone = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "mobile");
	if(val != NULL){
		g_debug("mobile: %s (%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
		info -> me -> mobile = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "face");
	if(val != NULL){
		g_debug("face: %s (%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
		info -> me -> face = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "email");
	if(val != NULL){
		g_debug("email: %s (%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
		info -> me -> email = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "occupation");
	if(val != NULL){
		vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		info -> me -> occupation = vs;
		g_debug("occupation: %s (%s, %d)", vs -> str
				, __FILE__, __LINE__);
	}
	val = json_find_first_label_all(json, "college");
	if(val != NULL){
		vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		info -> me -> college = vs;
		g_debug("college: %s (%s, %d)", vs -> str
				, __FILE__, __LINE__);
	}
	val = json_find_first_label_all(json, "homepage");
	if(val != NULL){
		g_debug("homepage: %s (%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
		info -> me -> homepage = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "personal");
	if(val != NULL){
		vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		info -> me -> personal = vs;
		g_debug("personal: %s (%s, %d)", vs -> str
				, __FILE__, __LINE__);
	}

	json_t *year, *month, *day;
	year = json_find_first_label_all(json, "year");
	month = json_find_first_label_all(json, "month");
	day = json_find_first_label_all(json, "day");
	if(year != NULL && month != NULL && day != NULL){
		gint tmpi;
		gchar *endptr;
		tmpi = strtol(year -> child -> text, &endptr, 10);
		if(endptr == year -> child -> text){
			g_warning("strtol error. input: %s (%s, %d)"
					, year -> child -> text, __FILE__, __LINE__);
		}else{
			info -> me -> birthday.year = tmpi;
		}

		tmpi = strtol(month -> child -> text, &endptr, 10);
		if(endptr == month -> child -> text){
			g_warning("strtol error. input: %s (%s, %d)"
					, month -> child -> text, __FILE__, __LINE__);
		}else{
			info -> me -> birthday.month = tmpi;
		}

		tmpi = strtol(day -> child -> text, &endptr, 10);
		if(endptr == day -> child -> text){
			g_warning("strtol error. input: %s (%s, %d)"
					, day -> child -> text, __FILE__, __LINE__);
		}else{
			info -> me -> birthday.day = tmpi;
		}

		g_debug("Birthday:%d/%d/%d (%s, %d)", info -> me -> birthday.year
				, info -> me -> birthday.month
				, info -> me -> birthday.day
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
			info -> me -> blood = tmpi;
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
			info -> me -> shengxiao = tmpi;
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
			info -> me -> constel = tmpi;
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
			info -> me -> allow = tmpi;
			g_debug("allow: %d (%s, %d)", tmpi, __FILE__, __LINE__);
		}
	}
	/*
	 * Just to check error.
	 */
	val = json_find_first_label_all(json, "uin");
	if(val == NULL){
		g_debug("(%s, %d) %s", __FILE__, __LINE__, rps -> msg -> str);
	}

	/*
	 * get long nick
	 */
	struct GetLNickPar *glnpar = g_slice_new(struct GetLNickPar);
	glnpar -> info = info;
	glnpar -> cb = cb;
	glnpar -> usrdata = usrdata;
	glnpar -> bdy = info -> me;
	do_get_single_long_nick(glnpar);

	if(cb != NULL){
		cb(CB_SUCCESS, "GET_MY_INFO", usrdata);
	}
error:
	json_free_value(&json);
	request_del(req);
	response_del(rps);
	return FALSE;
}


static gboolean do_get_my_friends(gpointer data)
{
	DoFuncParam *par = (DoFuncParam*)data;
	if(par == NULL){
		g_warning("par == NULL in do_get_my_friends.(%s, %d)", __FILE__
				, __LINE__);
		return FALSE;
	}
	QQInfo *info = par -> info;
	QQCallBack cb = par -> cb;
	gpointer usrdata = par -> usrdata;
	g_free(par);

	gchar params[300];
	g_debug("Get my friends!(%s, %d)", __FILE__, __LINE__);

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
	g_snprintf(params, 300, "%d", strlen(euri));
	request_add_header(req, "Content-Length", params);
	g_free(euri);

	Connection *con = connect_to_host(SWQQHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_NETWORKERR, "Can not connect to server!"
						, usrdata);
		}
		request_del(req);
		g_free(par);
		return FALSE;
	}

	send_request(con, req);
	rcv_response(con, &rps);
	close_con(con);
	connection_free(con);

	g_printf("(%s, %d)%s\n", __FILE__, __LINE__, rps -> msg -> str);
	const gchar *retstatus = rps -> status -> str;
	if(g_strstr_len(retstatus, -1, "200") == NULL){
		/*
		 * Maybe some error occured.
		 */
		g_warning("Resoponse status is NOT 200, but %s (%s, %d)"
				, retstatus, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_ERROR, "Response error!", usrdata);
		}
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
		goto error;
	}
	
	json_t *val;
	val = json_find_first_label_all(json, "categories");
	if(val != NULL){
		val = val -> child;	//point to the array.[]	
		json_t *cur, *index, *name;
		GString *sn;
		gint ii;
		gchar *endptr;
		for(cur = val -> child; cur != NULL; cur = cur -> next){
			index = cur -> child -> child;
			name = cur -> child -> next -> child;
			sn = g_string_new(NULL);
			ucs4toutf8(sn, name -> text);
			ii = strtol(index -> text, &endptr, 10);
			if(endptr == index -> text){
				g_warning("strtol error! s:%s (%s, %d)"
						, index -> text
						, __FILE__, __LINE__);
			}
			QQCategory *cate = qq_category_new();
			cate -> name = sn;
			cate -> index = ii;
			g_ptr_array_add(info -> categories, (gpointer)cate);
			g_debug("category: %d %s (%s, %d)", ii, sn -> str
					, __FILE__, __LINE__);
		}
	}

	/*
	 * qq friends' info
	 */
	val = json_find_first_label_all(json, "info");
	if(val != NULL){
		val = val -> child;
		const gchar *uin, *nick, *face, *flag;
		json_t *cur, *tmp;
		GString *ns;
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
			ns = g_string_new(NULL);
			ucs4toutf8(ns, nick);
			g_debug("uin:%s nick:%s face:%s flag:%s (%s, %d)"
					, uin, ns -> str, face, flag
					, __FILE__, __LINE__);
			QQBuddy *buddy = qq_buddy_new();
			buddy -> uin = g_string_new(uin);
			buddy -> nick = g_string_new(nick);
			buddy -> face = g_string_new(face);
			buddy -> flag = g_string_new(flag);
			g_ptr_array_add(info -> buddies, buddy);
		}
	}
	/*
	 * qq friends' marknames
	 */
	val = json_find_first_label_all(json, "marknames");
	if(val != NULL){
		val = val -> child;
		const gchar *uin, *markname;
		json_t *cur, *tmp;
		for(cur = val -> child; cur != NULL; cur = cur -> next){
			tmp = json_find_first_label(cur, "uin");
			if(tmp != NULL){
				uin = tmp -> child -> text;
			}
			tmp = json_find_first_label(cur, "markname");
			if(tmp != NULL){
				markname = tmp -> child -> text;
			}
			gint i;
			QQBuddy *tmpb;
			for(i = 0; i < info -> buddies -> len; ++i){
				tmpb = (QQBuddy *)(info -> buddies -> pdata[i]);
				if(g_strstr_len(tmpb -> uin -> str, -1, uin) 
						!= NULL){
					/*
					 * Find the buddy
					 */
					tmpb -> markname = g_string_new(NULL);
					ucs4toutf8(tmpb -> markname, markname);
					g_debug("uin:%s markname:%s (%s, %d)"
						, uin, tmpb -> markname -> str
						, __FILE__, __LINE__);

				}else{
					g_warning("No buddy(%s) for markname:%s"
							" (%s, %d)", uin
							, tmpb -> markname -> str
							, __FILE__, __LINE__);
				}
			}
		}
	}
	/*
	 * qq friends' categories
	 */
	val = json_find_first_label_all(json, "friends");
	if(val != NULL){
		val = val -> child;
		const gchar *uin, *cate;
		json_t *cur, *tmp;
		for(cur = val -> child; cur != NULL; cur = cur -> next){
			tmp = json_find_first_label(cur, "uin");
			if(tmp != NULL){
				uin = tmp -> child -> text;
			}
			tmp = json_find_first_label(cur, "categories");
			if(tmp != NULL){
				cate = tmp -> child -> text;
			}
			g_debug("uin:%s cate:%s(%s, %d)"
					, uin, cate, __FILE__, __LINE__);
			gint i;
			QQCategory *qc = NULL;
			QQBuddy *bdy = NULL;
			for(i = 0; i < info -> buddies -> len; ++i){
				bdy = (QQBuddy *)info -> buddies -> pdata[i];
				if(g_strstr_len(bdy -> uin -> str, -1, uin)
						!= NULL){
					g_debug("Find buddy %s (%s, %d)"
							, uin, __FILE__
							, __LINE__);
					break;
				}
			}
			gint idx;
			char *endptr;
			idx = strtol(cate, &endptr, 10);
			if(endptr == cate){
				g_warning("strtol error. %s:%d (%s, %d)"
						, cate, idx, __FILE__
						, __LINE__);
				break;
			}
			bdy -> cate = info -> categories -> pdata[idx];
			g_ptr_array_add(bdy -> cate -> members, bdy);
		}
	}

	if(cb != NULL){
		cb(CB_SUCCESS, "GET_MY_FRIENDS", usrdata);
	}
error:
	json_free_value(&json);
	request_del(req);
	response_del(rps);
	return FALSE;
}

static gboolean do_get_group_name_list_mask(gpointer data)
{
	DoFuncParam *par = (DoFuncParam*)data;
	if(par == NULL){
		g_warning("par == NULL in do_get_my_friends.(%s, %d)", __FILE__
				, __LINE__);
		return FALSE;
	}
	QQInfo *info = par -> info;
	QQCallBack cb = par -> cb;
	gpointer usrdata = par -> usrdata;
	g_free(par);

	gchar params[300];
	g_debug("Get my group name list mask!(%s, %d)", __FILE__, __LINE__);

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
	g_snprintf(params, 300, "%d", strlen(euri));
	request_add_header(req, "Content-Length", params);
	g_free(euri);

	Connection *con = connect_to_host(SWQQHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_NETWORKERR, "Can not connect to server!"
					, usrdata);
		}
		request_del(req);
		return FALSE;
	}

	send_request(con, req);
	rcv_response(con, &rps);
	close_con(con);
	connection_free(con);

	g_printf("(%s, %d)%s\n", __FILE__, __LINE__, rps -> msg -> str);
	const gchar *retstatus = rps -> status -> str;
	if(g_strstr_len(retstatus, -1, "200") == NULL){
		/*
		 * Maybe some error occured.
		 */
		g_warning("Resoponse status is NOT 200, but %s (%s, %d)"
				, retstatus, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_ERROR, "Response error!", usrdata);
		}
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
		goto error;
	}
	
	/*
	 * gnamelist
	 */
	json_t *val;
	val = json_find_first_label_all(json, "gnamelist");
	if(val != NULL){
		val = val -> child;
		json_t *cur, *tmp;
		gchar *gid, *code, *flag, *name;
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
			QQGroup *grp = qq_group_new();
			grp -> gid = g_string_new(gid);
			grp -> code = g_string_new(code);
			grp -> flag = g_string_new(flag);
			grp -> name = g_string_new(NULL);
			ucs4toutf8(grp -> name, name);
			g_debug("gid: %s, code %s, flag %s, name %s (%s, %d)"
					, gid, code, flag, grp -> name -> str
					, __FILE__, __LINE__);
			g_ptr_array_add(info -> groups, grp);
		}
	}else{
		g_warning("No gnamelist find. (%s, %d)", __FILE__, __LINE__);
	}

	/*
	 * gmasklist
	 */
	val = json_find_first_label_all(json, "gmasklist");
	if(val != NULL){
		val = val -> child;
		json_t *cur, *tmp;
		gchar *gid, *mask;
		for(cur = val -> child; cur != NULL; cur = cur -> next){
			tmp = json_find_first_label(cur, "gid");
			if(tmp != NULL){
				gid = tmp -> child -> text;
			}
			tmp = json_find_first_label(cur, "mask");
			if(tmp != NULL){
				mask = tmp -> child -> text;
			}
			gint i;
			QQGroup *tmpg;
			for(i = 0; i < info -> groups -> len; ++i){
				tmpg = (QQGroup *)info -> groups -> pdata[i];
				if(g_strstr_len(tmpg -> gid -> str, -1, gid)
						!= NULL){
					g_debug("Find group %s (%s, %d)", gid
							, __FILE__, __LINE__);
					tmpg -> mask = g_string_new(mask);
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
		json_t *cur, *tmp;
		gchar *gid, *mark;
		for(cur = val -> child; cur != NULL; cur = cur -> next){
			tmp = json_find_first_label(cur, "gid");
			if(tmp != NULL){
				gid = tmp -> child -> text;
			}
			tmp = json_find_first_label(cur, "mark");
			if(tmp != NULL){
				mark = tmp -> child -> text;
			}
			gint i;
			QQGroup *tmpg;
			for(i = 0; i < info -> groups -> len; ++i){
				tmpg = (QQGroup *)info -> groups -> pdata[i];
				if(g_strstr_len(tmpg -> gid -> str, -1, gid)
						!= NULL){
					g_debug("Find group %s (%s, %d)", gid
							, __FILE__, __LINE__);
					tmpg -> mark = g_string_new(NULL);
					ucs4toutf8(tmpg -> mark, mark);
				}
			}
		}
	}else{
		g_warning("No gmarklist find. (%s, %d)", __FILE__, __LINE__);
	}

	if(cb != NULL){
		cb(CB_SUCCESS, "GET_GROUP_NAME_LIST_MASK", usrdata);
	}
error:
	json_free_value(&json);
	request_del(req);
	response_del(rps);
	return FALSE;
}

/*
 * Dispatch the GSource.
 * @param info 
 * @param cb
 * @param func the callback function of the GSource.
 */
static void dispatch(QQInfo *info, QQCallBack cb, gpointer usrdata
				, GSourceFunc func)
{
	GSource *src = g_idle_source_new();
	DoFuncParam *par = g_malloc(sizeof(*par));
	par -> info = info;
	par -> cb = cb;
	par -> usrdata = usrdata;
	g_source_set_callback(src, func, (gpointer)par, NULL);
	if(g_source_attach(src, info -> mainctx) <= 0){
		g_error("Attach logout source error.(%s, %d)"
				, __FILE__, __LINE__);
	}
	g_source_unref(src);
	return;
}

void qq_get_my_info(QQInfo *info, QQCallBack cb, gpointer usrdata)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR, "info == NULL in qq_get_my_info"
					, usrdata);
		}
		return;
	}

	dispatch(info, cb, usrdata, &do_get_my_info);
	return;
}

void qq_get_my_friends(QQInfo *info, QQCallBack cb, gpointer usrdata)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR, "info == NULL in qq_get_my_friends"
					, usrdata);
		}
		return;
	}

	dispatch(info, cb, usrdata, &do_get_my_friends);
	return;
}

void qq_get_group_name_list_mask(QQInfo *info, QQCallBack cb, gpointer usrdata)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR, "info == NULL in qq_get_my_friends"
					, usrdata);
		}
		return;
	}

	dispatch(info, cb, usrdata, &do_get_group_name_list_mask);
	return;
}

void qq_get_online_buddies(QQInfo *info, QQCallBack cb, gpointer usrdata)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR, "info == NULL in qq_get_online_buddies"
					, usrdata);
		}
		return;
	}

	dispatch(info, cb, usrdata, &do_get_online_buddies);
	return;
}

void qq_get_recent_contact(QQInfo *info, QQCallBack cb, gpointer usrdata)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR, "info == NULL in qq_get_recent_contact"
					, usrdata);
		}
		return;
	}

	dispatch(info, cb, usrdata, &do_get_recent_contact);
	return;
}

void qq_get_single_long_nick(QQInfo *info, QQBuddy *bdy, QQCallBack cb
					, gpointer usrdata)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR
				, "info == NULL in qq_get_single_long_nick"
				, usrdata);
		}
		return;
	}

	GSource *src = g_idle_source_new();
	struct GetLNickPar *par = g_slice_new(struct GetLNickPar);
	par -> info = info;
	par -> cb = cb;
	par -> bdy = bdy;
	g_source_set_callback(src, &do_get_single_long_nick, (gpointer)par
			, NULL);
	if(g_source_attach(src, info -> mainctx) <= 0){
		g_error("Attach logout source error.(%s, %d)"
				, __FILE__, __LINE__);
	}
	g_source_unref(src);
	return;
}


#endif

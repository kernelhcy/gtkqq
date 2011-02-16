#ifndef __LIBQQ_QQ_MANAGEINFO_H
#define __LIBQQ_QQ_MANAGEINFO_H
/*
 * Manage the information of the user
 *
 * 	1.Get friends' information.
 * 	2.Get friends' pictures.
 * 	3.Get groups' informatin.
 * 	4.Get self information.
 * 	5.Manage all above.
 */

#include <qq.h>
#include <qqtypes.h>
#include <url.h>
#include <http.h>
#include <json.h>
#include <qqhosts.h>
#include <unicode.h>
#include <string.h>

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
	g_free(par);

	gchar params[300];
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
			cb(CB_NETWORKERR, "Can not connect to server!");
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
			cb(CB_ERROR, "Response error!");
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
	json_free_value(&json);

	/*
	 * get long nick
	 */
	g_debug("Get long nick.(%s, %d)", __FILE__, __LINE__);
	g_sprintf(params, LNICKPATH"?tuin=%s&vfwebqq=%s&t=%lld"
			, info -> me -> uin -> str
			, info -> vfwebqq -> str, get_now_millisecond());
	request_set_uri(req, params);
	con = connect_to_host(SWQQHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_NETWORKERR, "Can not connect to server!");
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

	retstatus = rps -> status -> str;
	if(g_strstr_len(retstatus, -1, "200") == NULL){
		/*
		 * Maybe some error occured.
		 */
		g_warning("Resoponse status is NOT 200, but %s (%s, %d)"
				, retstatus, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_ERROR, "Response error!");
		}
		goto error;
	}

	json = NULL;
	switch(json_parse_document(&json, rps -> msg -> str))
	{
	case JSON_OK:
		break;
	default:
		g_warning("json_parser_document: syntax error. (%s, %d)"
				, __FILE__, __LINE__);
		goto error;
	}
	val = json_find_first_label_all(json, "lnick");
	if(val != NULL){
		vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> child -> text);
		info -> me -> lnick = vs;
		g_debug("lnick: %s (%s, %d)", vs -> str , __FILE__, __LINE__);
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
			cb(CB_NETWORKERR, "Can not connect to server!");
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
			cb(CB_ERROR, "Response error!");
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
		json_t *cur;
		GString *ns;
		for(cur = val -> child; cur != NULL; cur = cur -> next){
			uin = cur -> child -> child -> text;
			nick = cur -> child -> next -> child -> text;
			face = cur -> child -> next -> next -> child -> text;
			flag = cur -> child -> next -> next -> next 
				-> child -> text;
			ns = g_string_new(NULL);
			ucs4toutf8(ns, nick);
			g_debug("uin:%s nick:%s face:%s flag:%s (%s, %d)"
					, uin, ns -> str, face, flag
					, __FILE__, __LINE__);
		}
	}
	/*
	 * qq friends' marknames
	 */
	val = json_find_first_label_all(json, "marknames");
	if(val != NULL){
	}
	/*
	 * qq friends' categories
	 */
	val = json_find_first_label_all(json, "friends");
	if(val != NULL){
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
			cb(CB_NETWORKERR, "Can not connect to server!");
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
			cb(CB_ERROR, "Response error!");
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
static void dispatch(QQInfo *info, QQCallBack cb, GSourceFunc func)
{
	GSource *src = g_idle_source_new();
	DoFuncParam *par = g_malloc(sizeof(*par));
	par -> info = info;
	par -> cb = cb;
	g_source_set_callback(src, func, (gpointer)par, NULL);
	if(g_source_attach(src, info -> mainctx) <= 0){
		g_error("Attach logout source error.(%s, %d)"
				, __FILE__, __LINE__);
	}
	g_source_unref(src);
	return;
}

void qq_get_my_info(QQInfo *info, QQCallBack cb)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR, "info == NULL in qq_get_my_info");
		}
		return;
	}

	dispatch(info, cb, &do_get_my_info);
	return;
}

void qq_get_my_friends(QQInfo *info, QQCallBack cb)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR, "info == NULL in qq_get_my_friends");
		}
		return;
	}

	dispatch(info, cb, &do_get_my_friends);
	return;
}

void qq_get_group_name_list_mask(QQInfo *info, QQCallBack cb)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR, "info == NULL in qq_get_my_friends");
		}
		return;
	}

	dispatch(info, cb, &do_get_group_name_list_mask);
	return;
}

#endif

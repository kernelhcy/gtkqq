#include <qq.h>
#include <http.h>
#include <url.h>
#include <qqhosts.h>
#include <string.h>
#include <json.h>
#include <stdlib.h>
#include <glib/gprintf.h>

/*
 * Get cookie from r.
 * The cookie key is key
 */
static GString* get_cookie(Response *r, const gchar *key)
{
	gchar *cookie = response_get_header_chars(r, "Set-Cookie");
	if(cookie == NULL){
		g_warning("Server not set cookie!!(%s, %d)"
				,__FILE__, __LINE__);
		return NULL;
	}
	
	gchar *val = g_strstr_len(cookie, -1, key);
	if(val == NULL){
		g_warning("No cookie! %s(%s, %d)", key, __FILE__, __LINE__);
		return NULL;
	}
	val += (strlen(key) + 1);	//pass 'key='
	gchar *end = g_strstr_len(val, -1, ";");
	*end = '\0';
	GString *re = g_string_new(val);
	g_debug("Cookie %s=%s (%s, %d)", key, val, __FILE__, __LINE__);
	*end = ';';
	return re;
}

/*
 * Check if we need input the verify code.
 * The result is stored in info.
 *
 * return -1 if error ocurs or return 0
 */
static gint check_verify_code(QQInfo *info)
{
	g_debug("Check veriry code...(%s, %d)", __FILE__, __LINE__);
	gint ret = 0;
	gchar params[300];

	Request *req = request_new();
	Response *rps = NULL;
	int res = 0;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, VCCHECKPATH"?uin=%s&appid="APPID"&r=%.16f"
			, info -> me -> uin -> str, g_random_double());
	request_set_uri(req, params);
	request_set_default_headers(req);
	request_add_header(req, "Host", LOGINHOST);

	Connection *con = connect_to_host(LOGINHOST, 80);

	send_request(con, req);
	res = rcv_response(con, &rps);
	close_con(con);
	connection_free(con);

	if (-1 == res || !rps) {
		g_warning("Null point access (%s, %d)\n", __FILE__, __LINE__);
		ret = -1;
		goto error;
	}
	const gchar *retstatus = rps -> status -> str;
	if(g_strstr_len(retstatus, -1, "200") == NULL){
		g_warning("Server status %s (%s, %d)", retstatus
				, __FILE__, __LINE__);
		ret = NETWORK_ERR;
		goto error;
	}

	/*
	 * The http message body has two format:
	 *
	 * 	ptui_checkVC('1','9ed32e3f644d968809e8cbeaaf2cce42de62df
	 * 					ee12c14b74');
	 * 	ptui_checkVC('0','!LOB');
	 * The former means we need verify code image and the second 
	 * parameter is vc_type.
	 * The later means we don't need the verify code image. The second
	 * parameter is the verify code. The vc_type is in the header 
	 * "Set-Cookie".
	 */

	gchar *c, *s;
       	s = rps -> msg -> str;
	if(g_strstr_len(s, -1, "ptui_checkVC") == NULL){
		g_warning("Get vc_type error!(%s, %d)", __FILE__, __LINE__);
		ret = NETWORK_ERR;
		goto error;
	}

	g_debug("check vc return: %s(%s, %d)", s, __FILE__, __LINE__);
	c = g_strstr_len(s, -1, "'");
	++c;
	if(*c == '0'){
		/*
		 * We got the verify code.
		 */
		info -> need_vcimage = FALSE;
		s = c;
		c = g_strstr_len(s, -1, "'");
		s = c + 1; 
		c = g_strstr_len(s, -1, "'");
		s = c + 1;
		c = g_strstr_len(s, -1, "'");
		*c = '\0';
		info -> verify_code = g_string_new(s);
		g_debug("Verify code : %s (%s, %d)", info -> verify_code -> str
				, __FILE__, __LINE__);
		/*
		 * We need get the ptvfsession from the header "Set-Cookie"
		 */
		info -> ptvfsession = get_cookie(rps, "ptvfsession");
	}else if(*c == '1'){
		/*
		 * We need get the verify image.
		 */
		info -> need_vcimage = TRUE;
		s = c;
		c = g_strstr_len(s, -1, "'");
		s = c + 1;
		c = g_strstr_len(s, -1, "'");
		s = c + 1;
		c = g_strstr_len(s, -1, "'");
		*c = '\0';
		info -> vc_type = g_string_new(s);
		g_debug("We need verify code image! vc_type: %s (%s, %d)"
				, info -> vc_type -> str, __FILE__, __LINE__);
	}else{
		g_warning("Unknown return value!(%s, %d)", __FILE__, __LINE__);
		ret = NETWORK_ERR;
		goto error;
	}
error:
	request_del(req);
	response_del(rps);
	return ret;
}

/*
 * Get the verify code image form the server
 */
static gint get_vc_image(QQInfo *info)
{
	if(info -> vc_type == NULL || info -> vc_type -> len <=0){
		g_warning("Need vc_type!!(%s, %d)", __FILE__, __LINE__);
		return PARAMETER_ERR;
	}
	gint ret = 0;
	gchar params[500];
	gint res = 0;

	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, IMAGEPATH"?aid="APPID"&r=%.16f&uin=%s&vc_type=%s"
			, g_random_double(), info -> me -> uin -> str
			, info -> vc_type -> str);
	request_set_uri(req, params);
	request_set_default_headers(req);
	request_add_header(req, "Host", IMAGEHOST);

	Connection *con = connect_to_host(IMAGEHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		request_del(req);
		return NETWORK_ERR;
	}

	send_request(con, req);
	res = rcv_response(con, &rps);
	close_con(con);
	connection_free(con);
	const gchar *retstatus = rps -> status -> str;
	if(g_strstr_len(retstatus, -1, "200") == NULL){
		g_warning("Server status %s (%s, %d)", retstatus
				, __FILE__, __LINE__);
		ret = NETWORK_ERR;
		goto error;
	}

	info -> vc_image_data = g_string_new(NULL);
	g_string_append_len(info -> vc_image_data, rps -> msg -> str
				, rps -> msg -> len);
    info -> vc_image_size = rps -> msg -> len;

	gchar *ct = response_get_header_chars(rps, "Content-Type");
	gchar *vc_ftype = g_strstr_len(ct, -1, "image/");
	g_debug("vc content type: %s(%s, %d)", vc_ftype, __FILE__, __LINE__);
	if(vc_ftype == NULL){
		g_warning("Unknown verify code image file type!(%s, %d)"
				, __FILE__, __LINE__);
		g_string_free(info -> vc_image_data, TRUE);
		info -> vc_image_data = NULL;
		ret = NETWORK_ERR;
		goto error;

	}
	vc_ftype += (sizeof("image/") - 1);
	g_debug("Verify code image file type: %s len %d (%s, %d)", vc_ftype
                    , info -> vc_image_size, __FILE__, __LINE__);
	g_strstrip(vc_ftype);
	info -> vc_image_type = g_string_new(vc_ftype);

error:
	request_del(req);
	response_del(rps);
	return ret;
}


/*
 * Get version number
 */
static gint get_version(QQInfo *info)
{
	int ret = NO_ERR;
	Request *req = request_new();
	Response *rps = NULL;
	gint res = 0;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	request_set_uri(req, VERPATH);
	request_set_default_headers(req);
	request_add_header(req, "Host", LOGINPAGEHOST);

	Connection *con = connect_to_host(LOGINPAGEHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		request_del(req);
		return NETWORK_ERR;
	}
	send_request(con, req);
	res = rcv_response(con, &rps);
	close_con(con);
	connection_free(con);
	const gchar *retstatus = rps -> status -> str;
	if (-1 == res || !rps) {
		g_warning("Null point access (%s, %d)\n", __FILE__, __LINE__);
		ret = -1;
		goto error;
	}
	if(g_strstr_len(retstatus, -1, "200") == NULL){
		g_warning("Server status %s (%s, %d)", retstatus
				, __FILE__, __LINE__);
		ret = NETWORK_ERR;
		goto error;
	}
	
	gchar *lb, *rb;
	gchar *ms = rps -> msg -> str;
	lb = g_strstr_len(ms, -1, "(");
	if(lb == NULL){
		g_warning("Get version  error!! %s (%s, %d)",rps -> msg -> str
                                        , __FILE__, __LINE__);
		ret = NETWORK_ERR;
		goto error;
	}
	++lb;
	rb = g_strstr_len(ms, -1, ")");
	*rb = '\0';
	info -> version = g_string_new(lb);
	g_debug("Version: %s(%s, %d)", lb, __FILE__, __LINE__);
error:
	request_del(req);
	response_del(rps);
	return ret;
}

/*
 * Get the check sum of password and verify code.
 *
 * First, compute check sum of password for three times.
 * Then, join the result with the capitalizaion of the verify code.
 * Compute the chekc sum of the new string.
 */
GString* get_pwvc_md5(const gchar *pwd, const gchar *vc, GError **err)
{
	guint8 buf[100];
	gsize bsize = 100;
	
	GChecksum *cs = g_checksum_new(G_CHECKSUM_MD5);
	g_checksum_update(cs, (const guchar*)pwd, strlen(pwd));
	g_checksum_get_digest(cs, buf, &bsize);
	g_checksum_free(cs);
	
	cs = g_checksum_new(G_CHECKSUM_MD5);
	g_checksum_update(cs, buf, bsize);
	g_checksum_get_digest(cs, buf, &bsize);
	g_checksum_free(cs);
	
	cs = g_checksum_new(G_CHECKSUM_MD5);
	g_checksum_update(cs, buf, bsize);
	const gchar * md5_3 = g_checksum_get_string(cs);
	md5_3 = g_ascii_strup(md5_3, strlen(md5_3));
	gchar buf2[100];
	g_sprintf(buf2, "%s%s", md5_3, vc);
	g_checksum_free(cs);

	gchar *tmp1;
	tmp1 = g_ascii_strup(buf2, strlen(buf2));
	tmp1 = g_compute_checksum_for_string(G_CHECKSUM_MD5
						, tmp1, -1);
	tmp1 = g_ascii_strup(tmp1, strlen(tmp1));
	GString *re = g_string_new(tmp1);
	g_free(tmp1);

	return re;
}

/*
 * Get ptca and skey
 * return the status returned by the server.
 * If error occured, store the error message in info -> errmsg
 */
static gint get_ptcz_skey(QQInfo *info, const gchar *p)
{
	gint ret = 0;
	gint res = 0;
	gchar params[300];

	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, LOGINPATH"?u=%s&p=%s&verifycode=%s&webqq_type=40&"
			"remember_uin=0&aid="APPID"&login2qq=1&u1=%s&h=1&"
			"ptredirect=0&ptlang=2052&from_ui=1&pttype=1"
			"&dumy=&fp=loginerroralert&action=4-30-764935&mibao_css=m_webqq"
			, info -> me -> uin -> str, p, info -> verify_code -> str
			, LOGIN_S_URL);
	request_set_uri(req, params);
	request_set_default_headers(req);
	request_add_header(req, "Host", LOGINHOST);
	request_add_header(req, "Referer", "http://ui.ptlogin2.qq.com/cgi-bin/"
						"login?target=self&style=4&appid=1003903&enable_ql"
						"ogin=0&no_verifyimg=1&s_url=http%3A%2F%2Fweb2.qq.c"
						"om%2Floginproxy.html%3Flogin_level%3D3"
						"&f_url=loginerroralert");
	if(info -> ptvfsession != NULL){
		g_sprintf(params, "ptvfsession=%s; "
				, info -> ptvfsession -> str);
		request_add_header(req, "Cookie", params);
	}

	Connection *con = connect_to_host(LOGINHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		request_del(req);
		return -1;
	}

	send_request(con, req);
	res = rcv_response(con, &rps);
	close_con(con);
	connection_free(con);

	if (-1 == res || !rps) {
		g_warning("Null point access (%s, %d)\n", __FILE__, __LINE__);
		ret = -1;
		goto error;
	}
	const gchar *retstatus = rps -> status -> str;
	if(g_strstr_len(retstatus, -1, "200") == NULL){
		g_warning("Server status %s (%s, %d)", retstatus
				, __FILE__, __LINE__);
		ret = -1;
		goto error;
	}

	gint status;
	gchar *sbe = g_strstr_len(rps -> msg -> str, -1, "'");
	++sbe;
	gchar *sen = g_strstr_len(sbe, -1, "'");
	*sen = '\0';
	status = strtol(sbe, NULL, 10);
	*sen = '\'';
	ret = status;
	if(status == 0){
		g_debug("Success.(%s, %d)", __FILE__, __LINE__);
	}else if(status == 1){
		g_warning("Server busy! Please try again.(%s, %d)"
				, __FILE__, __LINE__);
		//g_sprintf(info -> errmsg, "Server busy!");
		goto error;
	}else if(status == 2){
		g_warning("Out of date QQ number!(%s, %d)"
				, __FILE__, __LINE__);
		//g_sprintf(info -> errmsg, "Out of date QQ number.");
		goto error;
	}else if(status == 3){
		g_warning("Wrong password!(%s, %d)", __FILE__, __LINE__);
		//g_sprintf(info -> errmsg, "Wrong password.");
		goto error;
	}else if(status == 4){
		g_warning("Wrong verify code!(%s, %d)", __FILE__, __LINE__);
		//g_sprintf(info -> errmsg, "Wrong verify code.");
		goto error;
	}else if(status == 5){
		g_warning("Verify failed!(%s, %d)", __FILE__, __LINE__);
		//g_sprintf(info -> errmsg, "Verify failed.");
		goto error;
	}else if(status == 6){
		g_warning("You may need to try login again.(%s, %d)", __FILE__
				, __LINE__);
		//g_sprintf(info -> errmsg, "Please try again.");
		goto error;
	}else if(status == 7){
		g_warning("Wrong input!(%s, %d)", __FILE__, __LINE__);
		//g_sprintf(info -> errmsg, "Wrong input.");
		goto error;
	}else if(status == 8){
		g_warning("Too many logins on this IP. Please try again.(%s, %d)"
				, __FILE__, __LINE__);
		//g_sprintf(info -> errmsg, "Too many logins on this IP.");
		goto error;
	}else{
		g_warning("Server response message:(%s, %d)\n\t%s"
				, __FILE__, __LINE__, rps -> msg -> str);
		goto error;
	}

	info -> ptcz = get_cookie(rps, "ptcz");
	info -> skey = get_cookie(rps, "skey");
	info -> ptwebqq = get_cookie(rps, "ptwebqq");
	info -> ptuserinfo = get_cookie(rps, "ptuserinfo");
    info -> uin = get_cookie(rps, "uin");
    info -> ptisp = get_cookie(rps, "ptisp");
    info -> pt2gguin = get_cookie(rps, "pt2gguin");
	//sotre the cookie
	info -> cookie = g_string_new("");
    g_string_append(info -> cookie, "ptcz=");
    g_string_append(info -> cookie, info -> ptcz -> str);
    g_string_append(info -> cookie, ";skey=");
    g_string_append(info -> cookie, info -> skey -> str);
    g_string_append(info -> cookie, ";ptwebqq=");
    g_string_append(info -> cookie, info -> ptwebqq -> str);
    g_string_append(info -> cookie, ";ptuserinfo=");
    g_string_append(info -> cookie, info -> ptuserinfo -> str);
    g_string_append(info -> cookie, ";uin=");
    g_string_append(info -> cookie, info -> uin -> str);
    g_string_append(info -> cookie, ";ptisp=");
    g_string_append(info -> cookie, info -> ptisp -> str);
    g_string_append(info -> cookie, ";pt2gguin=");
    g_string_append(info -> cookie, info -> pt2gguin -> str);
error:
	request_del(req);
	response_del(rps);
	return ret;
}

/*
 * Generate the clientid
 *
 * In eqq.all.js, webqq.eqq.rpcservice: getclientid
 * JS code:
 * 	clientid = String(k.random(0, 99)) + String((new Date()).getTime() %
 * 						1000000)
 */
static GString *generate_clientid()
{
	gint32 r = g_random_int_range(10, 99);
	GTimeVal now;
	g_get_current_time(&now);
	glong t = now.tv_usec % 1000000;

	gchar buf[20];
	g_sprintf(buf, "%d%ld", (int)r, t);

	return g_string_new(buf);
}

/*
 * Get the psessionid.
 *
 * This function is the last step of loginning
 */
static int get_psessionid(QQInfo *info)
{
	int ret = NO_ERR;
	gint res = 0;
	if(info -> ptwebqq == NULL || info -> ptwebqq -> len <= 0){
		g_warning("Need ptwebqq!!(%s, %d)", __FILE__, __LINE__);
		return PARAMETER_ERR;
	}
	
	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "POST");
	request_set_version(req, "HTTP/1.1");
	request_set_uri(req, PSIDPATH);
	request_set_default_headers(req);
	request_add_header(req, "Host", PSIDHOST);
	request_add_header(req, "Cookie2", "$Version=1");
	request_add_header(req, "Referer"
			, "http://d.web2.qq.com/proxy.html?v=20101025002");
	GString *clientid = generate_clientid();
	info -> clientid = clientid;
	g_debug("clientid: %s", clientid -> str);

	gchar* msg = g_malloc(500);
	g_snprintf(msg, 500, "{\"status\":\"%s\",\"ptwebqq\":\"%s\","
			"\"passwd_sig\":""\"\",\"clientid\":\"%s\""
			", \"psessionid\":null}"
			, info -> me -> status -> str, info -> ptwebqq -> str
			, clientid -> str);

	gchar *escape = g_uri_escape_string(msg, NULL, FALSE);
	g_snprintf(msg, 500, "r=%s", escape);
	g_free(escape);

	request_append_msg(req, msg, strlen(msg));
	gchar cl[10];
	g_sprintf(cl, "%u", (unsigned int)strlen(msg));
	request_add_header(req, "Content-Length", cl);
	request_add_header(req, "Content-Type"
			, "application/x-www-form-urlencoded");
	g_free(msg);

	gchar *cookie = g_malloc(2000);
	gint idx = 0;
	if(info -> ptvfsession != NULL){
		idx += g_snprintf(cookie + idx, 2000 - idx, "ptvfsession=%s; "
				, info -> ptvfsession -> str);
	}
	idx += g_snprintf(cookie + idx, 2000 - idx, "%s"
			, info -> cookie -> str);
	request_add_header(req, "Cookie", cookie);
	g_free(cookie);

	Connection *con = connect_to_host(PSIDHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		request_del(req);
		return NETWORK_ERR;
	}

	send_request(con, req);
	res = rcv_response(con, &rps);

	if (-1 == res || !rps) {
		g_warning("Null point access (%s, %d)\n", __FILE__, __LINE__);
		ret = -1;
		goto error;
	}
	const gchar *retstatus = rps -> status -> str;
	if(g_strstr_len(retstatus, -1, "200") == NULL){
		g_warning("Server status %s (%s, %d)", retstatus
				, __FILE__, __LINE__);
		ret = NETWORK_ERR;
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
		ret = NETWORK_ERR;
		goto error;
	}

	json_t *val;
	val = json_find_first_label_all(json, "retcode");
	if(val -> child -> text[0] != '0'){
		g_warning("Server return code %s(%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
		ret = NETWORK_ERR;
		goto error;
	}
	val = json_find_first_label_all(json, "seskey");
	if(val != NULL){
		g_debug("seskey: %s (%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
		info -> seskey = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "cip");
	if(val != NULL){
		info -> cip = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "index");
	if(val != NULL){
		info -> index = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "port");
	if(val != NULL){
		info -> port = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "status");
	{
		g_debug("status: %s (%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
	}
	val = json_find_first_label_all(json, "vfwebqq");
	if(val != NULL){
		g_debug("vfwebqq: %s (%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
		info -> vfwebqq = g_string_new(val -> child -> text);
	}
	val = json_find_first_label_all(json, "psessionid");
	if(val != NULL){
		g_debug("psessionid: %s (%s, %d)", val -> child -> text
				, __FILE__, __LINE__);
		info -> psessionid = g_string_new(val -> child -> text);
	}else{
		g_debug("Can not find pesssionid!(%s, %d): %s"
				, __FILE__, __LINE__, rps -> msg -> str);
	}

error:
	json_free_value(&json);	
	close_con(con);
	connection_free(con);
	request_del(req);
	response_del(rps);
	return ret;
}

/*
 * Do the real login.
 * Run in the main event loop.
 */
static gint do_login(QQInfo *info, const gchar *uin, const gchar *passwd
		        , const gchar *status, GError **err)
{
	g_debug("Get version...(%s, %d)", __FILE__, __LINE__);
    gint retcode = NO_ERR;
    retcode = get_version(info);
	if(retcode != NO_ERR){
        create_error_msg(err, retcode, "Get version error.");
		return retcode;
	}

    if(info -> verify_code == NULL){
        g_warning("Need verify code!!(%s, %d)", __FILE__, __LINE__);
        create_error_msg(err, WRONGVC_ERR, "Need verify code.");
        return WRONGVC_ERR;
    }

	g_debug("Login...(%s, %d)", __FILE__, __LINE__);
	GString *md5 = get_pwvc_md5(passwd, info -> verify_code -> str, err);

	g_debug("Get ptcz and skey...(%s, %d)", __FILE__, __LINE__);
	gint ret = get_ptcz_skey(info, md5 -> str);
	if(ret != 0){
		g_string_free(md5, TRUE);
		const gchar * msg;
		switch(ret)
		{
		case 1:
			msg = "System busy! Please try again.";
            retcode = NETWORK_ERR;
			break;
		case 2:
			msg = "Out of date QQ number.";
            retcode = WRONGUIN_ERR;
			break;
		case 3:
		case 6:
			msg = "Wrong password.";
            retcode = WRONGPWD_ERR;
			break;
		case 4:
			msg = "Wrong verify code.";
            retcode = WRONGVC_ERR;
			break;
		case 5:
			msg = "Verify failed.";
            retcode = OTHER_ERR;
			break;
		default:
			msg = "Error occured! Please try again.";
            retcode = OTHER_ERR;
			break;
		}
        create_error_msg(err, retcode, msg);
		return retcode;
	}
	g_string_free(md5, TRUE);

	g_debug("Get psessionid...(%s, %d)", __FILE__, __LINE__);
    retcode = get_psessionid(info);
	if(retcode != NO_ERR){
        create_error_msg(err, retcode, "Get psessionid error.");
		return retcode;
	}
    
	g_debug("Initial done.(%s, %d)", __FILE__, __LINE__);
	return NO_ERR;
}

gint qq_login(QQInfo *info, const gchar *qqnum, const gchar *passwd
		        , const gchar *status, GError **err)
{
	if(info == NULL){
		g_warning("info == NULL. (%s, %d)", __FILE__, __LINE__);
        create_error_msg(err, PARAMETER_ERR, "info == NULL");
		return PARAMETER_ERR;
	}
	if(qqnum == NULL || passwd == NULL || strlen(qqnum) == 0){
		g_warning("qqnumber or passwd == NULL.(%s, %d)"
				, __FILE__, __LINE__);
        create_error_msg(err, PARAMETER_ERR, "qqnum or passwd  == NULL");
		return PARAMETER_ERR;
	}

    //
    // The user's uin and qq number are the same.
    //
    qq_buddy_set(info -> me, "qqnumber", qqnum);
    qq_buddy_set(info -> me, "uin", qqnum);

	if(status != NULL){
        qq_buddy_set(info -> me, "status", status);
	}else{
        qq_buddy_set(info -> me, "status", "online");
	}

    return do_login(info, qqnum, passwd, status, err);
}

static gint do_logout(QQInfo *info, GError **err)
{
    gint ret_code = 0;
	gint res = 0;
	g_debug("Logout... (%s, %d)", __FILE__, __LINE__);
	if(info -> psessionid == NULL || info -> psessionid -> len <= 0){
		g_warning("Need psessionid !!(%s, %d)", __FILE__, __LINE__);
		return -1;
	}

	gchar params[300];
	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, LOGOUTPATH"?clientid=%s&psessionid=%s&t=%ld"
			, info -> clientid -> str
			, info -> psessionid -> str, get_now_millisecond());
	request_set_uri(req, params);
	request_set_default_headers(req);
	request_add_header(req, "Host", LOGOUTHOST);
	request_add_header(req, "Cookie", info -> cookie -> str);
	request_add_header(req, "Referer", REFERER);

	Connection *con = connect_to_host(LOGOUTHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		request_del(req);
		return -1;
	}

	send_request(con, req);
	res = rcv_response(con, &rps);
	close_con(con);
	connection_free(con);

	if (-1 == res || !rps) {
		g_warning("Null point access (%s, %d)\n", __FILE__, __LINE__);
		ret_code = -1;
		goto error;
	}
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

	json_t *retcode, *result;
	retcode = json_find_first_label_all(json, "retcode");
	result = json_find_first_label_all(json, "result");
	if(retcode != NULL && result != NULL){
		if(g_strstr_len(result -> child -> text, -1, "ok") != NULL){
			g_debug("Logout ok!(%s, %d)", __FILE__, __LINE__);
            ret_code = 0;
		}
	}else{
		g_debug("(%s, %d)%s", __FILE__, __LINE__, rps -> msg -> str);
	}
	
	json_free_value(&json);
error:
	request_del(req);
	response_del(rps);
	return ret_code;
}


gint qq_logout(QQInfo *info, GError **err)
{
	if(info == NULL){
		return -1;
	}
	return do_logout(info, err);;
}

/*
 * Check if we need the verify code.
 */
static gint do_check_verifycode(QQInfo *info, const gchar *uin, GError **err)
{

	if(check_verify_code(info) == -1){
		return -1;
	}

	if(info -> need_vcimage){
		//we need it.
		g_debug("Get verify code image...(%s, %d)", __FILE__, __LINE__);
		get_vc_image(info);
	}

	return 0;
}

gint qq_check_verifycode(QQInfo *info, const gchar *uin, GError **err)
{
	if(info == NULL){
		return -1;
	}

	if(uin == NULL){
		g_warning("Need uin in check_verifycode!(%s, %d)", __FILE__
				, __LINE__);
		return -1;
	}
	if(info -> me -> uin != NULL){
		g_string_free(info -> me -> uin, TRUE);
	}
	info -> me -> uin = g_string_new(uin);

	return do_check_verifycode(info, uin, err);
}

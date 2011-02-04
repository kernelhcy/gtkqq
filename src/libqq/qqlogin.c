#include <qq.h>
#include <http.h>
#include <url.h>
#include <qqhosts.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <json.h>

/*
 * The main loop thread's main function.
 */
static gpointer start_main_loop(gpointer *data)
{
	QQInfo *info = (QQInfo*)data;
	if(info == NULL){
		return NULL;
	}

	if(info -> mainloop == NULL || info -> mainctx == NULL){
		g_warning("Havn't create the main event loop!!(%s, %d)"
				, __FILE__, __LINE__);
		info -> mainloop = g_main_loop_new(NULL, FALSE);
		info -> mainctx = g_main_loop_get_context(info -> mainloop);
		g_warning("Create the main event loop. done.(%s, %d)"
				, __FILE__, __LINE__);
	}


	g_debug("Run the main event loop...(%s, %d)", __FILE__, __LINE__);
	//start the main loop
	g_main_loop_run(info -> mainloop);

	/*
	 * Will not arrive here untill the program exits. 
	 */
	g_debug("Quit main event loop.(%s, %d)", __FILE__, __LINE__);
	return NULL;
}

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
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, VCCHECKPATH"?uin=%s&appid="APPID"&r=%.16f"
			, info -> uin -> str, g_random_double());
	request_set_uri(req, params);
	request_set_default_headers(req);
	request_add_header(req, "Host", LOGINHOST);

	Connection *con = connect_to_host(LOGINHOST, 80);
	send_request(con, req);
	rcv_response(con, &rps);
	close_con(con);
	connection_free(con);

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
		ret = -1;
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
		ret = -1;
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
	gint ret = 0;
	gchar params[500];  

	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, IMAGEPATH"?uin=%s&aid="APPID"&r=%.16f&vc_type=%s"
			, info -> uin -> str, g_random_double()
			, info -> vc_type -> str);
	request_set_uri(req, params);
	request_set_default_headers(req);
	request_add_header(req, "Host", IMAGEHOST);

	Connection *con = connect_to_host(IMAGEHOST, 80);
	send_request(con, req);
	rcv_response(con, &rps);
	close_con(con);
	connection_free(con);

	info -> vc_image_data = g_string_new(NULL);
	g_string_append_len(info -> vc_image_data, rps -> msg -> str
				, rps -> msg -> len);

	gchar *ct = response_get_header_chars(rps, "Content-Type");
	gchar *vc_ftype = g_strstr_len(ct, -1, "image/");
	g_debug("vc content type: %s(%s, %d)", vc_ftype, __FILE__, __LINE__);
	if(vc_ftype == NULL){
		g_warning("Unknown verify code image file type!(%s, %d)"
				, __FILE__, __LINE__);
		g_string_free(info -> vc_image_data, TRUE);
		info -> vc_image_data = NULL;
		ret = -1;
		goto error;

	}
	vc_ftype += (sizeof("image/") - 1);
	g_debug("Verify code image file type: %s (%s, %d)", vc_ftype
				, __FILE__, __LINE__);
	g_strstrip(vc_ftype);
	info -> vc_image_type = g_string_new(vc_ftype);

error:
	request_del(req);
	response_del(rps);
	return ret;
}

/*
 * Save the verify code image to file.
 * 	~/verifycode.*ext*
 */
static gint save_vc_to_file(QQInfo *info)
{
	if(info == NULL || info -> vc_image_type == NULL){
		return -1;
	}
	gchar fn[100];
	g_sprintf(fn, "/home/hcy/verifycode.%s", info -> vc_image_type -> str);

	g_debug("Create image file : %s (%s, %d)", fn, __FILE__, __LINE__);
	gint fd = creat(fn, S_IRUSR | S_IWUSR);
	if(fd == -1){
		g_warning("Create verify code image error! %s (%s, %d)"
				, strerror(errno), __FILE__, __LINE__);
		return -1;
	}

	GError *err = NULL;
	GIOChannel *ioc = g_io_channel_unix_new(fd);
	if(ioc == NULL){
		g_warning("Create io channle error!! %d %s "
				"(%s, %d)", err -> code , err -> message
				, __FILE__, __LINE__);
		return -1;
	}
	err = NULL;
	g_io_channel_set_encoding(ioc, NULL, &err);
	err = NULL;
	GIOStatus status;
	gsize bytes_w;
	status = g_io_channel_write_chars(ioc, info -> vc_image_data -> str,
				info -> vc_image_data -> len, &bytes_w, &err);
	switch(status)
	{
	case G_IO_STATUS_NORMAL:
		if(bytes_w < info -> vc_image_data -> len){
			g_warning("Not write all verify code image data "
					"to file!(%s, %d)", __FILE__
					, __LINE__);
		}
		break;
	case G_IO_STATUS_ERROR:
		g_warning("Write verify code image file error! %d %s "
				"(%s, %d)", err -> code , err -> message
				, __FILE__, __LINE__);
		goto error;
	case G_IO_STATUS_EOF:
	case G_IO_STATUS_AGAIN:
	default:
		break;
	}

error:
	g_io_channel_close(ioc);
	g_io_channel_unref(ioc);
	return 0;
}

/*
 * Get version number
 */
static gint get_version(QQInfo *info)
{
	int ret = 0;
	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	request_set_uri(req, VERPATH);
	request_set_default_headers(req);
	request_add_header(req, "Host", LOGINPAGEHOST);

	Connection *con = connect_to_host(LOGINPAGEHOST, 80);
	send_request(con, req);
	rcv_response(con, &rps);
	close_con(con);
	connection_free(con);
	
	gchar *lb, *rb;
	gchar *ms = rps -> msg -> str;
	lb = g_strstr_len(ms, -1, "(");
	if(lb == NULL){
		g_warning("Get version  error!!(%s, %d)", __FILE__, __LINE__);
		ret = -1;
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
GString* get_pwvc_md5(const gchar *pwd, const gchar *vc)
{
	int ret = 0;
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
static int get_ptcz_skey(QQInfo *info, const gchar *p)
{
	int ret = 0;
	gchar params[300];

	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, LOGINPATH"?u=%s&p=%s&verifycode=%s&webqq_type=1&"
			"remember_uin=0&aid="APPID"&u1=%s&h=1&"
			"ptredirect=0&ptlang=2052&from_ui=1&pttype=1"
			"&dumy=&fp=loginerroralert&mibao_css="
			, info -> uin -> str, p, info -> verify_code -> str
			, LOGIN_S_URL);
	request_set_uri(req, params);
	request_set_default_headers(req);
	request_add_header(req, "Host", LOGINHOST);
	if(info -> ptvfsession != NULL){
		g_sprintf(params, "ptvfsession=%s; "
				, info -> ptvfsession -> str);
		request_add_header(req, "Cookie", params);
	}

	Connection *con = connect_to_host(LOGINHOST, 80);
	send_request(con, req);
	rcv_response(con, &rps);
	close_con(con);
	connection_free(con);

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
		g_debug("Server busy! Please try again.(%s, %d)"
				, __FILE__, __LINE__);
		g_sprintf(info -> errmsg, "Server busy!");
		goto error;
	}else if(status == 2){
		g_debug("Out of date QQ number!(%s, %d)"
				, __FILE__, __LINE__);
		g_sprintf(info -> errmsg, "Out of date QQ number.");
		goto error;
	}else if(status == 3){
		g_debug("Wrong password!(%s, %d)", __FILE__, __LINE__);
		g_sprintf(info -> errmsg, "Wrong password.");
		goto error;
	}else if(status == 4){
		g_debug("Wrong verify code!(%s, %d)", __FILE__, __LINE__);
		g_sprintf(info -> errmsg, "Wrong verify code.");
		goto error;
	}else if(status == 5){
		g_debug("Verify failed!(%s, %d)", __FILE__, __LINE__);
		g_sprintf(info -> errmsg, "Verify failed.");
		goto error;
	}else if(status == 6){
		g_debug("You may need to try login again.(%s, %d)", __FILE__
				, __LINE__);
		g_sprintf(info -> errmsg, "Please try again.");
		goto error;
	}else if(status == 7){
		g_debug("Wrong input!(%s, %d)", __FILE__, __LINE__);
		g_sprintf(info -> errmsg, "Wrong input.");
		goto error;
	}else if(status == 8){
		g_debug("Too many logins on this IP. Please try again.(%s, %d)"
				, __FILE__, __LINE__);
		g_sprintf(info -> errmsg, "Too many logins on this IP.");
		goto error;
	}else{
		g_debug("Server response message:(%s, %d)\n\t%s"
				, __FILE__, __LINE__, rps -> msg -> str);
		goto error;
	}

	info -> ptcz = get_cookie(rps, "ptcz");
	info -> skey = get_cookie(rps, "skey");
	info -> ptwebqq = get_cookie(rps, "ptwebqq");
	info -> ptuserinfo = get_cookie(rps, "ptuserinfo");
	//sotre the cookie
	info -> cookie = g_string_new(response_get_header_chars(rps
				, "Set-Cookie"));
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
	g_sprintf(buf, "%d%d", r, t);

	return g_string_new(buf);
}

/*
 * Get the psessionid.
 *
 * This function is the last step of loginning
 */
static int get_psessionid(QQInfo *info)
{
	int ret = 0;
	
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
	g_snprintf(msg, 500, "{\"status\":\"\",\"ptwebqq\":\"%s\","
			"\"passwd_sig\":""\"\",\"clientid\":\"%s\"}"
			, info -> ptwebqq -> str, clientid -> str);
	gchar *escape = g_uri_escape_string(msg, NULL, FALSE);
	g_snprintf(msg, 500, "r=%s", escape);
	g_free(escape);

	request_append_msg(req, msg, strlen(msg));
	gchar cl[10];
	g_sprintf(cl, "%d", strlen(msg));
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
	send_request(con, req);
	rcv_response(con, &rps);

	JSON *json = JSON_new();
	JSON_set_raw_data_c(json, rps -> msg -> str, rps -> msg -> len);
	if(JSON_parse(json) != 0){
		g_warning("Parse JSON data error!!(%s, %d)", __FILE__, __LINE__);
		ret = -1;
		goto error;
	}
	JSON_DATA_T type;
	JSON_data *jd = (JSON_data*)json -> result;
	if(jd -> type != JSON_OBJECT){
		g_warning("JSON data format error!!(%s, %d)", __FILE__, __LINE__);
		ret = -1;
		goto error;
	}
	GString *val;
	val = (GString *)JSON_find_pair_value(json, JSON_STRING, "retcode");
	if(val -> str[0] != '0'){
		g_warning("Server return code %s", val -> str);
		ret = -1;
		goto error;
	}
	val = (GString *)JSON_find_pair_value(json, JSON_STRING, "seskey");
	g_debug("seskey: %s (%s, %d)", val -> str, __FILE__, __LINE__);
	info -> seskey = g_string_new(val -> str);
	val = (GString *)JSON_find_pair_value(json, JSON_STRING, "cip");
	info -> cip = g_string_new(val -> str);
	val = (GString *)JSON_find_pair_value(json, JSON_STRING, "index");
	info -> index = g_string_new(val -> str);
	val = (GString *)JSON_find_pair_value(json, JSON_STRING, "port");
	info -> port = g_string_new(val -> str);
	val = (GString *)JSON_find_pair_value(json, JSON_STRING, "status");
	g_debug("status: %s (%s, %d)", val -> str, __FILE__, __LINE__);
	val = (GString *)JSON_find_pair_value(json, JSON_STRING, "vfwebqq");
	g_debug("vfwebqq: %s (%s, %d)", val -> str, __FILE__, __LINE__);
	info -> vfwebqq = g_string_new(val -> str);
	val = (GString *)JSON_find_pair_value(json, JSON_STRING, "psessionid");
	g_debug("psessionid: %s (%s, %d)", val -> str, __FILE__, __LINE__);
	info -> psessionid = g_string_new(val -> str);

error:
	JSON_free(json);
	close_con(con);
	connection_free(con);
	request_del(req);
	response_del(rps);
	return ret;
}

/*
 * do_init function parameter struct
 */
struct InitParam{
	QQCallBack cb;
	QQInfo *info;
	gchar *passwd;
	
};
/*
 * Do the real login.
 * Run in the main event loop.
 */
static gboolean do_login(gpointer data)
{
	if(data == NULL){
		return FALSE;
	}
	QQCallBack cb = ((struct InitParam *)data) -> cb;
	QQInfo *info = ((struct InitParam *)data) -> info;
	gchar *passwd = ((struct InitParam *)data) -> passwd;

	if(cb != NULL){
		g_debug("Call the callback function in do_init.(%s, %d)"
				, __FILE__, __LINE__);
		cb(CB_SUCCESS, NULL);
	}

	check_verify_code(info);

	if(info -> need_vcimage){
		g_debug("Get verify code image...(%s, %d)", __FILE__, __LINE__);
		get_vc_image(info);
		g_debug("Verify code image length: %d (%s, %d)"
				, info -> vc_image_data -> len
				, __FILE__, __LINE__);
		g_debug("Write verify code image data to file ... (%s, %d)"
				, __FILE__, __LINE__);
		save_vc_to_file(info);
	}
	g_debug("Get version...(%s, %d)", __FILE__, __LINE__);
	get_version(info);

	g_debug("Login...(%s, %d)", __FILE__, __LINE__);
	if(info -> need_vcimage){
		gchar vc[10];
		g_printf("Please input the verify code:\n");
		scanf("%s", vc);
		info -> verify_code = g_string_new(vc);

	}
	GString *md5 = get_pwvc_md5(passwd, info -> verify_code -> str);

	g_debug("Get ptcz and skey...(%s, %d)", __FILE__, __LINE__);
	if(get_ptcz_skey(info, md5 -> str) != 0){
		g_string_free(md5, TRUE);
		return FALSE;
	}
	g_string_free(md5, TRUE);

	g_debug("Get psessionid...(%s, %d)", __FILE__, __LINE__);
	get_psessionid(info);

	g_debug("Initial done.");

	g_slice_free(struct InitParam, data);
	g_debug("Free the struct InitParam.(%s, %d)", __FILE__, __LINE__);

	return FALSE;
}

void qq_login(QQInfo *info, const gchar *uin, const gchar *passwd
		, QQCallBack cb)
{
	if(info == NULL){
		g_warning("info == NULL. (%s, %d)", __FILE__, __LINE__);
		return;
	}
	if(uin == NULL || passwd == NULL){
		g_warning("uin or passwd == NULL.(%s, %d)"
				, __FILE__, __LINE__);
		return;
	}
	GString *usrnum = g_string_new(uin);
	info -> uin = usrnum;
	/*
	 * When call gtk_init(), the g_thread_init() also be called.
	 * This liberary may not be used with gtk, so we call g_thread_init()
	 * here.
	 */
	if(!g_thread_supported()){
		g_thread_init(NULL);
	}else{
		if(cb){
			cb(CB_FAILED, "Need thread supported!!");
		}
		g_error("Need thread supported!");
		return;
	}
	g_debug("Initial the thread done.(%s, %d)", __FILE__, __LINE__);

	info -> mainloop = g_main_loop_new(NULL, FALSE);
	info -> mainctx = g_main_loop_get_context(info -> mainloop);
	g_debug("Create the main event loop. done.(%s, %d)"
				, __FILE__, __LINE__);

	GError *err;
	info -> mainloopthread = g_thread_create((GThreadFunc)start_main_loop
						, (gpointer)info, FALSE
						, &err);
	if(info -> mainloopthread == NULL){
		g_error("Error code %d, msg: %s (%s, %d)", err -> code
					, err -> message, __FILE__, __LINE__);
		return;
	}
	g_debug("Start the main event loop thread. done.(%s, %d)", __FILE__
					, __LINE__);

	GSource *src = g_idle_source_new();
	struct InitParam *par = g_slice_new(struct InitParam);
	par -> cb = cb;
	par -> info = info;
	par -> passwd = passwd;
	g_source_set_callback(src, (GSourceFunc)do_login, (gpointer)par, NULL);
	if(g_source_attach(src, info -> mainctx) <= 0){
		g_error("Attach initial source error.(%s, %d)"
				, __FILE__, __LINE__);
	}
	g_debug("Attach the initial source. done.(%s, %d)", __FILE__
					, __LINE__);
	g_source_unref(src);
}

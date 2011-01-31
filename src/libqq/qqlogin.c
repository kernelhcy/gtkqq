#include <qq.h>
#include <http.h>
#include <url.h>
#include <qqhosts.h>
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
		g_warning("Havn't create the main event loop!!");
		info -> mainloop = g_main_loop_new(NULL, FALSE);
		info -> mainctx = g_main_loop_get_context(info -> mainloop);
		g_warning("Create the main event loop. done.");
	}


	g_debug("Run the main event loop...");
	//start the main loop
	g_main_loop_run(info -> mainloop);

	/*
	 * Will not arrive here untill the program exits. 
	 */
	g_debug("Quit main event loop.");
	return NULL;
}

/*
 * Check if we need input the verify code.
 * The result is stored in info.
 *
 * return -1 if error ocurs or return 0
 */
static gint check_verify_code(QQInfo *info)
{
	g_debug("Check veriry code...");
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
		 * We need get the vc_type form the header "Set-Cookie"
		 */
		gchar *cookie = response_get_header_chars(rps, "Set-Cookie");
		if(cookie == NULL){
			g_warning("Not found cookie when find ptvfsession!"
					"(%s, %d)", __FILE__, __LINE__);
			ret = -1;
			goto error;
		}
		
		g_debug("check vc cookie: %s(%s, %d)", cookie, __FILE__
				, __LINE__);
		gchar *ptvf = g_strstr_len(cookie, -1, "ptvfsession=");
		ptvf += (sizeof("ptvfsession=") - 1);
		gchar *end = g_strstr_len(ptvf, -1, ";");
		*end = '\0';
		info -> ptvfsession = g_string_new(ptvf);
		g_debug("ptvfsession: %s(%s, %d)", ptvf, __FILE__, __LINE__);
		*end = ';';

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
 * do_init function parameter struct
 */
struct InitParam{
	QQCallBack cb;
	QQInfo *info;
	
};
/*
 * Do initial.
 * Get information from the server.
 * Run in the main event loop.
 */
static gboolean do_init(gpointer data)
{
	if(data == NULL){
		return FALSE;
	}
	QQCallBack cb = ((struct InitParam *)data) -> cb;
	QQInfo *info = ((struct InitParam *)data) -> info;

	g_debug("Do init...");
	if(cb != NULL){
		g_debug("Call the callback function in do_init.");
		cb(CB_SUCCESS, NULL);
	}
	g_debug("Get login page source...");
	
	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	request_set_uri(req, LOGINPAGEPATH 
			"?target=self&appid=1003903&enable_qlogin=0"
			"&no_verifyimg=1&style=4&s_url="LOGIN_S_URL);
	request_set_default_headers(req);
	request_add_header(req, "Host", LOGINPAGEHOST);
	GString *qs = request_tostring(req);
	g_debug(qs-> str);
	g_string_free(qs, TRUE);

	Connection *con = connect_to_host(LOGINPAGEHOST, 80);
	send_request(con, req);
	rcv_response(con, &rps);
	close_con(con);
	connection_free(con);

	request_del(req);
	response_del(rps);

	if(check_verify_code(info) < 0){
	}

	g_debug("Get javascript sources...");
	g_debug("Parse the information...");
	g_debug("Initial done.");

	g_slice_free(struct InitParam, data);
	g_debug("Free the struct InitParam.");

	return FALSE;
}

/*
 * Initial
 *
 * get information from the server
 */
static void qq_init(QQInfo *info, QQCallBack cb)
{
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
	g_debug("Initial the thread done");

	info -> mainloop = g_main_loop_new(NULL, FALSE);
	info -> mainctx = g_main_loop_get_context(info -> mainloop);
	g_debug("Create the main event loop. done.");

	GError *err;
	info -> mainloopthread = g_thread_create((GThreadFunc)start_main_loop
						, (gpointer)info, FALSE
						, &err);
	if(info -> mainloopthread == NULL){
		g_error("Error code %d, msg: %s", err -> code
					, err -> message);
		return;
	}
	g_debug("Start the main event loop thread. done.");

	GSource *src = g_idle_source_new();
	struct InitParam *par = g_slice_new(struct InitParam);
	par -> cb = cb;
	par -> info = info;
	g_source_set_callback(src, (GSourceFunc)do_init, (gpointer)par, NULL);
	if(g_source_attach(src, info -> mainctx) <= 0){
		g_error("Attach initial source error.");
	}
	g_debug("Attach the initial source. done.");
	g_source_unref(src);
}

void qq_login(QQInfo *info, const gchar *uin, const gchar *passwd
		, const gchar *vc, QQCallBack cb)
{
	if(info == NULL){
		g_warning("info == NULL. (%s, %d)", __FILE__, __LINE__);
		return;
	}
	if(uin == NULL || passwd == NULL || vc == NULL){
		g_warning("uin or passwd or vc == NULL.(%s, %d)"
				, __FILE__, __LINE__);
		return;
	}
	GString *usrnum = g_string_new(uin);
	info -> uin = usrnum;
	qq_init(info, cb);
	
}

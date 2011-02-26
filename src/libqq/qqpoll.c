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

typedef struct{
	QQInfo *info;
	QQCallBack cb;
}PollPar;

static gboolean do_poll(gpointer data)
{
	PollPar *par = (PollPar*)data;
	if(par == NULL)
	{
		g_warning("do_poll error. (%s, %d)", __FILE__, __LINE__);
		return FALSE;
	}
	QQInfo *info = par -> info;
	QQCallBack cb = par -> cb;
	g_slice_free(PollPar, par);

	gchar params[300];

	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_sprintf(params, POLLPATH"?clientid=%s&psessionid=%s&t=%lld"
			, info -> clientid -> str 
			, info -> psessionid -> str, get_now_millisecond());
	request_set_uri(req, params);
	request_set_default_headers(req);
	request_add_header(req, "Host", POLLHOST);
	request_add_header(req, "Cookie", info -> cookie -> str);
	request_add_header(req, "Content-Type", "utf-8");
	request_add_header(req, "Referer"
			, "http://"POLLHOST"/proxy.html?v=20101025002");

again:
	rps = NULL;	//the again label needs this.
	g_debug("poll.(%s, %d)", __FILE__, __LINE__);
	Connection *con = connect_to_host(POLLHOST, 80);
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

	gchar *retstatus = rps -> status -> str;
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
	
	/*
	 * temporarily like this.
	 */
	json_t *val = json_find_first_label_all(json, "raw_content");
	if(val != NULL){
		GString *vs = g_string_new(NULL);
		ucs4toutf8(vs, val -> text);
		if(cb != NULL){
			cb(CB_SUCCESS, vs);
		}
		
	}

error:
	json_free_value(&json);
	response_del(rps);
	/*
	 * again poll
	 */
	goto again;

	request_del(req);
	/*
	 * This function will be called in the poll event loop
	 * again and again, until the program exits.
	 * So we return true here.
	 */
	return TRUE;
}


void qq_start_poll(QQInfo *info, QQCallBack cb)
{
	GSource *src = g_idle_source_new();
	PollPar *par = g_slice_new(PollPar);
	par -> info = info;
	par -> cb = cb;
	g_source_set_callback(src, &do_poll, (gpointer)par, NULL);
	if(g_source_attach(src, info -> pollctx) <= 0){
		g_error("Attach logout source error.(%s, %d)"
				, __FILE__, __LINE__);
	}
	g_debug("Add source success. (%s, %d)", __FILE__, __LINE__);
	g_source_unref(src);
	return;
}

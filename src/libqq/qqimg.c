#include <qq.h>
#include <qqtypes.h>
#include <qqhosts.h>
#include <url.h>
#include <http.h>
/*
 * Get face images from the server.
 */
typedef struct{
	QQInfo *info;
	QQCallBack cb;
	const gchar *uin;
}FaceImgPar;

/*
 * Get the image type from the content type.
 */
static GString* get_image_type(const gchar *ct)
{
	if(ct == NULL){
		return NULL;
	}

	if(g_strstr_len(ct, -1, "bmp") != NULL){
		return g_string_new("bmp");
	}
	if(g_strstr_len(ct, -1, "jpeg") != NULL){
		return g_string_new("jpeg");
	}
	if(g_strstr_len(ct, -1, "png") != NULL){
		return g_string_new("png");
	}

	return NULL;
}

/*
 * Do get the face image from the server.
 */
static gboolean do_get_face_img(gpointer data)
{
	FaceImgPar *par = (FaceImgPar*)data;
	if(par == NULL){
		g_warning("par == NULL in do_get_face_img.(%s, %d)", __FILE__
				, __LINE__);
		return FALSE;
	}
	QQInfo *info = par -> info;
	QQCallBack cb = par -> cb;
	const gchar *uin = par -> uin;
	g_slice_free(FaceImgPar, par);

	gchar params[300];
	g_debug("Get face image of %s!(%s, %d)", uin, __FILE__, __LINE__);

	Request *req = request_new();
	Response *rps = NULL;
	request_set_method(req, "GET");
	request_set_version(req, "HTTP/1.1");
	g_snprintf(params, 300, FIMGPATH"?cache=0&type=1&fid=0&uin=%s&"
			"vfwebqq=%s", uin, info -> vfwebqq -> str);
	request_set_uri(req, params);
	request_set_default_headers(req);
	request_add_header(req, "Host", FIMGHOST);
	request_add_header(req, "Cookie", info -> cookie -> str);
	request_add_header(req, "Referer", "http://web2.qq.com/");

	Connection *con = connect_to_host(FIMGHOST, 80);
	if(con == NULL){
		g_warning("Can NOT connect to server!(%s, %d)"
				, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_NETWORKERR, "Can not connect to server!");
		}
		request_del(req);
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

	QQFaceImg *img = qq_faceimg_new();
	img -> uin = g_string_new(uin);
	img -> data = g_string_new_len(rps -> msg -> str
					, rps -> msg -> len);
	img -> type = get_image_type(response_get_header_chars(rps
				, "Content-Type"));
	if(cb != NULL){
		cb(CB_SUCCESS, img);
	}
error:
	request_del(req);
	response_del(rps);
	return FALSE;
}

void qq_get_face_img(QQInfo *info, const gchar *uin, QQCallBack cb)
{
	if(info == NULL){
		if(cb != NULL){
			cb(CB_ERROR, "info == NULL in qq_get_face_img");
		}
		return;
	}

	GSource *src = g_idle_source_new();
	FaceImgPar *par = g_slice_new(FaceImgPar);
	par -> info = info;
	par -> cb = cb;
	par -> uin = uin;
	g_source_set_callback(src, &do_get_face_img, (gpointer)par, NULL);

	/*
	 * Maybe we should use mutilple threads to speed up to
	 * get face images.
	 */
	if(g_source_attach(src, info -> mainctx) <= 0){
		g_error("Attach logout source error.(%s, %d)"
				, __FILE__, __LINE__);
	}
	g_source_unref(src);
	return;
}


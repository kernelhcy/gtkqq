#include <qq.h>
#include <qqtypes.h>
#include <qqhosts.h>
#include <url.h>
#include <http.h>
#include <glib/gprintf.h>
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
    if(g_strstr_len(ct, -1, "gif") != NULL){
        return g_string_new("gif");
    }
    
    g_warning("Unknown image type: %s (%s, %d)", ct, __FILE__, __LINE__);
    return NULL;
}

/*
 * get the face image from the server.
 */
gint qq_get_face_img(QQInfo *info, QQFaceImg *img, GError **err)
{
    if(info == NULL || img == NULL){
        return PARAMETER_ERR;
    }
    gint ret_code = NO_ERR;
    gchar params[300];
	gint res = 0;
    g_debug("Get face image of %s!(%s, %d)", img -> uin -> str
                                , __FILE__, __LINE__);

    Request *req = request_new();
    Response *rps = NULL;
    request_set_method(req, "GET");
    request_set_version(req, "HTTP/1.1");
    g_snprintf(params, 300, FIMGPATH"?cache=0&type=1&fid=0&uin=%s&"
                            "vfwebqq=%s", img -> uin -> str
                            , info -> vfwebqq -> str);
    request_set_uri(req, params);
    request_set_default_headers(req);
    request_add_header(req, "Host", FIMGHOST);
    request_add_header(req, "Cookie", info -> cookie -> str);
    request_add_header(req, "Referer", "http://web2.qq.com/");

    Connection *con = connect_to_host(FIMGHOST, 80);
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
        ret_code = NETWORK_ERR;
        goto error;
    }

    qq_faceimg_set(img, "data", rps -> msg);
    qq_faceimg_set(img, "type", get_image_type(
                        response_get_header_chars(rps
                                    , "Content-Type")));
error:
    request_del(req);
    response_del(rps);
    return ret_code;
}


/*
 * in qqutils.c
 */
extern gint save_img_to_file(const gchar *data, gint len, const gchar *path);
//
//save the face image to file
//the file name is uin
//
gint qq_save_face_img(QQFaceImg *img, const gchar *path, GError **err)
{
    if(img == NULL || path == NULL){
        g_warning("img == NULL || path == NULL (%s, %d)"
                            , __FILE__, __LINE__);
        return PARAMETER_ERR;
    }
    return save_img_to_file(img -> data -> str, img -> data -> len, path);
}


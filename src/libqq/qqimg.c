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
 * Do get the face image from the server.
 */
static gint do_get_face_img(QQInfo *info, const gchar *uin, GError **err)
{
    gint ret_code = -1;
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

    QQFaceImg *img = qq_faceimg_new();
    img -> uin = g_string_new(uin);
    img -> data = g_string_new_len(rps -> msg -> str
                    , rps -> msg -> len);
    img -> type = get_image_type(response_get_header_chars(rps
                    , "Content-Type"));

    //store the image file name into the hashtable
    gchar *name = g_malloc(sizeof(gchar) * 50);
    g_sprintf(name, "%s.%s", img -> uin -> str, img -> type -> str);
    g_hash_table_insert(info -> buddies_image_ht, img -> uin -> str, name);

error:
    request_del(req);
    response_del(rps);
    return ret_code;
}

gint qq_get_face_img(QQInfo *info, const gchar *uin, GError **err)
{
    if(info == NULL){
        return -1;
    }

    return do_get_face_img(info, uin, err);
}

/*
 * in qqutils.c
 */
extern gint save_img_to_file(const gchar *data, gint len, const gchar *ext, 
                const gchar *path, const gchar *fname);
//
//save the face image to file
//the file name is uin.type
//
gint qq_save_face_img(QQBuddy *bdy, const gchar *path, GError **err)
{
    if(bdy == NULL || path == NULL){
        g_warning("bdy == NULL || path == NULL (%s, %d)"
                , __FILE__, __LINE__);
        return -1;
    }
    QQFaceImg *fimg = bdy -> faceimg;
    bdy -> faceimgfile = g_string_new(path);
    g_string_append(bdy -> faceimgfile, "/");
    g_string_append(bdy -> faceimgfile, fimg -> uin -> str);
    g_string_append(bdy -> faceimgfile, ".");
    g_string_append(bdy -> faceimgfile, fimg -> type -> str);

    return save_img_to_file(fimg -> data -> str, fimg -> data -> len
                , fimg -> type -> str
                , path, fimg -> uin -> str);        
}

const gchar* qq_lookup_image_name(QQInfo *info, const gchar *uin, GError **err)
{
    if(info == NULL || uin == NULL){
        g_warning("info == NULL || uin == NULL (%s, %d)"
                , __FILE__, __LINE__);
        return NULL;
    }

    return (const gchar *)g_hash_table_lookup(info -> buddies_image_ht
                                , uin);
}

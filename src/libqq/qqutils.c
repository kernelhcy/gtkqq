#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <qq.h>
#include <glib/gprintf.h>
#include <string.h>

/*
 * Get the milliseconds of now
 */
glong get_now_millisecond()
{
    glong re;
    GTimeVal now;
    g_get_current_time(&now);
    re = now.tv_usec / 1000;
    re += now.tv_sec;
    return re;
}

/*
 * Save the image data to file.
 * The file is path/fname.ext.
 */
gint save_img_to_file(const gchar *data, gint len, const gchar *path )
{
    if(data == NULL){
        return -1;
    }

    g_debug("Create image file : %s (%s, %d)", path, __FILE__, __LINE__);
    gint fd = creat(path, S_IRUSR | S_IWUSR);
    if(fd == -1){
        g_warning("Create image data to file error! %s (%s, %d)"
                , strerror(errno), __FILE__, __LINE__);
        return -1;
    }

    GError *err = NULL;
    GIOChannel *ioc = g_io_channel_unix_new(fd);
    if(ioc == NULL){
        g_warning("Create io channle error!! %d %s "
                "(%s, %d)", err -> code , err -> message
                , __FILE__, __LINE__);
        g_error_free(err);
        return -1;
    }
    err = NULL;
    g_io_channel_set_encoding(ioc, NULL, &err);
    err = NULL;
    GIOStatus status;
    gsize bytes_w;
    status = g_io_channel_write_chars(ioc, data, len, &bytes_w, &err);
    switch(status)
    {
    case G_IO_STATUS_NORMAL:
        if(bytes_w < len){
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
#if GLIB_CHECK_VERSION(2,31,0)
    g_io_channel_shutdown(ioc, TRUE, &err);
#else
    g_io_channel_close(ioc);
#endif
    g_io_channel_unref(ioc);
    return 0;
}

void create_error_msg(GError **err, gint code, const gchar *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    if(err != NULL){
        *err = g_error_new_valist(g_quark_from_string("GtkQQ")
                                        , code, fmt, ap);
    }
    va_end(ap);
}

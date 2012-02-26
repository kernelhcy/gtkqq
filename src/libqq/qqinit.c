/*
 * Do the initail.
 */
#include <qq.h>
#include <glib.h>

QQInfo* qq_init(GError **err)
{
#if !GLIB_CHECK_VERSION(2,31,0)
    /* g_thread_init and g_thread_supported has been deprecated
       since version 2.31. 
       These functions are no longer necessary. The GLib threading system is automatically initialized at the start of your program.
    */
    if(!g_thread_supported()){
        g_thread_init(NULL);
        g_message("Init the gthread system. (%s, %d)", __FILE__, __LINE__);
    }
#else
    g_message("gthread auto initialized. (%s, %d)", __FILE__, __LINE__);
#endif
    QQInfo *info = qq_info_new();
    return info;
}

void qq_finalize(QQInfo *info, GError **err)
{
    if(info == NULL){
        return;
    }
    
    qq_info_free(info);
}

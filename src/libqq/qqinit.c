/*
 * Do the initail.
 */
#include <qq.h>
#include <glib.h>

QQInfo* qq_init(GError **err)
{
    if(!g_thread_supported()){
        g_thread_init(NULL);
        g_message("Init the gthread system. (%s, %d)", __FILE__, __LINE__);
    }
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

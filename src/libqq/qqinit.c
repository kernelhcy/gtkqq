/*
 * Do the initail.
 */
#include <qq.h>
#include <glib.h>

QQInfo* qq_init(QQCallBack cb, gpointer usrdata)
{
	QQInfo *info = qq_info_new();
	return info;
}

void qq_finalize(QQInfo *info)
{
	if(info == NULL){
		return;
	}
	
    qq_info_free(info);
}

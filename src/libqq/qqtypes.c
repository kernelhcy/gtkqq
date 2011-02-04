#include <qqtypes.h>

/*
 * QQInfo
 */
QQInfo* qq_info_new()
{
	QQInfo *info = g_slice_new0(QQInfo);
	info -> need_vcimage = FALSE;
	return info;
}

void qq_info_free(QQInfo *info)
{
	if(info == NULL){
		return;
	}
	
	g_main_loop_unref(info -> mainloop);
	g_main_context_unref(info -> mainctx);

	g_string_free(info -> uin, TRUE);
	g_string_free(info -> status, TRUE);
	g_string_free(info -> prestatus, TRUE);
	g_string_free(info -> vc_type, TRUE);
	g_string_free(info -> vc_image_data, TRUE);
	g_string_free(info -> vc_image_type, TRUE);
	g_string_free(info -> verify_code, TRUE);
	g_string_free(info -> ptvfsession, TRUE);
	g_string_free(info -> version, TRUE);
	g_string_free(info -> ptuserinfo, TRUE);
	g_string_free(info -> ptwebqq, TRUE);
	g_string_free(info -> ptcz, TRUE);
	g_string_free(info -> skey, TRUE);
	g_string_free(info -> cookie, TRUE);
	g_string_free(info -> clientid, TRUE);
	g_string_free(info -> seskey, TRUE);
	g_string_free(info -> cip, TRUE);
	g_string_free(info -> index, TRUE);
	g_string_free(info -> port, TRUE);
	g_string_free(info -> psessionid, TRUE);
	g_string_free(info -> vfwebqq, TRUE);

	g_slice_free(QQInfo, info);
}

QQMsg* qq_msg_new()
{
	return NULL;
}

void qq_msg_free(QQMsg *msg)
{

}


QQUser* qq_user_new()
{
	return NULL;
}
void qq_user_free(QQUser *usr)
{

}

QQGroup* qq_group_new()
{
	return NULL;
}
void qq_group_free(QQGroup *grp)
{

}

QQCategory* qq_category_new()
{
	return NULL;
}
void qq_category_free(QQCategory *cty)
{
	
}

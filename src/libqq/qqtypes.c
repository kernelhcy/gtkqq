#include <qqtypes.h>

/*
 * QQInfo
 */
QQInfo* qq_info_new()
{
	QQInfo *info = g_slice_new0(QQInfo);
	info -> need_vcimage = FALSE;
	
	info -> me = qq_buddy_new();
	info -> buddies = g_ptr_array_new();
	info -> groups = g_ptr_array_new();
	info -> recentcon = g_ptr_array_new();
	info -> categories = g_ptr_array_new();

	info -> lock = g_mutex_new();
	return info;
}

void qq_info_free(QQInfo *info)
{
	if(info == NULL){
		return;
	}
	
	g_main_loop_unref(info -> mainloop);
	g_main_context_unref(info -> mainctx);

	qq_buddy_free(info -> me);
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

	g_ptr_array_free(info -> buddies, TRUE);
	g_ptr_array_free(info -> groups, TRUE);
	g_ptr_array_free(info -> recentcon, TRUE);
	g_ptr_array_free(info -> categories, TRUE);

	g_mutex_free(info -> lock);
	g_slice_free(QQInfo, info);
}

QQMsg* qq_msg_new()
{
	QQMsg *msg = g_slice_new0(QQMsg);
	return msg;
}

void qq_msg_free(QQMsg *msg)
{
	if(msg == NULL){
		return;
	}
	g_slice_free(QQMsg, msg);
}


QQBuddy* qq_buddy_new()
{
	QQBuddy *bd = g_slice_new0(QQBuddy);
	bd -> vip_info = -1;
	return bd;
}
void qq_buddy_free(QQBuddy *bd)
{
	if(bd == NULL){
		return;
	}	

	g_slice_free(QQBuddy, bd);
}

QQGMember* qq_gmember_new()
{
	QQGMember *m = g_slice_new0(QQGMember);

	return m;
}
void qq_gmember_free(QQGMember *m)
{
	if(m == NULL){
		return;
	}
	g_slice_free(QQGMember, m);
}
QQGroup* qq_group_new()
{
	QQGroup *grp = g_slice_new0(QQGroup);
	return grp;
}
void qq_group_free(QQGroup *grp)
{
	if(grp == NULL){
		return;
	}
	g_slice_free(QQGroup, grp);
}

QQCategory* qq_category_new()
{
	QQCategory *c = g_slice_new0(QQCategory);
	return c;
}
void qq_category_free(QQCategory *cty)
{
	if(cty == NULL){
		return;
	}
	g_slice_free(QQCategory, cty);
}


/*
 * Just put here.
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

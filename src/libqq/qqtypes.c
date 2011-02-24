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
	info -> recentcons = g_ptr_array_new();
	info -> categories = g_ptr_array_new();

	info -> lock = g_mutex_new();

	/*
	 * Just set to 1000.
	 * I don't know how to calculate the msg_id.
	 * But this works.
	 */
	info -> msg_id = 1000;
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

	gint i;
	for(i = 0; i < info -> buddies -> len; ++i){
		qq_buddy_free(info -> buddies -> pdata[i]);
	}
	g_ptr_array_free(info -> buddies, TRUE);

	for(i = 0; i < info -> groups -> len; ++i){
		qq_group_free(info -> groups -> pdata[i]);
	}
	g_ptr_array_free(info -> groups, TRUE);

	for(i = 0; i < info -> recentcons -> len; ++i){
		qq_recentcon_free(info -> recentcons -> pdata[i]);
	}
	g_ptr_array_free(info -> recentcons, TRUE);

	for(i = 0; i < info -> categories -> len; ++i){
		qq_category_free(info -> categories -> pdata[i]);
	}
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

	g_string_free(msg -> content, TRUE);
	g_string_free(msg -> font.name, TRUE);
	g_string_free(msg -> font.color, TRUE);

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
	g_string_free(bd -> uin, TRUE);
	g_string_free(bd -> status, TRUE);
	g_string_free(bd -> nick, TRUE);
	g_string_free(bd -> markname, TRUE);
	g_string_free(bd -> country, TRUE);
	g_string_free(bd -> city, TRUE);
	g_string_free(bd -> province, TRUE);
	g_string_free(bd -> gender, TRUE);
	g_string_free(bd -> face, TRUE);
	g_string_free(bd -> flag, TRUE);
	g_string_free(bd -> phone, TRUE);
	g_string_free(bd -> mobile, TRUE);
	g_string_free(bd -> email, TRUE);
	g_string_free(bd -> college, TRUE);
	g_string_free(bd -> occupation, TRUE);
	g_string_free(bd -> personal, TRUE);
	g_string_free(bd -> homepage, TRUE);
	g_string_free(bd -> lnick, TRUE);

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
	c -> members = g_ptr_array_new();
	return c;
}
void qq_category_free(QQCategory *cty)
{
	if(cty == NULL){
		return;
	}
	g_ptr_array_free(cty -> members, TRUE);
	g_slice_free(QQCategory, cty);
}

QQRecentCon* qq_recentcon_new()
{
	QQRecentCon *rc = g_slice_new0(QQRecentCon);
	rc -> type = -1;
	return rc;
}
void qq_recentcon_free(QQRecentCon *rc)
{
	if(rc == NULL){
		return;
	}
	g_string_free(rc -> uin, TRUE);
	g_slice_free(QQRecentCon, rc);
}

QQFaceImg* qq_faceimg_new()
{
	QQFaceImg *img = g_slice_new0(QQFaceImg);
	return img;
}
void qq_faceimg_free(QQFaceImg *img)
{
	if(img == NULL){
		return;
	}

	g_string_free(img -> data, TRUE);
	g_string_free(img -> type, TRUE);
	g_string_free(img -> uin, TRUE);

	g_slice_free(QQFaceImg, img);
}

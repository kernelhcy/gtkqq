#include <stdlib.h>
#include <qq.h>
#include <string.h>
#include <log.h>

static void callback(CallBackResult re, gpointer data)
{
	g_printf("Call back: %s\n", (gchar *)data);
	return;
}

static void logoutcb(CallBackResult re, gpointer data)
{
	exit(0);
}

extern gint save_img_to_file(const gchar *data, gint len, const gchar *ext, 
				const gchar *path, const gchar *fname);

static void faceimg_cb(CallBackResult re, gpointer data)
{
	if(re != CB_SUCCESS){
		g_printf("Error: %s\n", (const gchar *)data);
		return;
	}

	QQFaceImg *img = (QQFaceImg*)data;
	save_img_to_file(img -> data -> str, img -> data -> len
			, img -> type -> str, "/home/hcy"
			, img -> uin -> str);
	qq_faceimg_free(img);
}

int main(int argc, char **argv)
{
	log_init();
	g_message("Log initial done.");
	QQInfo *info = qq_init(NULL);
	qq_login(info, "1421032531", "1234567890", "online", callback);
//	qq_login(info, "494498045", "1988hcya!", "hidden",NULL);

	g_message("get my information...");
	qq_get_my_info(info, NULL);
	g_message("get my friends...");
	qq_get_my_friends(info, NULL);
	g_message("get group name list mask...");
	qq_get_group_name_list_mask(info, NULL);
	g_message("get online buddies...");
	qq_get_online_buddies(info, NULL);
	g_message("get recent contact...");
	qq_get_recent_contact(info, NULL);

	sleep(3);
	QQMsg *msg = qq_msg_new();
	msg -> info = info;
	msg -> bdy = info -> buddies -> pdata[0];
	msg -> content = g_string_new("test from gtkqq.来自gtkqq的测试."
			"我只是在测试程序，各位可以无视我。打扰了。^v^");
	msg -> font.name = g_string_new("微软雅黑");
	msg -> font.color = g_string_new("00000");
	msg -> font.size = 15;
	msg -> font.style.a = 0;
	msg -> font.style.b = 0;
	msg -> font.style.c = 0;
	//msg -> grp = info -> groups -> pdata[0];
	//qq_sendmsg_to_group(info, msg, NULL);

	g_message("start poll ...");
	qq_start_poll(info, callback);
	qq_sendmsg_to_friend(info, msg, NULL);
	const gchar *uin = ((QQBuddy*)info -> buddies -> pdata[0]) -> 
		uin -> str;
	qq_get_face_img(info, "65359140", faceimg_cb);
	qq_get_face_img(info, "1421032531", faceimg_cb);
	sleep(100);
	g_message("Will logout...");
	qq_logout(info, logoutcb);
	sleep(10000);
	return 0;
}

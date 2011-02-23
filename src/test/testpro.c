#include <stdlib.h>
#include <qq.h>
#include <string.h>
#include <log.h>

static void callback(CallBackResult re, gpointer data)
{
	if(re == CB_SUCCESS){
		g_printf("Call back: %s\n", (gchar *)data);
		return;
	}
	return;
}

static void logoutcb(CallBackResult re, gpointer data)
{
	exit(0);
}
int main(int argc, char **argv)
{
	log_init();
	g_debug("Log initial done.");
	QQInfo *info = qq_init(NULL);
	qq_login(info, "1421032531", "1234567890", "online", NULL);
//	qq_login(info, "494498045", "1988hcya!", "hidden",NULL);
	
	qq_get_my_info(info, NULL);
	qq_get_my_friends(info, NULL);
	qq_get_group_name_list_mask(info, NULL);
	qq_get_online_buddies(info, NULL);
	qq_get_recent_contact(info, NULL);

	sleep(5);
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

	qq_start_poll(info, callback);
	qq_sendmsg_to_friend(info, msg, NULL);
	sleep(10);
	g_debug("Will logout...");
	qq_logout(info, logoutcb);
	sleep(10000);
	return 0;
}

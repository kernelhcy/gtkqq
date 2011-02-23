#include <stdlib.h>
#include <qq.h>
#include <string.h>
#include <log.h>

static void callback(CallBackResult re, gpointer data)
{
	if(re == CB_SUCCESS){
		exit(0);
	}
	return;
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

	g_debug("Will logout...");
	qq_logout(info, callback);
	sleep(100);
	return 0;
}

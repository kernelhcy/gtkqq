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
	qq_init();
	QQInfo *info = qq_info_new();
	qq_login(info, "1421032531", "1234567890", "online", NULL);
//	qq_login(info, "494498045", "1988hcya!",NULL);
	
	qq_get_my_info(info, NULL);

	g_debug("Will logout...");
	qq_logout(info, callback);
	sleep(100);
	return 0;
}

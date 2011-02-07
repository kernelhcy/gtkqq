#include <qq.h>
#include <string.h>
#include <log.h>

int main(int argc, char **argv)
{
	log_init();
	QQInfo *info = qq_info_new();
//	qq_login(info, "1432531", "1234567890",NULL);
	qq_login(info, "1421032531", "1234567890", "online", NULL);
//	qq_login(info, "494498045", "1988hcya!",NULL);
 
	g_debug("Will logout after 10s...");
	sleep(10);
	qq_logout(info, NULL);

	sleep(100);
	return 0;
}

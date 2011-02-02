#include <qq.h>
#include <string.h>
#include <log.h>

int main(int argc, char **argv)
{
	log_init();
	
	QQInfo *info = qq_info_new();
	qq_login(info, "1421032531", "1234567890",NULL);
//	qq_login(info, "494498045", "1234567890",NULL);
 
	sleep(100000);
	return 0;
}

#include <qq.h>
#include <string.h>
#include <log.h>

int main(int argc, char **argv)
{
	log_init();
	QQInfo *info = qq_info_new();
	qq_init(info, NULL);
	const gchar *s = "acm:1234567890a!";
	gchar *encodes = g_base64_encode(s, strlen(s));
	g_print("Encode: %s\n", encodes);



	sleep(100000);
	return 0;
}

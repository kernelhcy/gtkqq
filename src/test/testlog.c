#include <glib.h>
#include <stdio.h>
#include <log.h>

int main(int argc, char ** argv)
{
	log_init();
	g_debug("debug %s %d\n", __FILE__, __LINE__);
	g_message("message %s %d\n", __FILE__, __LINE__);
	g_warning("warning  %s %d\n", __FILE__, __LINE__);
	g_error("error %s %d\n", __FILE__, __LINE__);
	return 0;
}

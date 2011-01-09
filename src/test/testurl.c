#include <url.h>
#include <log.h>

int main(int argc, char ** argv)
{
	log_init();
	int fd = connect_to_host(argv[1]);
	close_con(fd);
	return 0;
}

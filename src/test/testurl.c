#include <url.h>
#include <log.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
	log_init();
	int fd = connect_to_host("t.qq.com", 80);
	if(fd < 0){
		return 0;
	}
	Request *r = request_new();
	request_set_method(r, "GET");
	request_set_uri(r, "/");
	request_set_version(r, "HTTP/1.1");
	request_set_default_headers(r);
	request_add_header(r, "Proxy-Authorization", "Basic "
						"YWNtOjEyMzQ1Njc4OTBhIQ==");
	if(send_request(fd, r) < 0){
		close_con(fd);
		return 0;
	}
	Response *rps = NULL;
	if(rcv_response(fd, &rps) < 0){
		close_con(fd);
		return 0;
	}
	printf("%s\n", response_tostring(rps) -> str);
	close_con(fd);
	return 0;
}

#include <http.h>
#include <string.h>
#include <glib/gprintf.h>

int main(int argc, char** argv)
{
	Request* r = request_new();
	request_set_method(r, "GET");
	request_set_uri(r, "http://web.qq.com/hello.html?key=234");
	request_set_version(r, "HTTP/1.1");
	
	request_add_header(r, "Content-Lenght", "12345");
	request_add_header(r, "Host", "web.qq.com");
	
	gchar* s = "asdjfaslkdjfoqwiuertjakfdja;sldf";
	request_append_msg(r, s, strlen(s));
	request_append_msg(r, s, strlen(s));
	request_append_msg(r, s, strlen(s));
	request_append_msg(r, s, strlen(s));
	request_append_msg(r, s, strlen(s));
	request_append_msg(r, s, strlen(s));
	
	print_raw(request_tostring(r));
	g_printf("\n\n");
	
	s = "HTTP/1.1 200 OK\r\n"
"Date: Sun, 02 Jan 2011 19 19:09:55 GMT\r\n"
"Server: Tencent Login Server/2.0.0\r\n"
"Set-Cookie: pt2gguin=o0494498045; EXPIRES=Fri, 02-Jan-2020 00:00:00 GMT; PATH=/;DOMAIN=qq.com;\r\n"
"Set-Cookie: uin=o0494498045; PATH=/; DOMAIN=qq.com;\r\n"
"Set-Cookie: skey=@EK66ZxUe8; PATH=/; DOMAIN=qq.com;\r\n"
"Set-Cookie: clientuin=; EXPIRES=Fri, 02-Jan-1970 00:00:00 GMT; PATH=/; DOMAIN=qq.com;\r\n"
"Set-Cookie: clientkey=; EXPIRES=Fri, 02-Jan-1970 00:00:00 GMT; PATH=/; DOMAIN=qq.com;\r\n"
"Set-Cookie: zzpaneluin=; EXPIRES=Fri, 02-Jan-1970 00:00:00 GMT; PATH=/; DOMAIN=qq.com;\r\n"
"Set-Cookie: zzpanelkey=; EXPIRES=Fri, 02-Jan-1970 00:00:00 GMT; PATH=/; DOMAIN=qq.com;\r\n"
"Set-Cookie: ptisp=edu;PATH=/; DOMAIN=qq.com;\r\n"
"Set-Cookie:ptuserinfo=4b65726e656c;PATH=/;DOMAIN=ptlogin2.qq.com;\r\n"
"Set-Cookie: ptwebqq=b16f2ca37c56d8a2ce064a9494cc85044e78ac6c480041c3f5be450206ab6728;" "PATH=/; DOMAIN=qq.com;\r\n"
"Pragma: no-cache\r\n"
"Cache-Control: no-cache; must-revalidate\r\n"
"Connection: Close\r\n"
"Content-Type: application/x-javascript; charset=utf-8\r\n";
	Response* rps = response_new_parse(g_string_new(s));
	g_printf(response_tostring(rps) -> str);
	g_printf("\n\n");
	print_raw(rps -> reason);
	print_raw(rps -> msg);
	return 0;
}

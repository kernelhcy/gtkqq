#include <url.h>
#include <sys/socket.h>
#include <string.h>
#include <glib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int connect_to_host(const char *hostname, int port)
{
	if(NULL == hostname){
		return -1;
	}
	int sockfd = -1, err;
	struct addrinfo *ailist = NULL, *aip = NULL;
	struct addrinfo hint;
	
	memset(&hint, 0, sizeof(hint));
	hint.ai_socktype = SOCK_STREAM;
	
	if((err = getaddrinfo(hostname, NULL, &hint, &ailist)) != 0){
		g_warning("Can't get the address information of %s (%s, %d)\n"
				,hostname , __FILE__, __LINE__);
		return -1;
	}
	
	g_debug("Got addrinfo.\n");
	struct sockaddr_in *sinp;
	#define BUFLEN 200
	gchar buf[BUFLEN];
	
	for(aip = ailist; aip != NULL; aip = aip -> ai_next){
		
		if((sockfd =socket(aip -> ai_family, SOCK_STREAM, 0)) < 0){
			g_warning("Can't create a socket.(%s, %d)\n"
					, __FILE__, __LINE__);
			return -1;
		}
		
		g_debug("create a socket %d\n", sockfd);
		sinp = (struct sockaddr_in *)aip -> ai_addr;
		
		/*
		 * the http protocol uses port 80
		 */
		sinp -> sin_port = htons((gint16)port);
		
		g_debug("Address: %s Port %d\n"
				, inet_ntop(AF_INET, &sinp -> sin_addr
						, buf, BUFLEN)
				, ntohs(sinp -> sin_port));
		
		if(connect(sockfd,aip -> ai_addr, aip -> ai_addrlen) < 0){
			close(sockfd);
			sockfd = -1;
			g_warning("Can't connect to the server.(%s, %d)\n"
					, __FILE__, __LINE__);
			continue;
		}
		//connect to the host success.
		//break;
	}
	#undef BUFLEN
	if(aip != NULL){
		for(aip = aip -> ai_next; aip != NULL; aip = aip -> ai_next){
			freeaddrinfo(aip);
		}
	}
	return sockfd;
}

void close_con(int fd)
{
	//shutdown(fd);
	close(fd);
}

int send_request(int fd, Request *r)
{
	if(fd < 0 || r == NULL){
		return -1;
	}
	
	GString *rq = request_tostring(r);
	g_message("(%s, %d)Send reqeust : %s\n"
			,__FILE__, __LINE__,  rq -> str);
	int len = 0;
	int rval = 0;
	while(len < rq -> len){
		rval = write(fd, rq -> str + len, rq -> len - len);
		if(rval < 0){
			g_string_free(rq, TRUE);
			g_warning("(%s, %d)Sead request error! slen %d nlen%d\n"
					, __FILE__, __LINE__, len, rq -> len);
			return -1;
		}
		len += rval;
	}
	
	g_string_free(rq, TRUE);
	return 0;
}

int rcv_response(int fd, Response **r)
{
	if(fd < 0 || r == NULL){
		return -1;
	}
	
	#define BUFSIZE 200
	char buf[BUFSIZE];
	int rval;
	GString *s = g_string_new(NULL);
	do{
		rval = read(fd, buf, BUFSIZE);
		if(rval < 0){
			g_string_free(s, TRUE);
			g_warning("(%s, %d)Read response error! len%d\n"
					, __FILE__, __LINE__, s -> len);
			return -1;
		}
		g_string_append_len(s, buf, rval);
	}while(rval == BUFSIZE);
	
	*r = response_new_parse(s);
	g_string_free(s, TRUE);
	#undef BUFSIZE
	return 0;
}

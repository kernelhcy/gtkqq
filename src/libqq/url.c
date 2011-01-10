#include <url.h>
#include <sys/socket.h>
#include <string.h>
#include <glib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Connection* connection_new()
{
	Connection *con = g_slice_new(Connection);
	con -> fd = -1;
	con -> channel = NULL;
	return con;
}

void connection_free(Connection *con)
{
	if(con == NULL){
		return;
	}
	g_slice_free(Connection, con);
}

Connection* connect_to_host(const char *hostname, int port)
{
	if(NULL == hostname){
		return NULL;
	}
	int sockfd = -1, err;
	struct addrinfo *ailist = NULL, *aip = NULL;
	struct addrinfo hint;
	
	memset(&hint, 0, sizeof(hint));
	hint.ai_socktype = SOCK_STREAM;
	
	if((err = getaddrinfo(hostname, NULL, &hint, &ailist)) != 0){
		g_warning("Can't get the address information of %s (%s, %d)\n"
				,hostname , __FILE__, __LINE__);
		return NULL;
	}
	
	g_debug("Got addrinfo.\n");
	struct sockaddr_in *sinp;
	#define BUFLEN 200
	gchar buf[BUFLEN];
	
	for(aip = ailist; aip != NULL; aip = aip -> ai_next){
		
		if((sockfd =socket(aip -> ai_family, SOCK_STREAM, 0)) < 0){
			g_warning("Can't create a socket.(%s, %d)\n"
					, __FILE__, __LINE__);
			return NULL;
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
	
	Connection *con = connection_new();
	con -> fd = sockfd;
	
	GIOChannel *channel = g_io_channel_unix_new(sockfd);
	//read and write binary data.
	g_io_channel_set_encoding(channel, NULL, NULL);
	con -> channel = channel;
	
	return con;
}

void close_con(Connection *con)
{
	if(con == NULL){
		return;
	}
	g_io_channel_shutdown(con -> channel, TRUE, NULL);
	g_io_channel_unref(con -> channel);
	//shutdown(fd);
	close(con -> fd);
}

int send_request(Connection *con, Request *r)
{
	if(con == NULL || r == NULL){
		return -1;
	}
	
	GString *rq = request_tostring(r);
	//g_message("(%s, %d)Send reqeust : %s\n"
	//		,__FILE__, __LINE__,  rq -> str);

	GIOStatus status;
	GError *err = NULL;
	gsize bytes_written = 0;
	gsize has_written = 0;

	while(has_written < rq -> len){
		status = g_io_channel_write_chars(con -> channel
				, rq -> str + has_written
				, rq -> len - has_written
				, &bytes_written
				, &err);
		switch(status)
		{
		case G_IO_STATUS_NORMAL:
			//write success.
			has_written += bytes_written;
			g_debug("Write %d bytes data.(%s, %d)\n"
					, bytes_written, __FILE__
					, __LINE__);
			break;
		case G_IO_STATUS_EOF:
			g_warning("Write data EOF!! What's happenning??\n");
			return -1;
		case G_IO_STATUS_ERROR:
			g_warning("Write data ERROR!! code:%d msg:%s\n"
					, err -> code, err -> message);
			return -1;
		case G_IO_STATUS_AGAIN:
			g_debug("Channel temporarily unavailable.\n");
			break;
		default:
			g_warning("Unknown io status!\n");
			return -1;
		}
	}
	status = g_io_channel_flush(con -> channel, &err);
	if(status != G_IO_STATUS_NORMAL){
		g_warning("Flush io channel error! But don't warry...\n");
	}	
	g_debug("Write all date.(%s, %d)\n", __FILE__, __LINE__);
	g_string_free(rq, TRUE);
	return 0;
}

int rcv_response(Connection *con, Response **rp)
{
	if(con == NULL || rp == NULL){
		return -1;
	}
	Response *r = NULL;
	gsize need_to_read = G_MAXSIZE;		//we don't know the length of
						//the data that need to read.
	gsize has_read = 0;
	//store the data that has read
	GString *data = g_string_new(NULL);
	GString *content = g_string_new("Content-Length");

	#define BUFSIZE 500
	gchar buf[BUFSIZE];
	GIOStatus status;
	GError *err = NULL;
	gsize bytes_read = 0;
	
	g_debug("Begin to read data.(%s, %d)\n", __FILE__, __LINE__);
	while(has_read < need_to_read){
		status = g_io_channel_read_chars(con -> channel
				, buf, BUFSIZE, &bytes_read, &err);
		switch(status)
		{
		case G_IO_STATUS_NORMAL:
			g_debug("Read %d bytes data.(%s, %d)\n"
					,bytes_read,  __FILE__, __LINE__);
			//read success.
			has_read += bytes_read;
			break;
		case G_IO_STATUS_EOF:
			g_warning("Read data EOF!! What's happenning??"
					"(%s, %d)\n"
					, __FILE__, __LINE__);
			g_string_free(data, TRUE);
			g_string_free(content, TRUE);
			return -1;
		case G_IO_STATUS_ERROR:
			g_warning("Read data ERROR!! code:%d msg:%s"
					"(%s, %d)\n"
					, err -> code, err -> message
					, __FILE__, __LINE__);
			g_string_free(data, TRUE);
			g_string_free(content, TRUE);
			return -1;
		case G_IO_STATUS_AGAIN:
			g_debug("Channel temporarily unavailable.\n");
			break;
		default:
			g_warning("Unknown io status!(%s, %d)\n"
					, __FILE__, __LINE__);
			g_string_free(data, TRUE);
			g_string_free(content, TRUE);
			return -1;
		}
		g_string_append_len(data, buf, bytes_read);
		
		if(g_strstr_len(data -> str, data -> len, "\r\n\r\n") 
						!= NULL){
			//We have gotten all the headers;
			//Find the Content-Length 's value
			r = response_new_parse(data);
			GString *clen = response_get_header(r, content);
			if(clen == NULL){
				g_warning("No Content-Length!!"
						"(%s, %d)\n"
						, __FILE__, __LINE__);
				g_message(response_tostring(r) -> str);
				g_string_free(data, TRUE);
				g_string_free(content, TRUE);
				return -1;
			}
			//calculate the message we have not read.
			gint cl = atoi(clen -> str);
			need_to_read = cl - r -> msg -> len;
			has_read = 0;
			g_string_truncate(data, 0);
		}
	}
	g_debug("Read all data.(%s, %d)\n", __FILE__, __LINE__);
	if(r == NULL){
		//we do not find "\r\n\r\n".
		//Should not happen.
		g_warning("Read all data, but not find all headers.!"
				"(%s, %d)\n", __FILE__, __LINE__);
		g_string_free(data, TRUE);
		g_string_free(content, TRUE);
		return -1;
	}
	//copy the message to r -> msg;
	g_string_append(r -> msg, data -> str);
	g_debug("Append the message to r -> msg.(%s, %d)\n"
			, __FILE__, __LINE__);
	#undef BUFSIZE
	g_string_free(data, TRUE);
	g_string_free(content, TRUE);
	g_debug("Free the temporary memory.(%s, %d)\n", __FILE__, __LINE__);
	return 0;
}


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
		g_warning("Can't get the address information of %s (%s, %d)"
				,hostname , __FILE__, __LINE__);
		return NULL;
	}
	
	g_debug("Got addrinfo.");
	struct sockaddr_in *sinp;
	#define BUFLEN 200
	gchar buf[BUFLEN];
	
	for(aip = ailist; aip != NULL; aip = aip -> ai_next){
		
		if((sockfd =socket(aip -> ai_family, SOCK_STREAM, 0)) < 0){
			g_warning("Can't create a socket.(%s, %d)"
					, __FILE__, __LINE__);
			return NULL;
		}
		
		g_debug("create a socket %d", sockfd);
		sinp = (struct sockaddr_in *)aip -> ai_addr;
		
		/*
		 * the http protocol uses port 80
		 */
		sinp -> sin_port = htons((gint16)port);
		
		g_debug("Address: %s Port %d"
				, inet_ntop(AF_INET, &sinp -> sin_addr
						, buf, BUFLEN)
				, ntohs(sinp -> sin_port));
		
		if(connect(sockfd,aip -> ai_addr, aip -> ai_addrlen) < 0){
			close(sockfd);
			sockfd = -1;
			g_warning("Can't connect to the server.(%s, %d)"
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
			g_debug("Write %d bytes data.(%s, %d)"
					, bytes_written, __FILE__
					, __LINE__);
			break;
		case G_IO_STATUS_EOF:
			g_warning("Write data EOF!! What's happenning??");
			return -1;
		case G_IO_STATUS_ERROR:
			g_warning("Write data ERROR!! code:%d msg:%s"
					, err -> code, err -> message);
			return -1;
		case G_IO_STATUS_AGAIN:
			g_debug("Channel temporarily unavailable.");
			break;
		default:
			g_warning("Unknown io status!");
			return -1;
		}
	}
	status = g_io_channel_flush(con -> channel, &err);
	if(status != G_IO_STATUS_NORMAL){
		g_warning("Flush io channel error! But don't warry...");
	}	
	g_debug("Write all date.(%s, %d)", __FILE__, __LINE__);
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
	//store the data that has read
	GString *data = g_string_new(NULL);
	GString *content = g_string_new("Content-Length");
	gint cl = -1;				//the content length
	gboolean nocontentlength = FALSE;	//no content lenght
						//so, we should do sth

	#define BUFSIZE 500
	gchar buf[BUFSIZE];
	GIOStatus status;
	GError *err = NULL;
	gsize bytes_read = 0;
	
	g_debug("Begin to read data.(%s, %d)", __FILE__, __LINE__);
	while(need_to_read > 0){
		status = g_io_channel_read_chars(con -> channel, buf
				, BUFSIZE < need_to_read? 
					BUFSIZE : need_to_read
				, &bytes_read, &err);
		if(nocontentlength && bytes_read < BUFSIZE){
			//no content length sent to us.
			//So, we think we got all the data.
			g_string_append_len(data, buf, bytes_read);
			break;
		}
		switch(status)
		{
		case G_IO_STATUS_NORMAL:
			g_debug("Read %d bytes data.(%s, %d)"
					,bytes_read,  __FILE__, __LINE__);
			//read success.
			need_to_read -= bytes_read;
			break;
		case G_IO_STATUS_EOF:
			if(nocontentlength){
				/*
				 * no content lenght.
				 * So, the server will close the connection
				 * after send all the data.
				 */
				//we got all the data.
				break;
			}
			g_warning("Read data EOF!! What's happenning??"
					"(%s, %d)"
					, __FILE__, __LINE__);
			g_string_free(data, TRUE);
			g_string_free(content, TRUE);
			return -1;
		case G_IO_STATUS_ERROR:
			g_warning("Read data ERROR!! code:%d msg:%s"
					"(%s, %d)"
					, err -> code, err -> message
					, __FILE__, __LINE__);
			g_string_free(data, TRUE);
			g_string_free(content, TRUE);
			return -1;
		case G_IO_STATUS_AGAIN:
			g_debug("Channel temporarily unavailable.");
			break;
		default:
			g_warning("Unknown io status!(%s, %d)"
					, __FILE__, __LINE__);
			g_string_free(data, TRUE);
			g_string_free(content, TRUE);
			return -1;
		}
		g_string_append_len(data, buf, bytes_read);
		
		if(cl == -1 && g_strstr_len(data -> str, data -> len
							, "\r\n\r\n") 
						!= NULL){
			//We have gotten all the headers;
			//Find the Content-Length 's value
			r = response_new_parse(data);
			g_string_truncate(data, 0);
			GString *clen = response_get_header(r, content);
			if(clen == NULL){
				g_debug("No Content-Length!!"
						"(%s, %d)"
						, __FILE__, __LINE__);
				nocontentlength = TRUE;
				cl = 0;
				continue;
			}
			//calculate the message we have not read.
			cl = atoi(clen -> str);
			g_debug("Content-Length: %d.(%s, %d)"
					, cl, __FILE__, __LINE__);
			need_to_read = cl - r -> msg -> len;
			g_debug("Message need to read %d bytes.(%s, %d)"
					, need_to_read, __FILE__, __LINE__);
		}
	}
	g_debug("Read all data.(%s, %d)", __FILE__, __LINE__);
	if(r == NULL){
		//we do not find "\r\n\r\n".
		//Should not happen.
		g_warning("Read all data, but not find all headers.!"
				"(%s, %d)", __FILE__, __LINE__);
		g_string_free(data, TRUE);
		g_string_free(content, TRUE);
		return -1;
	}
	//copy the message to r -> msg;
	g_string_append_len(r -> msg, data -> str, data -> len);
	g_debug("Append %d bytes message to r -> msg."
			"r -> msg -> len: %d (%s, %d)"
			, data -> len, r -> msg -> len, __FILE__, __LINE__);
	#undef BUFSIZE
	*rp = r;
	if(!nocontentlength && r -> msg -> len != cl){
		g_warning("No read all the message!! content length:%d"
				" msg -> len: %d. (%s, %d)"
				, cl, r -> msg -> len, __FILE__, __LINE__);
	}
	g_string_free(data, TRUE);
	g_string_free(content, TRUE);
	g_debug("Free the temporary memory.(%s, %d)", __FILE__, __LINE__);
	return 0;
}


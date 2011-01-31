#ifndef __LIBWEBQQ_URL_H
#define __LIBWEBQQ_URL_H
#include <http.h>

/*
 * send and recive the http protocol
 */

/*
 * define a connection to any host
 */
typedef struct{
	int fd;			//socket fd
	GIOChannel *channel;	//the GIOChannel of the fd.

	gchar *buf;		//use to read data.
	gsize bufsize;		//buffer size
}Connection;

/*
 * create and free a connection.
 * Just allocate and free the memory.
 */
Connection* connection_new();
void connection_free(Connection *con);

/*
 * Connect to the url:port
 * return the socket file descriptor, or -1 for error.
 */
Connection* connect_to_host(const char *hostname, int port);
/*
 * close the connection
 */
void close_con(Connection* con);
/*
 * sent the request to fd.
 */
gint send_request(Connection* con, Request *r);

/*
 * read the response from fd.
 * the response will store in **r.
 */
gint rcv_response(Connection* con, Response **rp);
#endif

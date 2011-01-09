#ifndef __LIBWEBQQ_URL_H
#define __LIBWEBQQ_URL_H
#include <http.h>

/*
 * send and recive the http protocol
 */

/*
 * Connect to the url:port
 * return the socket file descriptor, or -1 for error.
 */
int connect_to_host(const char *hostname, int port);
/*
 * close the connection
 */
void close_con(int fd);
/*
 * sent the request to fd.
 */
int send_request(int fd, Request *r);

/*
 * read the response from fd.
 * the response will store in **r.
 */
int rcv_response(int fd, Response **r);

#endif

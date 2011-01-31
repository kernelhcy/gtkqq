#ifndef __LIBQQ_HTTP_H
#define __LIBQQ_HTTP_H
/*
 * http.h
 *
 * Describe the http protocol
 */
#include <glib.h>

/*
 * Request
 *
 * Request = Request-Line
 *		*(( general-header | request-header | entity-header ) CRLF)
 *		CRLF
 *		[ message-body ]
 * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
 */
typedef struct
{
	GString *method;
	GString *uri;
	GString *version;
	
	GTree *headers;		//the headers and values.
	GString *msg;		//message
}Request;


/*
 * Response
 * 
 * Response = Status-Line ; Section 6.1
 *		*(( general-header ; Section 4.5
 *		| response-header ; Section 6.2
 *		| entity-header ) CRLF) ; Section 7.1
 *		CRLF
 *		[ message-body ] ; Section 7.2
 * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
 */
typedef struct
{
	GString* version;
	GString* status;
	GString* reason;
	
	GTree *headers;		//the headers and values.
	GString *msg;		//message
}Response;

/*
 * create and destroy a request
 */
Request* request_new();
void request_del(Request *r);

/*
 * convert request to the string format
 */
GString* request_tostring(Request* r);

void request_set_method(Request* r, const gchar* m);
void request_set_uri(Request* r, const gchar* uri);
void request_set_version(Request* r, const gchar* v);
void request_add_header(Request* r, const gchar* name
				, const gchar* value);

/*
 * Set the some headers to simulate the explorer
 * we mask as chrome.
 */
void request_set_default_headers(Request *r);
/*
 * get the value of header name
 */
GString* request_get_header(Request* r, const GString* name);
void request_append_msg(Request* r, const gchar* msg
				, gsize len);


/*******************************************/
/*
 * create and destroy a response
 */
Response* response_new();
void response_del(Response *r);

/*
 * parse s and create a response
 */
Response* response_new_parse(GString* s);
/*
 * get the value of header
 */
GString* response_get_header(Response *rp, const GString *header);
gchar* response_get_header_chars(Response *rp, const gchar *header);
/*
 * convert response to the string format
 */
GString* response_tostring(Response* r);
/*
 * only convert the headers and status line to the string format
 */
GString* response_headers_tostring(Response* r);


/*
 * print \r or \n just like \\r \\n
 */
void print_raw(GString* s);

#endif

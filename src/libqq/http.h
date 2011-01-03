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
typedef struct _request
{
	GString *method;
	GString *uri;
	GString *version;
	
	GTree *headers;		//the headers and values.
	GString *msg;		//message
}request;


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
typedef struct _response
{
	GString* version;
	GString* status;
	GString* reason;
	
	GTree *headers;		//the headers and values.
	GString *msg;		//message
}response;

/*
 * create and destroy a request
 */
request* request_new();
void request_del(request *r);

/*
 * convert request to the string format
 */
GString* request_tostring(request* r);

void request_set_method(request* r, const gchar* m);
void request_set_uri(request* r, const gchar* uri);
void request_set_version(request* r, const gchar* v);
void request_add_header(request* r, const gchar* name
				, const gchar* value);
/*
 * get the value of header name
 */
GString* request_get_header(request* r, const GString* name);
void request_append_msg(request* r, const gchar* msg
				, gsize len);


/*******************************************/
/*
 * create and destroy a response
 */
response* response_new();
void response_del(response *r);

/*
 * parse s and create a response
 */
response* response_new_parse(GString* s);

/*
 * convert response to the string format
 */
GString* response_tostring(response* r);


/*
 * print \r or \n just like \\r \\n
 */
void print_raw(GString* s);

#endif

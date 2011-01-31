#include <http.h>
#include <glib/gprintf.h>

#define CRLF "\r\n"

/*
 * The compare function of the GTree
 */
static gint treeCompareFunc(gconstpointer a, gconstpointer b)
{
	if(a == NULL && b != NULL){
		return -1;
	}
	if(a != NULL && b == NULL){
		return 1;
	}
	if(a == NULL && b == NULL){
		return 0;
	}
	
	return g_strcmp0(((GString*)a) -> str, ((GString*)b) -> str);
}

/*
 * free all keys and values of the tree;
 */
static gboolean freeTraverseFunc(gpointer key
				, gpointer value
				, gpointer data)
{
	g_string_free((GString*)key, TRUE);
	g_string_free((GString*)value, TRUE);
	data = value; 		//no use
	return FALSE;		//not stop
}

/*
 * key/value to key:value\r\n
 */
static gboolean toStringTraverseFunc(gpointer key
				, gpointer value
				, gpointer data)
{
	if(data == NULL || key == NULL || data == NULL){
		return TRUE;
	}
	
	g_string_append(data, ((GString*)key) -> str);
	g_string_append(data, ": ");
	g_string_append(data, ((GString*)value) -> str);
	g_string_append(data, CRLF);
	
	return FALSE;		//not stop
}

/*************************
	request
**************************/

Request* request_new()
{
	Request* r = NULL;
	r = g_slice_new(Request);
	
	r -> method = g_string_new(NULL);
	r -> uri = g_string_new(NULL);
	r -> version = g_string_new(NULL);
	r -> headers = g_tree_new(treeCompareFunc);
	r -> msg = g_string_new(NULL);
	
	return r;
	
}

void request_del(Request *r)
{
	if(r == NULL){
		return;
	}
	
	g_string_free(r -> method, TRUE);
	g_string_free(r -> uri, TRUE);
	g_string_free(r -> version, TRUE);
	
	g_tree_foreach(r -> headers, freeTraverseFunc, NULL);
	g_tree_destroy(r -> headers);
	g_string_free(r -> msg, TRUE);
	
	g_slice_free(Request, r);
}

GString* request_tostring(Request *r)
{
	if(r == NULL){
		return NULL;
	}
	
	GString* s = g_string_new(NULL);
	g_string_append(s, r -> method -> str);
	g_string_append(s, " ");
	g_string_append(s, r -> uri -> str);
	g_string_append(s, " ");
	g_string_append(s, r -> version -> str);
	g_string_append(s, CRLF);
	
	g_tree_foreach(r -> headers, toStringTraverseFunc, s);
	g_string_append(s, CRLF);
	
	g_string_append(s, r -> msg -> str);
	
	return s;
}

void request_set_method(Request* r, const gchar* m)
{
	if(r == NULL || m == NULL){
		return;
	}
	g_string_truncate(r -> method, 0);
	g_string_append(r -> method, m);
}
void request_set_uri(Request* r, const gchar* uri)
{
	if(r == NULL || uri == NULL){
		return;
	}
	g_string_truncate(r -> uri, 0);
	g_string_append(r -> uri, uri);
}
void request_set_version(Request* r, const gchar* v)
{
	if(r == NULL || v == NULL){
		return;
	}
	g_string_truncate(r -> version, 0);
	g_string_append(r -> version, v);
}

void request_add_header(Request* r, const gchar* name
				, const gchar* value)
{
	if(r == NULL || name == NULL){
		return;
	}
	GString* k = g_string_new(name);
	GString* v = g_string_new(value);
	g_tree_insert(r -> headers, k, v);
}

void request_set_default_headers(Request *r)
{
	request_add_header(r, "User-Agent", "Mozilla/5.0 (X11; U; Linux i686; "
						"en-US) AppleWebKit/534.13 "
						"(KHTML, like Gecko) "
						"Chrome/9.0.597.45 "
						"Safari/534.13");
	request_add_header(r, "Accept", "text/html, application/xml;q=0.9, "
					"application/xhtml+xml, image/png, "
					"image/jpeg, image/gif, "
					"image/x-xbitmap, */*;q=0.1");
	request_add_header(r, "Accept-Language", "en-US,zh-CN,zh;q=0.9,"
							"en;q=0.8");
	request_add_header(r, "Accept-Charset", "GBK, utf-8, utf-16, "
						"*;q=0.1");
	request_add_header(r, "Accept-Encoding", "deflate, gzip, x-gzip, "
							"identity, *;q=0");
	request_add_header(r, "Connection","Keep-Alive");
}


void request_append_msg(Request* r, const gchar* msg
				, gsize len)
{
	if(r == NULL || msg == NULL || len <= 0){
		return;
	}
	g_string_append_len(r -> msg, msg, len);
}

GString* request_get_header(Request* r, const GString* name)
{
	if(r == NULL || name == NULL){
		return NULL;
	}
	
	return (GString*)g_tree_lookup(r -> headers, name);
}

/*************************
	response
**************************/
Response* response_new()
{
	Response* r = NULL;
	r = g_slice_new(Response);
	
	r -> status = g_string_new(NULL);
	r -> reason = g_string_new(NULL);
	r -> version = g_string_new(NULL);
	r -> headers = g_tree_new(treeCompareFunc);
	r -> msg = g_string_new(NULL);
	
	return r;
}

void response_del(Response *r)
{
	if(r == NULL){
		return;
	}
	
	g_string_free(r -> status, TRUE);
	g_string_free(r -> reason, TRUE);
	g_string_free(r -> version, TRUE);
	
	g_tree_foreach(r -> headers, freeTraverseFunc, NULL);
	g_tree_destroy(r -> headers);
	g_string_free(r -> msg, TRUE);
	
	g_slice_free(Response, r);
}

Response* response_new_parse(GString* s)
{
	if(s == NULL || s -> len <= 0){
		return NULL;
	}
	
	Response* r = response_new();
	gchar* rps = s -> str;
	
	gint stage = 0; 		
	gint pre, msgbegin = -1;
	gboolean isheader = FALSE, ismsg = FALSE, findcolon = FALSE;
	gchar *name = NULL, *value = NULL;
	pre = 0;
	gsize i;
	for(i = 0; i < s -> len; ++i){
		if(ismsg){
			msgbegin = i + 2;
			break;
		}
		if(!isheader){	//status line
			switch(rps[i])
			{
			case ' ':
				++stage;
				rps[i] = '\0';
				switch(stage)
				{
				case 1:	//version
					g_string_append(r -> version
							, rps + pre);
					break;
				case 2:	//status code
					g_string_append(r -> status
							, rps + pre);
					break;
				}
				pre = i + 1;
				break;
			case '\r':
				if(rps[i + 1] != '\n'){
					g_printf("Status line.Need \\r\\n!!\n");
					response_del(r);
					return NULL;
				}
				if(rps[i + 2] == '\r' && rps[i + 3] == '\n'){
					//the end of headers
					ismsg = TRUE;
					break;
				}
				rps[i] = '\0';
				//reason
				g_string_append(r -> reason, rps + pre);
				isheader = TRUE;
				++i; 	//pass the \n
				name = rps + i + 1;
				break;
			}
		}else{
			switch(rps[i])
			{
			case ':':
				if(findcolon){
					break;	
				}
				findcolon = TRUE;
				rps[i] = '\0';
				value = rps + i + 1;
				break;
			case '\r':
				if(rps[i + 1] != '\n'){
					g_debug("Headers.Need \\r\\n!!");
					response_del(r);
					return NULL;
				}
				if(rps[i + 2] == '\r' && rps[i + 3] == '\n'){
					//the end of headers
					ismsg = TRUE;
				}
				rps[i] = '\0';
				GString* n = g_string_new(name);
				GString* v = g_string_new(value);
				g_debug("Header: %s:%s", n -> str, v -> str);

				GString* tmpv = g_tree_lookup(r -> headers, n);
				if(tmpv != NULL){
					//already has the key, append value
					//use \r\n to split them
					g_string_append(tmpv, CRLF);
					g_string_append(tmpv, value);
					g_string_free(n, TRUE);
					g_string_free(v, TRUE);
				}else{	//insert 				
					g_tree_insert(r -> headers, n, v);
				}
				++i; 	//pass the \n
				name = rps + i + 1;
				findcolon = FALSE;
				break;
			}
		}
	}//end of for
	
	//copy message body
	if(msgbegin < s -> len){
		g_string_append_len(r -> msg, rps + msgbegin
				, s -> len - msgbegin);
	}
	return r;
}

GString* response_tostring(Response *r)
{
	if(r == NULL){
		return NULL;
	}
	
	GString* s = g_string_new(NULL);
	g_string_append(s, r -> version -> str);
	g_string_append(s, " ");
	g_string_append(s, r -> status -> str);
	g_string_append(s, " ");
	g_string_append(s, r -> reason -> str);
	g_string_append(s, CRLF);
	
	g_tree_foreach(r -> headers, toStringTraverseFunc, s);
	g_string_append(s, CRLF);
	
	//g_string_append(s, r -> msg -> str);
	
	return s;
}

void print_raw(GString* s)
{
	if(s == NULL){
		return;
	}
	gsize i;
	for(i = 0; i < s -> len; ++i){
		if(s -> str[i] == '\r'){
			g_printf("\\r");
			continue;
		}
		if(s -> str[i] == '\n'){
			g_printf("\\n");
			continue;
		}
		g_printf("%c", s -> str[i]);
	}
	g_printf("\n");
}

GString* response_get_header(Response *rp, const GString *header)
{
	if(rp == NULL || header == NULL){
		return NULL;
	}

	return (GString*)g_tree_lookup(rp -> headers, header);
}

gchar* response_get_header_chars(Response *rp, const gchar *header)
{
	GString *hr = g_string_new(header);
	GString *re = response_get_header(rp, hr);
	g_string_free(hr, TRUE);
	return re != NULL ? re -> str : NULL;
}

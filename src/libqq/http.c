#include "http.h"
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

request* request_new()
{
	request* r = NULL;
	r = g_slice_new(request);
	
	r -> method = g_string_new(NULL);
	r -> uri = g_string_new(NULL);
	r -> version = g_string_new(NULL);
	r -> headers = g_tree_new(treeCompareFunc);
	r -> msg = g_string_new(NULL);
	
	return r;
	
}

void request_del(request *r)
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
	
	g_slice_free(request, r);
}

GString* request_tostring(request *r)
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

void request_set_method(request* r, const gchar* m)
{
	if(r == NULL || m == NULL){
		return;
	}
	g_string_truncate(r -> method, 0);
	g_string_append(r -> method, m);
}
void request_set_uri(request* r, const gchar* uri)
{
	if(r == NULL || uri == NULL){
		return;
	}
	g_string_truncate(r -> uri, 0);
	g_string_append(r -> uri, uri);
}
void request_set_version(request* r, const gchar* v)
{
	if(r == NULL || v == NULL){
		return;
	}
	g_string_truncate(r -> version, 0);
	g_string_append(r -> version, v);
}

void request_add_header(request* r, const gchar* name
				, const gchar* value)
{
	if(r == NULL || name == NULL){
		return;
	}
	GString* k = g_string_new(name);
	GString* v = g_string_new(value);
	g_tree_insert(r -> headers, k, v);
}
void request_append_msg(request* r, const gchar* msg
				, gsize len)
{
	if(r == NULL || msg == NULL || len <= 0){
		return;
	}
	g_string_append_len(r -> msg, msg, len);
}

GString* request_get_header(request* r, const GString* name)
{
	if(r == NULL || name == NULL){
		return NULL;
	}
	
	return (GString*)g_tree_lookup(r -> headers, name);
}

/*************************
	response
**************************/
response* response_new()
{
	response* r = NULL;
	r = g_slice_new(response);
	
	r -> status = g_string_new(NULL);
	r -> reason = g_string_new(NULL);
	r -> version = g_string_new(NULL);
	r -> headers = g_tree_new(treeCompareFunc);
	r -> msg = g_string_new(NULL);
	
	return r;
}

void response_del(response *r)
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
	
	g_slice_free(response, r);
}

response* response_new_parse(GString* s)
{
	if(s == NULL || s -> len <= 0){
		return NULL;
	}
	
	response* r = response_new();
	gchar* rps = s -> str;
	
	gint stage = 0; 		
	gint pre, msgbegin = -1;
	gboolean isheader = FALSE, ismsg = FALSE, findcolon = FALSE;
	gchar *name = NULL, *value = NULL;
	pre = 0;
	gsize i;
	for(i = 0; i < s -> len; ++i){
		if(ismsg){
			msgbegin = i + 3;
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
					g_printf("Headers.Need \\r\\n!!\n");
					response_del(r);
					return NULL;
				}
				if(rps[i + 2] == '\r' && rps[i + 3] == '\n'){
					//the end of headers
					ismsg = TRUE;
					break;
				}
				rps[i] = '\0';
				GString* n = g_string_new(name);
				GString* v = g_string_new(value);
				
				GString* tmpv = g_tree_lookup(r -> headers, n);
				if(tmpv != NULL){
					//already has the key, append value
					//use \r\n to split them
					g_string_append(tmpv, CRLF);
					g_string_append(tmpv, value);
					g_string_free(n, FALSE);
					g_string_free(v, FALSE);
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

GString* response_tostring(response *r)
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
	
	g_string_append(s, r -> msg -> str);
	
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



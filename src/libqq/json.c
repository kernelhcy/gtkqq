#include "json.h"
#include <glib/gprintf.h>
#include <string.h>

/*
 * create and delete a JSON instance
 */
JSON* JSON_new()
{
	JSON *j = g_slice_new(JSON);
	j -> data = g_string_new(NULL);
	j -> curr = 0;
	j -> result = g_ptr_array_new();
	j -> algn = 0;
	return j;
}
void JSON_del(JSON *j)
{
	if(j == NULL){
		return;
	}
	g_string_free(j -> data, TRUE);
	j -> result = NULL;
	g_slice_free(JSON, j);
}

/*
 * reset the JSON instance
 * delete all the data.
 */
void JSON_reset(JSON *j){
	if(j == NULL){
		return;
	}
	g_string_truncate(j -> data, 0);
	j -> curr = 0;
	j -> result = g_ptr_array_new();
	j -> algn = 0;
}

/*
 * set the data need to parse
 */
void JSON_set_data(JSON *j, GString *data)
{
	if(j == NULL || data == NULL){
		return;
	}
	
	g_string_truncate(j -> data, 0);
	g_string_append(j -> data, data -> str);
}
void JSON_set_data_c(JSON *j, const gchar *data, gsize len)
{
	if(j == NULL || data == NULL || len < 0){
		return;
	}
	
	g_string_truncate(j -> data, 0);
	g_string_append_len(j -> data, data, len);
}

/*
 * get the result
 */
GPtrArray* JSON_get_result(JSON *j)
{
	if(j == NULL){
		return NULL;
	}
	return j -> result;
}

static gboolean is_space(gchar c)
{
	if(c == ' ' || c == '\t' || c == '\n' || c == '\r'){
		return TRUE;
	}
	return FALSE;
}

static void printalgn(gint a)
{
	gint i;
	for(i = 0; i < a; ++i){
		g_printf("\t");
	}
}

static guint hex2char(gchar c)
{
	if(c >= '0' && c <= '9'){
		return c - '0';
	}
	
	if(c >= 'a' && c <= 'f'){
		return c - 'a' + 10;
	}
	
	if(c >= 'A' && c <= 'F'){
		return c - 'A' + 10;
	}
	return 0;	
}

/*
 * decode UCS-4 to utf8
 * 	UCS-4 code		UTF-8
 * U+00000000–U+0000007F 0xxxxxxx
 * U+00000080–U+000007FF 110xxxxx 10xxxxxx
 * U+00000800–U+0000FFFF 1110xxxx 10xxxxxx 10xxxxxx
 * U+00010000–U+001FFFFF 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * U+00200000–U+03FFFFFF 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * U+04000000–U+7FFFFFFF 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 
 * Chinese are all in  U+00000800–U+0000FFFF.
 * So only decode to the third format.
 */
static void decodeunicode(GString *to, const gchar *from)
{
	if(to == NULL || from == NULL){
		return;
	}
	if(from[0] != '\\' || from[1] != 'u'){
		return;
	}
	
	static guint E = 0xe0;
	static guint T = 0x2;;
	
	guint tmp[4];
	gchar re[3];
	
	gint i;
	for(i = 0; i < 4; ++i){
		tmp[i] = 0xf & hex2char(from[ i + 2]);
	}
	
	//decode
	re[0] = E | (tmp[0] & 0xf);
	re[1] = (T << 6) | (tmp[1] << 2) | ((tmp[2] >> 2) & 0x3);
	re[2] = (T << 6) | (tmp[2] & 0x3) << 4 | tmp[3];
	
	//copy to @to.
	for(i = 0; i < 3; ++i){
		g_string_append_c(to, re[i]);
	}
}

static GString* JSON_parse_string(JSON *j)
{
	gchar *s = j -> data -> str;
	gsize begin, end;
	begin = j -> curr;
	end = begin;
	while(end < j -> data -> len){
		if(s[end -1] != '\\' && s[end] == '"'){
			break;
		}
		++end;
	}
	GString* v = g_string_new(NULL);
	gint i = begin;
	s[end] = '\0';
	for(; s[i] != '\0'; ++i){
		if(s[i] == '\\' && s[i + 1] == 'u'){
			decodeunicode(v, s + i);
			i += 5;
		}else{
			g_string_append_c(v, s[i]);
		}
	}
	s[end] = '"';
	++end;
	//move the curr to next position
	while(is_space(s[end])){
		++end;
	}
	j -> curr = end;
	
	return v;
}

static GString* JSON_parse_key(JSON *j)
{
	gchar *s = j -> data -> str;
	while(is_space(s[j -> curr])){
		++(j -> curr);
	}
	
	//key must be string
	if(s[j -> curr] != '"'){
		return NULL;
	}
	
	++(j -> curr);		//pass the "
	GString *key = JSON_parse_string(j);
	
	while(j -> curr < j -> data -> len && s[j -> curr] != ':'){
		++(j -> curr);
	}
	
	++(j -> curr);	//pass the colon
	
	printalgn(j -> algn);
	g_printf("key : %s", key -> str);
	
	return key;
}

static GPtrArray* JSON_parse_object(JSON *j);
static GPtrArray* JSON_parse_array(JSON *j);

static gpointer JSON_parse_value(JSON *j)
{
	gchar *s = j -> data -> str;
	
	while(is_space(s[j -> curr])){
		++(j -> curr);
	}
	gchar cc = s[j -> curr];
	switch(cc)
	{
	case '{':
		++(j -> curr);
		return JSON_parse_object(j);
	case '[':
		++(j -> curr);
		return JSON_parse_array(j);
	case '"':
		++(j -> curr);
		GString *s = JSON_parse_string(j);
		printalgn(j -> algn);
		g_printf("value : %s\n", s -> str);
		return s;
	default:
		break;
	}
	//number, true, false, null
	//treate them as string
	gsize begin, end;
	begin = j -> curr;
	end = begin;
	while(!is_space(s[end]) && s[end] != ',' 
			&& s[end] != '}' && s[end] != ']'){
		++end;
	}
	gchar tmp = s[end];
	s[end] = '\0';
	GString* v = g_string_new(begin + s);
	s[end] = tmp;
	
	//move the curr to next position
	while(is_space(s[end])){
		++end;
	}
	j -> curr = end;
	
	printalgn(j -> algn);
	g_printf("value : %s\n", v -> str);
	return v;
}

static GPtrArray* JSON_parse_object(JSON *j)
{
	gchar *s = j -> data -> str;
	
	while(is_space(s[j -> curr])){
		++(j -> curr);
	}
		
	g_printf("\n");
	printalgn(j -> algn);
	g_printf("object{\n");
	j -> algn += 1;
	
	//{}
	if(s[j -> curr] == '}'){
		++(j -> curr);
		return NULL;
	}
	
	GPtrArray* array = g_ptr_array_new();
	GString *key = NULL, *value = NULL;
	for(;j -> curr < j -> data -> len; ++(j -> curr)){
		if(is_space(s[j -> curr]) || s[j -> curr] == ','){
			continue;
		}
		
		key = JSON_parse_key(j);
		value = JSON_parse_value(j);
		JPair *p = g_slice_new(JPair);
		p -> key = key;
		p -> value = value;
		g_ptr_array_add(array, p);
		if(s[j -> curr] == '}'){
			++(j -> curr);
			while(j -> curr < j -> data -> len 
					&& is_space(s[j -> curr])){
				++(j -> curr);
			}
			break;
		}
	}
	
	j -> algn -= 1;
	printalgn(j -> algn);
	g_printf("}\n");
	
	return array;
}

static GPtrArray* JSON_parse_array(JSON *j)
{
	gchar *s = j -> data -> str;
	
	while(is_space(s[j -> curr])){
		++(j -> curr);
	}
	
	g_printf("\n");
	printalgn(j -> algn);
	g_printf("array[\n");
	j -> algn += 1;
	
	//[]
	if(s[j -> curr] == ']'){
		++(j -> curr);
		j -> algn -= 1;
		printalgn(j -> algn);
		g_printf("]\n");
		return NULL;
	}
	
	GPtrArray* array = g_ptr_array_new();
	GString *value = NULL;
	for(;j -> curr < j -> data -> len; ++(j -> curr)){
		if(is_space(s[j -> curr]) || s[j -> curr] == ','){
			continue;
		}
		
		value = JSON_parse_value(j);
		g_ptr_array_add(array, value);
		if(s[j -> curr] == ']'){
			++(j -> curr);
			while(j -> curr < j -> data -> len 
					&& is_space(s[j -> curr])){
				++(j -> curr);
			}
			break;
		}
	}
	
	j -> algn -= 1;
	printalgn(j -> algn);
	g_printf("]\n");
	
	return array;
}

/*
 * parse the data.
 * return 0 means success, -1 means failed or error
 */
gint JSON_parse(JSON *j)
{
	if(j == NULL || j -> data -> len <=0){
		return 0;
	}
	
	gchar *s = j -> data -> str;
	gchar cc;
	for(j -> curr = 0; j -> curr < j -> data -> len; ++(j -> curr)){
		cc = s[j -> curr];
		if(is_space(cc)){
			continue;
		}
		switch(cc)
		{
		case '{':
			++(j -> curr);
			j -> result = JSON_parse_object(j);
			return 0;
		case '[':
			++(j -> curr);
			j -> result = JSON_parse_array(j);
			return 0;
		}
	}
	return -1;
}


#include "json.h"
#include <glib/gprintf.h>
#include <string.h>

JSON_data* JSON_data_new()
{
	JSON_data *d;
	d = g_slice_new(JSON_data);
	d -> data = NULL;
	d -> type = JSON_UNKNOWN;
	return d;
}

/*
 * cascaded free the JSON_data.
 */
void JSON_data_free(JSON_data *data)
{
	if(data == NULL){
		return;
	}
	
	gint i = -1;
	GPtrArray *array = NULL;
	JPair *p = NULL;
	
	switch(data -> type)
	{
	case JSON_STRING:
		g_string_free((GString*)data -> data, TRUE);
		break;
		
	case JSON_ARRAY:
	case JSON_OBJECT:
		/*
		 * object and array are all arrays;
		 */
		
		array = (GPtrArray*)(data -> data);
		if(array == NULL){
			break;
		}
		for(i = 0; i < array -> len; ++i){
			JSON_data_free(array -> pdata[i]);
		}
		g_ptr_array_free(array, TRUE);
		
		break;
	case JSON_PAIR:
		p = (JPair*)data -> data;
		g_string_free(p -> key, TRUE);
		JSON_data_free(p -> value);
		g_slice_free(JPair, p);
		break;
	case JSON_UNKNOWN:
	default:
		break;
	}
}

static void printalgn(gint a)
{
	gint i;
	for(i = 0; i < a; ++i){
		g_printf("\t");
	}
}

static void JSON_data_print_help(JSON_data *jd, gint alg)
{
	if(jd == NULL){
		return;
	}
	
	gint i = -1;
	GPtrArray *array = NULL;
	JPair *p = NULL;
	
	switch(jd -> type)
	{
	case JSON_STRING:
		printalgn(alg);
		g_printf("%s\n", ((GString*)jd -> data) -> str);
		break;
		
	case JSON_ARRAY:
		array = (GPtrArray*)(jd -> data);
		if(array == NULL){
			break;
		}
		printalgn(alg);
		g_printf("[\n");
		++alg;
		for(i = 0; i < array -> len; ++i){
			JSON_data_print_help(array -> pdata[i], alg);
		}
		--alg;
		printalgn(alg);
		g_printf("]\n");
		break;
	case JSON_OBJECT:
		array = (GPtrArray*)(jd -> data);
		if(array == NULL){
			break;
		}
		printalgn(alg);
		g_printf("{\n");
		++alg;
		for(i = 0; i < array -> len; ++i){
			JSON_data_print_help(array -> pdata[i], alg);
		}
		--alg;
		printalgn(alg);
		g_printf("}\n");
		break;
	case JSON_PAIR:
		p = (JPair*)jd -> data;
		printalgn(alg);
		g_printf("%s : ", ((GString*)p -> key) -> str);
		
		JSON_data_print_help(p -> value, 1);
		break;
	case JSON_UNKNOWN:
	default:
		break;
	}
}

void JSON_data_print(JSON_data *jd)
{
	JSON_data_print_help(jd, 0);
}

/*
 * create and free a JSON instance
 */
JSON* JSON_new()
{
	JSON *j = g_slice_new(JSON);
	j -> data = g_string_new(NULL);
	j -> curr = 0;
	j -> result = NULL;
	return j;
}
void JSON_free(JSON *j)
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
 * free all the data.
 */
void JSON_reset(JSON *j){
	if(j == NULL){
		return;
	}
	g_string_truncate(j -> data, 0);
	j -> curr = 0;
	JSON_data_free(j -> result);
	j -> result = NULL;
}

/*
 * set the data need to parse
 */
void JSON_set_rawdata(JSON *j, GString *raw)
{
	if(j == NULL || raw == NULL){
		return;
	}
	
	g_string_truncate(j -> data, 0);
	g_string_append(j -> data, raw -> str);
}
void JSON_set_raw_data_c(JSON *j, const gchar *raw, gsize len)
{
	if(j == NULL || raw == NULL || len < 0){
		return;
	}
	
	g_string_truncate(j -> data, 0);
	g_string_append_len(j -> data, raw, len);
}

/*
 * get the result
 */
JSON_data* JSON_get_result(JSON *j)
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
 * So only decode to the second and third format.
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
	static guint T = 0x2;
	static guint sep = 0x800;
	
	guint tmp[4];
	gchar re[3];
	guint ivalue = 0;
	
	gint i;
	for(i = 0; i < 4; ++i){
		tmp[i] = 0xf & hex2char(from[ i + 2]);
		ivalue *= 16;
		ivalue += tmp[i];
	}
	
	//decode
	if(ivalue < sep){
		//110xxxxx 10xxxxxx
		re[0] = (0x3 << 6) | ((tmp[1] & 7) << 2) | (tmp[2] >> 2);
		re[1] = (0x1 << 7) | ((tmp[2] & 3) << 4) | tmp[3];
		g_string_append_c(to, re[0]);
		g_string_append_c(to, re[1]);
	}else{
		//1110xxxx 10xxxxxx 10xxxxxx
		re[0] = E | (tmp[0] & 0xf);
		re[1] = (T << 6) | (tmp[1] << 2) | ((tmp[2] >> 2) & 0x3);
		re[2] = (T << 6) | (tmp[2] & 0x3) << 4 | tmp[3];
		//copy to @to.
		for(i = 0; i < 3; ++i){
			g_string_append_c(to, re[i]);
		}
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
	
	return key;
}

static JSON_data* JSON_parse_object(JSON *j);
static JSON_data* JSON_parse_array(JSON *j);

static JSON_data* JSON_parse_value(JSON *j)
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
		GString *ss = JSON_parse_string(j);
		
		JSON_data *jd = JSON_data_new();
		jd -> data = ss;
		jd -> type = JSON_STRING;
		
		return jd;
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
	
	JSON_data *jd = JSON_data_new();
	jd -> data = v;
	jd -> type = JSON_STRING;
	
	return jd;
}

static JSON_data* JSON_parse_object(JSON *j)
{
	gchar *s = j -> data -> str;
	
	while(is_space(s[j -> curr])){
		++(j -> curr);
	}
	
	//{}
	if(s[j -> curr] == '}'){
		++(j -> curr);
		return NULL;
	}
	
	GPtrArray* array = g_ptr_array_new();
	GString *key = NULL;
	JSON_data *value = NULL;
	JSON_data *jd = NULL;
	for(;j -> curr < j -> data -> len; ++(j -> curr)){
		if(is_space(s[j -> curr]) || s[j -> curr] == ','){
			continue;
		}
		
		key = JSON_parse_key(j);
		value = JSON_parse_value(j);
		JPair *p = g_slice_new(JPair);
		p -> key = key;
		p -> value = value;
		
		jd = JSON_data_new();
		jd -> data = p;
		jd -> type = JSON_PAIR;
		g_ptr_array_add(array, jd);
		
		if(s[j -> curr] == '}'){
			++(j -> curr);
			while(j -> curr < j -> data -> len 
					&& is_space(s[j -> curr])){
				++(j -> curr);
			}
			break;
		}
	}
	
	jd = JSON_data_new();
	jd -> data = array;
	jd -> type = JSON_OBJECT;
	
	return jd;
}

static JSON_data* JSON_parse_array(JSON *j)
{
	gchar *s = j -> data -> str;
	
	while(is_space(s[j -> curr])){
		++(j -> curr);
	}
	
	//[]
	if(s[j -> curr] == ']'){
		++(j -> curr);
		return NULL;
	}
	
	GPtrArray *array = g_ptr_array_new();
	JSON_data *value = NULL;
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
	
	JSON_data *jd = JSON_data_new();
	jd -> data = array;
	jd -> type = JSON_ARRAY;
	
	return jd;
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


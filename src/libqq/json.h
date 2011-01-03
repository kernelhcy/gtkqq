#ifndef ___GTKQQ__JSON_H
#define ___GTKQQ__JSON_H
/*
 * a simple JSON parser
 *
 * object = {} | {members}
 * members = pair | pair, members
 * pair = key:value
 * array = [] | [elements]
 * elements = value | value, elements
 * value = string | number | object | array | true | false | null
 */
#include <glib.h>

//json
typedef struct{
	GString *data;		//origin data, need to parse
	gsize curr; 		//the current postion
	GPtrArray *result;	//the result of parsing
	gint algn;		//use to print;
}JSON;

//pair
typedef struct{
	GString *key;
	gpointer value;
}JPair;

/*
 * create and delete a JSON instance
 * JSON_del will not free the result. You MUST free it by yourself.
 */
JSON* JSON_new();
void JSON_del(JSON *j);

/*
 * reset the JSON instance
 * delete all the data.
 */
void JSON_reset(JSON *j);

/*
 * set the data need to parse
 */
void JSON_set_data(JSON *j, GString *data);
void JSON_set_data_c(JSON *j, const gchar *data, gsize len);

/*
 * get the result
 */
GPtrArray* JSON_get_result(JSON *j);

/*
 * parse the data.
 * return 0 means success, -1 means failed or error
 */
gint JSON_parse(JSON *j);
#endif

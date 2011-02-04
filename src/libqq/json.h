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

/*
 * Usage:
 *	JSON *j = JSON_new();
 *	JSON_set_raw_data_c(j, rawdata, rawdatale);
 *	JSON_parse(j);
 *	JSON_data *re = JSON_get_result(j);
 *	JSON_data_free(re);
 *	JSON_free(j);
 *
 * The parsed result is stored in a JSON_data instance.
 * When you use the data in JSON_data, you would better copy the data,
 * because when you free the JSON_data, all the data in it will also BE freed.
 * You MUST free the JSON_data by yourself! No one will help you.
 * When you free the JSON instance. JSON_free will just set JSON->result to
 * NULL, so you should free the result.
 */

/*
 * The data type.
 * We treat all value as string. So, there are only four types --
 * string, array, object and pair.
 * We use array to represent object. The only difference is that
 * in an object, the elements are pointers of JPair, where in 
 * the normal array, the elements are pointers to any thing. 
 */
typedef enum{
	JSON_STRING,
	JSON_ARRAY,
	JSON_PAIR,
	JSON_OBJECT,
	JSON_UNKNOWN,
	
	//not used
	JSON_INT,
	JSON_FLOAT,
	JSON_TRUE,
	JSON_FALSE,
	JSON_NULL
}JSON_DATA_T;

/*
 * the data and it's type
 */
typedef struct{
	gpointer data;
	JSON_DATA_T type;
}JSON_data;

//json
typedef struct{
	GString *data;		//origin data, need to parse
	gsize curr; 		//the current postion
	JSON_data *result;	//the result of parsing
}JSON;

//pair
typedef struct{
	GString *key;
	JSON_data *value;
}JPair;

/*
 * create and delete a JSON instance
 * JSON_del will not free the result. You MUST free it by yourself.
 */
JSON* JSON_new();
void JSON_free(JSON *j);
void JSON_print(JSON *j);
/*
 * reset the JSON instance
 * delete all the data.
 */
void JSON_reset(JSON *j);

/*
 * Find the value of key whoes type is type.
 *
 * Return NULL, if not found or type not match
 */
gpointer JSON_find_pair_value(JSON *j, JSON_DATA_T type, const gchar *key);

/*
 * set the data need to parse
 */
void JSON_set_raw_data(JSON *j, GString *raw);
void JSON_set_raw_data_c(JSON *j, const gchar *raw, gsize len);

/*
 * parse the data.
 * return 0 means success, -1 means failed or error
 */
gint JSON_parse(JSON *j);
#endif

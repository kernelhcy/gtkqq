#include <json.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

int main(int argc, char ** argv)
{
	gchar d[2000];
	gchar *file;
	
	file = argv[1];
	GString * input = g_string_new(NULL);
	FILE *fd = fopen(file, "r");
	gsize len;
	while((len = fread(d, sizeof(gchar), 2000, fd)) > 0){
		g_string_append_len(input, d, len);
	}
	
	fclose(fd);
	
	enum json_error error;
	json_t *json = NULL;
	error = json_parse_document(&json, input -> str);
	switch(error)
	{
	case JSON_OK:
		break;
	default:
		g_warning("json_parse_docuemt error.");
		break;
	}
	json_t *val;
	val = json_find_first_label_all(json, "year");
	if(val != NULL){
		g_debug("year: %s", val -> child -> text);
	}
	json_free_value(&json);
	return 0;
}

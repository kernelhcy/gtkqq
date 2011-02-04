#include <json.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
	
	JSON *j = JSON_new();
	JSON_set_raw_data_c(j, input -> str, input -> len);
	JSON_parse(j);
	JSON_print(j);
	
	return 0;
}

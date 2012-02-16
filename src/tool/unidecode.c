#include <glib.h>
#include <glib/gprintf.h>
#include <stdio.h>

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
	return -1;	
}

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

#define BUFSIZE 100
int main(int argc, char **argv)
{
	gchar buf[BUFSIZE];
	gchar *file;
	
	file = argv[1];
	
	GString * input = g_string_new(NULL);
	
	FILE *fd = fopen(file, "r");
	gsize len;
	while((len = fread(buf, sizeof(gchar), BUFSIZE, fd)) > 0){
		g_string_append_len(input, buf, len);
	}
	
	fclose(fd);
	GString *out = g_string_new(NULL);
	
	gint i;
	for(i = 0; input -> str[i] != '\0'; ++i){
		if(input -> str[i] == '\\' && input -> str[i + 1] == 'u'){
			decodeunicode(out, input -> str + i);
			i += 5;
		}else{
			g_string_append_c(out, input -> str[i]);
		}
	}
	
	g_printf("%s\n", out -> str);
	g_string_free(out, TRUE);
	
	return 0;
}



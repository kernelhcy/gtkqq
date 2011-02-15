#include <unicode.h>

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
 *      UCS-4 code              UTF-8
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

static void do_ucs4toutf8(GString *to, const gchar *from)
{
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

void ucs4toutf8(GString *to, const gchar *from)
{
        if(to == NULL || from == NULL){
                return;
        }
	const gchar *c;
	for(c = from; *c != '\0'; ++c){
		if(*c == '\\' && *(c + 1) == 'u'){
			do_ucs4toutf8(to, c);
			c+= 5;
		}else{
			g_string_append_c(to, *c);
			continue;
		}
	}
}



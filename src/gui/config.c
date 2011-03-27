#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

QQConfig* qq_config_init()
{
	QQConfig *cfg = g_slice_new0(QQConfig);
	if(!g_file_test(CONFIGDIR, G_FILE_TEST_EXISTS)){
		/*
		 * The program is first run.
		 * Create the configure dir.
		 */
		if(-1 == g_mkdir(CONFIGDIR, 0777)){
			g_warning("Create config dir %s error!(%s, %d)"
					, CONFIGDIR, __FILE__, __LINE__);
			qq_config_free(cfg);
			return NULL;
		}
	}

	cfg -> userinfo = g_ptr_array_new();
	return cfg;
}

void qq_config_free(QQConfig *cfg)
{
	if(cfg == NULL){
		return;
	}
	
	gint i;
	for( i = 0; i < cfg -> userinfo -> len; ++i ){
		g_free(cfg -> userinfo -> pdata[i]);
	}
	g_ptr_array_free(cfg -> userinfo, TRUE);

	g_slice_free(QQConfig, cfg);
}

QQUserConfig* qq_userconfig_new(QQConfig *cfg, const gchar *uin)
{
	if(cfg == NULL || uin == NULL){
		g_warning("cfg == NULL || uin == NULL! (%s, %d)"
					, __FILE__, __LINE__);
		return NULL;
	}

	QQUserConfig *usrcfg = g_slice_new0(QQUserConfig);
	if(usrcfg == NULL){
		g_warning("malloc memory fro QQUserConfig error!");
	}
	gchar cfgdirname[200];
	g_sprintf(cfgdirname, CONFIGDIR"%s/", cfg -> me -> uin -> str);
	g_debug("configure dir %s. (%s, %d)", cfgdirname, __FILE__, __LINE__);
	
	if(!g_file_test(cfgdirname, G_FILE_TEST_EXISTS)){
		/*
		 * This user first uses this program. So craete the user
		 * configure dir.
		 */
		if(-1 == g_mkdir(cfgdirname, 0755)){
			g_warning("Create user config dir error!(%s, %d"
					, __FILE__, __LINE__);
			qq_userconfig_free(usrcfg);
			return NULL;
		}
	}

	cfg -> usr_cfg = usrcfg;
	return usrcfg;
}

void qq_userconfig_free(QQUserConfig *cfg)
{
	if(cfg == NULL){
		return;
	}
	g_string_free(cfg -> icondir, TRUE);

	g_slice_free(QQUserConfig, cfg);
}

//
// restart when interupt by signal.
//
static int safe_write(int fd, const char *buf, int len)
{
	if(fd < 0 || buf == NULL || len <=0){
		return 0;
	}

	int haswritten = 0;
	int re;
	while(haswritten < len){
		re = write(fd, buf + haswritten, len - haswritten);
		if(re == -1){
			g_warning("Write to file error. %s (%s, %d)"
					, strerror(errno), __FILE__, __LINE__);
			return -1;
		}
		haswritten += re;
	}
}


// read configuration from the config file
gint qq_config_read(QQConfig* cfg)
{
	if(cfg == NULL){
		return;
	}

	gchar filename[100];
	g_sprintf(filename, CONFIGDIR"/config");
	int fd = open(filename, O_RDONLY, 0755);
	if(fd == -1){
		g_warning("Error when open file: %s. %s (%s, %d)", filename
				, strerror(errno), __FILE__, __LINE__);
		return -1;
	}
	
	GString *str = g_string_new(NULL);
	char buf[100];
	int len;
	do{
		len = read(fd, buf, 100);
		if(len == -1){
			g_warning("Write to file error. %s (%s, %d)"
					, strerror(errno), __FILE__, __LINE__);
			return -1;
		}
		g_string_append_len(str, buf, len);
	}while(len >= 100);

	int i;
	for( i = 0; i < str -> len; ++i){
		if(str -> str[i] == '\n'){
			str -> str[i] = '\0';
		}
	}

	int idx = 0;
	QQUserInfo *info;
	gchar *tmp;
	while(1){
		info = g_malloc(sizeof(QQUserInfo));
		idx += g_sprintf(info -> uin, "%s", str -> str + idx);
		if(idx <= 0){
			break;
		}
		++idx; 		//pass the '\0'
		
		idx += g_sprintf(buf, "%s", str -> str + idx);
		tmp = g_base64_decode(buf, &len);
		g_sprintf(info -> passwd, "%s", tmp);
		g_free(tmp);
		++idx;

		idx += g_sprintf(info -> status, "%s", str -> str + idx);
		++idx;
	}

}
// write configuration to file
gint qq_config_write(QQConfig *cfg)
{
	if(cfg == NULL){
		return;
	}

	gchar filename[100];
	g_sprintf(filename, CONFIGDIR"/config");
	int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0755);
	if(fd == -1){
		g_warning("Error when open file: %s. %s (%s, %d)", filename
				, strerror(errno), __FILE__, __LINE__);
		return -1;
	}

	int len, i;
	char *tmp;
	QQUserInfo *info;
	for( i = 0; i < cfg -> userinfo -> len; ++i){
		info = (QQUserInfo*)cfg -> userinfo -> pdata[i];
		//save uin
		len = strlen(info -> uin);
		safe_write(fd, info -> uin, len);
		safe_write(fd, "\n", 1);

		//base64 encode the passwd
		len = strlen(info -> passwd);
		tmp = g_base64_encode(info -> passwd, len);
		len = strlen(tmp);
		safe_write(fd, tmp, len);
		safe_write(fd, "\n", 1);

		//save the status
		len = strlen(info -> status);
		safe_write(fd, info -> status, len);
		safe_write(fd, "\n", 1);
	}
}

// read the user configuration from the usrconfig file
gint qq_userconfig_read(QQUserConfig *cfg)
{

}
// write the user configuration to file.
gint qq_userconfig_write(QQUserConfig *cfg)
{

}

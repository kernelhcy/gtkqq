#include <config.h>

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
	return cfg;
}

void qq_config_free(QQConfig *cfg)
{
	if(cfg == NULL){
		return;
	}

	g_slice_free(QQConfig, cfg);
}

QQUserConfig* qq_userconfig_new(QQConfig *cfg)
{
	if(cfg == NULL){
		g_warning("cfg == NULL! (%s, %d)", __FILE__, __LINE__);
		return NULL;
	}

	if(cfg -> me == NULL || cfg -> me -> uin == NULL){
		g_warning("me == NULL || me -> uin == NULL!(%s, %d)"
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
		if(-1 == g_mkdir(cfgdirname, 755)){
			g_warning("Create user config dir error!(%s, %d"
					, __FILE__, __LINE__);
			qq_userconfig_free(usrcfg);
			return NULL;
		}
	}
	return usrcfg;
}

void qq_userconfig_free(QQUserConfig *cfg)
{
	if(cfg == NULL){
		return;
	}

	g_slice_free(QQUserConfig, cfg);
}

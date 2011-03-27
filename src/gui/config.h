#ifndef __GTKQQ_CONFIG_H
#define __GTKQQ_CONFIG_H
#include <glib.h>
#include <qq.h>

typedef struct _QQConfig 	QQConfig;
typedef struct _QQUserConfig 	QQUserConfig;

typedef struct _QQUserInfo 	QQUserInfo;

struct _QQUserInfo{
	gchar uin[50];
	gchar passwd[50];
	gchar status[50];
};

struct _QQConfig{
	QQBuddy *me;
	QQUserConfig *usr_cfg;

	GPtrArray *userinfo;
};
/*
 * This funcion MUST be called at the first of the program.
 */
QQConfig* qq_config_init();
void qq_config_free(QQConfig* cfg);
// read configuration from the config file
gint qq_config_read(QQConfig* cfg);
// write configuration to file
gint qq_config_write(QQConfig *cfg);

struct _QQUserConfig{
	GString *icondir;
	QQInfo *info;
};
QQUserConfig *qq_userconfig_new(QQConfig *cfg, const gchar *uin);
void qq_userconfig_free(QQUserConfig *cfg);
// read the user configuration from the usrconfig file
gint qq_userconfig_read(QQUserConfig *cfg);
// write the user configuration to file.
gint qq_userconfig_write(QQUserConfig *cfg);

#endif

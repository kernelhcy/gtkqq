#ifndef __GTKQQ_CONFIG_H
#define __GTKQQ_CONFIG_H
#include <glib.h>
#include <qq.h>

typedef struct _QQConfig 	QQConfig;
typedef struct _QQUserConfig 	QQUserConfig;

struct _QQConfig{
	QQBuddy *me;
	QQUserConfig *usr_cfg;
};
/*
 * This funcion MUST be called at the first of the program.
 */
QQConfig* qq_config_init();
void qq_config_free(QQConfig* cfg);

struct _QQUserConfig{
	GString *icondir;
};
QQUserConfig *qq_userconfig_new(QQConfig *cfg);
void qq_userconfig_free(QQUserConfig *cfg);

#endif

#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


//
// Private members
//
typedef struct{
    GString *uin, *passwd;
}GQQConfigPriv;

//
// Preperty ID
//
enum{
    GQQ_CONFIG_PROPERTY_0,
    GQQ_CONFIG_PROPERTY_UIN,
    GQQ_CONFIG_PROPERTY_PASSWD
};

static void gqq_config_init(GQQConfig *self);
static void gqq_config_class_init(GQQConfigClass *klass, gpointer data);

static gpointer gqq_config_parent_class = NULL;

GType gqq_config_get_type()
{
    static volatile gsize g_define_type_id__volatile = 0;
    static GType type_id = 0;
    if (g_once_init_enter(&g_define_type_id__volatile)) {
            if(type_id == 0){
                    GTypeInfo type_info={
                            sizeof(GQQConfigClass),     /* class size */
                            NULL,                       /* base init*/
                            NULL,                       /* base finalize*/
                            /* class init */
                            (GClassInitFunc)gqq_config_class_init,
                            NULL,                       /* class finalize */
                            NULL,                       /* class data */
                                
                            sizeof(GQQConfig),          /* instance size */
                            0,                          /* prealloc bytes */
                            /* instance init */
                            (GInstanceInitFunc)gqq_config_init,
                            NULL                        /* value table */
                    };
                    type_id = g_type_register_static(
                                G_TYPE_OBJECT,
                                "GQQConfig",
                                &type_info,
                                0);
                    
                    };
            }
            g_once_init_leave(&g_define_type_id__volatile, type_id);
    }
    return type_id;
}


/*
 * The getter.
 */
static void gqq_config_getter(GObject *object, guint property_id,  
                                    GValue *value, GParamSpec *pspec)
{
        if(object == NULL || value == NULL || property_id < 0){
                return;
        }
        
        GQQConfig *obj = G_TYPE_CHECK_INSTANCE_CAST(
                                        object, gqq_config_get_type(), GQQConfig);
        GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    obj, gqq_config_get_type(). GQQConfigPriv);
        
        switch (property_id)
        {
        case GQQ_CONFIG_PROPERTY_UIN:
                g_value_set_int(value, priv -> uin -> str);
                break;
        case GQQ_CONFIG_PROPERTY_PASSWD:
                g_value_set_string(value, priv -> passwd -> str);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
                break;
        }
}

/*
 * The setter.
 */
static void gqq_config_setter(GObject *object, guint property_id,  
                                 const GValue *value, GParamSpec *pspec)
{
        if(object == NULL || value == NULL || property_id < 0){
                return;
        }
        
        GQQConfig *obj = G_TYPE_CHECK_INSTANCE_CAST(
                                        object, gqq_config_get_type(), GQQConfig);
        GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    obj, gqq_config_get_type(). GQQConfigPriv);
        
        switch (property_id)
        {
        case GQQ_CONFIG_PROPERTY_UIN:
                g_string_truncate(priv -> uin, 0);
                g_string_append(priv -> uin, g_value_get_string(value));
                break;
        case GQQ_CONFIG_PROPERTY_PASSWD:
                g_string_truncate(priv -> passwd, 0);
                g_string_append(priv -> passwd, g_value_get_string(value));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
                break;
        }
}

static void gqq_config_init(GQQConfig *self)
{
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(self
                                        , GQQ_TYPE_CONFIG, GQQConfigPriv);
    priv -> uin = g_string_new("");
    priv -> passwd = g_string_new("");

}
static void gqq_config_class_init(GQQConfigClass *klass, gpointer data)
{
    //add the private members.
    g_type_class_add_private(klass, sizeof(GQQConfigPriv));
    
    G_OBJECT_CLASS(klass)-> get_property = gqq_config_getter;
    G_OBJECT_CLASS(klass)-> set_property = gqq_config_setter;

    GParamSpec *my_param_spec;
    // Property "uin"  
    my_param_spec = g_param_spec_string(
                "uin",
                "Uin Property",
                "The user's uin.",
                "",     /* default value */
                G_PARAM_READABLE | G_PARAM_WRITABLE 
                );
    g_object_class_install_property(
                G_OBJECT_CLASS(klass),
                GQQ_CONFIG_PROPERTY_UIN,
                my_param_spec
                );

    // Property "passwd"  
    my_param_spec = g_param_spec_string(
                "passwd",
                "Password Property",
                "The user's password.",
                "",     /* default value */
                G_PARAM_READABLE | G_PARAM_WRITABLE 
                );
    g_object_class_install_property(
                G_OBJECT_CLASS(klass),
                GQQ_CONFIG_PROPERTY_PASSWD,
                my_param_spec
                );
}

gint gqq_config_load(GQQConfig *cfg, GString *uin)
{
    if(cfg == NULL || uin == NULL){
        return -1;
    }
	if(!g_file_test(CONFIGDIR, G_FILE_TEST_EXISTS)){
		/*
		 * The program is first run. Create the configure dir.
		 */
		if(-1 == g_mkdir(CONFIGDIR, 0777)){
			g_error("Create config dir %s error!(%s, %d)"
                            , CONFIGDIR, __FILE__, __LINE__);
			return -1;
		}
	}

    gchar buf[500];
    g_snprintf(buf, 500, "%s/%s", CONFIGDIR, uin -> str);
	if(!g_file_test(buf, G_FILE_TEST_EXISTS)){
		/*
         * This is the first time that this user uses this program.
		 */
		if(-1 == g_mkdir(buf, 0777)){
			g_error("Create user config dir %s error!(%s, %d)"
                            , buf, __FILE__, __LINE__);
			return -1;
		}
        g_snprintf(buf, 500, "%s/%s/%s", CONFIGDIR, uin -> str, "faces");
		if(-1 == g_mkdir(buf, 0777)){
			g_error("Create dir %s error!(%s, %d)", buf, __FILE__, __LINE__);
			return -1;
		}
        // There is nothing to read. Just return.
        return 0;
	}
    


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


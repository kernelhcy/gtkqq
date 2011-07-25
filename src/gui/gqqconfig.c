#include <gqqconfig.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <qqtypes.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <stdlib.h>

//
// Private members
//
typedef struct{
    QQInfo *info;
    GHashTable *ht;

    GString *passwd;    //stored password
    GString *uin;       //current user's uin
    GString *lastuser;  //The last user's uin
}GQQConfigPriv;

//
// Preperty ID
//
enum{
    GQQ_CONFIG_PROPERTY_0,
    GQQ_CONFIG_PROPERTY_INFO,
    GQQ_CONFIG_PROPERTY_PASSWD,
    GQQ_CONFIG_PROPERTY_UIN,
    GQQ_CONFIG_PROPERTY_LASTUSER
};

static void gqq_config_init(GQQConfig *self);
static void gqq_config_class_init(GQQConfigClass *klass, gpointer data);

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
            }
            g_once_init_leave(&g_define_type_id__volatile, type_id);
    }
    return type_id;
}

GQQConfig* gqq_config_new(QQInfo *info)
{
    if(info == NULL){
        g_error("info == NULL (%s, %d)", __FILE__, __LINE__);
        return NULL;
    }

    GQQConfig *cfg = (GQQConfig*)g_object_new(GQQ_TYPE_CONFIG, "info", info);
    return cfg;
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
        
        g_debug("GQQConfig getter: %s (%s, %d)", pspec -> name, __FILE__, __LINE__); 
        GQQConfig *obj = G_TYPE_CHECK_INSTANCE_CAST(
                                        object, gqq_config_get_type(), GQQConfig);
        GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    obj, gqq_config_get_type(), GQQConfigPriv);
        
        switch (property_id)
        {
        case GQQ_CONFIG_PROPERTY_INFO:
            g_value_set_pointer(value, priv -> info);
            break;
        case GQQ_CONFIG_PROPERTY_PASSWD:
            g_value_set_static_string(value, priv -> passwd -> str);
            break;
        case GQQ_CONFIG_PROPERTY_UIN:
            g_value_set_static_string(value, priv -> uin -> str);
            break;
        case GQQ_CONFIG_PROPERTY_LASTUSER:
            g_value_set_static_string(value, priv -> lastuser -> str);
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
        g_debug("GQQConfig setter: %s (%s, %d)", pspec -> name, __FILE__, __LINE__); 
        GQQConfig *obj = G_TYPE_CHECK_INSTANCE_CAST(
                                        object, gqq_config_get_type(), GQQConfig);
        GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                    obj, gqq_config_get_type(), GQQConfigPriv);
        
        switch (property_id)
        {
        case GQQ_CONFIG_PROPERTY_INFO:
            priv -> info = g_value_get_pointer(value);
            break;
        case GQQ_CONFIG_PROPERTY_PASSWD:
            g_string_truncate(priv -> passwd, 0);
            g_string_append(priv -> passwd, g_value_get_string(value));
            break;
        case GQQ_CONFIG_PROPERTY_UIN:
            g_string_truncate(priv -> uin, 0);
            g_string_append(priv -> uin, g_value_get_string(value));
            break;
        case GQQ_CONFIG_PROPERTY_LASTUSER:
            g_string_truncate(priv -> lastuser, 0);
            g_string_append(priv -> lastuser, g_value_get_string(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
        }
}

#ifdef G_ENABLE_DEBUG
#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_variant(v)  g_value_get_variant (v)
#else 
#define g_marshal_value_peek_string(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_variant(v)  (v)->data[0].v_pointer
#endif

//
// Marshal for signal 
// The callback type is:
//      void (*handler)(gpointer instance, const gchar *key, const GVariant *value
//                      , gpointer user_data)
//
static void g_cclosure_user_marshal_VOID__STRING_VARIANT (GClosure     *closure,
                                              GValue       *return_value G_GNUC_UNUSED,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint G_GNUC_UNUSED,
                                              gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_VARIANT) (gpointer     data1,
                                                     gpointer     arg_1,
                                                     gpointer     arg_2,
                                                     gpointer     data2);
  register GMarshalFunc_VOID__STRING_VARIANT callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_VARIANT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_string (param_values + 1),
            g_marshal_value_peek_variant (param_values + 2),
            data2);
}

//
// Default handler of the 'gtkqq-config-changed' signal
// Just do nothing.
//
static void signal_default_handler(gpointer instance
                                    , const gchar *key
                                    , const GVariant *value
                                    , gpointer usr_data)
{
    //do nothing.
    return;
}
static void gqq_config_init(GQQConfig *self)
{
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(self
                                        , GQQ_TYPE_CONFIG, GQQConfigPriv);

    //
    // Store the configuration items.
    // The key is the item name, which is string.
    // The value is GVariant type.
    //
    priv -> ht = g_hash_table_new_full(g_str_hash, g_str_equal
                                , (GDestroyNotify)g_free
                                , (GDestroyNotify)g_variant_unref);
    priv -> passwd = g_string_new(NULL);
    priv -> lastuser = g_string_new(NULL);
    priv -> uin = g_string_new(NULL);
}

static void gqq_config_class_init(GQQConfigClass *klass, gpointer data)
{
    //add the private members.
    g_type_class_add_private(klass, sizeof(GQQConfigPriv));
    
    G_OBJECT_CLASS(klass)-> get_property = gqq_config_getter;
    G_OBJECT_CLASS(klass)-> set_property = gqq_config_setter;

    //install the 'gtkqq-config-changed' signal
    klass -> signal_default_handler = signal_default_handler;
    klass -> changed_signal_id = 
            g_signal_new("gtkqq-config-changed"
                , G_TYPE_FROM_CLASS(klass) 
                , G_SIGNAL_RUN_LAST     //run after the default handler
                , G_STRUCT_OFFSET(GQQConfigClass, signal_default_handler)
                , NULL, NULL            //no used
                , g_cclosure_user_marshal_VOID__STRING_VARIANT
                , G_TYPE_NONE
                , 2, G_TYPE_STRING, G_TYPE_VARIANT
                );
    //install the info property
    GParamSpec *pspec;
    pspec = g_param_spec_pointer("info"
                                , "QQInfo"
                                , "The pointer to the global instance of QQInfo"
                                , G_PARAM_READABLE | G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(klass)
                                    , GQQ_CONFIG_PROPERTY_INFO, pspec);

    //install the passwd property
    pspec = g_param_spec_string("passwd"
                                , "password"
                                , "The password of current user."
                                , ""
                                , G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(klass)
                                    , GQQ_CONFIG_PROPERTY_PASSWD, pspec);

    //install the uin property
    pspec = g_param_spec_string("uin"
                                , "uin"
                                , "The uin of current user."
                                , ""
                                , G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(klass)
                                    , GQQ_CONFIG_PROPERTY_UIN, pspec);

    //install the lastuser property
    pspec = g_param_spec_string("lastuser"
                                , "last user's uin"
                                , "The uin of the last user who uses this program."
                                , ""
                                , G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(klass)
                                    , GQQ_CONFIG_PROPERTY_LASTUSER, pspec);
}

gint gqq_config_load_last(GQQConfig *cfg)
{
    if(cfg == NULL){
        return -1;
    }
    if(!g_file_test(CONFIGDIR, G_FILE_TEST_EXISTS)){
        /*
         * The program is first run. Create the configure dir.
         */
        g_debug("Config mkdir "CONFIGDIR" (%s, %d)", __FILE__, __LINE__);
        if(-1 == g_mkdir(CONFIGDIR, 0777)){
            g_error("Create config dir %s error!(%s, %d)"
                            , CONFIGDIR, __FILE__, __LINE__);
            return -1;
        }
    }

    gchar buf[500];
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                cfg, gqq_config_get_type(), GQQConfigPriv);
    //read lastuser
    g_snprintf(buf, 500, "%s/lastuser", CONFIGDIR);
    g_debug("Config open %s (%s, %d)", buf, __FILE__, __LINE__);
    if(!g_file_test(buf, G_FILE_TEST_EXISTS)){
        //First run
        return 0; 
    }
    gint fd = g_open(buf, O_RDONLY);
    if(fd == -1){
        g_error("Open file %s error! %s (%s, %d)", buf, strerror(errno)
                                                , __FILE__, __LINE__);
        return -1;
    }
    gsize len = (gsize)read(fd, buf, 500);
    g_string_truncate(priv -> lastuser, 0);
    g_string_append_len(priv -> lastuser, buf, len);
    close(fd);

    g_string_truncate(priv -> uin, 0);
    g_string_append(priv -> uin, priv -> lastuser -> str);

    g_debug("Config: read lastuser. %s (%s, %d)", priv -> uin -> str
                                        , __FILE__, __LINE__);
    return gqq_config_load(cfg, priv -> lastuser);
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
    gsize len;
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
   
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                cfg, gqq_config_get_type(), GQQConfigPriv);

    //read config
    g_snprintf(buf, 500, "%s/%s/config", CONFIGDIR, uin -> str);
    gint fd = g_open(buf, O_RDONLY);
    if(fd == -1){
        g_error("Open file %s error! %s (%s, %d)", buf, strerror(errno)
                                                , __FILE__, __LINE__);
        return -1;
    }
    GString *confstr = g_string_new(NULL);
    while(TRUE){
        len = read(fd, buf, 500);
        if(len <= 0){
            break;
        }
        g_string_append_len(confstr, buf, len);
    }
    close(fd);
    gchar *key, *value, *eq, *nl;
    glong tmpl;
    eq = confstr -> str;
    while(TRUE){
        if(confstr -> len <=0){
            break;
        }
        key = eq;
        while(*eq != '\0' && *eq != '=') ++eq;
        if(*eq == '\0'){
            break;
        }
        nl = eq;
        while(*nl != '\0' && *nl != '\n') ++nl;
        if(*nl == '\n'){
            break;
        }
        value = eq + 1;
        *eq = '\0';
        *nl = '\0';
        switch(*key)
        {
        case 's':
            gqq_config_set_str(cfg, g_strdup(key + 2), value);
            break;
        case 'i':
            tmpl = strtol(value, NULL, 10);
            gqq_config_set_int(cfg, g_strdup(key + 2), tmpl);
            break;
        case 'b':
            if(g_strcmp0(value, "true") == 0){
                gqq_config_set_bool(cfg, g_strdup(key + 2), TRUE);
            }else if(g_strcmp0(value, "false") == 0){
                gqq_config_set_bool(cfg, g_strdup(key + 2), FALSE);
            }else{
                g_warning("Wrong value!! %s=%s (%s, %d)", key, value
                                    , __FILE__, __LINE__);
            }
            break;
        default:
            g_warning("Uknown configurations type!! %s=%s (%s, %d)", key, value
                                , __FILE__, __LINE__);
            break;
        }
        eq = nl + 1; 
    }
    g_debug("Config: read config %s.(%s, %d)", confstr -> str
                            , __FILE__, __LINE__);
    g_string_free(confstr, TRUE);

    //read .passwd
    g_snprintf(buf, 500, "%s/%s/.passwd", CONFIGDIR, uin -> str);
    fd = g_open(buf, O_RDONLY);
    if(fd == -1){
        g_error("Open file %s error! %s (%s, %d)", buf, strerror(errno)
                                                , __FILE__, __LINE__);
        return -1;
    }
    len = (gsize)read(fd, buf, 500);
    buf[len] = '\0';
    g_string_truncate(priv -> passwd, 0);
    guchar *depw = g_base64_decode(buf, &len);
    g_string_append_len(priv -> passwd, (const gchar *)depw, len);
    g_free(depw);
    close(fd);

    //read buddies
    
    //read groups
    
    //read categories
    return 0;
}

//
// restart when interupt by signal.
//
static gsize safe_write(gint fd, const gchar *buf, gsize len)
{
    if(fd < 0 || buf == NULL || len <=0){
        return 0;
    }

    gsize haswritten = 0;
    gsize re;
    while(haswritten < len){
        re = (gsize)write(fd, buf + haswritten, len - haswritten);
        if(re == -1){
            g_warning("Write to file error. %s (%s, %d)"
                    , strerror(errno), __FILE__, __LINE__);
            return -1;
        }
        haswritten += re;
    }
    return haswritten;
}

static void ht_foreach(gpointer key, gpointer value, gpointer usrdata)
{
    if(key == NULL || value == NULL || usrdata == NULL){
        return;
    }
    const gchar *kstr = (const gchar *)key;
    GVariant *vv = (GVariant*)value;
    GString *str = (GString*)usrdata;

    //
    // We add a prefix to the key name.
    // So, when we read from the configuration file, we know the type
    // of the value.
    //
    // The prefix is the same the its variant type and divided with the 
    // key name by a `_`
    //
    if(g_variant_is_of_type(vv, G_VARIANT_TYPE_STRING)){
        gsize len; 
        const gchar *vstr = g_variant_get_string(vv, &len);
        g_string_append(str, "s_%s="); 
        g_string_append_len(str, vstr, len);
    }else if(g_variant_is_of_type(vv, G_VARIANT_TYPE_INT32)){
        g_string_append_printf(str, "i_%s=%d\n", kstr
                            , (gint)g_variant_get_int32(vv));
    }else if(g_variant_is_of_type(vv, G_VARIANT_TYPE_BOOLEAN)){
        g_string_append_printf(str, "b_%s=%s\n", kstr
                            , g_variant_get_boolean(vv) ? "true" : "false");
    }else{
        g_warning("Unsupported variant type %s (%s, %d)", 
                        (gchar *)g_variant_get_type(vv), __FILE__, __LINE__);
        return;
    }

    return;
}
gint gqq_config_save(GQQConfig *cfg)
{
    if(cfg == NULL){
        return -1;
    }
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                cfg, gqq_config_get_type(), GQQConfigPriv);
    GHashTable *ht = priv -> ht;
    QQInfo *info = priv -> info;
    gchar buf[500];

    //write config
    g_snprintf(buf, 500, "%s/%s/config", CONFIGDIR, info -> me -> uin -> str);
    gint fd = g_open(buf, O_WRONLY | O_CREAT);
    if(fd == -1){
        g_error("Open file %s error! %s (%s, %d)", buf, strerror(errno)
                                                , __FILE__, __LINE__);
        return -1;
    }
    GString *configstr = g_string_new(NULL);
    g_hash_table_foreach(ht, ht_foreach, configstr);
    if(safe_write(fd, configstr -> str, configstr -> len)
                != configstr -> len){
        g_warning("Write to config error!!(%s, %d)", __FILE__, __LINE__);
    }
    g_string_free(configstr, TRUE);
    close(fd);

    //write .passwd 
    g_snprintf(buf, 500, "%s/%s/.passwd", CONFIGDIR, info -> me -> uin -> str);
    fd = g_open(buf, O_WRONLY | O_CREAT);
    if(fd == -1){
        g_error("Open file %s error! %s (%s, %d)", buf, strerror(errno)
                                                , __FILE__, __LINE__);
        return -1;
    }
    //Base64 encode
    gchar *b64pw = g_base64_encode((const guchar *)priv -> passwd -> str
                                    , priv -> passwd -> len);
    if(safe_write(fd, b64pw, (gsize)strlen(b64pw)) < 0){
        g_warning("Write to .passwd error!!(%s, %d)", __FILE__, __LINE__);
    }
    g_free(b64pw);
    close(fd);

    //write lastuser
    g_snprintf(buf, 500, "%s/lastuser", CONFIGDIR);
    fd = g_open(buf, O_WRONLY | O_CREAT);
    if(fd == -1){
        g_error("Open file %s error! %s (%s, %d)", buf, strerror(errno)
                                                , __FILE__, __LINE__);
        return -1;
    }
    if(safe_write(fd, priv -> lastuser -> str, priv -> lastuser -> len)
                            != priv -> lastuser -> len){
        g_warning("Write to lastuser error!!(%s, %d)", __FILE__, __LINE__);
    }
    close(fd);

    GString *tmp = NULL;
    guint i;
    //write buddies
    g_snprintf(buf, 500, "%s/%s/buddies", CONFIGDIR, info -> me -> uin -> str);
    fd = g_open(buf, O_WRONLY | O_CREAT);
    if(fd == -1){
        g_error("Open file %s error! %s (%s, %d)", buf, strerror(errno)
                                                , __FILE__, __LINE__);
        return -1;
    }
    QQBuddy *bdy = NULL;
    for(i = 0; i < info -> buddies -> len; ++i){
        bdy = (QQBuddy*)g_ptr_array_index(info -> buddies, i); 
        tmp = qq_buddy_tostring(bdy);
        safe_write(fd, tmp -> str, tmp -> len);
        g_string_free(tmp, TRUE);
    }
    close(fd);

    //write groups
    g_snprintf(buf, 500, "%s/%s/groups", CONFIGDIR, info -> me -> uin -> str);
    fd = g_open(buf, O_WRONLY | O_CREAT);
    if(fd == -1){
        g_error("Open file %s error! %s (%s, %d)", buf, strerror(errno)
                                                , __FILE__, __LINE__);
        return -1;
    }
    QQGroup *grp= NULL;
    for(i = 0; i < info -> groups -> len; ++i){
        grp = (QQGroup*)g_ptr_array_index(info -> groups, i); 
        tmp = qq_group_tostring(grp);
        safe_write(fd, tmp -> str, tmp -> len);
        g_string_free(tmp, TRUE);
    }
    close(fd);

    //write categories
    g_snprintf(buf, 500, "%s/%s/categories", CONFIGDIR, info -> me -> uin -> str);
    fd = g_open(buf, O_WRONLY | O_CREAT);
    if(fd == -1){
        g_error("Open file %s error! %s (%s, %d)", buf, strerror(errno)
                                                , __FILE__, __LINE__);
        return -1;
    }
    QQCategory *cate= NULL;
    for(i = 0; i < info -> categories -> len; ++i){
        cate = (QQCategory*)g_ptr_array_index(info -> categories, i); 
        tmp = qq_category_tostring(cate);
        safe_write(fd, tmp -> str, tmp -> len);
        g_string_free(tmp, TRUE);
    }
    close(fd);

    return 0;
}
gint gqq_config_get_str(GQQConfig *cfg, const gchar *key, const gchar **value)
{
    if(cfg == NULL || key == NULL || value == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                cfg, gqq_config_get_type(), GQQConfigPriv);
        
    GVariant *v = (GVariant*)g_hash_table_lookup(priv -> ht, key);
    if(!g_variant_is_of_type(v, G_VARIANT_TYPE_STRING)){
        g_warning("No key named %s with string value.(%s, %d)"
                    , key, __FILE__, __LINE__);
        return -1;
    }
    *value = g_variant_get_string(v, NULL);
    return 0;
}
gint gqq_config_get_int(GQQConfig *cfg, const gchar *key, gint *value)
{
    if(cfg == NULL || key == NULL || value == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                cfg, gqq_config_get_type(), GQQConfigPriv);
        
    GVariant *v = (GVariant*)g_hash_table_lookup(priv -> ht, key);
    if(!g_variant_is_of_type(v, G_VARIANT_TYPE_INT32)){
        g_warning("No key named %s with gint value.(%s, %d)"
                    , key, __FILE__, __LINE__);
        return -1;
    }
    gint32 vi = g_variant_get_int32(v);
    *value = (gint)vi;
    return 0;

}
gint gqq_config_get_bool(GQQConfig *cfg, const gchar *key, gboolean *value)
{
    if(cfg == NULL || key == NULL || value == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                cfg, gqq_config_get_type(), GQQConfigPriv);
        
    GVariant *v = (GVariant*)g_hash_table_lookup(priv -> ht, key);
    if(!g_variant_is_of_type(v, G_VARIANT_TYPE_BOOLEAN)){
        g_warning("No key named %s with gboolean value.(%s, %d)"
                    , key, __FILE__, __LINE__);
        return -1;
    }
    *value = g_variant_get_boolean(v);
    return 0;

}
gint gqq_config_set_str(GQQConfig *cfg, const gchar *key, const gchar *value)
{
    if(cfg == NULL || key == NULL || value == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                cfg, gqq_config_get_type(), GQQConfigPriv);
        
    GVariant *v = g_variant_new_string(value);
    g_hash_table_replace(priv -> ht, g_strdup(key), v);
    g_signal_emit_by_name(cfg, "gtkqq-config-changed", key, v);
    return 0;
}
gint gqq_config_set_int(GQQConfig *cfg, const gchar *key, gint value)
{
    if(cfg == NULL || key == NULL){
        return -1;
    }
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                cfg, gqq_config_get_type(), GQQConfigPriv);
        
    GVariant *v = g_variant_new_int32((gint32)value);
    g_hash_table_replace(priv -> ht, g_strdup(key), v);
    g_signal_emit_by_name(cfg, "gtkqq-config-changed", key, v);
    return 0;

}
gint gqq_config_set_bool(GQQConfig *cfg, const gchar *key, gboolean value)
{
    if(cfg == NULL || key == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
                                cfg, gqq_config_get_type(), GQQConfigPriv);
        
    GVariant *v = g_variant_new_boolean(value);
    g_hash_table_replace(priv -> ht, g_strdup(key), v);
    g_signal_emit_by_name(cfg, "gtkqq-config-changed", key, v);
    return 0;
}


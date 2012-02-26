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
#include <dao.h>

//
// Singleton
//
static GQQConfig *singleton = NULL;

static GObjectClass *parent_class = NULL;

//
// Private members
//
typedef struct{
    QQInfo *info;
    GHashTable *ht;

    GString *passwd;    //stored password
    GString *qqnum;     //current user's qq number
    GString *status;    //current user's status
	gint rempw;		//whether remember password
	gint mute;		//whether mute

    sqlite3 *db_con;    //database connection
    
    // the hash table of the string key hash table
    GHashTable *ht_ht;
    // lock of previous hash table
#if GLIB_CHECK_VERSION(2,32,0)
    GMutex  ht_lock_impl;
#endif
    GMutex *ht_lock;
}GQQConfigPriv;

//
// Preperty ID
//
enum{
    GQQ_CONFIG_PROPERTY_0,
    GQQ_CONFIG_PROPERTY_INFO,
    GQQ_CONFIG_PROPERTY_PASSWD,
    GQQ_CONFIG_PROPERTY_UIN,
    GQQ_CONFIG_PROPERTY_STATUS,
	GQQ_CONFIG_PROPERTY_REMPW,
	GQQ_CONFIG_PROPERTY_MUTE,
};

static void gqq_config_init(GQQConfig *self);
static void gqq_config_class_init(GQQConfigClass *klass, gpointer data);
static GObject *gqq_config_contructor(GType type, guint n_pars
                                      , GObjectConstructParam *pars);
static void gqq_config_finalize(GObject *obj);

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

    GQQConfig *cfg = (GQQConfig*)g_object_new(GQQ_TYPE_CONFIG, "info", info, NULL);
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
        case GQQ_CONFIG_PROPERTY_STATUS:
            g_value_set_static_string(value, priv -> status -> str);
            break;
        case GQQ_CONFIG_PROPERTY_UIN:
            g_value_set_static_string(value, priv -> qqnum -> str);
            break;
		case GQQ_CONFIG_PROPERTY_REMPW:
			g_value_set_int(value, priv->rempw);
            break;
		case GQQ_CONFIG_PROPERTY_MUTE:
			g_value_set_int(value, priv->mute);
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
    GQQConfig *obj = GQQ_CONFIG(object);
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
        case GQQ_CONFIG_PROPERTY_STATUS:
            g_string_truncate(priv -> status, 0);
            g_string_append(priv -> status, g_value_get_string(value));
            break;
        case GQQ_CONFIG_PROPERTY_UIN:
            g_string_truncate(priv -> qqnum , 0);
            g_string_append(priv -> qqnum , g_value_get_string(value));
            break;
		case GQQ_CONFIG_PROPERTY_REMPW:
			priv -> rempw = g_value_get_int(value);
            break;
		case GQQ_CONFIG_PROPERTY_MUTE:
			priv -> mute = g_value_get_int(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
} 

#ifdef G_ENABLE_DEBUG
#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#else 
#define g_marshal_value_peek_string(v)   (v)->data[0].v_pointer
#endif

//
// Marshal for signal 
// The callback type is:
//      void (*handler)(gpointer instance, const gchar *key, const gchar *value
//                      , gpointer user_data)
//
static void g_cclosure_user_marshal_VOID__STRING_STRING (GClosure     *closure,
                                                         GValue       *return_value G_GNUC_UNUSED,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint G_GNUC_UNUSED,
                                                         gpointer      marshal_data)
{
    typedef void (*GMarshalFunc_VOID__STRING_STRING)(gpointer     data1,
                                                     gpointer     arg_1,
                                                     gpointer     arg_2,
                                                     gpointer     data2);
    register GMarshalFunc_VOID__STRING_STRING callback;
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
    callback = (GMarshalFunc_VOID__STRING_STRING)(marshal_data ? marshal_data : cc->callback);

    callback (data1,
              g_marshal_value_peek_string(param_values + 1),
              g_marshal_value_peek_string(param_values + 2),
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
    GString *cfgdir = g_string_new (g_get_home_dir ());
    g_string_append (cfgdir, "/.gtkqq");
    if(!g_file_test(cfgdir->str, G_FILE_TEST_IS_DIR | G_FILE_TEST_EXISTS)
                    && -1 == g_mkdir(cfgdir->str, 0777)){
        g_error("Create config dir %s error!(%s, %d)"
                , cfgdir->str, __FILE__, __LINE__);
    }
    
    GString *facedir = g_string_new (cfgdir->str);
    g_string_free (cfgdir, FALSE);
    
    g_string_append (facedir, "/faces");
    if(!g_file_test (facedir->str, G_FILE_TEST_EXISTS) 
       && -1 == g_mkdir(facedir->str, 0777)){/* FIXME!!! 0777 is not safe enough! */
        g_error("Create config dir %s error!(%s, %d)"
                , facedir->str, __FILE__, __LINE__);
    }
    
    g_string_free (facedir, TRUE);
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(self
                                                      , GQQ_TYPE_CONFIG, GQQConfigPriv);

    //
    // Store the configuration items.
    // The key is the item name, which is string.
    // The value is GVariant type.
    //
    priv -> ht = g_hash_table_new_full(g_str_hash, g_str_equal
                                       , (GDestroyNotify)g_free
                                       , (GDestroyNotify)g_free);
    priv -> passwd = g_string_new(NULL);
    priv -> qqnum = g_string_new(NULL);
    priv -> status = g_string_new(NULL);
	priv -> rempw = 0;
	priv -> mute = 0;
    priv -> db_con = db_open();
    priv -> ht_ht = g_hash_table_new_full(g_str_hash, g_str_equal
                                          , g_free, NULL);
#if GLIB_CHECK_VERSION(2,32,0)
    g_mutex_init (&priv -> ht_lock_impl);
    priv -> ht_lock = &priv -> ht_lock_impl;
#else
    priv -> ht_lock = g_mutex_new();
#endif

 
}

static void gqq_config_class_init(GQQConfigClass *klass, gpointer data)
{
    //add the private members.
    g_type_class_add_private(klass, sizeof(GQQConfigPriv));

    parent_class = g_type_class_peek_parent(klass); 

    G_OBJECT_CLASS(klass)-> get_property = gqq_config_getter;
    G_OBJECT_CLASS(klass)-> set_property = gqq_config_setter;
    G_OBJECT_CLASS(klass)-> constructor = gqq_config_contructor;
    G_OBJECT_CLASS(klass)-> finalize = gqq_config_finalize;

    //install the 'gtkqq-config-changed' signal
    klass -> signal_default_handler = signal_default_handler;
    klass -> changed_signal_id = 
        g_signal_new("gtkqq-config-changed"
                     , G_TYPE_FROM_CLASS(klass) 
                     , G_SIGNAL_RUN_LAST     //run after the default handler
                     , G_STRUCT_OFFSET(GQQConfigClass, signal_default_handler)
                     , NULL, NULL            //no used
                     , g_cclosure_user_marshal_VOID__STRING_STRING
                     , G_TYPE_NONE
                     , 2, G_TYPE_STRING, G_TYPE_STRING
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

    //install the qq number property
    pspec = g_param_spec_string("qqnum"
                                , "qq number"
                                , "The qq number of current user."
                                , ""
                                , G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(klass)
                                    , GQQ_CONFIG_PROPERTY_UIN, pspec);

    //install the status property
    pspec = g_param_spec_string("status"
                                , "status"
                                , "The status of current user."
                                , ""
                                , G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(klass)
                                    , GQQ_CONFIG_PROPERTY_STATUS, pspec);

	//install the rempw property
    pspec = g_param_spec_int("rempw"
                                , "rempw"
                                , "Whather remember the password."
                                , 0
							 	, 1
							 	, 0
                                , G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(klass)
                                    , GQQ_CONFIG_PROPERTY_REMPW, pspec);

	//install the mute property
    pspec = g_param_spec_int("mute"
                                , "mute"
                                , "Whather mute."
                                , 0
							 	, 1
							 	, 0
                                , G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property(G_OBJECT_CLASS(klass)
                                    , GQQ_CONFIG_PROPERTY_MUTE, pspec);

}

//
// Singletion
// Only one instance of QQConfig will exist.
//
static GObject *gqq_config_contructor(GType type, guint n_pars
                                      , GObjectConstructParam *pars)
{
    GObject *obj;
    if(!singleton){
        obj = G_OBJECT_CLASS(parent_class) -> constructor(type, n_pars, pars);
        singleton = GQQ_CONFIG(obj);
    }else{
        obj = g_object_ref(G_OBJECT(singleton));
    }
    return obj;
}

//
// Last called of g_object_unref will call this function to 
// close the database connection.
//
static void gqq_config_finalize(GObject *obj)
{
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        obj, gqq_config_get_type(), GQQConfigPriv);
    db_close(priv -> db_con);

    g_hash_table_unref(priv -> ht_ht);
#if GLIB_CHECK_VERSION(2,32,0)
    g_mutex_clear(&priv -> ht_lock_impl);
#else
    g_mutex_free(priv -> ht_lock);
#endif
    // chain up
    GObjectClass *klass = (GObjectClass*)g_type_class_peek_parent(
        g_type_class_peek(gqq_config_get_type()));
    klass -> finalize(obj);
}

gint gqq_config_load(GQQConfig *cfg, const gchar *qqnum)
{
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
    QQInfo *info = priv -> info; 

    // get my info from the db
    qq_buddy_set(info -> me, "uin", qqnum);
    qq_buddy_set(info -> me, "qqnumber", qqnum);
    db_get_buddy(priv -> db_con, priv -> qqnum -> str, info -> me, NULL);
    return 0;
}

static void save_ht(gpointer key, gpointer value, gpointer usr_data)
{
    gchar *k = (gchar *)key;
    gchar *v = (gchar *)value;
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        usr_data, gqq_config_get_type(), GQQConfigPriv);
    db_config_save(priv -> db_con, priv -> qqnum -> str, k, v);
}

gint gqq_config_save(GQQConfig *cfg)
{
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
    //Save configuration items
    g_hash_table_foreach(priv -> ht, save_ht, cfg);
    return 0;
}

gint gqq_config_save_last_login_user(GQQConfig *cfg)
{
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
    if(priv -> qqnum -> len > 0){
        db_update_all(priv -> db_con, "qquser", "last", "0");
        db_qquser_save(priv -> db_con, priv -> qqnum -> str, priv -> passwd -> str
                       , priv -> status -> str, 1, priv->rempw, priv->mute); 
    }
    return 0;
}

gint gqq_config_get_str(GQQConfig *cfg, const gchar *key, const gchar **value)
{
    if(cfg == NULL || key == NULL || value == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
        
    const gchar *v = (const gchar *)g_hash_table_lookup(priv -> ht, key);
    *value = v;
    return 0;
}
gint gqq_config_get_int(GQQConfig *cfg, const gchar *key, gint *value)
{
    if(cfg == NULL || key == NULL || value == NULL){
        return -1;
    }
    const gchar *vstr = NULL;
    if(gqq_config_get_str(cfg, key, &vstr) == -1 || vstr == NULL){
        g_debug("gtk qq configuration has no value for key %s...(%s,%d)",\
                key,__FILE__, __LINE__);
        return -1;
    }
    gchar *end;    
    glong vint = strtol(vstr, &end, 10);
    //gint vint = atoi(vstr);
    if(vstr == end){
        return -1;
    }
    *value = (gint)vint;
    return 0;

}
gint gqq_config_get_bool(GQQConfig *cfg, const gchar *key, gboolean *value)
{
    if(cfg == NULL || key == NULL || value == NULL){
        return -1;
    }

    const gchar *vstr = NULL;
    if(gqq_config_get_str(cfg, key, &vstr) == -1){
        return -1;
    }
    if(g_ascii_strcasecmp(vstr, "true") == 0){
        *value = TRUE;
    }else if(g_ascii_strcasecmp(vstr, "false") == 0){
        *value = FALSE;
    }
    return 0;

}
gint gqq_config_set_str(GQQConfig *cfg, const gchar *key, const gchar *value)
{
    if(cfg == NULL || key == NULL || value == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
        
    g_hash_table_replace(priv -> ht, g_strdup(key), g_strdup(value));
    g_signal_emit_by_name(cfg, "gtkqq-config-changed", key, value);
    return 0;
}
gint gqq_config_set_int(GQQConfig *cfg, const gchar *key, gint value)
{
    if(cfg == NULL || key == NULL){
        return -1;
    }
    gchar buf[100];
    g_snprintf(buf, 100, "%d", value);
    return gqq_config_set_str(cfg, key, buf);

}
gint gqq_config_set_bool(GQQConfig *cfg, const gchar *key, gboolean value)
{
    if(cfg == NULL || key == NULL){
        return -1;
    }
    if(value){
        return gqq_config_set_str(cfg, key, "true");
    }else{
        return gqq_config_set_str(cfg, key, "false");
    }
}

GPtrArray* gqq_config_get_all_login_user(GQQConfig *cfg)
{
    if(cfg == NULL){
        return NULL;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
        
    GPtrArray *result = NULL;
    gint recode = db_get_all_users(priv -> db_con, &result);
    if(recode != SQLITE_OK){
        return NULL;
    }
    return result;
}


GHashTable* gqq_config_create_str_hash_table(GQQConfig *cfg, const gchar *name)
{
    if(cfg == NULL || name == NULL){
        return NULL;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);

    g_mutex_lock(priv -> ht_lock);
    GHashTable *ht = g_hash_table_lookup(priv -> ht_ht, name);
    if(ht != NULL){
        g_warning("There already be one hash map named %s (%s, %d)", name
                  , __FILE__, __LINE__);
        g_mutex_unlock(priv -> ht_lock);
        return ht;
    }

    ht = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    g_hash_table_insert(priv -> ht_ht, g_strdup(name), ht);
    g_mutex_unlock(priv -> ht_lock);
    return ht;
}

gint gqq_config_delete_ht_ht(GQQConfig *cfg, const gchar *name)
{
    if(cfg == NULL || name == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);

    g_mutex_lock(priv -> ht_lock);
    GHashTable *ht = g_hash_table_lookup(priv -> ht_ht, name);
    if(ht == NULL){
        g_warning("No hash map named %s (%s, %d)", name, __FILE__, __LINE__);
        g_mutex_unlock(priv -> ht_lock);
        return -1;
    }
    g_hash_table_remove(priv -> ht_ht, name);
    g_hash_table_remove_all(ht);
    g_hash_table_unref(ht);
    g_mutex_unlock(priv -> ht_lock);
    return 0;
}
gpointer gqq_config_lookup_ht(GQQConfig *cfg, const gchar *name
                              , const gchar *key)
{
    if(cfg == NULL || name == NULL){
        return NULL;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);

    g_mutex_lock(priv -> ht_lock);
    GHashTable *ht = g_hash_table_lookup(priv -> ht_ht, name);
    if(ht == NULL){
        g_warning("No hash map named %s (%s, %d)", name, __FILE__, __LINE__);
        g_mutex_unlock(priv -> ht_lock);
        return NULL;
    }
    gpointer re = g_hash_table_lookup(ht, key);
    g_mutex_unlock(priv -> ht_lock);
    return re;
}
gpointer gqq_config_remove_ht(GQQConfig *cfg, const gchar *name
                              , const gchar *key)
{
    if(cfg == NULL || name == NULL){
        return NULL;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);

    g_mutex_lock(priv -> ht_lock);
    GHashTable *ht = g_hash_table_lookup(priv -> ht_ht, name);
    if(ht == NULL){
        g_warning("No hash map named %s (%s, %d)", name, __FILE__, __LINE__);
        g_mutex_unlock(priv -> ht_lock);
        return NULL;
    }
    gpointer data = g_hash_table_lookup(ht, key); 
    g_hash_table_remove(ht, key);
    g_mutex_unlock(priv -> ht_lock);
    return data;
}

gint gqq_config_clear_ht(GQQConfig *cfg, const gchar *name)
{
    if(cfg == NULL || name == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);

    g_mutex_lock(priv -> ht_lock);
    GHashTable *ht = g_hash_table_lookup(priv -> ht_ht, name);
    if(ht == NULL){
        g_warning("No hash map named %s (%s, %d)", name, __FILE__, __LINE__);
        g_mutex_unlock(priv -> ht_lock);
        return -1;
    }
    g_hash_table_remove_all(ht);
    g_mutex_unlock(priv -> ht_lock);
    return 0;
}

gint gqq_config_insert_ht(GQQConfig *cfg, const gchar *name
                          , gchar *key
                          , gpointer value)
{
    if(cfg == NULL || name == NULL){
        return -1;
    }

    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);

    g_mutex_lock(priv -> ht_lock);
    GHashTable *ht = g_hash_table_lookup(priv -> ht_ht, name);
    if(ht == NULL){
        g_warning("No hash map named %s (%s, %d)", name, __FILE__, __LINE__);
        g_mutex_unlock(priv -> ht_lock);
        return -1;
    }
    g_hash_table_insert(ht, g_strdup(key), value);
    g_mutex_unlock(priv -> ht_lock);
    return 0;
}

gint gqq_config_get_buddy(GQQConfig *cfg, QQBuddy *bdy)
{
    if(cfg == NULL || bdy == NULL){
        return -1;
    }
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
    gint cnt = 0;
    if(db_get_buddy(priv -> db_con, priv -> qqnum -> str, bdy, &cnt) 
       != SQLITE_ERROR){
        return cnt;
    }
    return -1;
}

gint gqq_config_get_group(GQQConfig *cfg, QQGroup *grp)
{
    if(cfg == NULL || grp == NULL){
        return -1;
    }
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
    gint cnt = 0;
    if(db_get_group(priv -> db_con, priv -> qqnum -> str, grp, &cnt) 
       != SQLITE_ERROR){
        return cnt;
    }
    return -1;

}

gint gqq_config_save_buddy(GQQConfig *cfg, QQBuddy *bdy)
{
    if(cfg == NULL || bdy == NULL){
        return -1;
    }
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
    if(db_buddy_save(priv -> db_con, priv -> qqnum -> str, bdy) == SQLITE_OK){
        return 0;
    }

    return -1;
}

gint gqq_config_save_group(GQQConfig *cfg, QQGroup *grp)
{
    if(cfg == NULL || grp == NULL){
        return -1;
    }
    GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
    if(db_group_save(priv -> db_con, priv -> qqnum -> str, grp) == SQLITE_OK){
        return 0;
    }

    return -1;
}

/** 
 * Get user's config directory.
 * 
 * 
 * @return 
 */
gchar *gqq_config_get_cfgdir()
{
	static GString *cfgdir = NULL;

	if (!cfgdir){
		cfgdir = g_string_new(g_get_home_dir());
		g_string_append(cfgdir, "/.gtkqq");
	}

	return cfgdir->str;
}

/** 
 * Get user' face directory.
 * 
 * 
 * @return 
 */
gchar *gqq_config_get_facedir()
{
	static GString *facedir = NULL;

	if (!facedir){
		facedir = g_string_new(gqq_config_get_cfgdir());
		g_string_append(facedir, "/faces");
	}

	return facedir->str;
}

/** 
 * Whether mute.
 * 
 * @param cfg 
 * 
 * @return TRUE if mute, else FALSE.
 */
gint gqq_config_is_mute(GQQConfig *cfg)
{
	gint mute = 0;

	g_object_get(cfg, "mute", &mute, NULL);
	return mute;
}

/** 
 * Set the mute status.
 * 
 * @param cfg 
 * @param mute TRUE if we want to mute else FALSE.
 */
void gqq_config_set_mute(GQQConfig *cfg, gint mute)
{
	const gchar *str = NULL;
	
	if (mute) {
		str = "1";
	} else {
		str = "0";
	}
	
	GQQConfigPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        cfg, gqq_config_get_type(), GQQConfigPriv);
	
    if (priv -> qqnum -> len > 0) {
		db_update_user(priv->db_con, priv->qqnum->str, "mute", str);
		g_object_set(cfg, "mute", mute, NULL);
    }
}

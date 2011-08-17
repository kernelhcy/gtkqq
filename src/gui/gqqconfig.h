#ifndef __GTKQQ_CONFIG_H
#define __GTKQQ_CONFIG_H
#include <glib.h>
#include <qq.h>
#include <glib-object.h>

//
// Configuration
// The configuration directory is $HOME/.gtkqq.
// See doc/table.sql for details
// 

typedef struct __GQQLoginUser{
    gchar qqnumber[100];
    gchar passwd[100];
    gchar status[100];
    gint last;
}GQQLoginUser;

#define GQQ_TYPE_CONFIG             (gqq_config_get_type())
#define GQQ_CONFIG(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj)\
                                            , GQQ_TYPE_CONFIG\
                                            , GQQConfig))
#define GQQ_IS_CONFIG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj\
                                            , GQQ_TYPE_CONFIG)))
#define GQQ_CONFIG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass)\
                                            , GQQ_TYPE_CONFIG\
                                            , GQQConfigClass))
#define GQQ_IS_CONFIG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass)\
                                            , GQQ_TYPE_CONFIG))
#define GQQ_CONFIG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj)\
                                            , GQQ_TYPE_CONFIG\
                                            , GQQConfigClass))

typedef struct __GQQConfig          GQQConfig;
typedef struct __GQQConfigClass     GQQConfigClass;

struct __GQQConfig{
    GObject parent_instance;
};

struct __GQQConfigClass{
    GObjectClass parent_class;

    guint changed_signal_id;
    //
    // The default handler for 'gtkqq-config-changed' signal.
    //
    void (*signal_default_handler)(gpointer instance
                            , const gchar *key
                            , const GVariant *value
                            , gpointer usr_data);
};

GQQConfig* gqq_config_new(QQInfo *info);
GType gqq_config_get_type();
//
// Load the configuration of qq number  
//
gint gqq_config_load(GQQConfig *cfg, const gchar *qqnum);
//
// Save all the configuration into the configuration files.
// Contains:
//      QQGroup
//      QQBuddy
//      QQCategory
//      other config.
//
gint gqq_config_save(GQQConfig *cfg);

//
// Get all the users who has logined.
//
GPtrArray* gqq_config_get_all_login_user(GQQConfig *cfg);
//
// Save the last login user
//
gint gqq_config_save_last_login_user(GQQConfig *cfg);

//
// Get and set configurations.
// For getter, if no key found, return -1, else return 0.
// 

// Do NOT free *value!
gint gqq_config_get_str(GQQConfig *cfg, const gchar *key, const gchar **value);
gint gqq_config_get_int(GQQConfig *cfg, const gchar *key, gint *value);
gint gqq_config_get_bool(GQQConfig *cfg, const gchar *key, gboolean *value);
//
// If successed, return 0. If failed, return -1.
// If the key already exists, set the value to the new value.
//
gint gqq_config_set_str(GQQConfig *cfg, const gchar *key, const gchar *value);
gint gqq_config_set_int(GQQConfig *cfg, const gchar *key, gint value);
gint gqq_config_set_bool(GQQConfig *cfg, const gchar *key, gboolean value);

//
// The string key hash table.
// NOTE:
//      These functions are thread-safe.
//

//
// Create a hash table named `name`.
//
GHashTable* gqq_config_create_str_hash_table(GQQConfig *cfg, const gchar *name);

// Delete the hash table named `name`
gint gqq_config_delete_ht_ht(GQQConfig *cfg, const gchar *name);
//
// Operate the hash table named `name`
//
gpointer gqq_config_lookup_ht(GQQConfig *cfg, const gchar *name
                                            , const gchar *key);
gpointer gqq_config_remove_ht(GQQConfig *cfg, const gchar *name
                                            , const gchar *key);
gint gqq_config_clear_ht(GQQConfig *cfg, const gchar *name);
gint gqq_config_insert_ht(GQQConfig *cfg, const gchar *name
                                        , gchar *key
                                        , gpointer value);
#endif

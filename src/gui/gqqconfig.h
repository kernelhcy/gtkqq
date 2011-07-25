#ifndef __GTKQQ_CONFIG_H
#define __GTKQQ_CONFIG_H
#include <glib.h>
#include <qq.h>
#include <glib-object.h>

//
// Configuration
// The configuration directory is $HOME/.gtkqq.
// 
// In the config dir, there is one directory for each user. The name
// of these dirs are the uins of the users.
// The user config dir contains:
//      config      --- a file which contains the configuration of this user.
//      faces       --- a dir which contains all the face images of this user.
//      .passwd     --- a file which contains the password of this user. The
//                      password is encrypted.
//      buddies     --- a file contains the information of all buddies
//      groups      --- a file contains the information of all groups.
//      categories  --- a file contains the information of all categories.
//

#define GQQ_TYPE_CONFIG             (gqq_config_get_type())
#define GQQ_CONFIG(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj)\
                                            , GQQ_TYPE_CONFIG\
                                            , GQQConfig))
#define GQQ_IS_CONFIG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj\
                                            , GQQ_IS_CONFIG)))
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
// Load the configuration of uin
//
gint gqq_config_load(GQQConfig *cfg, GString *uin);
//
// Load the configuration fo the last user.
//
gint gqq_config_load_last(GQQConfig *cfg);
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
// Get and set configurations.
// All these configurations will be stored in the 
//  $HOME/.gtkqq/`user_uin`/config
//
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

#endif

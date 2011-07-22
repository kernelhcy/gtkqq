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
//      config  --- a file which contains the configuration of this user.
//      faces   --- a dir which contains all the face images of this user.
//      .passwd --- a file which contains the password of this user. The
//                  password is encrypted.
//      buddies --- a file contains the information of all buddies
//      groups  --- a file contains the information of all groups.
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
    GObject parent_class;
};

GType gqq_config_get_type();
//
// Load the configuration of uin
//
gint gqq_config_load(GQQConfig *cfg, GString *uin);
#endif

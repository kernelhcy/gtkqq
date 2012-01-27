#ifndef __QQ_SETTING_DIALOG_H_
#define __QQ_SETTING_DIALOG_H_
#include <gtk/gtk.h>
#include <qq.h>


#define QQ_SETTING_DIALOG(obj)	    G_TYPE_CHECK_INSTANCE_CAST(obj\
                                                , qq_setting_dialog_get_type()\
						                        , QQSettingDialog)
#define QQ_SETTING_DIALOG_CLASS(c)	G_TYPE_CHECK_CLASS_CAST(c\
						                        , qq_setting_dialog_get_type()\
						                        , QQSettingDialogClass)
#define QQ_IS_SETTING_DIALOG(obj)	G_TYPE_CHECK_INSTANCE_TYPE(obj\
                                                , qq_setting_dialog_get_type())

typedef struct __QQSettingDialog 		QQSettingDialog;
typedef struct __QQSettingDialogClass	QQSettingDialogClass;

struct __QQSettingDialog{
    GtkDialog parent;
};

struct __QQSettingDialogClass{
    GtkDialogClass parent_class;
};

GtkWidget* qq_setting_dialog_new();
#endif

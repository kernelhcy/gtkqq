#include <gqqconfig.h>
#include <stdlib.h>
#include "settingdialog.h"

extern QQInfo *info;
extern GQQConfig *cfg;
static void  qq_setting_dialog_init(QQSettingDialog* settingdialog);
static void qq_setting_dialog_class_init(QQSettingDialogClass* klass,
                                         gpointer user_data);

/** 
 *Get the type of QQSettingDialog
 */
GType qq_setting_dialog_get_type(){
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQSettingDialogClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc)qq_setting_dialog_class_init,
            NULL,       /* class finalize*/
            NULL,       /* class data */
            sizeof(QQSettingDialog),
            0,          /* n pre allocs */
            (GInstanceInitFunc)qq_setting_dialog_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_DIALOG, "QQSettingDialog"
                    , &info, 0);
    }
    return t;
}

/**
 * Create a new setting dialog 
 */
GtkWidget* qq_setting_dialog_new(){
    return GTK_WIDGET(g_object_new(qq_setting_dialog_get_type(), NULL));
}
static void  qq_setting_dialog_init(QQSettingDialog* settingdialog){
}
static void qq_setting_dialog_class_init(QQSettingDialogClass* klass,
                                         gpointer user_data)
{

}

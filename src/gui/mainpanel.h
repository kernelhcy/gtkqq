#ifndef __GTKQQ_MAINPANEL_H
#define __GTKQQ_MAINPANEL_H
#include <gtk/gtk.h>

#define QQ_MAINPANEL(obj)       GTK_CHECK_CAST(obj, qq_mainpanel_get_type()\
                                            , QQMainPanel)
#define QQ_MAINPANELCLASS(c)    GTK_CHECK_CLASS_CAST(c\
                                            , qq_mainpanel_get_type()\
                                            , QQMainPanelClass)
#define QQ_IS_MAINPANEL(obj)    GTK_CHECK_TYPE(obj, qq_mainpanel_get_type())

typedef struct _QQMainPanel         QQMainPanel;
typedef struct _QQMainPanelClass    QQMainPanelClass;

struct _QQMainPanel{
    GtkVBox parent;

    /*< private >*/
    GtkWidget *faceimgframe;         //a frame contains gtk image
    GtkWidget *faceeventbox;
    GdkPixbuf *facepixbuf;

    GtkWidget *status_btn;
    GtkWidget *nick;
    
    GtkWidget *longnick, *longnick_entry;
    GtkWidget *longnick_box, *longnick_eventbox;

    GtkWidget *search_entry;

    GtkWidget *contact_btn, *grp_btn, *recent_btn;
    
    GtkWidget *notebook;
    GtkWidget *buddy_tree;
    GtkWidget *group_list;
    GtkWidget *recent_list;

    GtkWidget *menubar;

    GtkWidget *container;
};

struct _QQMainPanelClass{
    GtkVBoxClass parent;

    /*< private >*/
    GtkWidget *contact_img[2];
    GtkWidget *grp_img[2];
    GtkWidget *recent_img[2];

    GdkCursor *hand;
};

GType qq_mainpanel_get_type();
GtkWidget* qq_mainpanel_new(GtkWidget *container);

//
//Update the information of the panel.
//NOTE:
//      This function will create a new model for the tree.
//
void qq_mainpanel_update(QQMainPanel *panel);
//
// Update my information
//
void qq_mainpanel_update_my_info(QQMainPanel *panel);
//
// Update the buddy info
//
void qq_mainpanel_update_buddy_info(QQMainPanel *panel);
void qq_mainpanel_update_buddy_faceimg(QQMainPanel *panel);
void qq_mainpanel_update_online_buddies(QQMainPanel *panel);
#endif

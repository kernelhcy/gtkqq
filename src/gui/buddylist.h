#ifndef __GTKQQ_BUDDY_LIST_H_
#define __GTKQQ_BUDDY_LIST_H_
#include <gtk/gtk.h>
#include <qq.h>
//
// Inherit from GtkTreeView
// Used to show the groups and recent contancts
//

#define QQ_TYPE_BUDDY_LIST              (qq_buddy_list_get_type())
#define QQ_BUDDY_LIST(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj)\
                                                    , QQ_TYPE_BUDDY_LIST\
                                                    , QQBuddyList))
#define QQ_IS_BUDDY_LIST(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj\
                                                    , QQ_TYPE_BUDDY_LIST)))
#define QQ_BUDDY_LIST_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass)\
                                                    , QQ_TYPE_BUDDY_LIST\
                                                    , QQBuddyListClass))
#define QQ_IS_BUDDY_LIST_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass)\
                                                    , QQ_TYPE_BUDDY_LIST))
#define QQ_BUDDY_LIST_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj)\
                                                    , QQ_TYPE_BUDDY_LIST\
                                                    , QQBuddyListClass))

typedef struct __QQBuddyList            QQBuddyList;
typedef struct __QQBuddyListClass       QQBuddyListClass;

struct __QQBuddyList{
    GtkTreeView parent;
};

struct __QQBuddyListClass{
    GtkTreeViewClass parent;
};

GType qq_buddy_list_get_type();
GtkWidget * qq_buddy_list_new();

//
// Add qq buddy and qq group
//
void qq_buddy_list_add_buddy(GtkWidget *widget, QQBuddy *bdy);
void qq_buddy_list_add_group(GtkWidget *widget, QQGroup *grp);

//
// Add multiple buddies and groups
//
void qq_buddy_list_add_buddies(GtkWidget *widget, GPtrArray *bdies);
void qq_buddy_list_add_groups(GtkWidget *widget, GPtrArray *grps);

//
// Update the information of the qq buddy and qq group
//
void qq_buddy_list_update_buddy_info(GtkWidget *widget, QQBuddy *bdy);
void qq_buddy_list_update_group_info(GtkWidget *widget, QQGroup *grp);

//
// Update buddies and groups info
//
void qq_buddy_list_update_buddies_info(GtkWidget *widget, GPtrArray *bdies);
void qq_buddy_list_update_groups_info(GtkWidget *widget, GPtrArray *grps);
#endif

#ifndef __GTKQQ_BUDDY_TREE_H
#define __GTKQQ_BUDDY_TREE_H
#include <qq.h>
#include <gtk/gtk.h>

//
// Create the buddy tree view
//
GtkWidget* qq_buddy_tree_new();
//
// Update the buddy tree view model
//
void qq_buddy_tree_update_model(GtkWidget *tree, QQInfo *info);
//
// Update the buddies' face images.
//
void qq_buddy_tree_update_faceimg(GtkWidget *tree, QQInfo *info);
//
// Update the online buddies
//
void qq_buddy_tree_update_online_buddies(GtkWidget *tree, QQInfo *info);
//
// Update the buddy info
//
void qq_buddy_tree_update_buddy_info(GtkWidget *tree, QQInfo *info);

#endif

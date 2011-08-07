#ifndef __GTKQQ_BUDDY_TREE_H
#define __GTKQQ_BUDDY_TREE_H
#include <qq.h>
#include <gtk/gtk.h>

//
// Create the buddy tree view
//
GtkWidget* qq_buddy_tree_new();
//
// Update the buddy tree view
//
void qq_buddy_tree_update(GtkWidget *tree, QQInfo *info);

#endif

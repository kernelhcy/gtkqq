#ifndef __GTKQQ_QQ_BUTTON_H
#define __GTKQQ_QQ_BUTTON_H
#include <gtk/gtk.h>

typedef enum{
	QQ_BUTTON_NORMAL,	//the normal button
	QQ_BUTTON_CLOSE,	//close button
	QQ_BUTTON_MIN,		//minimize button
	QQ_BUTTON_MAX,		//maximize button
	QQ_BUTTON_RESTORE,	//resotre button
	QQ_BUTTON_UNKNOWN	//unknown button, an error occurs.
}QQButtonType;

#define QQ_BUTTON_TYPE			(qq_button_get_type())
#define QQ_BUTTON(obj) 			(G_TYPE_CHECK_INSTANCE_CAST((obj)\
						, QQ_BUTTON_TYPE\
						, QQButton))
#define QQ_BUTTON_CLASS(klass) 		(G_TYPE_CHECK_CLASS_CAST((klass)\
						, QQ_BUTTON_TYPE\
						, QQButtonClass))
#define QQ_IS_BUTTON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj)\
						, QQ_BUTTON_TYPE))
#define QQ_IS_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass)\
						, QQ_BUTTON_TYPE))
#define QQ_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj)\
						, QQ_BUTTON_TYPE\
						, QQButtonClass))
typedef struct _QQButton 	QQButton;
typedef struct _QQButtonClass 	QQButtonClass;


struct _QQButton{
	GtkButton parent;
	
	/* private */
	gint width, height;

	/*
	 * The images
	 */
	GtkWidget *nor_img;	//the normal image
	GtkWidget *hl_img;	//when highlight 
	GtkWidget *push_img;	//when push
	GtkWidget *curr_img;	//current image

	QQButtonType type;
	GtkAllocation *alloc;
};

struct _QQButtonClass{
	GtkButtonClass parentclass;
	/*
	 * store the images of all the button
	 */
	GtkWidget *closebtn_nor_img;	
	GtkWidget *closebtn_hl_img;	 
	GtkWidget *closebtn_push_img;	

	GtkWidget *minbtn_nor_img;	
	GtkWidget *minbtn_hl_img;	 
	GtkWidget *minbtn_push_img;	

	GtkWidget *maxbtn_nor_img;	
	GtkWidget *maxbtn_hl_img;	 
	GtkWidget *maxbtn_push_img;	

	GtkWidget *restorebtn_nor_img;	
	GtkWidget *restorebtn_hl_img;	 
	GtkWidget *restorebtn_push_img;	
};

QQButton* 	qq_button_new(QQButtonType type);
GType 	qq_button_get_type();
#endif

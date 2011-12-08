#include <mainwindow.h>
#include <loginpanel.h>
#include <splashpanel.h>
#include <mainpanel.h>
#include <qq.h>
#include <gqqconfig.h>
#include <stdlib.h>

/*
 * The main event loop context of Gtk.
 */
extern QQInfo *info;
extern GQQConfig *cfg;
extern GtkWidget *main_win;

static void qq_mainwindow_init(QQMainWindow *win);
static void qq_mainwindowclass_init(QQMainWindowClass *wc);

/*
 * The handler of "destroy" singal
 */
#if 0
static void destroy_handler(GtkWidget *widget, gpointer  data)
{
    gtk_main_quit();
    g_debug("Destroy window.(%s, %d)", __FILE__, __LINE__);
    return;
}
#endif

gboolean
qq_mainwindow_close(GtkWidget *widget)
{
	qq_mainwindow_hide(widget);

	return TRUE;
}

GType qq_mainwindow_get_type()
{
    static GType t = 0;
    if(!t){
        const GTypeInfo info =
        {
            sizeof(QQMainWindowClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_mainwindowclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQMainWindow),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_mainwindow_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_WINDOW, "QQMainWindow"
                    , &info, 0);
    }
    return t;
}

/* Show the main window. */
void qq_mainwindow_show(GtkWidget *win)
{
	QQMainWindow *mainwin = (QQMainWindow *)win;

	mainwin->showed = TRUE;
	gtk_widget_show(win);
}

/* Hide the main window */
void qq_mainwindow_hide(GtkWidget *win)
{
	QQMainWindow *mainwin = (QQMainWindow *)win;

	mainwin->showed = FALSE;
	gtk_widget_hide(win);
}

/* If the window now is shown, hide it,
   else show it. */
void qq_mainwindow_show_hide(GtkWidget *win)
{
	QQMainWindow *mainwin = (QQMainWindow *)win;

	if (TRUE == mainwin->showed) {
		qq_mainwindow_hide(win);
	} else {
		qq_mainwindow_show(win);
	}
}

GtkWidget* qq_mainwindow_new()
{
    return GTK_WIDGET(g_object_new(qq_mainwindow_get_type()
                        , "type", GTK_WINDOW_TOPLEVEL, NULL));
}


static void qq_mainwindow_init(QQMainWindow *win)
{
    GtkWidget *w = GTK_WIDGET(win);
    gtk_widget_set_size_request(w, 200, 500);
    gtk_window_resize(GTK_WINDOW(w), 250, 550);

//    gtk_window_set_resizable(GTK_WINDOW(w), FALSE);
    g_signal_connect(w, "delete-event",
					 G_CALLBACK(qq_mainwindow_close), NULL);
    win -> login_panel = qq_loginpanel_new(w);
    win -> splash_panel = qq_splashpanel_new();
    win -> main_panel = qq_mainpanel_new(w);
	win -> showed = FALSE;

    win -> notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(win -> notebook), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(win -> notebook), FALSE);

    gtk_widget_show_all(win -> login_panel);
    gtk_widget_show_all(win -> splash_panel);
    gtk_widget_show_all(win -> main_panel);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(win -> notebook)
                                , win -> login_panel, NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(win -> notebook)
                                , win -> splash_panel, NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(win -> notebook)
                                , win -> main_panel, NULL);

    gtk_container_add(GTK_CONTAINER(win), win -> notebook);

    GdkPixbuf *pb = gdk_pixbuf_new_from_file(IMGDIR"webqq_icon.png", NULL);
    gtk_window_set_icon(GTK_WINDOW(win), pb);
    g_object_unref(pb);
    gtk_window_set_title(GTK_WINDOW(win), "GtkQQ");

    /*
     * Test
     */
//    gtk_notebook_set_current_page(GTK_NOTEBOOK(win -> notebook), 2);
//    qq_mainpanel_update(QQ_MAINPANEL(win -> main_panel));

}
static void qq_mainwindowclass_init(QQMainWindowClass *wc)
{
}

void qq_mainwindow_show_loginpanel(GtkWidget *win)
{
    if(!QQ_IS_MAINWINDOW(win)){
        g_warning("Not a mainwindow!!(%s, %d)", __FILE__, __LINE__);
        return;
    }
    gtk_notebook_set_current_page(GTK_NOTEBOOK(
                QQ_MAINWINDOW(win) -> notebook), 0);
}
void qq_mainwindow_show_splashpanel(GtkWidget *win)
{
    if(!QQ_IS_MAINWINDOW(win)){
        g_warning("Not a mainwindow!!(%s, %d)", __FILE__, __LINE__);
        return;
    }
    gtk_notebook_set_current_page(GTK_NOTEBOOK(
                QQ_MAINWINDOW(win) -> notebook), 1);
}
void qq_mainwindow_show_mainpanel(GtkWidget *win)
{
    if(!QQ_IS_MAINWINDOW(win)){
        g_warning("Not a mainwindow!!(%s, %d)", __FILE__, __LINE__);
        return;
    }
    gtk_notebook_set_current_page(GTK_NOTEBOOK(
                QQ_MAINWINDOW(win) -> notebook), 2);
}


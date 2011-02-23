/*
 * Do the initail.
 */
#include <qq.h>
#include <glib.h>

/*
 * The main loop thread's main function.
 */
static gpointer start_main_loop(gpointer *data)
{
	QQInfo *info = (QQInfo*)data;
	if(info == NULL){
		return NULL;
	}

	if(info -> mainloop == NULL || info -> mainctx == NULL){
		g_warning("Havn't create the main event loop!!(%s, %d)"
				, __FILE__, __LINE__);
		info -> mainloop = g_main_loop_new(NULL, FALSE);
		info -> mainctx = g_main_loop_get_context(info -> mainloop);
		g_warning("Create the main event loop. done.(%s, %d)"
				, __FILE__, __LINE__);
	}

	g_debug("Run the main event loop...(%s, %d)", __FILE__, __LINE__);
	//start the main loop
	g_main_loop_run(info -> mainloop);

	/*
	 * Will not arrive here untill the program exits. 
	 */
	g_debug("Quit main event loop.(%s, %d)", __FILE__, __LINE__);
	return NULL;
}

/*
 * The poll event loop.
 * In a new thread.
 */
static gpointer start_poll_loop(gpointer *data)
{
	QQInfo *info = (QQInfo*)data;
	if(info == NULL){
		return NULL;
	}

	if(info -> pollloop == NULL || info -> pollctx == NULL){
		g_warning("Havn't create the poll event loop!!(%s, %d)"
				, __FILE__, __LINE__);
		info -> pollloop = g_main_loop_new(NULL, FALSE);
		info -> pollctx = g_main_loop_get_context(info -> pollloop);
		g_warning("Create the poll event loop. done.(%s, %d)"
				, __FILE__, __LINE__);
	}

	g_debug("Run the poll event loop...(%s, %d)", __FILE__, __LINE__);
	//start the poll loop
	g_main_loop_run(info -> pollloop);

	/*
	 * Will not arrive here untill the program exits. 
	 */
	return NULL;
}
QQInfo* qq_init(QQCallBack cb)
{
	/*
	 * When call gtk_init(), the g_thread_init() also be called.
	 * This liberary may not be used with gtk, so we call g_thread_init()
	 * here.
	 */
	if(!g_thread_supported()){
		g_thread_init(NULL);
	}else{
		g_error("Need thread supported!");
		return NULL;
	}
	QQInfo *info = qq_info_new();

	/*
	 * The main event loop
	 */
	g_debug("Start the main event loop.(%s, %d)", __FILE__, __LINE__);
	info -> mainloop = g_main_loop_new(NULL, FALSE);
	info -> mainctx = g_main_loop_get_context(info -> mainloop);
	GError *err;
	info -> mainloopthread = g_thread_create((GThreadFunc)start_main_loop
						, (gpointer)info, FALSE
						, &err);
	if(info -> mainloopthread == NULL){
		g_error("Error code %d, msg: %s (%s, %d)", err -> code
					, err -> message, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_ERROR, "Create mainloop thread error.");
		}
		return NULL;
	}

	/*
	 * The poll event loop.
	 */
	g_debug("Start the poll event loop.(%s, %d)", __FILE__, __LINE__);
	info -> pollloop = g_main_loop_new(NULL, FALSE);
	info -> pollctx = g_main_loop_get_context(info -> pollloop);
	info -> pollloopthread = g_thread_create((GThreadFunc)start_poll_loop
						, (gpointer)info, FALSE
						, &err);
	if(info -> pollloopthread == NULL){
		g_error("Error code %d, msg: %s (%s, %d)", err -> code
					, err -> message, __FILE__, __LINE__);
		if(cb != NULL){
			cb(CB_ERROR, "Create poll loop thread error.");
		}
		return NULL;
	}

	return info;
}

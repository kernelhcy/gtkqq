#include <qq.h>

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
		g_warning("Havn't create the main event loop!!");
		info -> mainloop = g_main_loop_new(NULL, FALSE);
		info -> mainctx = g_main_loop_get_context(info -> mainloop);
		g_warning("Create the main event loop. done.");
	}


	g_debug("Run the main event loop...");
	//start the main loop
	g_main_loop_run(info -> mainloop);

	/*
	 * Will not arrive here untill the program exits. 
	 */
	g_debug("Quit main event loop.");
	return NULL;
}


struct InitParam{
	QQCallBack cb;
	QQInfo *info;
	
};
/*
 * Do initial.
 * Get information from the server.
 * Run in the main event loop.
 */
static gboolean do_init(gpointer data)
{
	if(data == NULL){
		return FALSE;
	}
	QQCallBack cb = ((struct InitParam *)data) -> cb;
	QQInfo *info = ((struct InitParam *)data) -> info;

	g_debug("Do init...");
	if(cb != NULL){
		g_debug("Call the callback function in do_init.");
		cb(CB_SUCCESS, NULL);
	}
	g_debug("Get pages' sources...");
	g_debug("Get javascript sources...");
	g_debug("Parse the information...");
	g_debug("Initial done.");

	g_slice_free(struct InitParam, data);
	g_debug("Free the struct InitParam.");

	return FALSE;
}


void qq_init(QQInfo *info, QQCallBack cb)
{
	/*
	 * When call gtk_init(), the g_thread_init() also be called.
	 * This liberary may not be used with gtk, so we call g_thread_init()
	 * here.
	 */
	if(!g_thread_supported()){
		g_thread_init(NULL);
	}else{
		if(cb){
			cb(CB_FAILED, "Need thread supported!!");
		}
		g_error("Need thread supported!");
		return;
	}
	g_debug("Initial the thread done");

	info -> mainloop = g_main_loop_new(NULL, FALSE);
	info -> mainctx = g_main_loop_get_context(info -> mainloop);
	g_debug("Create the main event loop. done.");

	GError *err;
	info -> mainloopthread = g_thread_create((GThreadFunc)start_main_loop
						, (gpointer)info, FALSE
						, &err);
	if(info -> mainloopthread == NULL){
		g_error("Error code %d, msg: %s", err -> code
					, err -> message);
		return;
	}
	g_debug("Start the main event loop thread. done.");

	GSource *src = g_idle_source_new();
	struct InitParam *par = g_slice_new(struct InitParam);
	par -> cb = cb;
	par -> info = info;
	g_source_set_callback(src, (GSourceFunc)do_init, (gpointer)par, NULL);
	if(g_source_attach(src, info -> mainctx) <= 0){
		g_error("Attach initial source error.");
	}
	g_debug("Attach the initial source. done.");
	g_source_unref(src);
}

void qq_login(const gchar *uin, const gchar *passwd, const gchar *vc
			, QQCallBack cb)
{
}

#include <msgloop.h>
//The Invoker type
typedef void (*Invoker)(gpointer method, gpointer *pars);

//
// GQQClosure
// Contain all the information to invoke a method whoes 
// parameters are all gpointer type.
//
typedef struct{
    Invoker invoker;
    gpointer method;
    gpointer pars[10];  // I think that no method's parameters' 
                        // number will greater ten...
}GQQClosure;

static gpointer thread_main(gpointer data)
{
    gchar *name;
    GQQMessageLoop *ml= (GQQMessageLoop*)data;

    ml -> ctx = g_main_context_new();
    if(ml -> ctx == NULL){
        g_error("Create context for %s loop failed... (%s, %d)",ml -> name
                                                        , __FILE__, __LINE__);
        return NULL;
    }
    GMainLoop *loop = g_main_loop_new(ml -> ctx, TRUE);
    if(loop == NULL){
        g_error("Create %s main loop failed...(%s, %d)", ml -> name
                                                        , __FILE__, __LINE__);
        return NULL;
    }
    ml -> loop = loop;

    g_debug("Start %s main loop...(%s, %d)", ml -> name, __FILE__, __LINE__);
    name = g_strdup(ml -> name);
    g_main_loop_run(ml -> loop);
    g_debug("%s main loop quit.(%s, %d)", name, __FILE__, __LINE__);
    g_free(name); 
    return NULL;
}

GQQMessageLoop* gqq_msgloop_start(const gchar *name)
{
    if(name == NULL){
        name = "";
    }

    GQQMessageLoop *ml = g_slice_new0(GQQMessageLoop);
    ml -> name = g_strdup(name);

    GError *err = NULL;
    if(
#if GLIB_CHECK_VERSION(2,32,0)
        g_thread_new("", thread_main, NULL);
#else
        g_thread_create(thread_main, ml, FALSE, &err)
#endif
        == NULL){
        g_warning("Create main loop thread failed... %s (%s, %d)"
                                , err -> message, __FILE__, __LINE__);
        g_error_free(err);
        g_free(ml -> name);
        g_slice_free(GQQMessageLoop, ml);
        return NULL;
    }
    return ml;
}
void gqq_msgloop_stop(GQQMessageLoop *loop)
{
    if(loop == NULL){
        return;
    }
    g_main_context_unref(loop -> ctx);
    g_main_loop_quit(loop -> loop);
    g_free(loop -> name);
    g_slice_free(GQQMessageLoop, loop);
    return;
}

void gqq_method_invoker_1(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0]);
}
void gqq_method_invoker_2(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer, gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0], pars[1]);
}
void gqq_method_invoker_3(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer, gpointer, gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0], pars[1], pars[2]);
}
void gqq_method_invoker_4(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer, gpointer, gpointer, gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0], pars[1], pars[2], pars[3]);
}
void gqq_method_invoker_5(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer, gpointer, gpointer
                                , gpointer, gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0], pars[1], pars[2], pars[3], pars[4]);
}

// The table of the addresses of the invokers
static Invoker invokers[] = {
    NULL,
    gqq_method_invoker_1,
    gqq_method_invoker_2,
    gqq_method_invoker_3,
    gqq_method_invoker_4,
    gqq_method_invoker_5,
    NULL
};

//
// The callback called in the main loop.
//
static gboolean gqq_ctx_callback(gpointer data)
{
    GQQClosure *c = (GQQClosure*)data;
    // invoke the method.
    c -> invoker(c -> method, c -> pars);
    // free the GQQClosure.
    g_slice_free(GQQClosure, c);
    return FALSE;
}

gint gqq_mainloop_attach(GQQMessageLoop *loop, gpointer method, gint par_num, ...)
{
    va_list par;
    va_start(par, par_num);
    GQQClosure *c = g_slice_new0(GQQClosure);
    c -> method = method;
    c -> invoker = invokers[par_num];
    gpointer p;
    gint i;
    for(i = 0; i < par_num; ++i){
        p = va_arg(par, gpointer);
        c -> pars[i] = p;
    }
    va_end(par);
    
    GSource *src = g_idle_source_new();
    g_source_set_callback(src, gqq_ctx_callback, c, NULL);
    g_source_attach(src, loop -> ctx);
    g_source_unref(src);
    return 0;
}

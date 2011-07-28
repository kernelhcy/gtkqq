#include <msgloop.h>

static GThread *tid = NULL;

static GMutex *lock = NULL;
static gboolean run = FALSE;

// Task queue
static GAsyncQueue *queue = NULL;

static gpointer loop_func(gpointer data)
{
    GQQClosure *c;
    while(TRUE){
        g_mutex_lock(lock);
        if(!run){
            g_mutex_unlock(lock);
            break;
        }
        g_mutex_unlock(lock);

        c = (GQQClosure*)g_async_queue_pop(queue);
        // invoke the method.
        c -> invoker(c -> method, c -> pars);
        // free the GQQClosure.
        g_slice_free(GQQClosure, c);
    }

    // Free the mutex
    g_mutex_free(lock);
    // Free the task queue
    g_async_queue_unref(queue);
    return NULL;
}

gint gqq_msgloop_start()
{
    GError *err = NULL;
    queue = g_async_queue_new();
    lock = g_mutex_new();
    run = TRUE;

    tid = g_thread_create(loop_func, NULL, FALSE, &err);
    if(tid == NULL){
        g_error("Create message loop thread error! %s (%s, %d)"
                        , err -> message, __FILE__, __LINE__);
        g_error_free(err);
        g_mutex_free(lock);
        g_async_queue_unref(queue);
        return -1;
    }
    return 0;
}
gint gqq_msgloop_stop()
{
    g_mutex_lock(lock); 
    run = FALSE;
    g_mutex_unlock(lock);
    return 0;
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

gint gqq_msgloop_attach(gpointer method, gint par_num, ...)
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

    // push into the task queue.
    g_async_queue_push(queue, c);
    va_end(par);
    return 0;
}

static gboolean gqq_ctx_callback(gpointer data)
{
    GQQClosure *c = (GQQClosure*)data;
    // invoke the method.
    c -> invoker(c -> method, c -> pars);
    // free the GQQClosure.
    g_slice_free(GQQClosure, c);
    return FALSE;
}

gint gqq_context_attach(GMainContext *ctx, gpointer method, gint par_num, ...)
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
    g_source_attach(src, ctx);
    g_source_unref(src);
    return 0;
}

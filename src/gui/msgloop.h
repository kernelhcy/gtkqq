#ifndef __GQQ_MSGLOOP_H_
#define __GQQ_MSGLOOP_H_
#include <glib.h>
#include <stdarg.h>
//
// msgloop.h
// This file contains the codes to send message to the server.
// A new thread is created and all the messages are sent in this thread.
//

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

// void method(gpointer)
void gqq_method_invoker_1(gpointer method, gpointer *pars);
// void method(gpointer, gpointer)
void gqq_method_invoker_2(gpointer method, gpointer *pars);
// void method(gpointer, gpointer, gpointer)
void gqq_method_invoker_3(gpointer method, gpointer *pars);
// void method(gpointer, gpointer, gpointer, gpointer)
void gqq_method_invoker_4(gpointer method, gpointer *pars);
// void method(gpointer, gpointer, gpointer, gpointer, gpointer)
void gqq_method_invoker_5(gpointer method, gpointer *pars);

//
// Start and stop the message loop
//
gint gqq_msgloop_start();
gint gqq_msgloop_stop();

//
// Attach a method in the message loop.
// @param method : the address of the method which will be called in the
//                  message loop.
// @param par_num: the number of the parameter of the method. 
//                  <= 5.
// NOTE:
//      The types of the parameters of the method must be pointer.
//
gint gqq_msgloop_attach(gpointer method, gint par_num, ...);

//
// Invoke the method in the main context `ctx`.
//
// @param method : the address of the method which will be called in the
//                  message loop.
// @param par_num: the number of the parameter of the method. 
//                  <= 5.
// NOTE:
//      The types of the parameters of the method must be pointer.
//
gint gqq_context_attach(GMainContext *ctx, gpointer method, gint par_num, ...);
#endif

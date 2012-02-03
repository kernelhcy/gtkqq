#ifndef __GTKQQ_LOG_H
#define __GTKQQ_LOG_H

#include <glib.h>

/*
 * Initial the log.Set the log handler.
 * 
 * Only set the message, debug and warning loggs' handler.
 * @debug Wheather handle debug message.
 *  
 */
void log_init(gboolean debug);

#endif

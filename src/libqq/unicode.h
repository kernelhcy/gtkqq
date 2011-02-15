#ifndef __LIBQQ_UNICODE_H
#define __LIBQQ_UNICODE_H
#include <glib.h>
/*
 * Convert UCS-4 to UTF8.
 *
 * The result will be stored in to.
 */
void ucs4toutf8(GString *to, const gchar *from);

#endif

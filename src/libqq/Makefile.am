lib_LTLIBRARIES=libwebqq.la
ZLIB_CFLAGS=-lz
libwebqq_la_CPPFLAGS= $(GTHREAD_CFLAGS) $(GLIB_CFLAGS)
libwebqq_la_LIBADD= $(GTHREAD_LIBS) $(GLIB_LIBS) $(ZLIB)
libwebqq_la_SOURCES=http.h http.c json.h json.c url.h url.c\
			qq.h qqlogin.c qqtypes.h qqtypes.c qqmanageinfo.c\
			unicode.c unicode.h qqinit.c qqpoll.c qqsendmsg.c\
			qqutils.c qqimg.c
AM_CFLAGS = -Wall -Werror

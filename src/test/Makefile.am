AM_CFLAGS=-I${srcdir}/../libqq/ 
AM_CFLAGS+=-I${srcdir}/../comm/
AM_CFLAGS+=-I${srcdir}/../gui/
AM_CFLAGS+=${GLIB_CFLAGS} -DGTKQQ_DEBUG -Wall -Werror

noinst_PROGRAMS=testhttp testjson testcprint testlog testurl testpro testproxy

testhttp_SOURCES= testhttp.c
testhttp_LDADD=${srcdir}/../libqq/libwebqq.la $(GLIB_LIBS)

testjson_SOURCES= testjson.c  ${srcdir}/../libqq/json.c ${srcdir}/../libqq/json.h
testjson_LDADD=$(GLIB_LIBS)

testcprint_SOURCES= ${srcdir}/../comm/cprint.c testcprint.c 
testcprint_LDADD=${srcdir}/../libqq/libwebqq.la $(GLIB_LIBS)

testlog_SOURCES= ${srcdir}/../comm/cprint.c ${srcdir}/../comm/log.c testlog.c
testlog_LDADD=${srcdir}/../libqq/libwebqq.la $(GLIB_LIBS)

testurl_SOURCES= ${srcdir}/../comm/log.c testurl.c ${srcdir}/../comm/cprint.c 
testurl_LDADD=${srcdir}/../libqq/libwebqq.la $(GLIB_LIBS)

testpro_SOURCES= ${srcdir}/../comm/cprint.c testpro.c ${srcdir}/../comm/log.c
testpro_LDADD=${srcdir}/../libqq/libwebqq.la $(GLIB_LIBS)

testproxy_SOURCES=testproxy.c
testproxy_LDADD=${srcdir}/../libqq/libwebqq.la $(GLIB_LIBS)

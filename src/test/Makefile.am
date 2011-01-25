AM_CFLAGS=-I${srcdir}/../libqq/ 
AM_CFLAGS+=-I${srcdir}/../comm/
AM_CFLAGS+=-I${srcdir}/../gui/
AM_CFLAGS+=${GTK_CFLAGS}

noinst_PROGRAMS=testhttp testjson testcprint testlog testurl

testhttp_SOURCES= testhttp.c
testhttp_LDADD=${srcdir}/../libqq/libwebqq.la $(GTK_LIBS)

testjson_SOURCES= testjson.c
testjson_LDADD=${srcdir}/../libqq/libwebqq.la $(GTK_LIBS)

testcprint_SOURCES= ${srcdir}/../comm/cprint.c testcprint.c 
testcprint_LDADD=${srcdir}/../libqq/libwebqq.la $(GTK_LIBS)

testlog_SOURCES= ${srcdir}/../comm/cprint.c ${srcdir}/../comm/log.c testlog.c
testlog_LDADD=${srcdir}/../libqq/libwebqq.la $(GTK_LIBS)

testurl_SOURCES= ${srcdir}/../comm/log.c testurl.c ${srcdir}/../comm/cprint.c 
testurl_LDADD=${srcdir}/../libqq/libwebqq.la $(GTK_LIBS)

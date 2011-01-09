AM_CFLAGS=-I${srcdir}/../libqq/ 
AM_CFLAGS+=-I${srcdir}/../comm/

noinst_PROGRAMS=testhttp testjson testcprint testlog testurl

testhttp_SOURCES= testhttp.c
testhttp_LDADD=${srcdir}/../libqq/libwebqq.la

testjson_SOURCES= testjson.c
testjson_LDADD=${srcdir}/../libqq/libwebqq.la

testcprint_SOURCES= ${srcdir}/../comm/cprint.c testcprint.c 

testlog_SOURCES= ${srcdir}/../comm/cprint.c ${srcdir}/../comm/log.c testlog.c

testurl_SOURCES= ${srcdir}/../comm/log.c testurl.c ${srcdir}/../comm/cprint.c 
testurl_LDADD=${srcdir}/../libqq/libwebqq.la

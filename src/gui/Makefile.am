bin_PROGRAMS=qq
qq_CPPFLAGS= -I$(srcdir)/. $(GTK_CFLAGS) -g -Wall -O2\
	     -DIMGDIR=\"$(pkgdatadir)/misc/\"
qq_LDADD = $(GTK_LIBS)
qq_SOURCES= qqwindow.h qqwindow.c\
	    qqbutton.h qqbutton.c\
	    qq.c

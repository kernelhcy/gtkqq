SUBDIRS=tencent test
bin_PROGRAMS=gtkqq
gtkqq_CPPFLAGS= -I$(srcdir)/. $(GTK_CFLAGS) \
	     -DIMGDIR=\"$(pkgdatadir)/misc/\"
gtkqq_LDADD = $(GTK_LIBS)
gtkqq_SOURCES=main.c loginpanel.c loginpanel.h mainpanel.h mainpanel.c\
	      mainwindow.h mainwindow.c

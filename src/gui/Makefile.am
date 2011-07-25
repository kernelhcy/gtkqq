SUBDIRS=tencent test

AM_CFLAGS=-I$(srcdir)/../libqq/\
      -I$(srcdir)/../comm/\
      -I$(srcdir)/.\
      $(GTK_CFLAGS)\
      -DIMGDIR=\"$(pkgdatadir)/misc/\"\
      -DCONFIGDIR=\"$(HOME)/.gtkqq/\"\
      -Wall -Werror

bin_PROGRAMS=gtkqq
gtkqq_LDADD = $(GTK_LIBS) ${srcdir}/../libqq/libwebqq.la
gtkqq_SOURCES=main.c \
          loginpanel.c loginpanel.h \
          mainpanel.h mainpanel.c\
          mainwindow.h mainwindow.c \
          ${srcdir}/../comm/log.c\
          ${srcdir}/../comm/cprint.c\
          splashpanel.h splashpanel.c\
          gqqconfig.h gqqconfig.c\
          statusbutton.h statusbutton.c


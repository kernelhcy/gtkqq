SUBDIRS=tencent test

AM_CFLAGS=-I$(srcdir)/../libqq/\
      -I$(srcdir)/../comm/\
      -I$(srcdir)/.\
      $(GTK_CFLAGS)\
      $(SQLITE3_CFLAGS)\
      -DIMGDIR=\"$(pkgdatadir)/misc/\"\
      -DCONFIGDIR=\"$(HOME)/.gtkqq/\"\
      -Wall -Werror

bin_PROGRAMS=gtkqq
gtkqq_LDADD = $(GTK_LIBS) ${srcdir}/../libqq/libwebqq.la $(SQLITE3_LIBS)
gtkqq_SOURCES=main.c \
          msgloop.c msgpoll.h\
          dao.c dao.h\
		  msgdispacher.c msgdispacher.h\
          tray.c tray.h\
          facepopupwindow.c facepopupwindow.h\
          chatwindow.c chatwindow.h\
          chatwidget.c chatwidget.h\
          chattextview.c chattextview.h\
          buddytree.c buddytree.h\
          loginpanel.c loginpanel.h \
          mainpanel.h mainpanel.c\
          mainwindow.h mainwindow.c \
          ${srcdir}/../comm/log.c\
          ${srcdir}/../comm/cprint.c\
          splashpanel.h splashpanel.c\
          gqqconfig.h gqqconfig.c\
          statusbutton.h statusbutton.c


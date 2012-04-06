SUBDIRS=test

AM_CFLAGS=-I$(srcdir)/../libqq/\
      -I$(srcdir)/../comm/\
      -I$(srcdir)\
      $(GTK_CFLAGS)\
      $(SQLITE3_CFLAGS)\
	  $(GSTREAMER_CFLAGS)\
	  $(LIBNOTIFY_CFLAGS)\
	  $(GSTINTERFACES_CFLAGS)\
      -DIMGDIR=\"$(pkgdatadir)/misc/\"\
      -DCONFIGDIR=\"$(HOME)/.gtkqq/\"\
      -DSOUNDDIR=\"$(pkgdatadir)/misc/sound\"\
      -Wall -Werror

bin_PROGRAMS=gtkqq
gtkqq_LDADD = $(GTK_LIBS) $(GSTREAMER_LIBS) $(GSTINTERFACES_LIBS) ../libqq/libwebqq.la $(SQLITE3_LIBS) $(LIBNOTIFY_LIBS)
gtkqq_SOURCES=main.c \
          msgloop.c msgloop.h\
          dao.c dao.h\
		  msgdispacher.c msgdispacher.h\
          tray.c tray.h\
          facepopupwindow.c facepopupwindow.h\
          chatwindow.c chatwindow.h\
          groupchatwindow.c groupchatwindow.h\
          chatwidget.c chatwidget.h\
          chattextview.c chattextview.h\
          buddytree.c buddytree.h\
          buddylist.c buddylist.h\
          loginpanel.c loginpanel.h \
          mainpanel.h mainpanel.c\
          mainwindow.h mainwindow.c \
          $(srcdir)/../comm/log.c\
          $(srcdir)/../comm/cprint.c\
          splashpanel.h splashpanel.c\
          gqqconfig.h gqqconfig.c\
          statusbutton.h statusbutton.c\
		  sound.h sound.c\
		  notify.h notify.c\
		  proxypanel.c proxypanel.h


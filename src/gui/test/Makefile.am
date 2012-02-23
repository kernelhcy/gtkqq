noinst_PROGRAMS=winmove splashpanel
winmove_CPPFLAGS= -I$(srcdir)/. $(GTK_CFLAGS) -g -Wall -O2
winmove_LDADD = $(GTK_LIBS)

winmove_SOURCES= winmove.c

splashpanel_SOURCES= test-splashpanel.c ../splashpanel.c

splashpanel_CFLAGS= -I.. $(GTK_CFLAGS) -DIMGDIR=\"../../misc/\"
splashpanel_LDADD = $(GTK_LIBS)

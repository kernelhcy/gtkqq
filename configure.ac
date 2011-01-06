AC_INIT([gtkqq], [0.1], [huangcongyu2006@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
CFLAGS+=`pkg-config --libs --cflags glib-2.0`
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CHECK_HEADERS([glib.h])
AC_CONFIG_FILES([Makefile src/Makefile src/libqq/Makefile src/tool/Makefile])
AC_OUTPUT

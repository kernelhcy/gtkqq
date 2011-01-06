AC_INIT([gtkqq], [0.1], [huangcongyu2006@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
CFLAGS+=`pkg-config --libs --cflags glib-2.0` 
CFLAGS+='-Wall -O2'
LT_INIT(disable-static)
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CHECK_HEADERS([glib.h])
AC_CONFIG_FILES([Makefile src/Makefile src/libqq/Makefile src/tool/Makefile
		 src/test/Makefile])
AC_OUTPUT

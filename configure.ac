#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([GtkQQ], [0.1], [huangcongyu2006@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_HEADERS([config.h])
LT_INIT(disable-static)

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

# libtools
AC_PROG_RANLIB
AC_PROG_LIBTOOL

#gettext
#IT_PROG_INTLTOOL([0.40.0])
#AC_SUBST([GETTEXT_PACKAGE], [GtkQQ])
#AM_GNU_GETTEXT_VERSION([0.17])
#AM_GLIB_GNU_GETTEXT
#AM_GNU_GETTEXT([external])
#AC_DEFINE([GETTEXT_PACKAGE], [PACKAGE_TARNAME], [Define to the gettext\
#package name.])

AC_CHECK_LIB([notify], [notify_init])

AC_CHECK_LIB([ssl], [SSL_library_init])

AC_CHECK_LIB([crypto], [RSA_new])

AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h\
		  stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h])

AC_ARG_ENABLE(libnotify,
              AS_HELP_STRING([--enable-libnotify],
                             [Enable libnotify support (used to show message\
			      notifications) @<:@default=yes@:>@]),,
              enable_libnotify="yes")

AC_ARG_ENABLE(gst,
              AS_HELP_STRING([--enable-gst],
                             [Enable gstreamer support (used to play alert\
			      sounds) @<:@default=yes@:>@]),,
              enable_gst="yes")
# checks for gtk+2.0
PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.16], [
			AC_DEFINE(USE_GTK, 1, [Use GTK])
			AC_SUBST(GTK_CFLAGS)
			AC_SUBST(GTK_LIBS)
			] ,
			)
# checks for glib2.0
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.16], [
			AC_DEFINE(USE_GLIB, 1, [Use GLIB])
			AC_SUBST(GLIB_CFLAGS)
			AC_SUBST(GLIB_LIBS)
			] ,
			)

# checks for gthread-2.0
PKG_CHECK_MODULES(GTHREAD, [gthread-2.0], [
			AC_DEFINE(USE_GTHREAD, 1, [Use GTHREAD-2.0])
			AC_SUBST(GTHREAD_CFLAGS)
			AC_SUBST(GTHREAD_LIBS)
		] , )

# checks for libxml2
PKG_CHECK_MODULES(LIBXML, [libxml-2.0], [
			AC_DEFINE(USE_LIBXML, 1, [Use LIBXML])
			AC_SUBST(LIBXML_CFLAGS)
			AC_SUBST(LIBXML_LIBS)
		] , )

# checks for libsqlite3
#PKG_CHECK_MODULES(SQLITE3, [sqlite3 > 3.3], [
#			AC_DEFINE(USE_SQLITE3, 1, [Use sqlite3])
#			AC_SUBST(SQLITE3_CFLAGS)
#			AC_SUBST(SQLITE3_LIBS)
#		] , [
#			echo ""
#			echo "configure: error: "
##			echo "libsqlite3 development headers not found."
#			echo "If you're using debian or ubuntu,install it by"
#			echo "\"sudo apt-get install libsqlite3-dev\""
#			echo ""
#			exit -1
#			 ])

# checks for libxss
if test "x$enable_screensaver" = "xyes" ;then
PKG_CHECK_MODULES(LIBXSS, [xscrnsaver], [
			AC_DEFINE(USE_LIBXSS, 1, [Use XScreenSaver\
						  for idleness])
			AC_SUBST(LIBXSS_CFLAGS)
			AC_SUBST(LIBXSS_LIBS)
		] , [
			echo ""
			echo "configure: error: "
			echo "XScreenSaver development headers not found."
			echo "Use --disable-screensaver if you do not need it."
			echo "If you're using debian or ubuntu,install it by"
			echo "\"sudo apt-get install libxss-dev\""
			echo ""
			exit -1
			 ])
fi

# checks for libnotify
if test "x$enable_libnotify" = "xyes"; then
PKG_CHECK_MODULES(LIBNOTIFY, [libnotify], [
			AC_DEFINE(USE_LIBNOTIFY, 1, [Use libnotify for\
						     notification])
			AC_SUBST(LIBNOTIFY_CFLAGS)
			AC_SUBST(LIBNOTIFY_LIBS)
		] , [
			echo ""
			echo "configure: error: "
			echo "libnotify development headers not found."
			echo "Use --disable-libnotify if you do not need it."
			echo "If you're using debian or ubuntu,install it by "
			echo "\"sudo apt-get install libnotify-dev\""
			echo ""
			exit -1
			])
fi

# checks for gstreamer-0.10
if test "x$enable_gst" = "xyes"; then
PKG_CHECK_MODULES(GSTREAMER, [gstreamer-0.10], [
			AC_DEFINE(USE_GSTREAMER, 1, [Use Gstreamer for\
						     playing sound])
			AC_SUBST(GSTREAMER_CFLAGS)
			AC_SUBST(GSTREAMER_LIBS)
		] , [
			echo ""
			echo "configure: error: "
			echo "gstreamer development headers not found."
			echo "Use --disable-gst if you do not need it."
			echo "If you're using debian or ubuntu,install it by "
			echo "\"sudo apt-get install libgstreamer0.10-dev\""
			echo ""
			exit -1
			])
fi

# checks for networkmanager
#if test "x$enable_nm" = "xyes"; then
#PKG_CHECK_MODULES(NETWORKMANAGER, [NetworkManager >= 0.5.0], [
#			AC_DEFINE(USE_NETWORKMANAGER, 1, [Use NetworkManager\
#					 for monitoring network status])
#			AC_SUBST(NETWORKMANAGER_CFLAGS)
#			AC_SUBST(NETWORKMANAGER_LIBS)
#		] , [
#			echo ""
#			echo "configure: error: "
#			echo "NetworkManager development headers not found."
#			echo "Use --disable-nm if you do not need it."
#			echo "If you're using debian or ubuntu,install it by "
#			echo "\"sudo apt-get install libnm-glib-dev\""
#			echo ""
#			exit -1
#			])
#fi

PKG_CHECK_MODULES(LIBSSL, [
		openssl
])

PKG_CHECK_MODULES(GLIB2, [
        glib-2.0 >= 2.14
])

PKG_CHECK_MODULES(GOBJECT2, [
        gobject-2.0 >= 2.14
])

AC_ARG_ENABLE(debug,
        [ --enable-debug turn on debug],
        CPPFLAGS+="-DGTKQQ_DEBUG -ggdb")

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([bzero gethostbyname memset mkdir select socket strstr strtol])
AC_CHECK_FUNCS([inet_ntoa])

AC_CONFIG_FILES([Makefile 
		 src/Makefile 
		 src/comm/Makefile 
		 src/libqq/Makefile 
		 src/cli/Makefile
		 src/gui/Makefile
		 src/tool/Makefile
		 src/misc/Makefile
		 src/test/Makefile])
AC_OUTPUT

echo
echo Use XScreenSaver Extension.... : $enable_screensaver
echo Use libnotify................. : $enable_libnotify
echo Use gstreamer0.10............. : $enable_gst
#echo Use NetworkManager............ : $enable_nm
echo

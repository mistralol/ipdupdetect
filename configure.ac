AC_INIT(ipdupdetect, 1.0.0, james@stev.org)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIR([m4])

LT_INIT
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

AX_CXX_COMPILE_STDCXX_11(noext, optional)

AX_COMPILER_FLAGS([WARN_CFLAGS],[WARN_LDFLAGS])
AC_SUBST([AM_CXXFLAGS], $WARN_CXXFLAGS)

dnl give error and exit if we don't have pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, [ ], [
  AC_MSG_ERROR([You need to have pkg-config installed!])
])

dnl Check for libpcap
AC_CHECK_LIB([pcap], [pcap_open_live], [], [
	AC_MSG_ERROR([You need to have libpcap installed])
])

dnl Check for libclientserver
PKG_CHECK_MODULES(LIBCLIENTSERVER, [
	libclientserver >= 1.0.0
], [
	AC_SUBST(LIBCLIENTSERVER_CFLGAGS)
	AC_SUBST(LIBCLIENTSERVER_LIBS)
], [
	AC_MSG_ERROR([
		You need to install or upgrade libclientserver
	])
])

dnl Check for liblogger
PKG_CHECK_MODULES(LIBLOGGER, [
	liblogger >= 1.0.0
], [
	AC_SUBST(LIBLOGGER_CFLGAGS)
	AC_SUBST(LIBLOGGER_LIBS)
], [
	AC_MSG_ERROR([
		You need to install or upgrade liblogger
	])
])

AC_CHECK_LIB([pthread], [pthread_mutex_init], [], [
	AC_MSG_ERROR([You need to have pthread installed])
])

AC_OUTPUT(Makefile src/Makefile)



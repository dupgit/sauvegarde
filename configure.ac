AC_PREREQ(2.59)

AC_INIT([cdpfgl], [0.0.6], [Olivier DELHOMME <olivier.delhomme@free.fr>])
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_INTLTOOL([0.23])


dnl ***********************************************************************
dnl * Put the preprocessor macros in a header file                        *
dnl * instead of command line                                             *
dnl ***********************************************************************
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([monitor/monitor.c])


dnl ***********************************************************************
dnl * Libraries requirements                                              *
dnl ***********************************************************************
PKG_PROG_PKG_CONFIG([0.23])
GLIB_VERSION=2.30.0
GIO_VERSION=2.30.0
SQLITE_VERSION=3.6.20
JANSSON_VERSION=2.5
MHD_VERSION=0.9.5
CURL_VERSION=7.22.0

AC_SUBST(GLIB_VERSION)
AC_SUBST(GIO_VERSION)
AC_SUBST(SQLITE_VERSION)
AC_SUBST(JANSSON_VERSION)
AC_SUBST(MHD_VERSION)
AC_SUBST(CURL_VERSION)


dnl ***********************************************************************
dnl * Build time sanity check                                             *
dnl ***********************************************************************
AM_SANITY_CHECK


dnl ***********************************************************************
dnl * Initialize maintainer mode                                          *
dnl ***********************************************************************
AM_MAINTAINER_MODE


dnl ***********************************************************************
dnl * Checks for headers                                                  *
dnl ***********************************************************************
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS( \
 signal.h         \
 sys/signalfd.h   \
 sys/fanotify.h   \
 math.h
)


dnl ***********************************************************************
dnl * Checks dynamic libraries capabilities                               *
dnl ***********************************************************************
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


dnl ***********************************************************************
dnl * languages                                                           *
dnl ***********************************************************************
ALL_LINGUAS="fr"


dnl ***********************************************************************
dnl * triggers gettext                                                    *
dnl ***********************************************************************
GETTEXT_PACKAGE=cdpfgl
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext Package used])
AM_GLIB_GNU_GETTEXT


dnl ***********************************************************************
dnl * checking for modules (glib, gio, sqlite, mhd, curl...)              *
dnl ***********************************************************************
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= $GLIB_VERSION])
PKG_CHECK_MODULES(GIO, [gio-2.0 >= $GIO_VERSION])
PKG_CHECK_MODULES(SQLITE, [sqlite3 >= $SQLITE_VERSION])
PKG_CHECK_MODULES(JANSSON, [jansson >= $JANSSON_VERSION])
PKG_CHECK_MODULES(MHD, [libmicrohttpd >= $MHD_VERSION])
PKG_CHECK_MODULES(CURL, [libcurl >= $CURL_VERSION])

AC_PROG_INSTALL

CFLAGS="$CFLAGS -Wall -Wstrict-prototypes -Wmissing-declarations \
-Wbad-function-cast -Wcast-align -Wnested-externs -Wunused \
-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE \
-DGSEAL_ENABLE -DG_DISABLE_DEPRECATED"


dnl ***********************************************************************
dnl * checking OS                                                         *
dnl ***********************************************************************

case $build_os in
    *linux*)
        AC_DEFINE_UNQUOTED(SYS_LINUX, 1, [This is a Linux system])
        LDFLAGS="$LDFLAGS -export-dynamic -no-undefined -module -lm"
    ;;
    *mingw*)
        AC_DEFINE_UNQUOTED(SYS_MINGW, 1, [This is a Windows system using MinGW])
        LDFLAGS="$LDFLAGS -export-dynamic -no-undefined -module -Wl,--export-all-symbols"
        CFLAGS="$CFLAGS -mms-bitfields -mwindows -mno-cygwin"
    ;;
esac


dnl ***********************************************************************
dnl * directories definitions                                             *
dnl ***********************************************************************
LOCALE_DIR="$prefix/share/locale"
AC_DEFINE_UNQUOTED(LOCALE_DIR, "$prefix/share/locale", [Locale Directory])


dnl ***********************************************************************
dnl * ./configure options                                                 *
dnl ***********************************************************************


dnl ***********************************************************************
dnl * debug option                                                        *
dnl ***********************************************************************
AC_ARG_ENABLE([debug],
     [  --enable-debug          Turn on debugging],
     [case "${enableval}" in
       yes) debug=true ;;
       no)  debug=false ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
     esac],[debug=false])
if test x$debug = xtrue
then
 AC_DEFINE_UNQUOTED(ENABLE_DEBUG, TRUE, [Debug mode On])
 CFLAGS="$CFLAGS -ggdb"
else
 AC_DEFINE_UNQUOTED(ENABLE_DEBUG, FALSE, [Debug mode Off])
 CFLAGS="$CFLAGS -ggdb"
fi


dnl ***********************************************************************
dnl * If we are running automake >= 1.11, we can                          *
dnl * have nicer compilation output. silent option                        *
dnl ***********************************************************************
AC_ARG_ENABLE([silent],
     [  --disable-silent        Disable quiet compilation],
     [case "${enableval}" in
       yes) silent=false ;;
       no)  silent=true ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --disable-silent]) ;;
     esac],[silent=false])
if test x$silent = xtrue
then
    AM_SILENT_RULES([no])
else
    AM_SILENT_RULES([yes])
fi


dnl ***********************************************************************
dnl * gcov option                                                         *
dnl ***********************************************************************
AC_ARG_ENABLE([gcov],
     [  --enable-gcov           Enables gcov coverage tests],
     [case "${enableval}" in
       yes) gcov=true ;;
       no)  gcov=false ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-gcov]) ;;
     esac],[gcov=false])
if test x$gcov = xtrue
then
 CFLAGS="-g -fprofile-arcs -ftest-coverage $CFLAGS "
else
 CFLAGS="$CFLAGS"
fi


dnl ***********************************************************************
dnl * gprof option                                                        *
dnl ***********************************************************************
AC_ARG_ENABLE([gprof],
     [  --enable-gprof          Enables gprof profiling],
     [case "${enableval}" in
       yes) gprof=true ;;
       no)  gprof=false ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-gprof]) ;;
     esac],[gprof=false])
if test x$gprof = xtrue
then
 CFLAGS="-pg $CFLAGS"
else
 CFLAGS="$CFLAGS"
fi

REVISION=`git rev-parse --short --revs-only HEAD 2>/dev/null || echo 0`
AC_DEFINE_UNQUOTED(REVISION, "$REVISION", [git revision hash])

dnl ***********************************************************************
dnl * exporting variables                                                 *
dnl ***********************************************************************
AC_SUBST(LDFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)
AC_SUBST(SQLITE_CFLAGS)
AC_SUBST(SQLITE_LIBS)
AC_SUBST(JANSSON_CFLAGS)
AC_SUBST(JANSSON_LIBS)
AC_SUBST(MHD_CFLAGS)
AC_SUBST(MHD_LIBS)
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)


AC_CONFIG_FILES([
Makefile
libsauvegarde/Makefile
monitor/Makefile
serveur/Makefile
restaure/Makefile
po/Makefile.in
libsauvegarde/libcdpfgl.pc
])

AC_OUTPUT

AC_MSG_NOTICE([

 *** Flags that will be used to compile Sauvegarde ***

 CFLAGS         : ${CFLAGS}
 LDFLAGS        : ${LDFLAGS}

 GLIB CFLAGS    : ${GLIB_CFLAGS}
 GLIB LIBS      : ${GLIB_LIBS}

 GIO CFLAGS     : ${GIO_CFLAGS}
 GIO LIBS       : ${GIO_LIBS}

 SQLITE CFLAGS  : ${SQLITE_CFLAGS}
 SQLITE LIBS    : ${SQLITE_LIBS}

 JANSSON CFLAGS : ${JANSSON_CFLAGS}
 JANSSON LIBS   : ${JANSSON_LIBS}

 MHD CFLAGS     : ${MHD_CFLAGS}
 MHD LIBS       : ${MHD_LIBS}

 CURL CFLAGS     : ${CURL_CFLAGS}
 CURL LIBS       : ${CURL_LIBS}

 *** Dumping configuration ***

     - Build For OS             : $build_os
     - Compiler                 : ${CC}
     - Prefix                   : $prefix
     - Exec prefix              : ${exec_prefix}
     - Locale dir               : ${LOCALE_DIR}
     - git revision             : ${REVISION}

     - Options
       . is code profiling on ?       : $gprof
       . Code coverage is on ?        : $gcov
       . debbuging is on ?            : $debug
       . Disabled quiet compilation ? : $silent

You can now run 'make' to compile Sauvegarde.

])

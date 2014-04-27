AC_PREREQ(2.59)

AC_INIT([Sauvegarde], [0.0.1],
        [Olivier DELHOMME <olivier.delhomme@free.fr>],
        [Sauvegarde])

AM_INIT_AUTOMAKE([$PACKAGE_TARNAME], [$PACKAGE_VERSION])

AC_PROG_CC
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_INTLTOOL([0.23])


dnl **************************************************
dnl * Put the preprocessor macros in a header file   *
dnl * instead of command line                        *
dnl **************************************************
AC_CONFIG_HEADER([config.h])

dnl **************************************************
dnl * Libraries requirements                         *
dnl **************************************************
PKG_PROG_PKG_CONFIG([0.10])
GLIB_VERSION=2.26.0
GIO_VERSION=2.26.0
SQLITE_VERSION=3.6.20
ZMQ_VERSION=3.2.4

AC_SUBST(GLIB_VERSION)
AC_SUBST(GIO_VERSION)

dnl **************************************************
dnl * Build time sanity check                        *
dnl **************************************************
AM_SANITY_CHECK

dnl **************************************************
dnl * Initialize maintainer mode                     *
dnl **************************************************
AM_MAINTAINER_MODE

dnl **************************************************
dnl * Checks for headers                             *
dnl **************************************************
AC_HEADER_STDC

dnl **************************************************
dnl * Checks dynamic libraries capabilities          *
dnl **************************************************
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

dnl **************************************************
dnl * languages                                      *
dnl **************************************************
ALL_LINGUAS="fr"

dnl **************************************************
dnl * triggers gettext                               *
dnl **************************************************
GETTEXT_PACKAGE=sauvegarde
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext Package used])
AM_GLIB_GNU_GETTEXT

dnl **************************************************
dnl * checking for modules (glib, gio, sqlite, zmq)  *
dnl **************************************************
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= $GLIB_VERSION])
PKG_CHECK_MODULES(GIO, [gio-2.0 >= $GIO_VERSION])
PKG_CHECK_MODULES(SQLITE, [sqlite3 >= $SQLITE_VERSION])
PKG_CHECK_MODULES(ZMQ, [libzmq >= $ZMQ_VERSION])

AC_PROG_INSTALL

CFLAGS="$CFLAGS -Wall -Wstrict-prototypes -Wmissing-declarations \
-Wbad-function-cast -Wcast-align -Wnested-externs -Wunused \
-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE \
-DGSEAL_ENABLE -DG_DISABLE_DEPRECATED"


dnl **************************************************
dnl * checking OS                                    *
dnl **************************************************

case $build_os in
    *linux*)
        AC_DEFINE_UNQUOTED(SYS_LINUX, 1, [This is a Linux system])
        LDFLAGS="$LDFLAGS -export-dynamic -no-undefined -module"
    ;;
    *mingw*)
        AC_DEFINE_UNQUOTED(SYS_MINGW, 1, [This is a Windows system using MinGW])
        LDFLAGS="$LDFLAGS -export-dynamic -no-undefined -module -Wl,--export-all-symbols"
        CFLAGS="$CFLAGS -mms-bitfields -mwindows -mno-cygwin"
    ;;
esac


dnl **************************************************
dnl * directories definitions                        *
dnl **************************************************
LOCALE_DIR="$prefix/share/locale"
AC_DEFINE_UNQUOTED(LOCALE_DIR, "$prefix/share/locale", [Locale Directory])


dnl **************************************************
dnl * ./configure options                            *
dnl **************************************************

dnl **************************************************
dnl * debug option                                   *
dnl * At this early stage debugging IS activated     *
dnl * by default                                     *
dnl **************************************************
AC_ARG_ENABLE([debug],
     [  --enable-debug         Turn on debugging],
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


dnl **************************************************
dnl * gcov option                                    *
dnl **************************************************
AC_ARG_ENABLE([gcov],
     [  --enable-gcov          Enables gcov coverage tests],
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


dnl **************************************************
dnl * gprof option                                   *
dnl **************************************************
AC_ARG_ENABLE([gprof],
     [  --enable-gprof         Enables gprof profiling],
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

dnl **************************************************
dnl * exporting variables                            *
dnl **************************************************
AC_SUBST(LDFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)
AC_SUBST(SQLITE_CFLAGS)
AC_SUBST(SQLITE_LIBS)
AC_SUBST(ZMQ_CFLAGS)
AC_SUBST(ZMQ_LIBS)


AC_CONFIG_FILES([
Makefile
libsauvegarde/Makefile
monitor/Makefile
serveur/Makefile
po/Makefile.in
])
AC_OUTPUT
echo
echo
echo "*** Flags that will be used to compile Sauvegarde ***"
echo
echo "CFLAGS        : ${CFLAGS}"
echo "LDFLAGS       : ${LDFLAGS}"
echo
echo "GLIB CFLAGS   : ${GLIB_CFLAGS}"
echo "GLIB LIBS     : ${GLIB_LIBS}"
echo "GIO CFLAGS    : ${GIO_CFLAGS}"
echo "GIO LIBS      : ${GIO_LIBS}"
echo "SQLITE CFLAGS : ${SQLITE_CFLAGS}"
echo "SQLITE LIBS   : ${SQLITE_LIBS}"
echo "LIBZMQ CFLAGS : ${ZMQ_CFLAGS}"
echo "LIBZMP LIBS   : ${ZMQ_LIBS}"
echo
echo
echo "*** Dumping configuration ***"
echo
echo "    - Build For OS             : $build_os"
echo "    - Compiler                 : ${CC}"
echo "    - Options"
echo "      . is code profiling on ? : $gprof"
echo "      . Code coverage is on ?  : $gcov"
echo "      . debbuging is on ?      : $debug"
echo "    - Prefix                   : $prefix"
echo "    - Exec prefix              : ${exec_prefix}"
echo "    - Locale dir               : ${LOCALE_DIR}"
echo
echo "    You can now run 'make -s' to compile Sauvegarde"
echo

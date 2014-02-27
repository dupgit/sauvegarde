
pkglib_LTLIBRARIES = libsauvegarde.la

headerfiles = libsauvegarde.h

libsauvegarde_la_SOURCES =	libsauvegarde.c 			\
							$(headerfiles)

libsauvegarde_la_CFLAGS = $(CFLAGS) $(GLIB_CFLAGS) $(GIO_CFLAGS)

AM_LDFLAGS = $(GLIB_LIBS) $(GIO_LIBS)

include_HEADERS = $(headerfiles)

libsauvegardedir =$(libdir)/sauvegarde



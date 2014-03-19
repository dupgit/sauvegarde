
pkglib_LTLIBRARIES = libsauvegarde.la

headerfiles = libsauvegarde.h	\
			  configuration.h	\
			  communique.h		\
			  files.h			\
			  hashs.h

libsauvegarde_la_SOURCES =	libsauvegarde.c 	\
							configuration.c		\
							communique.c		\
							files.c				\
							hashs.c				\
							$(headerfiles)

libsauvegarde_la_CFLAGS = $(CFLAGS) $(GLIB_CFLAGS) $(GIO_CFLAGS)	\
						  $(ZMQ_CFLAGS) $(SQLITE_CFLAGS)

AM_LDFLAGS = $(LDFLAGS) $(GLIB_LIBS) $(GIO_LIBS) $(ZMQ_LIBS) $(SQLITE_LIBS)

include_HEADERS = $(headerfiles)

libsauvegardedir =$(libdir)/sauvegarde



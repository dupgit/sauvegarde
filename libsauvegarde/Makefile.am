
pkglib_LTLIBRARIES = libsauvegarde.la

headerfiles = libsauvegarde.h	\
			  configuration.h	\
			  communique.h		\
			  files.h			\
			  hashs.h			\
			  packing.h			\
			  database.h

libsauvegarde_la_SOURCES =	libsauvegarde.c 	\
							configuration.c		\
							communique.c		\
							files.c				\
							hashs.c				\
							database.c			\
							packing.c			\
							$(headerfiles)

libsauvegarde_la_CFLAGS = $(CFLAGS) $(GLIB_CFLAGS) $(GIO_CFLAGS) \
						  $(ZMQ_CFLAGS) $(SQLITE_CFLAGS)         \
						  $(JANSSON_CFLAGS)

AM_LDFLAGS = $(LDFLAGS) $(GLIB_LIBS) $(GIO_LIBS) $(ZMQ_LIBS) \
             $(SQLITE_LIBS) $(JANSSON_LIBS)

include_HEADERS = $(headerfiles)

libsauvegardedir =$(libdir)/sauvegarde



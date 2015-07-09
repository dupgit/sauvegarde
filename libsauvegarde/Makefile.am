pkglib_LTLIBRARIES = libsauvegarde.la

headerfiles = libsauvegarde.h	\
			  configuration.h	\
			  communique.h		\
			  files.h			\
			  hashs.h			\
			  packing.h			\
			  database.h		\
			  query.h

libsauvegarde_la_SOURCES = libsauvegarde.c 	    \
						   configuration.c		\
						   communique.c		    \
						   files.c				\
						   hashs.c				\
						   database.c			\
						   packing.c			\
						   unpacking.c			\
						   query.c				\
						   $(headerfiles)

libsauvegarde_la_CFLAGS = $(CFLAGS) $(GLIB_CFLAGS) $(GIO_CFLAGS)   \
                          $(SQLITE_CFLAGS) $(JANSSON_CFLAGS)       \
                          $(MHD_CFLAGS) $(CURL_CFLAGS)

AM_LDFLAGS = $(LDFLAGS) $(GLIB_LIBS) $(GIO_LIBS) $(SQLITE_LIBS)    \
             $(JANSSON_LIBS) $(MHD_LIBS) $(CURL_LIBS)

include_HEADERS = $(headerfiles)

libsauvegardedir =$(libdir)/sauvegarde



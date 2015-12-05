pkglib_LTLIBRARIES = libcdpfgl.la

headerfiles = libsauvegarde.h	\
			  configuration.h	\
			  communique.h		\
			  files.h			\
			  hashs.h			\
			  packing.h			\
			  database.h		\
			  query.h			\
			  clock.h

libcdpfgl_la_SOURCES = libsauvegarde.c 	    \
						   configuration.c		\
						   communique.c		    \
						   files.c				\
						   hashs.c				\
						   database.c			\
						   packing.c			\
						   unpacking.c			\
						   query.c				\
						   clock.c              \
						   $(headerfiles)

libcdpfgl_la_CFLAGS = $(CFLAGS) $(GLIB_CFLAGS) $(GIO_CFLAGS)    \
                          $(SQLITE_CFLAGS) $(JANSSON_CFLAGS)    \
                          $(MHD_CFLAGS) $(CURL_CFLAGS)

AM_LDFLAGS = $(LDFLAGS) $(GLIB_LIBS) $(GIO_LIBS) $(SQLITE_LIBS) \
             $(JANSSON_LIBS) $(MHD_LIBS) $(CURL_LIBS)


includedir=$(prefix)/include/cdpfgl

include_HEADERS = $(headerfiles)

libcdpfgldir =$(libdir)/cdpfgl

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = libcdpfgl.pc
$(pkgconfig_DATA): ../config.status





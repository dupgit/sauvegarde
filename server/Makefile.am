bin_PROGRAMS = cdpfglserver

DEFS = -I../libcdpfgl $(GLIB_CFLAGS) $(GIO_CFLAGS)       \
	              $(JANSSON_CFLAGS) $(MHD_CFLAGS)    \
		      $(SQLITE_CFLAGS) $(CURL_CFLAGS)

cdpfglserver_LDFLAGS = $(LDFLAGS) -lm
cdpfglserver_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libcdpfgl -lcdpfgl \
		     $(JANSSON_LIBS) $(MHD_LIBS) $(SQLITE_LIBS)        \
		     $(CURL_LIBS)

cdpfglserver_HEADERFILES =  server.h        \
                            options.h       \
                            backend.h       \
                            file_backend.h  \
                            stats.h

cdpfglserver_SOURCES =  server.c                    \
			options.c                   \
			backend.c                   \
			file_backend.c              \
			stats.c			    \
			$(cdpfglserver_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS) $(JANSSON_CFLAGS) $(MHD_CFLAGS)

bin_PROGRAMS = cdpfglrestore

DEFS = -I../libcdpfgl $(GLIB_CFLAGS) $(GIO_CFLAGS)   \
       $(SQLITE_CFLAGS) $(JANSSON_CFLAGS) $(CURL_CFLAGS)

cdpfglrestore_LDFLAGS = $(LDFLAGS)
cdpfglrestore_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libcdpfgl -lcdpfgl \
		       $(SQLITE_LIBS) $(JANSSON_LIBS) $(CURL_LIBS)

cdpfglrestore_HEADERFILES =  restaure.h \
			     options.h

cdpfglrestore_SOURCES =  restaure.c                   \
			 options.c                    \
			 $(cdpfglrestore_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS) $(SQLITE_CFLAGS) \
              $(JANSSON_CFLAGS) $(CURL_CFLAGS)

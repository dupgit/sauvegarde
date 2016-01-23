bin_PROGRAMS = cdpfglrestore

DEFS = -I../libcdpfgl $(GLIB_CFLAGS) $(GIO_CFLAGS)       \
       $(JANSSON_CFLAGS) $(CURL_CFLAGS)

cdpfglrestore_LDFLAGS = $(LDFLAGS)
cdpfglrestore_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libcdpfgl -lcdpfgl \
		      $(JANSSON_LIBS) $(CURL_LIBS)

cdpfglrestore_HEADERFILES =  restore.h \
			     options.h

cdpfglrestore_SOURCES =  restore.c                    \
			 options.c                    \
			 $(cdpfglrestore_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)     \
              $(JANSSON_CFLAGS) $(CURL_CFLAGS)

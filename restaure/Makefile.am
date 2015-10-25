bin_PROGRAMS = cdpfglrestore

DEFS = -I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS)   \
       $(SQLITE_CFLAGS) $(JANSSON_CFLAGS) $(CURL_CFLAGS)

cdpfglrestore_LDFLAGS = $(LDFLAGS)
cdpfglrestore_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde \
		       $(SQLITE_LIBS) $(JANSSON_LIBS) $(CURL_LIBS)

cdpfglrestore_HEADERFILES = restaure.h	   \
			     options.h

cdpfglrestore_SOURCES = restaure.c 		   \
			 options.c  		   \
			 $(restaure_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS) $(SQLITE_CFLAGS) \
              $(JANSSON_CFLAGS) $(CURL_CFLAGS)

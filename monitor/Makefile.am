bin_PROGRAMS = client

DEFS = -I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS)   \
	   $(SQLITE_CFLAGS) $(JANSSON_CFLAGS) $(CURL_CFLAGS)

client_LDFLAGS = $(LDFLAGS)
client_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde \
			   $(SQLITE_LIBS) $(JANSSON_LIBS) $(CURL_LIBS)

client_HEADERFILES = monitor.h            \
                     options.h			  \
                     m_fanotify.h

client_SOURCES = monitor.c                \
				 options.c				  \
				 m_fanotify.c			  \
				 $(client_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

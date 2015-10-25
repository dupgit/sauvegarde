bin_PROGRAMS = cdpfglclient

DEFS = -I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS)   \
	   $(SQLITE_CFLAGS) $(JANSSON_CFLAGS) $(CURL_CFLAGS)

cdpfglclient_LDFLAGS = $(LDFLAGS)
cdpfglclient_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde \
					  $(SQLITE_LIBS) $(JANSSON_LIBS) $(CURL_LIBS)

cdpfglclient_HEADERFILES = monitor.h            \
							options.h			  \
							m_fanotify.h

cdpfglclient_SOURCES = monitor.c                \
						options.c				  \
						m_fanotify.c			  \
						$(client_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

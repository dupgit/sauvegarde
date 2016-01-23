bin_PROGRAMS = cdpfglclient

DEFS = -I../libcdpfgl $(GLIB_CFLAGS) $(GIO_CFLAGS)   \
	   $(JANSSON_CFLAGS) $(CURL_CFLAGS)

cdpfglclient_LDFLAGS = $(LDFLAGS)
cdpfglclient_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libcdpfgl -lcdpfgl \
					 $(JANSSON_LIBS) $(CURL_LIBS)

cdpfglclient_HEADERFILES =  client.h       \
							options.h      \
							m_fanotify.h

cdpfglclient_SOURCES =  client.c                    \
						options.c                   \
						m_fanotify.c                \
						$(cdpfglclient_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS) $(JANSSON_CFLAGS) $(CURL_CFLAGS)

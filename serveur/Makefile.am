bin_PROGRAMS = cdpfglserver

DEFS = -I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS)   \
	    $(SQLITE_CFLAGS) $(JANSSON_CFLAGS) $(MHD_CFLAGS)

cdpfglserver_LDFLAGS = $(LDFLAGS)
cdpfglserver_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde \
					  $(SQLITE_LIBS) $(JANSSON_LIBS) $(MHD_LIBS)

cdpfglserver_HEADERFILES = serveur.h			\
							options.h			\
							backend.h			\
							file_backend.h

cdpfglserver_SOURCES = serveur.c             \
						options.c				\
						backend.c				\
						file_backend.c		\
						$(cdpfglserver_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

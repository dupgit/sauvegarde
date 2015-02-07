bin_PROGRAMS = serveur

DEFS =	-I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS) $(ZMQ_CFLAGS)     \
	    $(CZMQ_FLAGS) $(SQLITE_CFLAGS) $(JANSSON_CFLAGS) $(MHD_CFLAGS)

serveur_LDFLAGS = $(LDFLAGS)
serveur_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde \
		        $(ZMQ_LIBS) $(CZMQ_LIBS) $(SQLITE_LIBS) $(JANSSON_LIBS)   \
		        $(MHD_LIBS)

SER_HEADERFILES = serveur.h
				  options.h

serveur_SOURCES = serveur.c                \
				  options.c				   \
		          $(SER_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

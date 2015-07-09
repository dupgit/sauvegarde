bin_PROGRAMS = serveur

DEFS = -I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS)   \
	    $(SQLITE_CFLAGS) $(JANSSON_CFLAGS) $(MHD_CFLAGS)

serveur_LDFLAGS = $(LDFLAGS)
serveur_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde \
		        $(SQLITE_LIBS) $(JANSSON_LIBS) $(MHD_LIBS)

serveur_HEADERFILES = serveur.h			\
				      options.h			\
				      backend.h			\
				      file_backend.h

serveur_SOURCES = serveur.c             \
				  options.c				\
				  backend.c				\
				  file_backend.c		\
		          $(serveur_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

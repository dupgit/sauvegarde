bin_PROGRAMS = restaure

DEFS =	-I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS)   \
        $(SQLITE_CFLAGS) $(JANSSON_CFLAGS) $(CURL_CFLAGS)

restaure_LDFLAGS = $(LDFLAGS)
restaure_LDADD =  $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde \
		  $(SQLITE_LIBS) $(JANSSON_LIBS) $(CURL_LIBS)

RESTAURE_HEADERFILES = 	restaure.h		\
			options.h				 

restaure_SOURCES = 	restaure.c		\
			options.c		\
			$(RESTAURE_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

bin_PROGRAMS = client

DEFS =	-I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS) $(ZMQ_CFLAGS)     \
		$(CZMQ_FLAGS) $(SQLITE_CFLAGS) $(JANSSON_CFLAGS)

client_LDFLAGS = $(LDFLAGS)
client_LDADD =  $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde \
				$(ZMQ_LIBS) $(CZMQ_LIBS) $(SQLITE_LIBS) $(JANSSON_LIBS)

MON_HEADERFILES = monitor.h               \
                  options.h				  \
                  ciseaux.h				  \
                  antememoire.h

client_SOURCES =  monitor.c               \
				  options.c				  \
				  ciseaux.c				  \
				  antememoire.c		      \
				  $(MON_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

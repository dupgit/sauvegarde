bin_PROGRAMS = ciseaux

DEFS =  -I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS) $(ZMQ_CFLAGS) \
		$(SQLITE_CFLAGS)

ciseaux_LDFLAGS = $(LDFLAGS)
ciseaux_LDADD = $(GLIB_LIBS) $(GIO_LIBS) -L../libsauvegarde -lsauvegarde \
				$(ZMQ_LIBS) $(SQLITE_LIBS)

CIS_HEADERFILES = ciseaux.h               \
                  options.h				 

ciseaux_SOURCES = ciseaux.c               \
				  options.c
				  $(MON_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

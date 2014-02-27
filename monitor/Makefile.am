bin_PROGRAMS = monitor

DEFS =	-I../libsauvegarde $(GLIB_CFLAGS) $(GIO_CFLAGS) $(ZMQ_CFLAGS) \
		$(SQLITE_CFLAGS)

monitor_LDFLAGS = $(LDFLAGS)
monitor_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde \
				$(ZMQ_LIBS) $(SQLITE_LIBS)

MON_HEADERFILES = monitor.h               \
                  options.h				  \
                  path.h

monitor_SOURCES = monitor.c               \
				  options.c				  \
				  path.c				  \
				  $(MON_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

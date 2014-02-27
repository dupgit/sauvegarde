bin_PROGRAMS = monitor

DEFS = -I../libsauvegarde

monitor_LDFLAGS = $(LDFLAGS)
monitor_LDADD = $(GLIB_LIBS) $(GIO_LIBS)  -L../libsauvegarde -lsauvegarde

MON_HEADERFILES = monitor.h               \
                  options.h				  \
                  path.h

monitor_SOURCES = monitor.c               \
				  options.c				  \
				  path.c				  \
				  $(MON_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

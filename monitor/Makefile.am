bin_PROGRAMS = monitor

monitor_LDFLAGS = $(LDFLAGS)
monitor_LDADD = $(GLIB_LIBS) $(GIO_LIBS)

MON_HEADERFILES = monitor.h               \
                  options.h				  \
                  path.h

monitor_SOURCES = monitor.c               \
				  options.c				  \
				  path.c				  \
				  $(MON_HEADERFILES)

AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)

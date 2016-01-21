SUBDIRS = libcdpfgl server client restore pixmaps po

ACLOCAL_AMFLAGS = -I m4

cdpfglconfdir = $(sysconfdir)/cdpfgl
cdpfglconf_DATA = client.conf    \
			      server.conf    \
			      restore.conf

EXTRA_DIST = $(cdpfglconf_DATA)            \
			 libcdpfgl/libcdpfgl.pc.in     \
			 manual/installation.md        \
			 docs/coding_in_cdpfgl.md      \
			 docs/API.md                   \
			 docs/infrastructure.md        \
			 docs/cdpfgl.doxygen		   \
			 autogen.sh					   \
			 LICENSE

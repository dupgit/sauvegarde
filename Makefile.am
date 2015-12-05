SUBDIRS = libsauvegarde serveur monitor restaure po

ACLOCAL_AMFLAGS = -I m4

cdpfglconfdir = $(sysconfdir)/cdpfgl
cdpfglconf_DATA = client.conf    \
			      server.conf    \
			      restore.conf

EXTRA_DIST = $(cdpfglconf_DATA)                \
			 libsauvegarde/libcdpfgl.pc.in     \
			 manual/installation.md            \
			 docs/coding_in_sauvegarde.md 	   \
			 docs/API.md                       \
			 docs/infrastructure.md            \
			 docs/sauvegarde.doxygen		   \
			 autogen.sh						   \
			 LICENSE

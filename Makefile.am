SUBDIRS = libsauvegarde serveur monitor restaure po

ACLOCAL_AMFLAGS = -I m4

sauvegardeconfdir = $(sysconfdir)/sauvegarde
sauvegardeconf_DATA = client.conf		\
					  server.conf	    \
					  restore.conf

EXTRA_DIST = $(sauvegardeconf_DATA)            \
			 libsauvegarde/libsauvegarde.pc.in \
			 manual/installation.md            \
			 docs\coding_in_sauvegarde.md 	   \
			 docs\API.md                       \
			 docs\infrastructure.md            \
			 docs\sauvegarde.doxygen

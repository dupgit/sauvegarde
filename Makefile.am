SUBDIRS = libsauvegarde serveur monitor restaure po

ACLOCAL_AMFLAGS = -I m4

sauvegardeconfdir = $(sysconfdir)/sauvegarde
sauvegardeconf_DATA = client.conf		\
					  server.conf	    \
					  restore.conf

EXTRA_DIST = $(sauvegardeconf_DATA)            \
			 libsauvegarde/libsauvegarde.pc.in

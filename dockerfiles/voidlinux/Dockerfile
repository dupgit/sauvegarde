FROM voidlinux/voidlinux
MAINTAINER Olivier Delhomme <olivier.delhomme@free.fr>
RUN xbps-install -A -y -S ; \
    xbps-install -A -y -S jansson-devel; \
    xbps-install -A -y -S glib-devel; \
    xbps-remove -F ncurses-6.0_1; \
    xbps-install -A -y -S libmicrohttpd-devel; \
    xbps-install -A -y -S automake; \
    xbps-install -A -y -S intltool; \
    xbps-install -A -y -S libtool; \
    xbps-install -A -y -S gettext-devel; \
    xbps-install -A -y -S gcc; \
    xbps-install -A -y -S pkg-config; \
    xbps-install -A -y -S sqlite-devel; \
    xbps-install -A -y -S git; \
    xbps-remove -F libressl-2.2.4_3; \
    xbps-install -A -y -S libcurl-devel
RUN cd; git clone https://github.com/dupgit/sauvegarde.git;

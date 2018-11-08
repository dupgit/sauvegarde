FROM voidlinux/voidlinux

LABEL maintainer="Olivier Delhomme <olivier.delhomme@free.fr>"

RUN xbps-install -A -y -S ; \
    xbps-install -A -y -S jansson-devel; \
    xbps-install -A -y -S glib-devel; \
    xbps-remove -F -y ncurses-6.0_1; \
    xbps-install -A -y -S libmicrohttpd-devel; \
    xbps-install -A -y -S automake; \
    xbps-install -A -y -S intltool; \
    xbps-install -A -y -S libtool; \
    xbps-install -A -y -S gettext-devel; \
    xbps-install -A -y -S gcc; \
    xbps-install -A -y -S pkg-config; \
    xbps-install -A -y -S sqlite-devel; \
    xbps-install -A -y -S git; \
    xbps-remove -F -y libressl-2.2.4_3; \
    xbps-install -A -y -S make; \
    xbps-install -A -y -S libcurl-devel

RUN cd; git clone https://github.com/dupgit/sauvegarde.git; cd sauvegarde; ./autogen.sh && ./configure && make && make check && make install;

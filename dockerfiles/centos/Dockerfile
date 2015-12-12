FROM centos:latest
MAINTAINER Olivier Delhomme <olivier.delhomme@free.fr>
ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
RUN yum -y update
RUN yum -y install git autoconf automake libtool make sqlite-devel glib2-devel libmicrohttpd-devel libcurl-devel intltool
RUN git clone git://github.com/akheron/jansson.git; cd jansson; autoreconf -f -i; ./configure --enable-silent-rules; make; make install; cd ..; rm -fr jansson;
RUN git clone https://github.com/dupgit/sauvegarde.git; cd sauvegarde; ./autogen.sh; ./configure; make; make install;
CMD ["/usr/sbin/init"]

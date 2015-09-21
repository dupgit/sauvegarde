# Coding into this project

Any help on the project will be very welcomed and appreciated. Have a
look at the TODO file, the Roadmap in this file or the @todo notes in
the code if you do not know where to begin or what to do. Translations
are welcomed !


## irc

When I'm coding into this project I'm hanging onto #sauvegarde irc channel
on oftc servers.


## Comments

Sauvegarde uses Javadoc comment style to be parsed by doxygen. Please
continue to use this style into comments in the whole project as it will
be parsed by doxygen. doxygen has been configured to figure out which
parameters are not documented. Please have a look at doxygen output and
avoid any warning. If you want the documentation not to include the whole
path for files please have a look at sauvegarde.doxygen file and modify
STRIP_FROM_PATH variable with your path (where sauvegarde's sources are).


## Verifying complexity

I discovered 'lizard' a simple but excellent python program that checks the
complexity of a program (ok, how complex it look like because it does not
take into account preprocessor macros...). If you plan to hack into this
project please look at the result of this program (you may find it
following this link: [https://github.com/terryyin/lizard](https://github.com/terryyin/lizard))


# Learnt things from experiments

* GFileMonitor in Glib 2.26 does not warn when the file descriptors are
  exhausted. The default limit on my system is 1024 files monitored
* inotify interface does warn when one can not add a new watch. It says
  that the disk is full ! The limit on my system is 8192 watchs at a time.
* We should use another mechanism to monitor a whole filesystem as limits
  for GFileMonitor and inotify techniques are very low compared to the
  number of directories a filesystem might have (my /home have 37826
  directories !). We might want to look at redirfs
  (http://www.redirfs.org/tiki-index.php) or pluginfs
  (http://www.pluginfs.org/).
* redirfs, pluginfs and dazuko seems dead (on 02 march 2014). As of
  28.03.2015 we can notice that:
 * redirfs has its last commit in Jun 2014
 * pluginfs.org is no more accessible (and all ressources it had). Found
   plugins on github along with redirfs !!:
  * [https://github.com/fhrbata/pluginfs](https://github.com/fhrbata/pluginfs)
  * [https://github.com/fhrbata/redirfs](https://github.com/fhrbata/redirfs)
 * dazuko still seems dead since 06.09.2008
* fanotify can not handle file permission changes (chmod and chown) nor
  file deletion (rm). This is really anoying if one wants restore a
  directory at a specified time (and we can not decide if a file was
  deleted at that time.
* It seems (on my system) that the more the blocksize is small the more the
  intra-deduplication rate is high (from 2.56 % at 32768 bytes to 8% at 512
  bytes). But if the blocksize is smaller than cpu overhead is higher and
  the maximum filesystem size is smaller... For the default value we will
  have to find a value that will fit best intra-deduplication rate, cpu
  demand, maximum filesystem size and low memory consumption. For now the
  choosen value is 16384 bytes.
* It seems that it will be difficult to get things via 'simple' message
  passing between threads -> Use of GAsyncQueue that does all that is
  needed.
* I do not feel confortable with msgpack library. I miss documentation. I
  switched to jansson that is better documented and fills my needs.
* 0MQ is overkill for my needs. I switched to libcurl and libmicrohttpd.


# May be usefull links

* Filesystem events notifycation:
 * redirfs        : [http://www.redirfs.org/tiki-index.php](http://www.redirfs.org/tiki-index.php)
 * pluginfs       : [http://www.pluginfs.org/](http://www.pluginfs.org/)
 * dazuko         : [http://dazuko.dnsalias.org/wiki/index.php/Main_Page](http://dazuko.dnsalias.org/wiki/index.php/Main_Page)
 * fanotify       : [http://www.xypron.de/projects/fanotify-manpages/man7/fanotify.7.html](http://www.xypron.de/projects/fanotify-manpages/man7/fanotify.7.html)
* sqlite          :
 * main site      :[http://www.sqlite.org/cintro.html](http://www.sqlite.org/cintro.html)
 * firefox add on : [https://addons.mozilla.org/fr/firefox/addon/sqlite-manager/](https://addons.mozilla.org/fr/firefox/addon/sqlite-manager/)
* zmq             : [http://czmq.zeromq.org/](http://czmq.zeromq.org/)
* data packing over the wire:
 * msgpack        : [http://msgpack.org/](http://msgpack.org/)
 * jansson        : [http://www.digip.org/jansson/](http://www.digip.org/jansson/). See also the RFC 7159: [http://tools.ietf.org/html/rfc7159.html](http://tools.ietf.org/html/rfc7159.html)
* doxygen         : [http://www.stack.nl/~dimitri/doxygen/index.html](http://www.stack.nl/~dimitri/doxygen/index.html)
* Software quality checkers:
 * lizard         : [https://github.com/terryyin/lizard](https://github.com/terryyin/lizard)
* Third party libraries:
 * libmicrohttpd   : [https://www.gnu.org/software/libmicrohttpd/](https://www.gnu.org/software/libmicrohttpd/)
 * libevent        : [http://libevent.org/](http://libevent.org/)
* Similar projects:
 * clsync          : [https://github.com/xaionaro/clsync](https://github.com/xaionaro/clsync)
 * data-14         : [http://www.data-14.org/](http://www.data-14.org/)
* other dependencies:
 * libcurl         : [http://curl.haxx.se/](http://curl.haxx.se/)
 * glib            : [https://wiki.gnome.org/Projects/GLib](https://wiki.gnome.org/Projects/GLib)
* Distribution doc:
 * Debian          : [https://www.debian.org/devel/wnpp/](https://www.debian.org/devel/wnpp/)
 * Gentoo          : [https://wiki.gentoo.org/wiki/Submitting_ebuilds](https://wiki.gentoo.org/wiki/Submitting_ebuilds)
* JOSE:
 * [https://securityblog.redhat.com/2015/04/01/jose-json-object-signing-and-encryption/](https://securityblog.redhat.com/2015/04/01/jose-json-object-signing-and-encryption/)
 * [https://www.iana.org/assignments/jose/jose.xhtml](https://www.iana.org/assignments/jose/jose.xhtml)

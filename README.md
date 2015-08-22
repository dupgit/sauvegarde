# Sauvegarde

Continuous data protection for GNU/Linux.

This page is sauvegarde doxygen's @mainpage.

[![Build Status](https://travis-ci.org/dupgit/sauvegarde.png?branch=master)](https://travis-ci.org/dupgit/sauvegarde)

With this code you can backup and restore files in a live and continuous
way. Compile the code put the `serveur` on the machine where you want to
save your files. Let `client` crawl your files to save them. If needed
use `restaure` to restore a file.


## License

This is free software an real open source as GPLv3 is used for this
collection of programs. Feel free to contibute and/or give help to the
projects used here (see dependencies section below).


## Dependencies

This project depends on the following projects (minimum version required is
stated between brakets):

* `autotools`      (2.59)
* `glib` and `gio` (2.26)
* `libmicrohttpd`  (0.9.5)
* `libcurl`        (7.22.0)
* `sqlite`         (3.6.20)
* `jansson`        (2.5)

doxygen (1.6.1) is used to generate code's documentation but is not
required to build the project. jansson's library version is quite recent
but it compiles nicely and depends on nothing (as far as I know).
Compiling sauvegarde also needs gnutls development files (used by
libmicrohttpd). Please have a look at .travis.yml file in order to see how
to compile dependencies before compiling sauvegarde.

Sauvegarde's project is known to compile (sometimes at the expense of
recompiling and installing newer versions of dependencies) under Centos 7,
Debian Jessie, Ubuntu 12.04 LTS and raspbian 3 and on x86_64 and arm7l
architectures. Please let me know if you have compiled Sauvegarde
successfully in a system that is not listed here.


## Download

You can download the source code from github or a packed defined version
at [http://src.delhomme.org/download/sauvegarde/releases/](http://src.delhomme.org/download/sauvegarde/releases/)


## Installation

If you had the code from the git repository you may generate the configure
script by invoking `./autogen.sh` script. It will execute aclocal, libtoolize
automake, autoconf, glib-gettextize and intltoolize for you. If everything
went ok you should have an executable configure script that you may use
in the classic way :

    ./configure --prefix=/my/local/install --enable-debug
    make
    make install


## Usage

First you need to run the program serveur on a machine that will act as
a server. You can configure options in `serveur.conf` (located by default
in `{prefix}/etc/sauvegarde/`) or use the command's line options:

    Usage:
      serveur [OPTION...]

    This program is monitoring file changes in the filesystem and is hashing
    files with SHA256 algorithms from Glib.

    Help Options:
      -h, --help                       Show help options

    Application Options:
      -v, --version                    Prints program version.
      -d, --debug=BOOLEAN              Activates (1) or desactivates (0) debug mode.
      -c, --configuration=FILENAME     Specify an alternative configuration file.
      -p, --port=NUMBER                Port NUMBER on which to listen.

Then you may use client program to report modified files to the serveur
and save them accordingly. Configuration file is named `client.conf` and
command line options are:

    Usage:
      client [OPTION...]

    This program is monitoring file changes in the filesystem and is hashing
    files with SHA256 algorithms from Glib.

    Help Options:
      -h, --help                       Show help options

    Application Options:
      -v, --version                    Prints program version
      -d, --debug=BOOLEAN              Activates (1) or desactivates (0) debug mode.
      -c, --configuration=FILENAME     Specify an alternative configuration file.
      -b, --blocksize=SIZE             Block SIZE used to compute hashs.
      -n, --noprint                    Quiets the program while calculating checksum.
      -r, --dircache=DIRNAME           Directory DIRNAME where to cache files.
      -f, --dbname=FILENAME            Database FILENAME.
      -i, --ip=IP                      IP address where serveur program is.
      -p, --port=NUMBER                Port NUMBER on which to listen.

At a time you may need to restaure a file then you'll have to use restaure
program. Configuration file is named `restaure.conf` and command line
options are:

    Usage:
      restaure [OPTION...]

    This program is restoring files from serveur's server.


    Help Options:
      -h, --help                       Show help options

    Application Options:
      -v, --version                    Prints program version.
      -l, --list=REGEX                 Gives a list of saved files that correspond to the given REGEX.
      -r, --restore=REGEX              Restore requested filename (REGEX) (by default latest version).
      -t, --date=DATE                  restores the selected file at that specific DATE.
      -d, --debug=BOOLEAN              Activates (1) or desactivates (0) debug mode.
      -c, --configuration=FILENAME     Specify an alternative configuration file.
      -i, --ip=IP                      IP address where serveur program is.
      -p, --port=NUMBER                Port NUMBER on which serveur program is listening.


## Roadmap

I am coding on spare hours so I may not tell any release dates and you
should consider that "It ships when its ready".

* 0.0.4  new serveur url to post a bunch of hashs and associated datas.
         Clean the answer list of needed hashs to avoid having duplicated
         hashs into it.
* 0.0.5  fanotify's code reviewed. Avoid having one entire file in memory
* 0.0.6  restore to some specific place
* 0.0.7  hability to exclude some files by extension or path
* 0.0.8  restore the latest version of a file before a specific date
* 0.0.9  restore all the versions of a file
* 0.0.10 restore a directory and it's subfiles and directories at a
         specific date.
* 0.1.0 nettoie program to clean serveur's datas and metas datas


### Releases

* 0.0.1  [08 07 2015] First usable version
* 0.0.2  [15.08.2015] client redesigned
* 0.0.3  [21.08.2015] saves and restores links


## Infrastucture

Some inside insights are in the [infrastructure](docs/infrastructure.md)
documentation as well for the [API](docs/API.md) part.


## Coding into this project

Any help on the project will be very welcomed and appreciated. Have a
look at the TODO file, the Roadmap in this file or the @todo notes in
the code if you do not know where to begin or what to do. Translations
are welcomed !


### irc

When I'm coding into this project I'm hanging onto #sauvegarde irc channel
on oftc servers.


### Comments

Sauvegarde uses Javadoc comment style to be parsed by doxygen. Please
continue to use this style into comments in the whole project as it will
be parsed by doxygen. doxygen has been configured to figure out which
parameters are not documented. Please have a look at doxygen output and
avoid any warning. If you want the documentation not to include the whole
path for files please have a look at sauvegarde.doxygen file and modify
STRIP_FROM_PATH variable with your path (where sauvegarde's sources are).


### Verifying complexity

I discovered 'lizard' a simple but excellent python program that checks the
complexity of a program (ok, how complex it look like because it does not
take into account preprocessor macros...). If you plan to hack into this
project please look at the result of this program (you may find it
following this link : [https://github.com/terryyin/lizard](https://github.com/terryyin/lizard))


## Learnt things from experiments

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
  28.03.2015 we can notice that :
 * redirfs has its last commit in Jun 2014
 * pluginfs.org is no more accessible (and all ressources it had). Found
   plugins on github along with redirfs !! :
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


## May be usefull links

* Filesystem events notifycation :
 * redirfs        : [http://www.redirfs.org/tiki-index.php](http://www.redirfs.org/tiki-index.php)
 * pluginfs       : [http://www.pluginfs.org/](http://www.pluginfs.org/)
 * dazuko         : [http://dazuko.dnsalias.org/wiki/index.php/Main_Page](http://dazuko.dnsalias.org/wiki/index.php/Main_Page)
 * fanotify       : [http://www.xypron.de/projects/fanotify-manpages/man7/fanotify.7.html](http://www.xypron.de/projects/fanotify-manpages/man7/fanotify.7.html)
* sqlite          :
 * main site      :[http://www.sqlite.org/cintro.html](http://www.sqlite.org/cintro.html)
 * firefox add on : [https://addons.mozilla.org/fr/firefox/addon/sqlite-manager/](https://addons.mozilla.org/fr/firefox/addon/sqlite-manager/)
* zmq             : [http://czmq.zeromq.org/](http://czmq.zeromq.org/)
* data packing over the wire :
 * msgpack        : [http://msgpack.org/](http://msgpack.org/)
 * jansson        : [http://www.digip.org/jansson/](http://www.digip.org/jansson/). See also the RFC 7159: [http://tools.ietf.org/html/rfc7159.html](http://tools.ietf.org/html/rfc7159.html)
* doxygen         : [http://www.stack.nl/~dimitri/doxygen/index.html](http://www.stack.nl/~dimitri/doxygen/index.html)
* Software quality checkers :
 * lizard         : [https://github.com/terryyin/lizard](https://github.com/terryyin/lizard)
* Third party libraries :
 * libmicrohttpd   : [https://www.gnu.org/software/libmicrohttpd/](https://www.gnu.org/software/libmicrohttpd/)
 * libevent        : [http://libevent.org/](http://libevent.org/)
* Similar projects :
 * clsync          : [https://github.com/xaionaro/clsync](https://github.com/xaionaro/clsync)
 * data-14         : [http://www.data-14.org/](http://www.data-14.org/)
* other dependencies :
 * libcurl         : [http://curl.haxx.se/](http://curl.haxx.se/)
 * glib            : [https://wiki.gnome.org/Projects/GLib](https://wiki.gnome.org/Projects/GLib)
* Distribution doc :
 * Debian          : [https://www.debian.org/devel/wnpp/](https://www.debian.org/devel/wnpp/)
 * Gentoo          : [https://wiki.gentoo.org/wiki/Submitting_ebuilds](https://wiki.gentoo.org/wiki/Submitting_ebuilds)

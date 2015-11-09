# Introduction

Sauvegarde is a set of programs that will save your files while being created and written to your disks. It will save your files from a client machine (containing the files to be saved) to a server machine (that will contain all the saved files and informations about thoses files). It will allow you to restore the saved files at their latest version or at any older versions.


# Getting sauvegarde

You can get sauvegarde downloading a specific release from http://src.delhomme.org/download/sauvegarde/releases/ or by cloning the github repository.


## Cloning from github

Use the following command to clone sauvegarde's git repository:

    $ git clone https://github.com/dupgit/sauvegarde.git
    Cloning into 'sauvegarde'...
    remote: Counting objects: 4369, done.
    remote: Compressing objects: 100% (59/59), done.
    remote: Total 4369 (delta 21), reused 0 (delta 0), pack-reused 4303
    Receiving objects: 100% (4369/4369), 1.48 MiB | 779.00 KiB/s, done.
    Resolving deltas: 100% (3347/3347), done.
    Checking connectivity... done.

When git is done you have to generate the configure script that we will use to install the project. First make sure that you have installed autotools, automake, autoconf, libtool and intltool.

    $ ./autogen.sh
    libtoolize: putting auxiliary files in `.'.
    libtoolize: linking file `./ltmain.sh'
    configure.ac:6: installing './compile'
    configure.ac:67: installing './config.guess'
    configure.ac:67: installing './config.sub'
    configure.ac:4: installing './install-sh'
    configure.ac:4: installing './missing'
    libsauvegarde/Makefile.am: installing './depcomp'
    Copying file po/Makefile.in.in

    Please add the files
      codeset.m4 gettext.m4 glibc21.m4 iconv.m4 isc-posix.m4 lcmessage.m4
      progtest.m4
    from the /usr/share/aclocal directory to your autoconf macro directory
    or directly to your aclocal.m4 file.
    You will also need config.guess and config.sub, which you can get from
    ftp://ftp.gnu.org/pub/gnu/config/.


    You can now run ./configure


## Downloading a tar.[xg]z

Download the latest release version from http://src.delhomme.org/download/sauvegarde/releases/ for instance v0.0.4 and decompress it:

    $ wget -c http://src.delhomme.org/download/sauvegarde/releases/sauvegarde-0.0.4.tar.xz
    --2015-09-13 14:16:55--  http://src.delhomme.org/download/sauvegarde/releases/sauvegarde-0.0.4.tar.xz
    Résolution de src.delhomme.org (src.delhomme.org)… 62.210.16.61
    Connexion à src.delhomme.org (src.delhomme.org)|62.210.16.61|:80… connecté.
    requête HTTP transmise, en attente de la réponse… 200 OK
    Taille : 296280 (289K) [application/x-tar]
    Sauvegarde en : « sauvegarde-0.0.4.tar.xz »

    sauvegarde-0.0.4.tar.xz                                    100%[=========================================================================================================================================>] 289,34K   659KB/s   ds 0,4s

    2015-09-13 14:16:55 (659 KB/s) — « sauvegarde-0.0.4.tar.xz » sauvegardé [296280/296280]
    $ tar Jxf sauvegarde-0.0.4.tar.xz
    $ cd sauvegarde-0.0.4

Before using the configure script to install sauvegarde you'll have to install all dependencies. If you forgot a dependency the configure script will tell you!


# Installing dependencies

You'll have to install the dependencies in order to be able to compile sauvegarde's project. Some dependencies are fine to be installed with the system's package manager some are too old and needs to be installed manually. In this section we will install everthing to /usr/local. Consider your onw case to choose the right installation directory.


## Install dependencies from the system (here for debian):

    $ sudo apt-get install gettext pkg-config libglib2.0-dev libcurl4-openssl-dev


## Install jansson, libmicrohttpd, sqlite:

### jansson

    $ git clone git://github.com/akheron/jansson.git
    $ cd jansson
    $ autoreconf -f -i
    $ CFLAGS=-Werror ./configure --prefix=/usr/local
    $ make
    $ sudo make install


### libmicrohttpd

    $ wget --quiet -c http://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.46.tar.gz
    $ tar zxf libmicrohttpd-0.9.46.tar.gz
    $ cd libmicrohttpd-0.9.46
    $ ./configure --prefix=/usr/local
    $ make
    $ sudo make install


### sqlite

    $ wget --quiet -c https://www.sqlite.org/2015/sqlite-autoconf-3090100.tar.gz
    $ tar zxf sqlite-autoconf-3090100.tar.gz
    $ cd sqlite-autoconf-3090100
    $ ./configure --prefix=/usr/local
    $ make
    $ sudo make install




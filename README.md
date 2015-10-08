# Sauvegarde

Continuous data protection for GNU/Linux.

This page is sauvegarde doxygen's @mainpage.

[![Build Status](https://travis-ci.org/dupgit/sauvegarde.png?branch=master)](https://travis-ci.org/dupgit/sauvegarde)

With this code you can backup and restore files in a live and continuous
way. Compile the code put the `serveur` on the machine where you want to
save your files. Let `client` crawl your files to save them. If needed
use `restaure` to restore a file.


## License

This is free software and real open source as GPLv3 is used for this
collection of programs and Creative Commons Attribution-ShareAlike 4.0
is used for the artwork. You'll find on LICENSE file in this directory
for the programs (GPL v3) and one in the pixmaps file for the artwork
(CC-BY-SA 4.0).

Feel free to contibute to sauvegarde's project and/or give help to the
projects used here (see dependencies section below).


## Dependencies

This project depends on the following projects (minimum version required is
stated between brakets):

* `autotools`      (2.59)
* `glib` and `gio` (2.30)
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
in the classic way:

    ./configure --prefix=/my/local/install --enable-debug
    make
    make install


## Usage

First you need to run the program `serveur` on a machine that will act as
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


Then you may use `client` program to report modified files to the serveur
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
      -r, --dircache=DIRNAME           Directory DIRNAME where to cache files.
      -f, --dbname=FILENAME            Database FILENAME.
      -i, --ip=IP                      IP address where serveur program is.
      -p, --port=NUMBER                Port NUMBER on which to listen.


At a time you may need to restore a file then you'll have to use `restaure`
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


## Manual

If you need more than the above hints, please have a look at the user
[manual](manual/installation.md).


## Roadmap

I am coding on spare hours so I may not tell any release dates and you
should consider that "It ships when its ready".

* 0.0.6  restore to some specific place. client adaptative blocksize with
         option to choose fixed or adaptative. Add options to choose
         CLIENT_MIN_BUFFER and CLIENT_SMALL_FILE_SIZE.
* 0.0.7  hability to exclude some files by extension or path. Caching
         mecanism in client in case the serveur is unreachable.
* 0.0.8  restore the latest version of a file before a specific date.
         Add a new post url such as (Hash_Array.json) to submit an array
         of hashs to serveur that will say which hashs are needed.
* 0.0.9  restore all the versions of a file
* 0.0.10 restore a directory and it's subfiles and directories at a
         specific date.
* 0.1.0 nettoie program to clean serveur's data and meta-data


### Releases

* 0.0.1  [08 07 2015] First usable version
* 0.0.2  [15.08.2015] client redesigned
* 0.0.3  [21.08.2015] saves and restores links
* 0.0.4  [06.09.2015] new serveur url to post a bunch of hashs and
                      associated data. Clean the answer list of needed
                      hashs to avoid having duplicated hashs into it.
* 0.0.5  [04.10.2015] fanotify's code reviewed. Avoid having one entire
                      file in memory.


## Code documentation

If you are interrested in helping the projet please have a look at
[coding_in_sauvegarde.md](docs/coding_in_sauvegarde.md) file. Some
inside insights are in the [infrastructure](docs/infrastructure.md)
documentation as well for the [API](docs/API.md) part.


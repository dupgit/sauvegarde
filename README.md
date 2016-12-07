# cdpfgl: Continuous data protection for GNU/Linux

This page is cdpfgl's doxygen's @mainpage.

[![cdpfgl Version](https://badge.fury.io/gh/dupgit%2Fsauvegarde.svg)](https://badge.fury.io/gh/dupgit%2Fsauvegarde)
[![Build Status](https://travis-ci.org/dupgit/sauvegarde.svg?branch=master)](https://travis-ci.org/dupgit/sauvegarde)
[![Gitter](https://badges.gitter.im/dupgit/sauvegarde.svg)](https://gitter.im/dupgit/sauvegarde?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

With this code you can backup and restore files in a live and continuous
way. Compile the code put the `cdpfglserver` on the machine where you want to
save your files. Let `cdpfglclient` crawl your files to save them. If needed
use `cdpfglrestore` to restore a file.


## License

This is free software and real open source as GPLv3 is used for this
collection of programs and Creative Commons Attribution-ShareAlike 4.0
is used for the artwork. You'll find a LICENSE file in this directory
for the programs (GPL v3) and one in the pixmaps directory for the artwork
(CC-BY-SA 4.0).

Feel free to contribute to cdpfgl's project and/or give help to the
projects used here (see dependencies section below).


## Dependencies

### Mandatory

This project depends on the following projects (minimum version required is
stated between () and recommended version between []):

* `autotools`      (2.59)
* `glib` and `gio` (2.34)
* `libmicrohttpd`  (0.9.5)  [0.9.46]
* `libcurl`        (7.22.0)
* `sqlite`         (3.7.15)
* `jansson`        (2.5)    [2.7]

jansson's library version is quite recent but it compiles nicely and
depends on nothing (as far as I know).
Compiling cdpfgl also needs gnutls development files (used by
libmicrohttpd). Please have a look at .travis.yml file in order to see how
to compile dependencies before compiling cdpfgl.

cdpfgl's project is known to compile (sometimes at the expense of
recompiling and installing newer versions of dependencies) and run under
Centos 7, Debian Jessie, Ubuntu 12.04 LTS, VoidLinux and raspbian 3 and
on x86_64 and arm7l architectures. Please let me know if you have compiled
cdpfgl successfully in a system that is not listed here.


### Optional

* `doxygen` (1.6.1) is used to generate code's documentation.
* `gnuplot` (4.6) is used to generate graphs for the documentation.
* `gource`  (0.43) is used to generate the video about project's life in
            git
* `pandoc`  (1.12.4.2) is used to generate a .tex and then a .pdf from
            .md manual files. It is also used to generate man pages from
            .md files in 'man' directory.


## Download

You can download the source code from github or a packed defined version
at [http://cdpfgl.delhomme.org/download/releases/](http://cdpfgl.delhomme.org/download/releases/)


## Installation

If you had the code from the git repository you may generate the configure
script by invoking `./autogen.sh` script. It will execute aclocal, libtoolize
automake, autoconf, glib-gettextize and intltoolize for you. If everything
went ok you should have an executable configure script that you may use
in the classic way:

    ./configure --prefix=/my/local/install --enable-debug
    make
    make install

If you need more information about how to install cdpfgl you can
also have a look at the [installation manual](manual/installation.md).


## Usage

### The server (the one that collects everything)

First you need to run the program `cdpflgserver` on a machine that will act as
a server. You can configure options in `server.conf` (located by default
in `{prefix}/etc/cdpfgl/`) or use the command's line options as shown in
[cdpfglserver man page](man/cdpfglserver.md).


### The client (the one that saves your files).

Then you may use `cdpfglclient` program to report modified files to the server
and save them accordingly. Configuration file is named `client.conf` and
command line options are documented in [cdpfglclient man page](man/cdpfglclient.md).


### The restore program (the one that saves your life when needed)

At a time you may need to restore a file then you'll have to use `cdpfglrestore`
program. Configuration file is named `restore.conf` and command line
options are documented in [cdpfglrestore man page](man/cdpfglrestore.md).


## Manual

If you need more than the above hints, please have a look at the user
[manual](manual/installation.md).


## Roadmap

I am coding on spare hours so I may not tell any release dates and you
should consider that "It ships when its ready".

* 0.0.11 have some statistics with the server.
* 0.0.12 Use compression in file_backend.
* 0.0.13 Add some security beetween clients and server to avoid sending
         plain clear text messages.


### Releases

* 0.0.1  [08.07.2015] First usable version
* 0.0.2  [15.08.2015] client redesigned
* 0.0.3  [21.08.2015] saves and restores links
* 0.0.4  [06.09.2015] new server url to post a bunch of hashs and
                      associated data. Clean the answer list of needed
                      hashs to avoid having duplicated hashs into it.
* 0.0.5  [04.10.2015] fanotify's code reviewed. Avoid having one entire
                      file in memory.
* 0.0.6  [02.11.2015] restore to some specific place. client adaptive
                      blocksize with option to choose fixed or adaptive.
                      Add options to choose CLIENT_MIN_BUFFER.
* 0.0.7  [03.01.2016] Ability to exclude some files by extension or path.
                      Caching mechanism in client in case the server is
                      unreachable. Change GSList hash_data_list from
                      meta_data_t structure to something that allows
                      deletion of elements while walking through it.
* 0.0.8  [27.04.2016] restore the latest version of a file before a
                      specific date. Add a new post url such as
                      (Hash_Array.json) to submit an array of hashs to
                      server that will say which hashs are needed.
                      Avoids sql injection in sqlite queries.
* 0.0.9  [24.08.2016] New GET url in order to get a bunch of hashs and
                      their associated data to go quicker when restoring
                      files. Restore all the versions of a file. Now
                      cdpfglclient handles signals and should close its
                      database connection cleanly.
* 0.0.10 [29.11.2016] restore a directory and it's subfiles and directories 
                      at a specific date. Write a man page for each programs.

                      
## Code documentation

If you are interested in helping the project please have a look at
[coding_in_cdpfgl.md](docs/coding_in_cdpfgl.md) file. Some
inside insights are in the [infrastructure](docs/infrastructure.md)
documentation as well for the [API](docs/API.md) part.


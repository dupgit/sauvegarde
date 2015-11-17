
# VoidLinux binary packages 

This document focuses (only) on howto generate and use binary packages for the VoidLinux distribution. 
For all other aspects (including usage of this software), please read the general documentation.


## Changelog


* 2015-11-xx cdpfgl version 0.0.6: basic runtime tests are fine.
* 2015-11-xx initial revision of this documentation

## Packaging internal notes

TODO list:

* check build log.
* config files in /etc/sauvegarde/, not in /etc/cdpfgl/ ?
* v0.0.6: source file are different from github.com and src.delhomme.org ?!
* /usr/lib/sauvegarde/libsauvegarde.so.0.0.0: not in /usr/lib/ instead ?
* keep a copy of config file also in samples ?
* manage localization ?
* cross-build not yet tested

## Building binary package(s)

cdpgfl/Sauvegarde software is not yet available as binary package(s) within the VoidLinux distribution.

However, here the steps to build such a binary package(s), with the help of the other files provided in the same folder than this document.

First, retrieve a clone of xbps-src, the packaging source system of VoidLinux.
Then create the initial build environment:

    $ git clone git://github.com/voidlinux/void-packages.git
    $ cd void-packages
    $ ./xbps-src binary-bootstrap

Next step is to add files for the project: copy the files of the same folder
than this document into the tree of xbps-src.
The tree of folders/files then looks like:

    void-packages/srcpkgs/cdpfgl-server/template
    void-packages/srcpkgs/cdpfgl-server/files/cdpfgl-client/run
    void-packages/srcpkgs/cdpfgl-server/files/cdpfgl-server/run

You will also have to create the following symbolic links for the other defined packages:

    $ cd void-packages/srcpkgs
    $ ln -s cdpfgl-server cdpfgl-client
    $ ln -s cdpfgl-server cdpfgl-lib
    $ ln -s cdpfgl-server cdpfgl-devel

Last step is to update file `void-packages/common/shlibs` in order to indicate that `libsauvegarde.so.0` file is owned by the package `cdpfgl-lib`:

    $ echo "libsauvegarde.so.0 cdpfgl-lib-0.0.6_1" >> void-packages/common/shlibs


Optional: if source file version is outdated.
The template file contains the source version and a sha256 fingerprint of the
archive source file using for building.
If these informations are outdated, you will have to modify yourself.

Example with the version 0.0.6 :

* archive file has the following checksum:

        $ cd tmp
        $ wget https://github.com/dupgit/sauvegarde/archive/v0.0.6.tar.gz
        $ sha256sum v0.0.6.tar.gz
        a8135c581a55aaf5a7a776301a25f23f0f1dfa531921adc142de44e07e095a95  v0.0.6.tar.gz

* `template` file contains the same informations:

        $ cat void-packages/srcpkgs/cdpfgl-server/template
        [...]
        version=0.0.6
        checksum=a8135c581a55aaf5a7a776301a25f23f0f1dfa531921adc142de44e07e095a95
        [...]

Once package definition files in place, package(s) building is started with the following command:

    $ cd void-packages
    $ ./xbps-src -f pkg cdpfgl-server

See an output example of package building at the end of this document.

If the build succeeds, binary packages will be stored into a local repository:

    void-packages/hostdir/binpkgs/cdpfgl-server-<version>.xpbs
    void-packages/hostdir/binpkgs/cdpfgl-client-<version>.xpbs
    void-packages/hostdir/binpkgs/cdpfgl-lib-<version>.xpbs
    void-packages/hostdir/binpkgs/cdpfgl-devel-<version>.xpbs


# HOWTO use XBPS generated packages on VoidLinux

Once binary packages have been successfully builted (see previous section), you
can then install them on your system.

Since XBPS packaging system is using only repositories, you can't install directly the
`.xbps` package files.

Instead, add definition for this new repository that has been populated previously:

    $ sudo echo "repository=/full/path/to/void-packages/hostdir/binpkgs" \
                > /etc/xbps.d/building-repo.conf

We can then install the `cdpfgl-client` package or `cdpfgl-server` (similar steps).

Install the `-client` package and its required dependencies:

```
$ sudo xbps-install -S cdpfgl-client
```

```
[*] Updating `http://repo.voidlinux.eu/current/i686-repodata' ...

Name          Action    Version           New version            Download size
libmicrohttpd install   -                 0.9.44_1               42KB 
cdpfgl-lib    install   -                 0.0.6_1                - 
cdpfgl-client install   -                 0.0.6_1                - 

Size to download:               42KB
Size required on disk:         298KB
Free space on disk:           5732MB

Do you want to continue? [Y/n] y

[*] Downloading binary packages
libmicrohttpd-0.9.44_1.i686.xbps: 42KB [avg rate: 41MB/s]
libmicrohttpd-0.9.44_1.i686.xbps.sig: 512B [avg rate: 12MB/s]

[*] Verifying package integrity
libmicrohttpd-0.9.44_1: verifying RSA signature...
cdpfgl-lib-0.0.6_1: verifying SHA256 hash...
cdpfgl-client-0.0.6_1: verifying SHA256 hash...

[*] Running transaction tasks
libmicrohttpd-0.9.44_1: unpacking ...
cdpfgl-lib-0.0.6_1: unpacking ...
cdpfgl-client-0.0.6_1: unpacking ...

[*] Configuring unpacked packages
libmicrohttpd-0.9.44_1: configuring ...
libmicrohttpd-0.9.44_1: installed successfully.
cdpfgl-lib-0.0.6_1: configuring ...
cdpfgl-lib-0.0.6_1: installed successfully.
cdpfgl-client-0.0.6_1: configuring ...
cdpfgl-client-0.0.6_1: installed successfully.

1 downloaded, 3 installed, 0 updated, 3 configured, 0 removed.
```


(optional) List the files in this newly installed package:

    $ xbps-query -f cdpfgl-client | sort
    /etc/sauvegarde/client.conf
    /etc/sauvegarde/restore.conf
    /etc/sv/cdpfgl-client/run
    /etc/sv/cdpfgl-client/supervise -> /run/runit/supervise.cdpfgl-client
    /usr/bin/cdpfglclient
    /usr/bin/cdpfglrestore


VoidLinux distribution uses `runit` as deamon/service launcher.
A runit compatible service is provided with this package (file
`/etc/sv/cdpfgl-client/run`), but not enabled on package installation.

To enable it this service on system startup:

    $ sudo ln -s /etc/sv/cdpfgl-client /var/service

runit will then automatically start it, as soon as the link has been created.

Check:

    $ sudo sv status cdpfgl-client
    run: cdpfgl-client: (pid 14707) 13s

## Annex: output of package building

Context: building cdpfgl version 0.0.6 with void-packages as of 2015-11

```
$ ./xbps-src -f pkg cdpfgl-server
```

```
=> Using `/build/packages/hostdir/binpkgs/sauvegarde.local' as local repository.
[*] Updating `http://repo.voidlinux.eu/current/i686-repodata' ...
[*] Updating `http://repo.voidlinux.eu/current/nonfree/i686-repodata' ...
i686-repodata: [14KB 28%] 13MB/s ETA: 00m00s
i686-repodata: [14KB 100%] 582KB/s ETA: 00m00s
i686-repodata: 14KB [avg rate: 582KB/s]
[*] Updating `http://muslrepo.voidlinux.eu/current/i686-repodata' ...
[*] Updating `http://muslrepo.voidlinux.eu/current/nonfree/i686-repodata' ...
=> Reconfiguring bootstrap packages...
ca-certificates: configuring ...
ca-certificates: configured successfully.
=> cdpfgl-server-0.0.6_1: building ...
   [host] automake-1.15_4: found (http://repo.voidlinux.eu/current)
   [host] libtool-2.4.6_1: found (http://repo.voidlinux.eu/current)
   [host] perl-5.22.0_1: found (http://repo.voidlinux.eu/current)
   [host] pkg-config-0.29_1: found (http://repo.voidlinux.eu/current)
   [target] intltool-0.51.0_3: found (http://repo.voidlinux.eu/current)
   [target] glib-devel-2.46.2_1: found (http://repo.voidlinux.eu/current)
   [target] gettext-devel-0.19.6_1: found (http://repo.voidlinux.eu/current)
   [target] libmicrohttpd-devel-0.9.44_1: found (http://repo.voidlinux.eu/current)
   [target] libcurl-devel-7.45.0_1: found (http://repo.voidlinux.eu/current)
   [target] sqlite-devel-3.9.2_1: found (http://repo.voidlinux.eu/current)
   [target] jansson-devel-2.7_1: found (http://repo.voidlinux.eu/current)
=> cdpfgl-server-0.0.6_1: installing host dependency 'automake-1.15_4' ...
=> cdpfgl-server-0.0.6_1: installing host dependency 'libtool-2.4.6_1' ...
=> cdpfgl-server-0.0.6_1: installing host dependency 'perl-5.22.0_1' ...
=> cdpfgl-server-0.0.6_1: installing host dependency 'pkg-config-0.29_1' ...
=> cdpfgl-server-0.0.6_1: installing target dependency 'intltool-0.51.0_3' ...
=> cdpfgl-server-0.0.6_1: installing target dependency 'glib-devel-2.46.2_1' ...
=> cdpfgl-server-0.0.6_1: installing target dependency 'gettext-devel-0.19.6_1' ...
=> cdpfgl-server-0.0.6_1: installing target dependency 'libmicrohttpd-devel-0.9.44_1' ...
=> cdpfgl-server-0.0.6_1: installing target dependency 'libcurl-devel-7.45.0_1' ...
=> cdpfgl-server-0.0.6_1: installing target dependency 'sqlite-devel-3.9.2_1' ...
=> cdpfgl-server-0.0.6_1: installing target dependency 'jansson-devel-2.7_1' ...
=> cdpfgl-server-0.0.6_1: running do-fetch hook: 00-distfiles ...
=> cdpfgl-server-0.0.6_1: verifying checksum for distfile 'sauvegarde-0.0.6.tar.gz'... OK.
=> cdpfgl-server-0.0.6_1: running do-extract hook: 00-distfiles ...
=> cdpfgl-server-0.0.6_1: extracting distfile(s), please wait...
=> cdpfgl-server-0.0.6_1: running post-extract hook: 00-patches ...
=> cdpfgl-server-0.0.6_1: running pre-configure hook: 00-gnu-configure-asneeded ...
=> cdpfgl-server-0.0.6_1: running pre-configure hook: 01-override-config ...
=> cdpfgl-server-0.0.6_1: running pre-configure hook: 02-script-wrapper ...
=> cdpfgl-server-0.0.6_1: running pre_configure ...
aclocal: warning: couldn't open directory 'm4': No such file or directory
libtoolize: putting auxiliary files in '.'.
libtoolize: linking file './ltmain.sh'
libtoolize: putting macros in AC_CONFIG_MACRO_DIRS, 'm4'.
libtoolize: linking file 'm4/libtool.m4'
libtoolize: linking file 'm4/ltoptions.m4'
libtoolize: linking file 'm4/ltsugar.m4'
libtoolize: linking file 'm4/ltversion.m4'
libtoolize: linking file 'm4/lt~obsolete.m4'
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

=> cdpfgl-server-0.0.6_1: running do_configure ...
checking for a BSD-compatible install... /builddir/.xbps-cdpfgl-server/wrappers/install -c
checking whether build environment is sane... yes
checking for a thread-safe mkdir -p... /usr/bin/mkdir -p
checking for gawk... gawk
checking whether make sets $(MAKE)... yes
checking whether make supports nested variables... yes
checking for i686-pc-linux-gnu-gcc... cc
checking whether the C compiler works... yes
checking for C compiler default output file name... a.out
checking for suffix of executables... 
checking whether we are cross compiling... no
checking for suffix of object files... o
checking whether we are using the GNU C compiler... yes
checking whether cc accepts -g... yes
checking for cc option to accept ISO C89... none needed
checking whether cc understands -c and -o together... yes
checking for style of include used by make... GNU
checking dependency style of cc... gcc3
checking whether NLS is requested... yes
checking for intltool >= 0.23... 0.51.0 found
checking for intltool-update... /usr/bin/intltool-update
checking for intltool-merge... /usr/bin/intltool-merge
checking for intltool-extract... /usr/bin/intltool-extract
checking for xgettext... /usr/bin/xgettext
checking for msgmerge... /usr/bin/msgmerge
checking for msgfmt... /usr/bin/msgfmt
checking for gmsgfmt... /usr/bin/msgfmt
checking for perl... /usr/bin/perl
checking for perl >= 5.8.1... 5.22.0
checking for XML::Parser... ok
checking for i686-pc-linux-gnu-pkg-config... no
checking for pkg-config... /usr/bin/pkg-config
checking pkg-config is at least version 0.23... yes
checking whether build environment is sane... yes
checking whether to enable maintainer-specific portions of Makefiles... no
checking for dirent.h that defines DIR... yes
checking for library containing opendir... none required
checking how to run the C preprocessor... cpp
checking for grep that handles long lines and -e... /usr/bin/grep
checking for egrep... /usr/bin/grep -E
checking for ANSI C header files... yes
checking for sys/types.h... yes
checking for sys/stat.h... yes
checking for stdlib.h... yes
checking for string.h... yes
checking for memory.h... yes
checking for strings.h... yes
checking for inttypes.h... yes
checking for stdint.h... yes
checking for unistd.h... yes
checking signal.h usability... yes
checking signal.h presence... yes
checking for signal.h... yes
checking sys/signalfd.h usability... yes
checking sys/signalfd.h presence... yes
checking for sys/signalfd.h... yes
checking sys/fanotify.h usability... yes
checking sys/fanotify.h presence... yes
checking for sys/fanotify.h... yes
checking math.h usability... yes
checking math.h presence... yes
checking for math.h... yes
checking build system type... i686-pc-linux-gnu
checking host system type... i686-pc-linux-gnu
checking how to print strings... printf
checking for a sed that does not truncate output... /usr/bin/sed
checking for fgrep... /usr/bin/grep -F
checking for ld used by cc... ld
checking if the linker (ld) is GNU ld... yes
checking for BSD- or MS-compatible name lister (nm)... nm
checking the name lister (nm) interface... BSD nm
checking whether ln -s works... yes
checking the maximum length of command line arguments... 1572864
checking how to convert i686-pc-linux-gnu file names to i686-pc-linux-gnu format... func_convert_file_noop
checking how to convert i686-pc-linux-gnu file names to toolchain format... func_convert_file_noop
checking for ld option to reload object files... -r
checking for i686-pc-linux-gnu-objdump... objdump
checking how to recognize dependent libraries... pass_all
checking for i686-pc-linux-gnu-dlltool... no
checking for dlltool... no
checking how to associate runtime and link libraries... printf %s\n
checking for i686-pc-linux-gnu-ar... ar
checking for archiver @FILE support... @
checking for i686-pc-linux-gnu-strip... strip
checking for i686-pc-linux-gnu-ranlib... ranlib
checking command to parse nm output from cc object... ok
checking for libtool-sysroot... no
checking for a working dd... /usr/bin/dd
checking how to truncate binary pipes... /usr/bin/dd bs=4096 count=1
checking for i686-pc-linux-gnu-mt... no
checking for mt... no
checking if : is a manifest tool... no
checking for dlfcn.h... yes
checking for objdir... .libs
checking if cc supports -fno-rtti -fno-exceptions... no
checking for cc option to produce PIC... -fPIC -DPIC
checking if cc PIC flag -fPIC -DPIC works... yes
checking if cc static flag -static works... yes
checking if cc supports -c -o file.o... yes
checking if cc supports -c -o file.o... (cached) yes
checking whether the cc linker (ld) supports shared libraries... yes
checking whether -lc should be explicitly linked in... no
checking dynamic linker characteristics... GNU/Linux ld.so
checking how to hardcode library paths into programs... immediate
checking whether stripping libraries is possible... no
checking if libtool supports shared libraries... yes
checking whether to build shared libraries... yes
checking whether to build static libraries... no
checking locale.h usability... yes
checking locale.h presence... yes
checking for locale.h... yes
checking for LC_MESSAGES... yes
checking libintl.h usability... yes
checking libintl.h presence... yes
checking for libintl.h... yes
checking for ngettext in libc... yes
checking for dgettext in libc... yes
checking for bind_textdomain_codeset... yes
checking for msgfmt... (cached) /usr/bin/msgfmt
checking for dcgettext... yes
checking if msgfmt accepts -c... yes
checking for gmsgfmt... (cached) /usr/bin/msgfmt
checking for xgettext... (cached) /usr/bin/xgettext
checking for catalogs to be installed...  fr
checking for GLIB... yes
checking for GIO... yes
checking for SQLITE... yes
checking for JANSSON... yes
checking for MHD... yes
checking for CURL... yes
checking whether make supports nested variables... (cached) yes
checking that generated files are newer than configure... done
checking that generated files are newer than configure... done
configure: creating ./config.status
config.status: creating Makefile
config.status: creating libsauvegarde/Makefile
config.status: creating monitor/Makefile
config.status: creating serveur/Makefile
config.status: creating restaure/Makefile
config.status: creating po/Makefile.in
config.status: creating libsauvegarde/libsauvegarde.pc
config.status: creating config.h
config.status: executing depfiles commands
config.status: executing libtool commands
config.status: executing default-1 commands
config.status: executing po/stamp-it commands
configure:

 *** Flags that will be used to compile Sauvegarde ***

 CFLAGS         : -mtune=i686 -O2 -pipe -fstack-protector-strong -D_FORTIFY_SOURCE=2    -Wall -Wstrict-prototypes -Wmissing-declarations -Wbad-function-cast -Wcast-align -Wnested-externs -Wunused -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -DGSEAL_ENABLE -DG_DISABLE_DEPRECATED -ggdb
 LDFLAGS        :   -Wl,--as-needed -Wl,-z,relro  -export-dynamic -no-undefined -module -lm

 GLIB CFLAGS    : -I/usr/include/glib-2.0 -I/usr/lib32/glib-2.0/include
 GLIB LIBS      : -lglib-2.0

 GIO CFLAGS     : -pthread -I/usr/include/glib-2.0 -I/usr/lib32/glib-2.0/include
 GIO LIBS       : -lgio-2.0 -lgobject-2.0 -lglib-2.0

 SQLITE CFLAGS  : 
 SQLITE LIBS    : -lsqlite3

 JANSSON CFLAGS : 
 JANSSON LIBS   : -ljansson

 MHD CFLAGS     : 
 MHD LIBS       : -lmicrohttpd

 CURL CFLAGS     : 
 CURL LIBS       : -lcurl

 *** Dumping configuration ***

     - Build For OS             : linux-gnu
     - Compiler                 : cc
     - Prefix                   : /usr
     - Exec prefix              : ${prefix}
     - Locale dir               : /usr/share/locale
     - git revision             : 0

     - Options
       . is code profiling on ?       : false
       . Code coverage is on ?        : false
       . debbuging is on ?            : false
       . Disabled quiet compilation ? : false

You can now run 'make' to compile Sauvegarde.


=> cdpfgl-server-0.0.6_1: running pre-build hook: 02-script-wrapper ...
=> cdpfgl-server-0.0.6_1: running do_build ...
make  all-recursive
make[1]: Entering directory '/builddir/sauvegarde-0.0.6'
Making all in libsauvegarde
make[2]: Entering directory '/builddir/sauvegarde-0.0.6/libsauvegarde'
  CC       libsauvegarde_la-libsauvegarde.lo
  CC       libsauvegarde_la-configuration.lo
  CC       libsauvegarde_la-communique.lo
  CC       libsauvegarde_la-files.lo
  CC       libsauvegarde_la-hashs.lo
  CC       libsauvegarde_la-database.lo
  CC       libsauvegarde_la-packing.lo
  CC       libsauvegarde_la-unpacking.lo
  CC       libsauvegarde_la-query.lo
  CC       libsauvegarde_la-clock.lo
  CCLD     libsauvegarde.la
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6/libsauvegarde'
Making all in serveur
make[2]: Entering directory '/builddir/sauvegarde-0.0.6/serveur'
  CC       serveur.o
  CC       options.o
  CC       backend.o
  CC       file_backend.o
  CCLD     cdpfglserver

*** Warning: Linking the executable cdpfglserver against the loadable module
*** libsauvegarde.so is not portable!
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6/serveur'
Making all in monitor
make[2]: Entering directory '/builddir/sauvegarde-0.0.6/monitor'
  CC       monitor.o
  CC       options.o
  CC       m_fanotify.o
  CCLD     cdpfglclient

*** Warning: Linking the executable cdpfglclient against the loadable module
*** libsauvegarde.so is not portable!
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6/monitor'
Making all in restaure
make[2]: Entering directory '/builddir/sauvegarde-0.0.6/restaure'
  CC       restaure.o
  CC       options.o
  CCLD     cdpfglrestore

*** Warning: Linking the executable cdpfglrestore against the loadable module
*** libsauvegarde.so is not portable!
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6/restaure'
Making all in po
make[2]: Entering directory '/builddir/sauvegarde-0.0.6/po'
  MSGFMT fr.gmo
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6/po'
make[2]: Entering directory '/builddir/sauvegarde-0.0.6'
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6'
make[1]: Leaving directory '/builddir/sauvegarde-0.0.6'
=> cdpfgl-server-0.0.6_1: running pre-install hook: 00-lib32 ...
=> cdpfgl-server-0.0.6_1: running pre-install hook: 02-script-wrapper ...
=> cdpfgl-server-0.0.6_1: running do_install ...
Making install in libsauvegarde
make[1]: Entering directory '/builddir/sauvegarde-0.0.6/libsauvegarde'
make[2]: Entering directory '/builddir/sauvegarde-0.0.6/libsauvegarde'
 /usr/bin/mkdir -p '/destdir//cdpfgl-server-0.0.6/usr/lib32/sauvegarde'
 /bin/sh ../libtool   --mode=install /builddir/.xbps-cdpfgl-server/wrappers/install -c   libsauvegarde.la '/destdir//cdpfgl-server-0.0.6/usr/lib32/sauvegarde'
libtool: install: /builddir/.xbps-cdpfgl-server/wrappers/install -c .libs/libsauvegarde.so.0.0.0 /destdir//cdpfgl-server-0.0.6/usr/lib32/sauvegarde/libsauvegarde.so.0.0.0
libtool: install: (cd /destdir//cdpfgl-server-0.0.6/usr/lib32/sauvegarde && { ln -s -f libsauvegarde.so.0.0.0 libsauvegarde.so.0 || { rm -f libsauvegarde.so.0 && ln -s libsauvegarde.so.0.0.0 libsauvegarde.so.0; }; })
libtool: install: (cd /destdir//cdpfgl-server-0.0.6/usr/lib32/sauvegarde && { ln -s -f libsauvegarde.so.0.0.0 libsauvegarde.so || { rm -f libsauvegarde.so && ln -s libsauvegarde.so.0.0.0 libsauvegarde.so; }; })
libtool: install: /builddir/.xbps-cdpfgl-server/wrappers/install -c .libs/libsauvegarde.lai /destdir//cdpfgl-server-0.0.6/usr/lib32/sauvegarde/libsauvegarde.la
libtool: warning: remember to run 'libtool --finish /usr/lib32/sauvegarde'
 /usr/bin/mkdir -p '/destdir//cdpfgl-server-0.0.6/usr/include/sauvegarde'
 /builddir/.xbps-cdpfgl-server/wrappers/install -c -m 644 libsauvegarde.h configuration.h communique.h files.h hashs.h packing.h database.h query.h clock.h '/destdir//cdpfgl-server-0.0.6/usr/include/sauvegarde'
 /usr/bin/mkdir -p '/destdir//cdpfgl-server-0.0.6/usr/lib32/pkgconfig'
 /builddir/.xbps-cdpfgl-server/wrappers/install -c -m 644 libsauvegarde.pc '/destdir//cdpfgl-server-0.0.6/usr/lib32/pkgconfig'
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6/libsauvegarde'
make[1]: Leaving directory '/builddir/sauvegarde-0.0.6/libsauvegarde'
Making install in serveur
make[1]: Entering directory '/builddir/sauvegarde-0.0.6/serveur'
make[2]: Entering directory '/builddir/sauvegarde-0.0.6/serveur'
 /usr/bin/mkdir -p '/destdir//cdpfgl-server-0.0.6/usr/bin'
  /bin/sh ../libtool   --mode=install /builddir/.xbps-cdpfgl-server/wrappers/install -c cdpfglserver '/destdir//cdpfgl-server-0.0.6/usr/bin'
libtool: warning: '/builddir/sauvegarde-0.0.6/libsauvegarde/libsauvegarde.la' has not been installed in '/usr/lib32/sauvegarde'
libtool: install: /builddir/.xbps-cdpfgl-server/wrappers/install -c .libs/cdpfglserver /destdir//cdpfgl-server-0.0.6/usr/bin/cdpfglserver
make[2]: Nothing to be done for 'install-data-am'.
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6/serveur'
make[1]: Leaving directory '/builddir/sauvegarde-0.0.6/serveur'
Making install in monitor
make[1]: Entering directory '/builddir/sauvegarde-0.0.6/monitor'
make[2]: Entering directory '/builddir/sauvegarde-0.0.6/monitor'
 /usr/bin/mkdir -p '/destdir//cdpfgl-server-0.0.6/usr/bin'
  /bin/sh ../libtool   --mode=install /builddir/.xbps-cdpfgl-server/wrappers/install -c cdpfglclient '/destdir//cdpfgl-server-0.0.6/usr/bin'
libtool: warning: '/builddir/sauvegarde-0.0.6/libsauvegarde/libsauvegarde.la' has not been installed in '/usr/lib32/sauvegarde'
libtool: install: /builddir/.xbps-cdpfgl-server/wrappers/install -c .libs/cdpfglclient /destdir//cdpfgl-server-0.0.6/usr/bin/cdpfglclient
make[2]: Nothing to be done for 'install-data-am'.
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6/monitor'
make[1]: Leaving directory '/builddir/sauvegarde-0.0.6/monitor'
Making install in restaure
make[1]: Entering directory '/builddir/sauvegarde-0.0.6/restaure'
make[2]: Entering directory '/builddir/sauvegarde-0.0.6/restaure'
 /usr/bin/mkdir -p '/destdir//cdpfgl-server-0.0.6/usr/bin'
  /bin/sh ../libtool   --mode=install /builddir/.xbps-cdpfgl-server/wrappers/install -c cdpfglrestore '/destdir//cdpfgl-server-0.0.6/usr/bin'
libtool: warning: '/builddir/sauvegarde-0.0.6/libsauvegarde/libsauvegarde.la' has not been installed in '/usr/lib32/sauvegarde'
libtool: install: /builddir/.xbps-cdpfgl-server/wrappers/install -c .libs/cdpfglrestore /destdir//cdpfgl-server-0.0.6/usr/bin/cdpfglrestore
make[2]: Nothing to be done for 'install-data-am'.
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6/restaure'
make[1]: Leaving directory '/builddir/sauvegarde-0.0.6/restaure'
Making install in po
make[1]: Entering directory '/builddir/sauvegarde-0.0.6/po'
linguas="fr "; \
for lang in $linguas; do \
  dir=/destdir//cdpfgl-server-0.0.6/usr/share/locale/$lang/LC_MESSAGES; \
  /bin/sh /builddir/sauvegarde-0.0.6/install-sh -d $dir; \
  if test -r $lang.gmo; then \
    /builddir/.xbps-cdpfgl-server/wrappers/install -c -m 644 $lang.gmo $dir/sauvegarde.mo; \
    echo "installing $lang.gmo as $dir/sauvegarde.mo"; \
  else \
    /builddir/.xbps-cdpfgl-server/wrappers/install -c -m 644 ./$lang.gmo $dir/sauvegarde.mo; \
    echo "installing ./$lang.gmo as" \
	 "$dir/sauvegarde.mo"; \
  fi; \
  if test -r $lang.gmo.m; then \
    /builddir/.xbps-cdpfgl-server/wrappers/install -c -m 644 $lang.gmo.m $dir/sauvegarde.mo.m; \
    echo "installing $lang.gmo.m as $dir/sauvegarde.mo.m"; \
  else \
    if test -r ./$lang.gmo.m ; then \
      /builddir/.xbps-cdpfgl-server/wrappers/install -c -m 644 ./$lang.gmo.m \
	$dir/sauvegarde.mo.m; \
      echo "installing ./$lang.gmo.m as" \
	   "$dir/sauvegarde.mo.m"; \
    else \
      true; \
    fi; \
  fi; \
done
installing fr.gmo as /destdir//cdpfgl-server-0.0.6/usr/share/locale/fr/LC_MESSAGES/sauvegarde.mo
make[1]: Leaving directory '/builddir/sauvegarde-0.0.6/po'
make[1]: Entering directory '/builddir/sauvegarde-0.0.6'
make[2]: Entering directory '/builddir/sauvegarde-0.0.6'
make[2]: Nothing to be done for 'install-exec-am'.
 /usr/bin/mkdir -p '/destdir//cdpfgl-server-0.0.6/etc/sauvegarde'
 /builddir/.xbps-cdpfgl-server/wrappers/install -c -m 644 client.conf server.conf restore.conf '/destdir//cdpfgl-server-0.0.6/etc/sauvegarde'
make[2]: Leaving directory '/builddir/sauvegarde-0.0.6'
make[1]: Leaving directory '/builddir/sauvegarde-0.0.6'
=> cdpfgl-server-0.0.6_1: running post_install ...
=> cdpfgl-client-0.0.6_1: running pre-install hook: 00-lib32 ...
=> cdpfgl-client-0.0.6_1: running pre-install hook: 02-script-wrapper ...
=> cdpfgl-client-0.0.6_1: running pkg_install ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 00-compress-info-files ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 00-uncompress-manpages ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 01-remove-localized-manpages ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 01-remove-misc ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 02-remove-libtool-archives ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 02-remove-perl-files ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 02-remove-python-bytecode-files ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 03-remove-empty-dirs ...
=> WARNING: cdpfgl-client-0.0.6_1: removed empty dir: /usr/lib
=> cdpfgl-client-0.0.6_1: running post-install hook: 04-create-xbps-metadata-scripts ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 05-generate-gitrevs ...
=> cdpfgl-client-0.0.6_1: running post-install hook: 06-strip-and-debug-pkgs ...
   Stripped executable: /usr/bin/cdpfglclient
   Stripped executable: /usr/bin/cdpfglrestore
=> cdpfgl-client-0.0.6_1: running post-install hook: 98-lib32 ...
=> cdpfgl-devel-0.0.6_1: running pre-install hook: 00-lib32 ...
=> cdpfgl-devel-0.0.6_1: running pre-install hook: 02-script-wrapper ...
=> cdpfgl-devel-0.0.6_1: running pkg_install ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 00-compress-info-files ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 00-uncompress-manpages ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 01-remove-localized-manpages ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 01-remove-misc ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 02-remove-libtool-archives ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 02-remove-perl-files ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 02-remove-python-bytecode-files ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 03-remove-empty-dirs ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 04-create-xbps-metadata-scripts ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 05-generate-gitrevs ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 06-strip-and-debug-pkgs ...
=> cdpfgl-devel-0.0.6_1: running post-install hook: 98-lib32 ...
=> cdpfgl-lib-0.0.6_1: running pre-install hook: 00-lib32 ...
=> cdpfgl-lib-0.0.6_1: running pre-install hook: 02-script-wrapper ...
=> cdpfgl-lib-0.0.6_1: running pkg_install ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 00-compress-info-files ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 00-uncompress-manpages ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 01-remove-localized-manpages ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 01-remove-misc ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 02-remove-libtool-archives ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 02-remove-perl-files ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 02-remove-python-bytecode-files ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 03-remove-empty-dirs ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 04-create-xbps-metadata-scripts ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 05-generate-gitrevs ...
=> cdpfgl-lib-0.0.6_1: running post-install hook: 06-strip-and-debug-pkgs ...
   Stripped library: /usr/lib/sauvegarde/libsauvegarde.so.0.0.0
=> cdpfgl-lib-0.0.6_1: running post-install hook: 98-lib32 ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 00-compress-info-files ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 00-uncompress-manpages ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 01-remove-localized-manpages ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 01-remove-misc ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 02-remove-libtool-archives ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 02-remove-perl-files ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 02-remove-python-bytecode-files ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 03-remove-empty-dirs ...
=> WARNING: cdpfgl-server-0.0.6_1: removed empty dir: /usr/share
=> WARNING: cdpfgl-server-0.0.6_1: removed empty dir: /usr/lib/sauvegarde
=> cdpfgl-server-0.0.6_1: running post-install hook: 04-create-xbps-metadata-scripts ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 05-generate-gitrevs ...
=> cdpfgl-server-0.0.6_1: running post-install hook: 06-strip-and-debug-pkgs ...
   Stripped executable: /usr/bin/cdpfglserver
=> cdpfgl-server-0.0.6_1: running post-install hook: 98-lib32 ...
=> cdpfgl-client-0.0.6_1: running pre-pkg hook: 04-generate-runtime-deps ...
   SONAME: libsauvegarde.so.0 <-> cdpfgl-lib-0.0.6_1
   SONAME: libgio-2.0.so.0 <-> glib>=2.18.0_1
   SONAME: libglib-2.0.so.0 <-> glib>=2.18.0_1
   SONAME: libjansson.so.4 <-> jansson>=2.4_1
   SONAME: libcurl.so.4 <-> libcurl>=7.19_1
   SONAME: libpthread.so.0 <-> glibc>=2.8_1
   SONAME: libc.so.6 <-> glibc>=2.8_1
=> cdpfgl-client-0.0.6_1: running pre-pkg hook: 05-prepare-32bit ...
=> cdpfgl-client-0.0.6_1: running pre-pkg hook: 06-shlib-provides ...
=> cdpfgl-client-0.0.6_1: running pre-pkg hook: 99-pkglint ...
=> cdpfgl-devel-0.0.6_1: running pre-pkg hook: 04-generate-runtime-deps ...
=> cdpfgl-devel-0.0.6_1: running pre-pkg hook: 05-prepare-32bit ...
   RDEP: cdpfgl-lib>=0.0.6_1 -> cdpfgl-lib>=0.0.6_1 (subpkg, no shlib-provides)
   RDEP: cdpfgl-devel-0.0.6_1
=> cdpfgl-devel-0.0.6_1: running pre-pkg hook: 06-shlib-provides ...
=> cdpfgl-devel-0.0.6_1: running pre-pkg hook: 99-pkglint ...
=> cdpfgl-lib-0.0.6_1: running pre-pkg hook: 04-generate-runtime-deps ...
   SONAME: libgio-2.0.so.0 <-> glib>=2.18.0_1
   SONAME: libgobject-2.0.so.0 <-> glib>=2.18.0_1
   SONAME: libglib-2.0.so.0 <-> glib>=2.18.0_1
   SONAME: libsqlite3.so.0 <-> sqlite>=3.8.11.1_3
   SONAME: libjansson.so.4 <-> jansson>=2.4_1
   SONAME: libmicrohttpd.so.10 <-> libmicrohttpd>=0.9.26_1
   SONAME: libcurl.so.4 <-> libcurl>=7.19_1
   SONAME: libm.so.6 <-> glibc>=2.8_1
   SONAME: libpthread.so.0 <-> glibc>=2.8_1
   SONAME: libc.so.6 <-> glibc>=2.8_1
=> cdpfgl-lib-0.0.6_1: running pre-pkg hook: 05-prepare-32bit ...
   RDEP: glib>=2.18.0_1 -> glib-32bit>=2.18.0_1 (shlib-provides)
   RDEP: sqlite>=3.8.11.1_3 -> sqlite-32bit>=3.8.11.1_3 (shlib-provides)
   RDEP: jansson>=2.4_1 -> jansson-32bit>=2.4_1 (shlib-provides)
   RDEP: libmicrohttpd>=0.9.26_1 -> libmicrohttpd-32bit>=0.9.26_1 (shlib-provides)
   RDEP: libcurl>=7.19_1 -> libcurl-32bit>=7.19_1 (shlib-provides)
   RDEP: glibc>=2.8_1 -> glibc-32bit>=2.8_1 (shlib-provides)
=> cdpfgl-lib-0.0.6_1: running pre-pkg hook: 06-shlib-provides ...
   SONAME libsauvegarde.so.0 from /usr/lib/sauvegarde/libsauvegarde.so.0.0.0
   SONAME libsauvegarde.so.0 from /usr/lib32/sauvegarde/libsauvegarde.so.0.0.0
=> cdpfgl-lib-0.0.6_1: running pre-pkg hook: 99-pkglint ...
=> cdpfgl-server-0.0.6_1: running pre-pkg hook: 04-generate-runtime-deps ...
   SONAME: libsauvegarde.so.0 <-> cdpfgl-lib-0.0.6_1
   SONAME: libgio-2.0.so.0 <-> glib>=2.18.0_1
   SONAME: libgobject-2.0.so.0 <-> glib>=2.18.0_1
   SONAME: libglib-2.0.so.0 <-> glib>=2.18.0_1
   SONAME: libm.so.6 <-> glibc>=2.8_1
   SONAME: libjansson.so.4 <-> jansson>=2.4_1
   SONAME: libmicrohttpd.so.10 <-> libmicrohttpd>=0.9.26_1
   SONAME: libpthread.so.0 <-> glibc>=2.8_1
   SONAME: libc.so.6 <-> glibc>=2.8_1
=> cdpfgl-server-0.0.6_1: running pre-pkg hook: 05-prepare-32bit ...
=> cdpfgl-server-0.0.6_1: running pre-pkg hook: 06-shlib-provides ...
=> cdpfgl-server-0.0.6_1: running pre-pkg hook: 99-pkglint ...
=> cdpfgl-client-0.0.6_1: running do-pkg hook: 00-gen-pkg ...
=> Creating cdpfgl-client-0.0.6_1.i686.xbps for repository /host/binpkgs/sauvegarde.local ...
=> cdpfgl-client-0.0.6_1: running post-pkg hook: 00-register-pkg ...
=> Registering cdpfgl-client-0.0.6_1.i686.xbps into /host/binpkgs/sauvegarde.local ...
index: added `cdpfgl-client-0.0.6_1' (i686).
index: 8 packages registered.
=> cdpfgl-devel-0.0.6_1: running do-pkg hook: 00-gen-pkg ...
=> Creating cdpfgl-devel-0.0.6_1.i686.xbps for repository /host/binpkgs/sauvegarde.local ...
=> Creating cdpfgl-devel-32bit-0.0.6_1.x86_64.xbps for repository /host/binpkgs/sauvegarde.local/multilib ...
=> cdpfgl-devel-0.0.6_1: running post-pkg hook: 00-register-pkg ...
=> Registering cdpfgl-devel-0.0.6_1.i686.xbps into /host/binpkgs/sauvegarde.local ...
index: added `cdpfgl-devel-0.0.6_1' (i686).
index: 8 packages registered.
=> Registering cdpfgl-devel-32bit-0.0.6_1.x86_64.xbps into /host/binpkgs/sauvegarde.local/multilib ...
index: added `cdpfgl-devel-32bit-0.0.6_1' (x86_64).
index: 5 packages registered.
=> cdpfgl-lib-0.0.6_1: running do-pkg hook: 00-gen-pkg ...
=> Creating cdpfgl-lib-0.0.6_1.i686.xbps for repository /host/binpkgs/sauvegarde.local ...
=> Creating cdpfgl-lib-32bit-0.0.6_1.x86_64.xbps for repository /host/binpkgs/sauvegarde.local/multilib ...
=> cdpfgl-lib-0.0.6_1: running post-pkg hook: 00-register-pkg ...
=> Registering cdpfgl-lib-0.0.6_1.i686.xbps into /host/binpkgs/sauvegarde.local ...
index: added `cdpfgl-lib-0.0.6_1' (i686).
index: 8 packages registered.
=> Registering cdpfgl-lib-32bit-0.0.6_1.x86_64.xbps into /host/binpkgs/sauvegarde.local/multilib ...
index: added `cdpfgl-lib-32bit-0.0.6_1' (x86_64).
index: 5 packages registered.
=> cdpfgl-server-0.0.6_1: running do-pkg hook: 00-gen-pkg ...
=> Creating cdpfgl-server-0.0.6_1.i686.xbps for repository /host/binpkgs/sauvegarde.local ...
=> cdpfgl-server-0.0.6_1: running post-pkg hook: 00-register-pkg ...
=> Registering cdpfgl-server-0.0.6_1.i686.xbps into /host/binpkgs/sauvegarde.local ...
index: added `cdpfgl-server-0.0.6_1' (i686).
index: 8 packages registered.
```

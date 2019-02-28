% cdpfglclient(1) cdpfglclient user manual | version 0.0.12

# NAME

**cdpfglclient** - saves your files to a cdpfglserver while being modified or created


# SYNOPSIS

**cdpfglclient** [options]


# DESCRIPTION

cdpfglclient will monitor the file system for file creation or changes and will save them to a cdpfglserver that might be up and working.


# COMMAND LINE OPTIONS

   Default options configuration values may be replaced by values read from a configuration file (the default one for instance) and those one may be replaced by options read from the command line.


**-h**, **--help**:

   Gives some basic help about program invocation and its options.

**-d**, **--debug=0|1**:

   When invoked with 0 debug mode is turned off. Debug mode is turned on when invoked with 1. When on this mode is really verbose and may slow down the program. You should not use this option in daily normal use.

**-v**, **--version**:

   Gives compiled version of cdpfglclient plus some informations about libraries that were compiled with it and also some configuration informations as the program has loaded them.

**-c**, **--configuration=FILENAME**:

   Specify an alternative configuration file. This file is read instead of the default one.

**-b**, **--blocksize=SIZE**:

   Fixed block SIZE (in bytes) used to compute hashs (default is 16384). This option is not taken into account when adaptive blocksize option is set to 1.

**-a**, **--adaptive=BOOLEAN**:

   Adaptive block size used to compute hashs. Blocks have sizes that depends on their file size.

**-s**, **--buffersize=SIZE**:

   SIZE (in bytes) of the cache used to send data to server. For correct operations SIZE value should not be less than 1048576 (the default size).

**-r**, **--dircache=DIRNAME**:

   Directory DIRNAME where to cache files. DIRNAME default is `/var/tmp/cdpfgl`. In that directory will be saved an sqlite file cache used to cache things for the client.

**-f**, **--dbname=FILENAME**:

   Database FILENAME, the cache file (default is `filecache.db`).

**-i**, **--ip=IP**:

   IP address where server program is waiting for the client to send POST and GET commands.

**-p**, **--port=NUMBER**:

   Port NUMBER on which to listen (default is 5468).

**-x**, **--exclude=FILENAME**:

   Exclude FILENAME from being saved. You can specify more than once this option to exclude more files or directories. This option will add excluded directories to the list of excluded directory obtained from a specified configuration file if any.

**-n**, **--no-scan**:

   Does not do the first directory scan.

**-z TYPE**, **--compression=TYPE**:

   Allow to choose compression TYPE used by the cdpfglclient. 0 means no compression at all and 1 uses zlib (gz compression type). Other values may end the program with an error.


# CONFIGURATION FILE

By default the configuration file is named `/etc/cdpfgl/client.conf`. It is organised in sections [Example section]. By convention section names begins with a capital. Sections contains some key=value definitions. It is strongly recommended to modify and adapt client.conf file as it contains only example values that will not fit your needs.


# SEE ALSO

**cdpfglrestore**(1), **cdpfglserver**(1)


# FILES

`/etc/cdpfgl/client.conf` is the default configuration file.


# BUGS

Please report bugs by either filling an issue at [https://github.com/dupgit/sauvegarde/issues](https://github.com/dupgit/sauvegarde/issues "Github Issues") or a pull request at [https://github.com/dupgit/sauvegarde/pulls](https://github.com/dupgit/sauvegarde/pulls "Github PR") or by sending a mail to Olivier Delhomme <olivier.delhomme@free.fr>


# AUTHORS

Authors in alphabetical order are :

* Pierre Bourgin <pierre.bourgin@free.fr>  
* Olivier Delhomme <olivier.delhomme@free.fr>  
* Sebastien Tricaud <sebastien@honeynet.org>  

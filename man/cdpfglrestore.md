% cdpfglrestore(1) cdpfglrestore user manual | version 0.0.10

# NAME

**cdpfglrestore** - restores already saved files from a cdpfglserver to your local filesystem.


# SYNOPSIS

**cdpfglrestore** [options]


# DESCRIPTION

cdpfglrestore will restore already saved files from a cdpfglserver to your local filesystem. cdpfglrestore will not overwrite an existing file, in that case it will create a backup file before restoring over it.


# COMMAND LINE OPTIONS

   Default options configuration values may be replaced by values read from a configuration file (the default one for instance) and those one may be replaced by options read from the command line.


**-h**, **--help**:

   Gives some basic help about program invocation and its options.

**-d**, **--debug=0|1**:
   
   When invoked with 0 debug mode is turned off. Debug mode is turned on when invoked with 1. When on this mode is really verbose and may slow down the program. You should not use this option in daily normal use.

**-v**, **--version**:

   Gives compiled version of cdpfglrestore plus some informations about libraries that were compiled with it and also some configuration informations as the program has loaded them.  

**-c**, **--configuration=FILENAME**:

   Specify an alternative configuration file. This file is read instead of the default one.  

**-l**, **--list=REGEX**:

   Gives a list of saved files that correspond to the given REGEX. For instance one may use --list=\.mp3$ to list all files ending with '.mp3' or -l ^/home/me/tmp to list all files from the '/home/me/tmp' directory. 

**-r**, **--restore=REGEX**:

   Restores requested filename (REGEX). It restores the latest file in the returned list with -l option.

**-t**, **--date=DATE**:

   Lists or restores the selected file at that specific DATE (YYYY-MM-DD HH:MM:SS format).

**-a**, **--after=DATE**:

   Lists or restores the selected file with mtime after DATE (YYYY-MM-DD HH:MM:SS format).

**-b**, **--before=DATE**:

   Lists or restores the selected file with mtime before DATE (YYYY-MM-DD HH:MM:SS format).

**-e**, **--all-versions**:

   Lists or restores all versions of a file.

**-f**, **--all-files**:

   Restores all files found by -r REGEX (or -l REGEX). This option has no real effect with -l option and only works when restoring in conjunction with -r option.

**-g**, **--latest**:

   Filters out all versions of a file to keep only the latest one.

**-P**, **--parents**:

   Restores files with their full path and creates directories if needed. For instance, with this option, restoring `/home/dup/directory1/example.txt` file in `/tmp/restore` directory will create `/tmp/restore/home/dup/directory1/example.txt` file.

**-w**, **--where=DIRECTORY**:

   Specify a DIRECTORY where to restore a file.

**-i**, **--ip=IP**:

   IP address where server program is waiting for the restore program to send POST and GET commands.

**-p**, **--port=NUMBER**:

   Port NUMBER on which to listen (default is 5468).


# SEE ALSO

**cdpfglclient**(1), **cdpfglserver**(1)


# FILES

`/etc/cdpfgl/restore.conf` is the default configuration file.


# BUGS

Please report bugs by either filling an issue at [https://github.com/dupgit/sauvegarde/issues](https://github.com/dupgit/sauvegarde/issues "Github Issues") or a  pull request at [https://github.com/dupgit/sauvegarde/pulls](https://github.com/dupgit/sauvegarde/pulls "Github PR") or by sending a mail to Olivier Delhomme <olivier.delhomme@free.fr>


# AUTHORS

Authors in alphabetical order are :

* Pierre Bourgin <pierre.bourgin@free.fr>  
* Olivier Delhomme <olivier.delhomme@free.fr>  
* Sebastien Tricaud <sebastien@honeynet.org>  

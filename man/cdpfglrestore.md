% cdpfglrestore(1) cdpfglrestore user manual | version 0.0.9

# NAME

**cdpfglrestore** - restores already saved files from a cdpfglserver to your local filesystem.

# SYNOPSIS

**cdpfglrestore** [options]

# DESCRIPTION

cdpfglrestore will restore already saved files from a cdpfglserver to your local filesystem.

# COMMAND-LINE OPTIONS

-h, --help Gives some basic help about program invocation and its options.  
-d, --debug=0|1 When invoked with 0 debug mode is turned off. Debug mode is turned on when invoked with 1. When on this mode is really verbose and may slow down the program. You should not use this option in daily normal use.  
-v, --version Gives compiled version of cdpfglrestore plus some informations about libraries that were compiled with it and also some configuration informations as the program has loaded them.  
-c, --configuration=FILENAME Specify an alternative configuration file. This file is read instead of the default one.  


# SEE ALSO

**cdpfglclient**(1), **cdpfglserver**(1)

# FILES

/etc/cdpfgl/restore.conf

# BUGS

Please report bugs by either filling a pull request at (http://example.com/ [https://github.com/dupgit/sauvegarde/pulls](https://github.com/dupgit/sauvegarde/pulls "Github") or by sending a mail to Olivier Delhomme <olivier.delhomme@free.fr>

# AUTHORS

Authors in alphabetical order are :

* Pierre Bourgin <pierre.bourgin@free.fr>  
* Olivier Delhomme <olivier.delhomme@free.fr>  
* Sebastien Tricaud <sebastien@honeynet.org>  

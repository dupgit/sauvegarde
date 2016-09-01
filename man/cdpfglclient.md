% cdpfglclient(1) cdpfglclient user manual | version 0.0.9

# NAME

**cdpfglclient** - saves your files to a cdpfglserver while being modified or created 

# SYNOPSIS

**cdpfglclient** [options]

# DESCRIPTION

cdpfglclient will monitor the filesystem for file creation or changes and will save them to a cdpfglserver that might be up and working.


# COMMAND-LINE OPTIONS

-h, --help			Gives some basic help about program invocation and its options.   
-v, --version			Gives compiled version of cdpfgclient plus some informations about libraries that were compiled with it and also some configuration informations as the program has loaded them.   
-c, --configuration=FILENAME	Specify an alternative configuration file. This file is read instead of the default one.   

# SEE ALSO

**cdpfglrestore**(1), **cdpfglserver**(1)


# FILES

/etc/cdpfgl/client.conf	Default configuration file.


# BUGS

Please report bugs by either filling a pull request at (http://example.com/ [https://github.com/dupgit/sauvegarde/pulls](https://github.com/dupgit/sauvegarde/pulls "Github") or by sending a mail to Olivier Delhomme <olivier.delhomme@free.fr> 


# AUTHORS

Authors in alphabetical order are :

- Pierre Bourgin <pierre.bourgin@free.fr>  
- Olivier Delhomme <olivier.delhomme@free.fr>  
- Sebastien Tricaud <sebastien@honeynet.org>  

# Sauvegarde

Some experiments around backup and continuous data protection (CDP).


## License

This is free software an real open source as GPLv3 is used for this
collection of programs. Feel free to contibute and/or give help to the
projects used here (see dependencies section)


## Dependencies

This project depends on :

* autotools     (2.59)
* glib and gio  (2.26)
* 0MQ           (3.2.4)
* sqlite        (3.6.20)


## Infrastucture

This is how I imagine the programs may interact themselves but it may
evolve in the future.

    hid   | -------------------------------------        -----------
    to    | | monitor + ciseaux + antememoire   |  <---> | serveur |
    user  | -------------------------------------        -----------
                                                               ^
    user                     ----------------                  |
    client                   | restauration | <----------------|
    (GUI ?)                  ----------------
    \                                          /          \          /
     ----- Client side (on a notebook) --------            - server -
                                                              side


* "monitor" may monitor a filesystem and send events to "ciseaux".
* "ciseaux" cuts files into pieces of 32768 bytes (by default) and transmits
  every pieces to "antememoire"
* "antememoire" stores everything in a local database before communicating
  with "serveur"'s main sauvegarde server.
* "serveur" is the main sauvegarde server. Each client communicates with it
  and it keeps every chunks of every files with their attributes.
* "restauration" is a tool that will provide the ability to restore some
  files or paths to some locations. It communicates directly with "serveur"
  main's sauvegarde server.


### Messages contents

I'll try to pass everything into strings.

* "\n" will be the delimiter for the fields in the string.
* The last field is the filename and the last delimiter is \x0 (thus a file
  can have a "\n" in its name).


#### Directory

    G_FILE_TYPE_DIRECTORY
    user:group uid:gid
    access_time changed_time modified_time
    dir_mode
    dirname


#### File

    G_FILE_TYPE_REGULAR
    user:group uid:gid
    access_time changed_time modified_time
    file_mode
    filename
        [
            -> number_of_chunk
            size
            checksum
        ]


## Coding into this project

### Comments

Sauvegarde uses Javadoc comment style to be parsed by doxygen. Please
continue to use this style into comments in the whole project as it will
be parsed by doxygen.


### Verifying complexity

I discovered 'lizard' a simple but excellent python program that checks the
complexity of a program (ok, how complex it look like because it does not
take into account preprocessor macros...). If you plan to hack into this
project please look at the result of this program (you may find it
following this link : [https://github.com/terryyin/lizard](https://github.com/terryyin/lizard)


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
* redirfs, pluginfs and dazuko seems dead (on 02 march 2014).
* It seems (on my system) that the more the blocksize is small the more the
  intra-deduplication rate is high (from 2.56 % at 32768 bytes to 8% at 512
  bytes).


## Usefull links

* Filesystem events notifycation :
 * redirfs  : [http://www.redirfs.org/tiki-index.php](http://www.redirfs.org/tiki-index.php)
 * pluginfs : [http://www.pluginfs.org/](http://www.pluginfs.org/)
 * dazuko   : [http://dazuko.dnsalias.org/wiki/index.php/Main_Page](http://dazuko.dnsalias.org/wiki/index.php/Main_Page)
 * fanotify : [http://www.xypron.de/projects/fanotify-manpages/man7/fanotify.7.html](http://www.xypron.de/projects/fanotify-manpages/man7/fanotify.7.html)
* sqlite    : [http://www.sqlite.org/cintro.html](http://www.sqlite.org/cintro.html)
* zmq       : [http://czmq.zeromq.org/](http://czmq.zeromq.org/)
* Software quality checkers :
 * lizard   : [https://github.com/terryyin/lizard](https://github.com/terryyin/lizard)




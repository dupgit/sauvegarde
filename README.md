Sauvegarde
==========

Some experiments around backup and continuous data protection (CDP)


Infrastucture
-------------

This is how I imagine the programs may interact themselves but it may
evolve in the future.

hid   | -----------      -----------      ---------------        -----------
to    | | monitor | ---> | ciseaux | ---> | antememoire |  <---> | serveur |
user  | -----------      -----------      ---------------        -----------
                                                                      ^
user                                ----------------                  |
client                              | restauration | <----------------|
(GUI ?)                             ----------------
\                                                      /        \          /
 ------------ Client side (on a notebook) -------------          - server -
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


Comments
--------

Sauvegarde uses Javadoc comment style to be parsed by doxygen. Please
continue to use this style into comments in the whole project as it will
be parsed by doxygen.


Learnt things from experiments
------------------------------

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


Usefull links
-------------
* redirfs  : [http://www.redirfs.org/tiki-index.php](http://www.redirfs.org/tiki-index.php)
* pluginfs : [http://www.pluginfs.org/](http://www.pluginfs.org/)
* sqlite   : [http://www.sqlite.org/cintro.html](http://www.sqlite.org/cintro.html)
* zmq      : [http://czmq.zeromq.org/](http://czmq.zeromq.org/)




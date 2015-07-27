# Infrastructure

This is how I imagine the programs may interact themselves but it may
evolve in the future.

    hid   | -------------------------------------        -----------
    to    | | monitor + ciseaux + antememoire   |  <---> | serveur |
    user  | -------------------------------------        -----------
                                                               ^
    user                         ------------                  |
    client                       | restaure | <----------------|
    (GUI ?)                      ------------
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
* "restaure" is a tool that will provide the ability to restore some
  files or paths to some locations. It communicates directly with "serveur"
  main's sauvegarde server.


## Messages contents

Between threads messages are passed via pointers to C structures into
GAsyncQueue (glib). Binary data are not transformed and structures are
passed in place (no copy in memory).

Between programs messages are passed throught HTTP protocol via JSON
structured messages (jansson). All binary data are transformed into base64
encoded strings.

msg_id field is used to identify message type. It is based on ENC_* macros
defined in packing.h. ENC_META_DATA indicates that the JSON string contains
a serveur_meta_data_t structure.


### Directory

    G_FILE_TYPE_DIRECTORY
    user:group uid:gid
    access_time changed_time modified_time
    dir_mode size
    dirname

As an example a JSON structure for a directory looks like :

    {"msg_id": 1,
     "hash_list": [],
     "filetype": 2,
     "inode": 23456,
     "group": "admin",
     "mode": 16877,
     "owner": "dup",
     "atime": 1401024480,
     "uid": 530,
     "ctime": 1401024479,
     "name": "/home/dup/Dossiers_Perso/projets/sauvegarde/monitor/.deps",
     "mtime": 1401024479,
     "gid": 530,
     "fsize": 0
    }


### File

    G_FILE_TYPE_REGULAR
    user:group uid:gid
    access_time changed_time modified_time
    file_mode size
    filename
        [
            -> number_of_chunk
            size
            checksum
        ]

As an example a JSON structure for a file looks like :

    {"hash_list": ["0CVUtILbsnYD3Royz7Yu8sOSxjl9f/b+b6wPa9DCRTY=",
                   "AAXUaflNSSHEbZnjVAiY1b2Fz7YvQHwle/iZUHct09U=",
                   "MunCygnx7AU/49Kdo+H3s72OludZ7KjzfA4k1ui2NAw="],
     "msg_id": 1,
     "filetype": 1,
     "group": "admin",
     "inode": 284938,
     "mode": 33188,
     "owner": "dup",
     "atime": 1401023677,
     "uid": 530,
     "ctime": 1401023685,
     "name": "/home/dup/Dossiers_Perso/projets/sauvegarde/config.log",
     "mtime": 1401023685,
     "gid": 530,
     "fsize": 37803
    }


## Database (local cache)

For now Information is stored with the following scheme :

    --- files ----          -- buffers ---
    | file_id *  |     /n-> | cache_time |
    | cache_time | <-1/     | buf_order  |         --- data ----
    | type       |          | checksum   | <-n,1-> | checksum *|
    | file_user  |          --------------         | size      |
    | file_group |                                 | data      |
    | uid        |                                 -------------
    | gid        |
    | atime      |
    | ctime      |
    | mtime      |
    | mode       |
    | size       |
    | name       |
    --------------

Buffer order has to be kept. In the SQLITE cache we can keep it in an
explicit manner (by storing the order of the buffer of the checksum). In
the programs (when we pass things into memory with C structure or into
JSON formatted message) We keep buffer order in an implicit manner (by
storing the ordered list of checksums of a file). Fields marked with '*'
are primary keys.

SQLITE is not used with asynchronous mode (PRAGMA synchronous = OFF;). When
asynchronous mode is used all reads to the database for records that has
just been written to fails (nothing has been written at all!). Even if it
is really faster (by 19 times) it leads to data inconsistancy and may lead
to database corruption and/or data loss.


## API

The API is described in [API.md](docs/API.md)


## Server backends

### File backend

Stores metas datas into flat files and datas directly in directories and
subdirectories named by their hash. Default level of indirection is 2. This
means that each hash is stored in 2 subdirectories : beef0345... is stored
in /be/ef/beef0345... with level 2 and in /be/ef/03/beef0345.... with level
3.

With level 2 one may store up to 512 Gb of deduplicated datas. Level 3 can
store up to 256 Tb of datas and level 4 up to 65536 Tb ! Keep in mind also
that creating such amount of directories takes space : 256 Mb for level 2,
64 Gb for level 3 and 16 Tb for level 4 ! Also it may take a long time to
create those directories: level 3 took nearly 1 hour on my system where
level 2 took only 2 seconds !

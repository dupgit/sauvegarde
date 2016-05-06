# Infrastructure

This is how I imagine the programs may interact themselves but it may
evolve in the future.

    hid   | ----------                                   -----------
    to    | | client |  <------------------------------> | server |
    user  | ----------                                   -----------
                                                              ^
    user                  -----------                         |
    client                | restore | <-----------------------|
    (GUI ?)               -----------
    \                                       /            \          /
     ----- Client side (on a notebook) -----              - server -
                                                             side


* "client" carve and monitors a filesystem, cuts files into pieces of
  16384 bytes (by default) and transmits every pieces along side with
  meta data of each files to server, the server that saves everything.

* "server" is the main cdpfgl server. Each client communicates with it
  and it keeps every chunks of every files with their attributes.

* "restore" is a tool that will provide the ability to restore some
  files or paths to some locations. It communicates directly with "cdpfglserver"
  main's cdpfgl server.


## Messages contents

Between threads messages are passed via pointers to C structures into
GAsyncQueue (glib). Binary data are not transformed and structures are
passed in place (no copy in memory).

Between programs messages are passed throught HTTP protocol via JSON
structured messages (jansson). All binary data are transformed into base64
encoded strings.

msg_id field is used to identify message type. It is based on ENC_* macros
defined in packing.h. ENC_META_DATA indicates that the JSON string contains
a server_meta_data_t structure.


### Directory

    G_FILE_TYPE_DIRECTORY
    user:group uid:gid
    access_time changed_time modified_time
    dir_mode size
    dirname

As an example a JSON structure for a directory looks like:

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
     "name": "/home/dup/Dossiers_Perso/projets/sauvegarde/client/.deps",
     "link": "",
     "hostname": "d630",
     "mtime": 1401024479,
     "gid": 530,
     "fsize": 0,
     "data_sent": 0
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

As an example a JSON structure for a file looks like:

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
     "link": "",
     "hostname": "d630",
     "mtime": 1401023685,
     "gid": 530,
     "fsize": 37803,
     "data_sent": 1
    }


## Database (local cache)

For now Information is stored with the following scheme:

    --- files ----          -- buffers ---    - transmited -
    | file_id *  |          | buffer_id *|    | buffer_id *|
    | cache_time |          | url        |    --------------
    | inode      |          | data       |
    | type       |          --------------
    | file_user  |
    | file_group |
    | uid        |
    | gid        |
    | atime      |
    | ctime      |
    | mtime      |
    | mode       |
    | size       |
    | name       |
    | transmitted|
    | link       |
    --------------

Two indexes are created: transmited_buffer_id which indexes buffer_id
from transmited table in ascending order and files_inodes which indexes
inode from files table in ascending order.

Buffer order has to be kept. In the programs (when we pass things into
memory with C structure or into JSON formatted message) we keep buffer
order in an implicit manner (by storing the ordered list of checksums
of a file). So we store every JSON buffer we should have sent to server
into a simple table named buffers. 'file_id' and 'buffer_id' fields
marked with '*' are primary keys.

SQLITE is not used with asynchronous mode (PRAGMA synchronous = OFF;). When
asynchronous mode is used all reads to the database for records that has
just been written to fails (nothing has been written at all!). Even if it
is really faster (by 19 times) it leads to data inconsistancy and may lead
to database corruption and/or data loss.


## API

The API is described in [API.md](docs/API.md)


## Server backends

### File backend

Stores meta data into flat files and data directly in directories and
subdirectories named by their hash. Default level of indirection is 2. This
means that each hash is stored in 2 subdirectories: beef0345... is stored
in /be/ef/beef0345... with level 2 and in /be/ef/03/beef0345.... with level
3.

Considering that we do not want more than 256 files in (level + 1) and
considering that each hash saved has the default size of 16 Kb then
with level 2 one may store up to 512 Gb of deduplicated data. Level 3 can
store up to 256 Tb of data and level 4 up to 65536 Tb ! Keep in mind also
that creating such amount of directories takes space: 256 Mb for level 2,
64 Gb for level 3 and 16 Tb for level 4 ! Also it may take a long time to
create those directories: level 3 took nearly 1 hour on my system where
level 2 took only 2 seconds (the hard drive was a SSD at that time)!

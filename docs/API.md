# API

All urls ending with .json waits or provide json strings.


## GET

### /Version

Gets the version of the serveur in a more human readable than a json string
producing an output such as :

    serveur version : 0.0.1-cb6c92b (27 04 2014)
    Author(s) : Olivier DELHOMME <olivier.delhomme@free.fr>
    License : GPL v3 or later

    serveur was compiled with the following libraries:
        . GLIB version : 2.36.3
        . LIBMHD : 0.9.39-2
        . SQLITE version : 3.8.6
        . JANSSON version : 2.7.0

    serveur options are :
    Configuration file : /home/dup/local/bin/../etc/sauvegarde/serveur.conf
    Port number : 5468


### /Version.json

Gets the version of the serveur in a json parsable string producing an
output such as :

    {"revision": "cb6c92b",
     "name": "serveur",
     "date": "27 04 2014",
     "librairies": [{"glib": "2.36.3"},
                    {"mhd": "0.9.39-2"},
                    {"sqlite": "3.8.6"},
                    {"jansson": "2.7.0"}],
     "authors": ["Olivier DELHOMME <olivier.delhomme@free.fr>"],
     "version": "0.0.1",
     "licence": "GPL v3 or later"}


## POST

### /Meta.json

Waits for a json string with "hash_list", "filetype", "group", "mode",
"owner", "atime", "uid", "ctime", "name", "mtime", "gid", "fsize" meta-data
fields and a "msg_id" field in no specific order. "hash_list" field should
contains hashs of data in an ordered way and base64 encoded.

The serveur answers a hash_list that may be empty. This hash list is the
hashs that the server needs. The server should not ask for hash that it
already has. Example :

    {"hash_list": ["QKlVxnrQi/A338uHJUSKw3C+RfmJ2KUdiZPMru/Z9kE=",
                   ...,
                   "nZjIbSp6tJHAQJ71rzVt/KGoTeX4BXDLYvXx/PQM5j8="]}


### /Data.json

Waits for a json string with "hash", "data" and "size" fields. hash
and data fields must be base 64 encoded.

    {"hash": "QKlVxnrQi/A338uHJUSKw3C+RfmJ2KUdiZPMru/Z9kE=",
     "data": "8oBc08li ... WW9mr20u4=",
     "size": 12140}
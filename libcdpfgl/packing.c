/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    packing.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2019 Olivier Delhomme
 *     e-mail : olivier.delhomme@free.fr
 *
 *    "Sauvegarde" is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    "Sauvegarde" is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with "Sauvegarde".  If not, see <http://www.gnu.org/licenses/>
 */

/**
 * @file packing.c
 * This file contains the functions to pack messages for all the
 * programs of "Sauvegarde" project.
 */

#include "libcdpfgl.h"

static void insert_guint8_into_json_root(json_t *root, gchar *keyname, guint8 number);
static void insert_guint32_into_json_root(json_t *root, gchar *keyname, guint32 number);
static void insert_guint64_into_json_root(json_t *root, gchar *keyname, guint64 number);
static void insert_gshort_into_json_root(json_t *root, gchar *keyname, gshort number);
static json_t *create_json_code(guint32 http_code, gchar *message);
static json_t *create_json_answer(gchar *type, guint32 http_code, gchar *message);


/**
 * Inserts an integer value into a json…t *root array
 * @param[in,out] root is the root that will contain all meta data values
 * @param keyname is the keyname associated with the value (in fact it is
 *        variable's name)
 * @param value is the integer value to insert into root
 */
void insert_integer_value_into_json_root(json_t *root, gchar *keyname, guint64 value)
{
    json_t *nbr = NULL;

    nbr = json_integer(value);
    insert_json_value_into_json_root(root, keyname, nbr);
}


/**
 * Inserts a json_t *value into the json_t *root array.
 * @param[in,out] root is the root that will contain all meta data values
 * @param keyname is the keyname associated with the value (in fact it is
 *        variable's name)
 * @param value is the json_t "encoded" value to insert into the root
 */
void insert_json_value_into_json_root(json_t *root, gchar *keyname, json_t *value)
{
    int result = 0;

    if (root != NULL && keyname != NULL && value != NULL)
        {
            result = json_object_set_new(root, keyname, value);

            if (result != JANSSON_SUCCESS)
                {
                    print_error( __FILE__, __LINE__, _("Error while converting to JSON\n"));
                    exit(EXIT_FAILURE); /* An error here means that we will do nothing good */
                }
        }
}


/**
 * appends a string into the array (the array is ordered and should
 * not mess itself)
 * @param[in,out] array is an array of strings (may be hashs of filenames
 *                for instance).
 * @param to_append is the string to be appended to the array
 */
void append_string_to_array(json_t *array, gchar *to_append)
{
    json_t *string = NULL;

    if (array != NULL && to_append != NULL)
        {
            string = json_string_nocheck((const char *) to_append);

            json_array_append_new(array, string);
        }
}


/**
 * Inserts the boolean data_sent into the json tree root with key keyname.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we will insert a new value
 * @param data_sent is the boolean value to be inserted with key keyname.
 */
void insert_boolean_into_json_root(json_t *root, gchar *keyname, gboolean data_sent)
{
    json_t *bool = NULL;

    if (root != NULL && keyname != NULL)
        {
            if (data_sent == TRUE)
                {
                    bool = json_true();
                }
            else
                {
                    bool = json_false();
                }

            insert_json_value_into_json_root(root, keyname, bool);
        }
}


/**
 * Inserts the string a_string into the json tree root with key keyname.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we will insert a new value
 * @param a_string is the value to be inserted with key keyname.
 */
void insert_string_into_json_root(json_t *root, gchar *keyname, gchar *a_string)
{
    json_t *str = NULL;

    if (root != NULL && keyname != NULL && a_string != NULL)
        {
            str = json_string_nocheck((const char *) a_string);
            insert_json_value_into_json_root(root, keyname, str);
        }
}


/**
 * Inserts the guint8 number into the json tree root with key keyname.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we will insert a new value
 * @param number is the value to be inserted with key keyname.
 */
static void insert_guint8_into_json_root(json_t *root, gchar *keyname, guint8 number)
{
    json_t *value = NULL;

    if (root != NULL && keyname != NULL)
        {
            value = json_integer(number);
            insert_json_value_into_json_root(root, keyname, value);
        }
}


/**
 * Inserts the guint32 number into the json tree root with key keyname.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we will insert a new value
 * @param number is the value to be inserted with key keyname.
 */
static void insert_guint32_into_json_root(json_t *root, gchar *keyname, guint32 number)
{
    json_t *value = NULL;

    if (root != NULL && keyname != NULL)
        {
            value = json_integer(number);
            insert_json_value_into_json_root(root, keyname, value);
        }
}


/**
 * Inserts the guint64 number into the json tree root with key keyname.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we will insert a new value
 * @param number is the value to be inserted with key keyname.
 */
static void insert_guint64_into_json_root(json_t *root, gchar *keyname, guint64 number)
{
    json_t *value = NULL;

    if (root != NULL && keyname != NULL)
        {
            value = json_integer(number);
            insert_json_value_into_json_root(root, keyname, value);
        }
}


/**
 * Inserts the guint64 number into the json tree root with key keyname.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we will insert a new value
 * @param number is the value to be inserted with key keyname.
 */
static void insert_gshort_into_json_root(json_t *root, gchar *keyname, gshort number)
{
    json_t *value = NULL;

    if (root != NULL && keyname != NULL)
        {
            value = json_integer(number);
            insert_json_value_into_json_root(root, keyname, value);
        }
}


/**
 * Converts the hash list to a json_t * array
 * @param hash_list : the GList * list of hashs
 * @returns a json_t * array with the element of the list in it (if any).
 */
json_t *convert_hash_list_to_json(GList *hash_list)
{
    json_t *array = NULL;           /** json_t *array is the array that will receive base64 encoded hashs        */
    gchar *encoded_hash = NULL;     /** gchar encoded_hash is an hash base64 encoded                             */
    GList *head = NULL;             /** GSList *head is a list to iter over that will contain the hash data list */
    hash_data_t *hash_data = NULL;  /** A pointer to get the hash_data structure */

    /* creating an array with the whole hash list */
    array = json_array();
    head = hash_list;

    while (head != NULL)
        {
            hash_data = head->data;
            encoded_hash = g_base64_encode(hash_data->hash, HASH_LEN);

            append_string_to_array(array, encoded_hash);

            free_variable(encoded_hash);

            head = g_list_next(head);
        }

    return array;
}


/**
 * Converts the file list (a list of gchar *) to a json_t * array
 * @param file_list : the GSList * list of hashs
 * @returns a json_t * array with the element of the list in it (if any).
 */
json_t *convert_file_list_to_json(GSList *file_list)
{
    json_t *array = NULL;       /** json_t *array is the array that will receive base64 encoded hashs   */
    GSList *head = NULL;        /** GSList *head is a list to iter over that will contain the hash list */

    /* creating an array with the whole hash list */
    array = json_array();
    head = file_list;

    while (head != NULL)
        {
            append_string_to_array(array, head->data);

            head = g_slist_next(head);
        }

    return array;
}


/**
 * Converts the file list (a list of gchar *) to a json string
 * @param file_list : the GSList * list of hashs
 * @returns a gchar * string json formatted with all files (if any) in a
 *          json array
 */
gchar *convert_file_list_to_json_string(GSList *file_list)
{
    json_t *array = NULL;
    json_t *root = NULL;
    gchar *string = NULL;

    root = json_object();

    array = convert_file_list_to_json(file_list);

    insert_json_value_into_json_root(root, "file_list", array);

    string = json_dumps(root, 0);

    json_decref(array);
    json_decref(root);

    return string;
}


/**
 * Converts hash_data_t structure to a json_t * structure
 * @param hash_data the hash_data_t structure that contains the data to
 *        be converted.
 * @returns a json_t * structure with informations of hash_data in it
 */
json_t *convert_hash_data_t_to_json(hash_data_t *hash_data)
{
    gchar *encoded_data = NULL;
    gchar *encoded_hash = NULL;
    json_t *root = NULL;

    if (hash_data != NULL && hash_data->data != NULL && hash_data->hash != NULL && hash_data->read >= 0)
        {
            encoded_data = g_base64_encode((guchar*) hash_data->data, hash_data->read);
            encoded_hash = g_base64_encode((guchar*) hash_data->hash, HASH_LEN);

            root = json_object();
            insert_string_into_json_root(root, "hash", encoded_hash);
            insert_string_into_json_root(root, "data", encoded_data);
            insert_guint64_into_json_root(root, "size", hash_data->read);
            insert_gshort_into_json_root(root, "cmptype", hash_data->cmptype);
            insert_guint64_into_json_root(root, "uncmpsize", hash_data->uncmplen);
            free_variable(encoded_data);
            free_variable(encoded_hash);
        }

    return root;
}


/**
 * Converts hash_data_t structure  to a json formatted string.
 * @param hash_data the hash_data_t structure that contains the data to
 *        be converted.
 * @returns a json formatted string with those informations
 */
gchar *convert_hash_data_t_to_string(hash_data_t *hash_data)
{
    gchar *json_str = NULL;
    json_t *root = NULL;

    root =  convert_hash_data_t_to_json(hash_data);

    if (root != NULL)
        {
            json_str = json_dumps(root, 0);
            json_decref(root);
        }

    return json_str;
}



/**
 * This function should return a JSON object with all informations from
 * the meta_data_t structure.
 * @param meta is the structure that contains all meta data for a file or
 *        a directory.
 * @param hostname is the name of the host onw hich we are running and that
 *        we want to include into the json string.
 * @param data_sent is a boolean that is TRUE when data has already been
 *        sent to server, FALSE otherwise.
 * @returns a json_t structure or NULL
 */
json_t *convert_meta_data_to_json(meta_data_t *meta, const gchar *hostname, gboolean data_sent)
{
    json_t *root = NULL;        /** json_t *root is the root that will contain all meta data json       */
    json_t *array = NULL;       /** json_t *array is the array that will receive base64 encoded hashs   */

    if (meta != NULL)
        {
            root = json_object();

            insert_guint8_into_json_root(root, "msg_id", ENC_META_DATA);
            insert_guint8_into_json_root(root, "filetype", meta->file_type);
            insert_guint32_into_json_root(root, "mode", meta->mode);

            insert_guint64_into_json_root(root, "atime", meta->atime);
            insert_guint64_into_json_root(root, "ctime", meta->ctime);
            insert_guint64_into_json_root(root, "mtime", meta->mtime);
            insert_guint64_into_json_root(root, "fsize", meta->size);
            insert_guint64_into_json_root(root, "inode", meta->inode);

            insert_string_into_json_root(root, "owner", meta->owner);
            insert_string_into_json_root(root, "group", meta->group);

            insert_guint32_into_json_root(root, "uid", meta->uid);
            insert_guint32_into_json_root(root, "gid", meta->uid);

            insert_string_into_json_root(root, "name", meta->name);
            insert_string_into_json_root(root, "link", meta->link);
            insert_string_into_json_root(root, "hostname", (gchar *) hostname);
            insert_boolean_into_json_root(root, "data_sent", data_sent);

            array = convert_hash_list_to_json(meta->hash_data_list);

            insert_json_value_into_json_root(root, "hash_list", array);
        }

    return root;
}


/**
 * This function should return a JSON string with all informations from
 * the meta_data_t structure.
 * @param meta is the structure that contains all meta data for a file or
 *        a directory.
 * @param hostname is the name of the host onw hich we are running and that
 *        we want to include into the json string.
 * @param data_sent is a boolean that is TRUE when data has already been
 *        sent to server, FALSE otherwise.
 * @returns a JSON formated string or NULL
 */
gchar *convert_meta_data_to_json_string(meta_data_t *meta, const gchar *hostname, gboolean data_sent)
{
    json_t *root = NULL;        /** json_t *root is the root that will contain all meta data json       */
    gchar *json_str = NULL;     /** gchar *json_str is the string to be returned at the end             */


    if (meta != NULL)
        {
            root = convert_meta_data_to_json(meta, hostname, data_sent);
            json_str = json_dumps(root, 0);
            json_decref(root);
        }

    return json_str;
}


/**
 * Converts meta_data_t list to json array
 * @param list is a GList * of meta_data_t to be converted
 * @param hostname is the name of the host onw hich we are running and that
 *        we want to include into the json string.
 * @param data_sent is a boolean that is TRUE when data has already been
 *        sent to server, FALSE otherwise.
 * @returns a json array
 */
json_t *convert_meta_data_list_to_json_array(GList *list, gchar *hostname, gboolean data_sent)
{
    GList *head = list;
    json_t *array = NULL;
    json_t *meta_json = NULL;

    array = json_array();

    while (head != NULL)
        {
            meta_json = convert_meta_data_to_json((meta_data_t *) head->data, hostname, data_sent);
            json_array_append_new(array, meta_json);
            head = g_list_next(head);
        }

    return array;
}


/**
 * Converts to a json gchar * string. Used only by server's program
 * @param name : name of the program of which we want to print the version.
 * @param date : publication date of this version
 * @param version : version of the program.
 * @param authors : authors that contributed to this program
 * @param license : license in use for this program and its sources
 * @returns a newlly allocated gchar * string in json format that can be
 *          freed when no longer needed.
 */
gchar *convert_version_to_json(gchar *name, gchar *date, gchar *version, gchar *authors, gchar *license)
{
    json_t *root = NULL;    /** json_t *root is the root that will contain all data in json format     */
    json_t *libs = NULL;    /** json_t *libs is the array that will contain all libraries and versions */
    json_t *auths = NULL;   /** json_t *auths is the array containing all authors                      */
    json_t *objs = NULL;    /** json_t *objs will store version of libraries                           */
    gchar *buffer = NULL;
    gchar *json_str = NULL; /** gchar *json_str is the string to be returned at the end                */


    root = json_object();

    insert_string_into_json_root(root, "name", name);
    insert_string_into_json_root(root, "date", date);
    insert_string_into_json_root(root, "version", version);
    insert_string_into_json_root(root, "revision", REVISION);
    insert_string_into_json_root(root, "licence", license);

    /**
     * @todo use g_strsplit to split authors string if more than one author
     * is in the string.
     */
    auths = json_array();
    json_array_append_new(auths, json_string(authors));
    insert_json_value_into_json_root(root, "authors", auths);



    libs = json_array();

    /* glib */
    buffer = g_strdup_printf("%d.%d.%d", glib_major_version, glib_minor_version, glib_micro_version);
    objs = json_object();
    json_object_set_new(objs, "glib", json_string(buffer));
    json_array_append_new(libs, objs);
    free_variable(buffer);

    /* libmicrohttpd */
    buffer = make_MHD_version();
    objs = json_object();
    json_object_set_new(objs, "mhd", json_string(buffer));
    json_array_append_new(libs, objs);
    free_variable(buffer);

    /* sqlite */
    buffer = g_strdup_printf("%s", db_version());
    objs = json_object();
    json_object_set_new(objs, "sqlite", json_string(buffer));
    json_array_append_new(libs, objs);
    free_variable(buffer);

    /* jansson ! */
    buffer = g_strdup_printf("%d.%d.%d", JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION, JANSSON_MICRO_VERSION);
    objs = json_object();
    json_object_set_new(objs, "jansson", json_string(buffer));
    json_array_append_new(libs, objs);
    free_variable(buffer);

    /* zilb */
    buffer = g_strdup_printf("%s", ZLIB_VERSION);
    objs = json_object();
    json_object_set_new(objs, "zlib", json_string(buffer));
    json_array_append_new(libs, objs);
    free_variable(buffer);

    insert_json_value_into_json_root(root, "librairies", libs);

    json_str = json_dumps(root, 0);

    json_decref(root);

    return json_str;

}


/**
 * Encodes a gchar string into a base64 formated gchar * string.
 * @param string is the gchar string to be encoded (MUST be 0 terminated).
 * @returns a newly allocated gchar * string in base64 or NULL.
 */
gchar *encode_to_base64(gchar *string)
{
    gchar *encoded_string = NULL;

    if (string != NULL)
        {
            encoded_string = g_base64_encode((const guchar *) string, strlen(string));
        }

    return encoded_string;
}


/**
 * Makes a 'code' json objet
 * @param http_code is an integer that represents the HTTP code number.
 * @param is the message associated with the code number.
 * @returns a json_t object containing 'code' and 'message' keys and their
 *          corresponding values.
 */
static json_t *create_json_code(guint32 http_code, gchar *message)
{
    json_t *code = NULL;

    code = json_object();

    insert_guint32_into_json_root(code, "code", http_code);
    insert_string_into_json_root(code, "message", message);

    return code;
}


/**
 * Makes a json object answer message
 * @param type is a string that should represents the type of the message.
 *        For instance it may be 'error' or 'success' or 'information'.
 * @param http_code is an integer that represents the HTTP code number.
 * @param is the message associated with the HTTP CODE.
 * @returns a json_t object containing the type in json format containing
 *          'code' and 'message' keys and their corresponding values.
 */
static json_t *create_json_answer(gchar *type, guint32 http_code, gchar *message)
{
    json_t *answer = NULL;
    json_t *code = NULL;

    answer = json_object();
    code = create_json_code(http_code, message);
    insert_json_value_into_json_root(answer, type, code);

    return answer;
}


/**
 * Makes a json answer error message string.
 * @param error_code is an integer that represents the error number.
 * @param is the message associated with the error.
 * @returns a gchar * string containing the 'error' in json format
 *          containing 'code' and 'message' keys and their corresponding
 *          values.
 */
gchar *answer_json_error_string(guint32 error_code, gchar *message)
{
    json_t *json_answer = NULL;
    gchar *string_answer = NULL;

    json_answer = create_json_answer("error", error_code, message);
    string_answer = json_dumps(json_answer, 0);

    return string_answer;
}



/**
 * Makes a json answer success message string.
 * @param succes_code is an integer that represents the success number (See
 *        HTTP CODES in wikipedia).
 * @param is the message associated with the success.
 * @returns a gchar * string containing the 'success' in json format
 *          containing 'code' and 'message' keys and their corresponding
 *          values.
 */
gchar *answer_json_success_string(guint32 error_code, gchar *message)
{
    json_t *json_answer = NULL;
    gchar *string_answer = NULL;

    json_answer = create_json_answer("success", error_code, message);
    string_answer = json_dumps(json_answer, 0);

    return string_answer;
}


/**
 * Makes a json structure with some parameters that comes from stats_t *
 * structure.
 * @param title is a string representing the title of the corresponding
 *        nb_request number.
 * @param nb_request is a guint64 representing the total number of requests
 *        of this type.
 * @returns a json_t structure filled with stats values.
 */
json_t *make_json_from_stats(gchar *title, guint64 nb_request)
{
    json_t *stats = NULL;
    json_t *nbr = NULL;


    if (title != NULL)
        {
            stats = json_object();

            nbr = json_integer(nb_request);
            insert_json_value_into_json_root(stats, title, nbr);
        }

    return stats;
}

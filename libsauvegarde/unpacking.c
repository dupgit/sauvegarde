/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    unpacking.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2015 Olivier Delhomme
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
 * @file unpacking.c
 * This file contains the functions to unpack messages for all the
 * programs of "Sauvegarde" project.
 */

#include "libsauvegarde.h"

static guint8 get_guint8_from_json_root(json_t *root, gchar *keyname);
static guint32 get_guint32_from_json_root(json_t *root, gchar *keyname);
static guint64 get_guint64_from_json_root(json_t *root, gchar *keyname);

/**
 * gets a json_t *value into the json_t *root array.
 * @param[in,out] root is the root that contains all meta data values
 * @param keyname is the keyname associated with the value that we want to
 *        get back.
 * @returns the json_t "encoded" value from key keyname from the root
 */
json_t *get_json_value_from_json_root(json_t *root, gchar *keyname)
{
    json_t *value = NULL;

    if (root != NULL && keyname != NULL)
        {
            value = json_object_get(root, keyname);

            if (value == NULL)
                {
                    print_error(__FILE__, __LINE__, _("Error while converting to JSON from keyname %s\n"), keyname);
                    /* exit(EXIT_FAILURE); *//* An error here means that we will do nothing good */
                }
        }

    return value;
}


/**
 * returns the string with key keyname from the json tree root. It is used
 * by serveur to get the hostname from the json received message.
 * @note Freeing json_t *str here is a bad idea as it will free it into
 *       json_t *root variable that is freed afterwards.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we seek the string value.
 * @returns a newlly allocated gchar * string that is the value associated
 *          with key keyname. It can be freed with free_variable() when no longer
 *          needed.
 */
gchar *get_string_from_json_root(json_t *root, gchar *keyname)
{
    json_t *str = NULL;
    gchar *a_string = NULL;

    if (root != NULL && keyname != NULL)
        {
            str = get_json_value_from_json_root(root, keyname);
            a_string = g_strdup(json_string_value(str));
        }

    return a_string;
}


/**
 * returns the guint8 value associated with key keyname from the json tree
 * root.
 * @note Freeing json_t *value here is a bad idea as it will free it into
 *       json_t *root variable that is freed afterwards.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we seek the guint8 value.
 * @returns a guint8 number that is the value associated with key keyname.
 */
static guint8 get_guint8_from_json_root(json_t *root, gchar *keyname)
{
    json_t *value = NULL;
    guint8 number = 0;

    if (root != NULL && keyname != NULL)
        {
            value = get_json_value_from_json_root(root, keyname);
            number = (guint8) json_integer_value(value);
        }

    return number;
}


/**
 * returns the guint32 value associated with key keyname from the json tree
 * root.
 * @note Freeing json_t *value here is a bad idea as it will free it into
 *       json_t *root variable that is freed afterwards.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we seek the guint32 value.
 * @returns a guint32 number that is the value associated with key keyname.
 */
static guint32 get_guint32_from_json_root(json_t *root, gchar *keyname)
{
    json_t *value = NULL;
    guint32 number = 0;

    if (root != NULL && keyname != NULL)
        {
            value = get_json_value_from_json_root(root, keyname);
            number = (guint32) json_integer_value(value);
        }

    return number;
}


/**
 * returns the guint64 value associated with key keyname from the json tree
 * root.
 * @note Freeing json_t *value here is a bad idea as it will free it into
 *       json_t *root variable that is freed afterwards.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we seek the guint64 value.
 * @returns a guint64 number that is the value associated with key keyname.
 */
static guint64 get_guint64_from_json_root(json_t *root, gchar *keyname)
{
    json_t *value = NULL;
    guint64 number = 0;

    if (root != NULL && keyname != NULL)
        {
            value = get_json_value_from_json_root(root, keyname);
            number = (guint64) json_integer_value(value);
        }

    return number;
}


/**
 * This function loads a JSON string into a json_t struture
 * @param json_str is the json string
 * @returns a pointer to a filled json_t * structure or NULL upon error
 */
json_t *load_json(gchar *json_str)
{
    json_t *root = NULL;   /** json_t *root is the json tree where we will extract things */
    json_error_t error;    /** json_error_t *error handle json errors if any.             */

    if (json_str != NULL)
        {
            root = json_loads(json_str, 0, &error);

            if (root == NULL)
                {
                     print_error(__FILE__, __LINE__, _("Error while trying to load JSON : %s\nline: %d, column: %d, position: %d, string: %s\n"), error.text, error.line, error.column, error.position, json_str);
                }
        }

    return root;
}


/**
 * This function returns the MESSAGE_ID from msg_id JSON field
 * @param json_str : a gchar * containing the JSON formated string.
 * @returns a gint that correspond to the msg_id field found in json_str.
 *          If the field is not found it returns ENC_NOT_FOUND. This field
 *          is based on ENC_* constants that are also used for the
 *          communication between threads in client
 */
gint get_json_message_id(gchar *json_str)
{
    json_t *root = NULL;           /** json_t *root is the json tree from which we will extract msg_id                   */
    gint msg_id = ENC_NOT_FOUND;   /** gint msg_id is the message id from the JSON string by default it is ENC_NOT_FOUND */

    if (json_str != NULL)
        {
            root = load_json(json_str);

            if (root != NULL)
                {
                    msg_id = get_guint8_from_json_root(root, "msg_id");

                    json_decref(root);
                }
        }

    return msg_id;
}


/**
 * Gets the version of a version json string as returned by serveur's
 * server.
 * @param json_str : a gchar * containing the JSON formated string.
 * @returns version string or NULL
 */
gchar *get_json_version(gchar *json_str)
{
    json_t *root = NULL;     /** json_t *root is the json tree from which we will extract version's string */
    gchar *version = NULL;   /** version'string has extracted or NULL                                      */

    if (json_str != NULL)
        {
            root = load_json(json_str);

            if (root != NULL)
                {
                    version = get_string_from_json_root(root, "version");

                    json_decref(root);
                }
        }

    return version;
}


/**
 * This function returns a list of hash_data_t * from an json array
 * @param root is the root json string that may contain an array named "name"
 * @param name is the name of the array to look for into
 * @param only_hash is a boolean saying that we only have a hash list in
 *        root if set to TRUE and that we have a complete hash_data_t list
 *        if set to FALSE.
 * @returns a GSList that me be composed of 0 element (ie NULL). Elements
 *          are of type hash_data_t *.
 */
GSList *extract_gslist_from_array(json_t *root, gchar *name, gboolean only_hash)
{
    json_t *array =  NULL;   /** json_t *array is the retrieved array used to iter over to fill the list     */
    size_t index = 0;        /** size_t index is the iterator to iter over the array                         */
    json_t *value = NULL;    /** json_t *value : value = array[index] when iterating with json_array_foreach */
    GSList *head = NULL;     /** GSList *head the list to build and iclude into meta_data_t *meta            */
    guchar *a_hash = NULL;   /** guchar *a_hash is one base64 decoded hash (binary format)                   */
    gsize hash_len = 0;      /** gsize hash_len is the length of the decoded hash (must alwas be HASH_LEN)   */
    hash_data_t *hash_data = NULL;

    if (root != NULL && name != NULL)
        {

            /* creating a list with the json array found in root json string */
            array = get_json_value_from_json_root(root, name);

            /**
             * @note : This is a loop from jansson library for the array.
             *         One needs at least jansson 2.5 to compile this.
             */
            json_array_foreach(array, index, value)
                {
                    if (only_hash == TRUE)
                        {
                            a_hash = g_base64_decode(json_string_value(value), &hash_len);
                            hash_data = new_hash_data_t(NULL, 0, a_hash);
                        }

                    head = g_slist_prepend(head, hash_data);
                }

            head = g_slist_reverse(head);
        }

    return head;
}


/**
 * Fills a serveur_meta_data_t from data that are in json_t *root
 * @param root is the JSON string that should contain all data needed
 *        to fill the serveur_meta_data_t * structure.
 * @returns a newly allocated serveur_meta_data_t * structure filled
 *          accordingly.
 */
static serveur_meta_data_t *fills_serveur_meta_data_t_from_json_t(json_t *root)
{
    meta_data_t *meta = NULL;          /** meta_data_t *meta will be returned in smeta and contain file's metadata     */
    serveur_meta_data_t *smeta = NULL; /** serveur_meta_data_t *smeta will be returned at the end                      */

    if (root != NULL)
        {
            smeta = new_smeta_data_t();
            meta = new_meta_data_t();

            meta->file_type = get_guint8_from_json_root(root, "filetype");
            meta->mode = get_guint32_from_json_root(root, "mode");

            meta->atime = get_guint64_from_json_root(root, "atime");
            meta->ctime = get_guint64_from_json_root(root, "ctime");
            meta->mtime = get_guint64_from_json_root(root, "mtime");
            meta->size  = get_guint64_from_json_root(root, "fsize");
            meta->inode = get_guint64_from_json_root(root, "inode");

            meta->owner = get_string_from_json_root(root, "owner");
            meta->group = get_string_from_json_root(root, "group");

            meta->uid = get_guint32_from_json_root(root, "uid");
            meta->gid = get_guint32_from_json_root(root, "gid");

            meta->name = get_string_from_json_root(root, "name");
            meta->link = get_string_from_json_root(root, "link");

            meta->hash_data_list = extract_gslist_from_array(root, "hash_list", TRUE);

            smeta->meta = meta;
            smeta->hostname =  get_string_from_json_root(root, "hostname");
        }

    return smeta;
}


/**
 * This function returns a list from an json array.
 * @param root is the root json string that must contain an array named
 *        "file_list"
 * @returns a GSList that may be composed of 0 element (ie NULL). Elements
 *          are of type serveur_meta_data_t *.
 */
GSList *extract_smeta_gslist_from_file_list(json_t *root)
{
    json_t *array =  NULL;   /** json_t *array is the retrieved array used to iter over to fill the list     */
    size_t index = 0;        /** size_t index is the iterator to iter over the array                         */
    json_t *value = NULL;    /** json_t *value : value = array[index] when iterating with json_array_foreach */
    GSList *head = NULL;     /** GSList *head the list to build and iclude into meta_data_t *meta            */
    serveur_meta_data_t *smeta = NULL; /** serveur_meta_data_t *smeta will be returned at the end                      */

    if (root != NULL)
        {

            /* creating a list with the json array found in root json string */
            array = get_json_value_from_json_root(root, "file_list");

            /**
             * @note : This is a loop from jansson library for the array.
             *         One needs at least jansson 2.5 to compile this.
             */
            json_array_foreach(array, index, value)
                {
                    smeta = fills_serveur_meta_data_t_from_json_t(value);
                    head = g_slist_prepend(head, smeta);
                }
        }

    return head;
}


/**
 * Function that converts json_str containing the keys "hash", "data"
 * and "read" into hash_data_t structure.
 * @param json_str is a json string containing the keys "hash", "data"
 *        and read
 * @returns a newly allocated hash_data_t structure with the
 *          corresponding data in it.
 */
hash_data_t *convert_json_to_hash_data(gchar *json_str)
 {
    json_t *root = NULL;
    guchar *data = NULL;
    guint8 *hash = NULL;
    gchar *string = NULL;
    gsize data_len = 0;
    gsize hash_len = 0;
    gssize read = 0;
    hash_data_t *hash_data = NULL;

    if (json_str != NULL)
        {
            root = load_json(json_str);

            if (root != NULL)
                {
                    string = get_string_from_json_root(root, "data");
                    data = (guchar *) g_base64_decode(string, &data_len);
                    free_variable(string);

                    string = get_string_from_json_root(root, "hash");
                    hash = (guint8 *) g_base64_decode(string, &hash_len);
                    free_variable(string);

                    read = get_guint64_from_json_root(root, "size");

                    /* Some basic verifications */
                    if (data_len == read && hash_len == HASH_LEN)
                        {
                            hash_data = new_hash_data_t(data, read, hash);
                        }
                    else
                        {
                            print_error(__FILE__, __LINE__, _("Something is wrong with lengths: data_len = %ld, read = %ld, hash_len = %ld, HASH_LEN = %ld\n"), data_len, read, hash_len, HASH_LEN);
                        }

                    json_decref(root);
                }
        }

    return hash_data;
 }



/**
 * This function should return a newly allocated serveur_meta_data_t *
 * structure with all informations included from the json string.
 * @param json_str is a gchar * containing the JSON formated string.
 * @returns a newly_allocated serveur_meta_data_t * structure that can be
 *          freed when no longer needed with free_smeta_data_t() function.
 *          This function can return NULL if json_str is NULL itself.
 */
serveur_meta_data_t *convert_json_to_smeta_data(gchar *json_str)
{
    json_t *root = NULL;                 /** json_t *root is the json tree from which we will extract everything         */
    serveur_meta_data_t *smeta = NULL;   /** serveur_meta_data_t *smeta will be returned at the end                      */

    /**
     * @todo : validate that we have a json string and make
     *         sure that this is a meta_data one.
     */

    if (json_str != NULL)
        {

            root = load_json(json_str);

            if (root != NULL)
                {
                    smeta = fills_serveur_meta_data_t_from_json_t(root);

                    json_decref(root);
                }
        }

    return smeta;
}

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    packing.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 Olivier Delhomme
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
 * This file contains the functions to pack and unpack messages for all the
 * programs of "Sauvegarde" project.
 */

#include "libsauvegarde.h"

static void insert_json_value_into_json_root(json_t *root, gchar *keyname, json_t *value);
static void append_hash_to_array(json_t *array, gchar *encoded_hash);


/**
 * Inserts a json_t *value into the json_t *root array.
 * @param[in,out] root is the root that will contain all meta data value
 * @param keyname is the keyname associated with the value (in fact it is
 *        variable's name)
 * @param value is the json_t "encoded" value to insert into the root
 */
static void insert_json_value_into_json_root(json_t *root, gchar *keyname, json_t *value)
{
    int result = 0;

    result = json_object_set_new(root, keyname, value);

    if (result != JANSSON_SUCCESS)
        {
            fprintf(stderr, _("Error while converting to JSON\n"));
            exit(EXIT_FAILURE); /* An error here means that we will do nothing good */
        }
}


/**
 *
 * @param[in,out] array is an array that will contain all base64 encoded
 *                hashs
 * @param encoded_hash is the base64 encoded hash
 */
static void append_hash_to_array(json_t *array, gchar *encoded_hash)
{
    json_t *string = NULL;

    string = json_string_nocheck((const char *) encoded_hash);
    json_array_append_new(array, string);
}


/**
 * This function should return a JSON string with all informations from
 * the meta_data_t structure.
 * @param meta is the structure that contains all meta data for a file or
 *        a directory.
 * @returns a JSON formated string
 */
gchar *convert_meta_data_to_json(meta_data_t *meta)
{
    json_t *root = NULL;        /**< the root that will contain all meta data json */
    json_t *string = NULL;      /**< temporary variable for strings                */
    json_t *value = NULL;       /**< temporary variable for integers and numbers   */
    json_t *array = NULL;       /**< array that will receive base64 encoded hashs  */
    gchar *encoded_hash = NULL; /**< hash base64 encoded                           */
    GSList *head = NULL;        /**< list to iter over                             */
    gchar *json_str = NULL;     /**< the string to be returned                     */

    if (meta != NULL)
        {
            root = json_object();

            value = json_integer(meta->file_type);
            insert_json_value_into_json_root(root, "filetype", value);

            value = json_integer(meta->mode);
            insert_json_value_into_json_root(root, "mode", value);

            value = json_integer(meta->atime);
            insert_json_value_into_json_root(root, "atime", value);

            value = json_integer(meta->ctime);
            insert_json_value_into_json_root(root, "ctime", value);

            value = json_integer(meta->mtime);
            insert_json_value_into_json_root(root, "mtime", value);

            string = json_string_nocheck((const char *) meta->owner);
            insert_json_value_into_json_root(root, "owner", string);

            string = json_string_nocheck((const char *) meta->group);
            insert_json_value_into_json_root(root, "group", string);

            value = json_integer(meta->uid);
            insert_json_value_into_json_root(root, "uid", value);

            value = json_integer(meta->gid);
            insert_json_value_into_json_root(root, "gid", value);

            string = json_string_nocheck((const char *) meta->name);
            insert_json_value_into_json_root(root, "name", string);

            /* creating an array with the hash list */
            head = meta->hash_list;
            array = json_array();

            while (head != NULL)
                {
                    encoded_hash = g_base64_encode(head->data, HASH_LEN);

                    append_hash_to_array(array, encoded_hash);

                    g_free(encoded_hash);

                    head = g_slist_next(head);

                }

            insert_json_value_into_json_root(root, "hash_list", array);

            json_str = json_dumps(root, 0);
        }

    return json_str;
}






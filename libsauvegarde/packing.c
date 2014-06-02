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
 * This file contains the functions to pack messages for all the
 * programs of "Sauvegarde" project.
 */

#include "libsauvegarde.h"

static void insert_json_value_into_json_root(json_t *root, gchar *keyname, json_t *value);
static void append_hash_to_array(json_t *array, gchar *encoded_hash);
static void insert_string_into_json_root(json_t *root, gchar *keyname, gchar *a_string);
static void insert_guint8_into_json_root(json_t *root, gchar *keyname, guint8 number);
static void insert_guint32_into_json_root(json_t *root, gchar *keyname, guint32 number);
static void insert_guint64_into_json_root(json_t *root, gchar *keyname, guint64 number);


/**
 * Inserts a json_t *value into the json_t *root array.
 * @param[in,out] root is the root that will contain all meta data values
 * @param keyname is the keyname associated with the value (in fact it is
 *        variable's name)
 * @param value is the json_t "encoded" value to insert into the root
 */
static void insert_json_value_into_json_root(json_t *root, gchar *keyname, json_t *value)
{
    int result = 0;

    if (root != NULL && keyname != NULL && value != NULL)
        {
            result = json_object_set_new(root, keyname, value);

            if (result != JANSSON_SUCCESS)
                {
                    fprintf(stderr, _("[%s, %d] Error while converting to JSON\n"), __FILE__, __LINE__);
                    exit(EXIT_FAILURE); /* An error here means that we will do nothing good */
                }
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

    if (array != NULL && encoded_hash != NULL)
        {
            string = json_string_nocheck((const char *) encoded_hash);
            json_array_append_new(array, string);
        }
}


/**
 * Inserts the string a_string into the json tree root with key keyname.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we will insert a new value
 * @param a_string is the value to be inserted with key keyname.
 */
static void insert_string_into_json_root(json_t *root, gchar *keyname, gchar *a_string)
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
 * This function should return a JSON string with all informations from
 * the meta_data_t structure.
 * @param meta is the structure that contains all meta data for a file or
 *        a directory.
 * @param hostname is the name of the host onw hich we are running and that
 *        we want to include into the json string.
 * @returns a JSON formated string or NULL
 */
gchar *convert_meta_data_to_json(meta_data_t *meta, const gchar *hostname)
{
    json_t *root = NULL;        /**< the root that will contain all meta data json */
    json_t *array = NULL;       /**< array that will receive base64 encoded hashs  */
    gchar *encoded_hash = NULL; /**< hash base64 encoded                           */
    GSList *head = NULL;        /**< list to iter over                             */
    gchar *json_str = NULL;     /**< the string to be returned                     */

    if (meta != NULL)
        {
            root = json_object();

            insert_guint8_into_json_root(root, "filetype", meta->file_type);
            insert_guint32_into_json_root(root, "mode", meta->mode);

            insert_guint64_into_json_root(root, "atime", meta->atime);
            insert_guint64_into_json_root(root, "ctime", meta->ctime);
            insert_guint64_into_json_root(root, "mtime", meta->mtime);

            insert_string_into_json_root(root, "owner", meta->owner);
            insert_string_into_json_root(root, "group", meta->group);

            insert_guint32_into_json_root(root, "uid", meta->uid);
            insert_guint32_into_json_root(root, "gid", meta->uid);

            insert_string_into_json_root(root, "name", meta->name);
            insert_string_into_json_root(root, "hostname", (gchar *) hostname);

            /* creating an array with the whole hash list */
            array = json_array();
            head = meta->hash_list;

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


/**
 * Function that encapsulate a meta_data_t * variable into a capsule_t *
 * one. It does not check that meta is not NULL so it may encapsulate a
 * NULL pointer !
 * @param meta is the meta_data_t * variable to be encapsulated
 * @returns a capsule_t * with command field set to ENC_META_DATA stating
 *          that the data field is of type meta_data_t *.
 */
capsule_t *encapsulate_meta_data_t(meta_data_t *meta)
{

    capsule_t *capsule = NULL;

    capsule = (capsule_t *) g_malloc0(sizeof(capsule_t));

    capsule->command = ENC_META_DATA;
    capsule->data = (void *) meta;

    return capsule;
}


/**
 * Function that encapsulate an END command.
 * @returns a capsule_t * with command field set to ENC_END stating
 *          that this is the end my friend (some famous song) !
 */
capsule_t *encapsulate_end(void)
{

    capsule_t *capsule = NULL;

    capsule = (capsule_t *) g_malloc0(sizeof(capsule_t));

    capsule->command = ENC_END;
    capsule->data = NULL;

    return capsule;
}



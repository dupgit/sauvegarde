/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    unpacking.c
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
 * @file unpacking.c
 * This file contains the functions to unpack messages for all the
 * programs of "Sauvegarde" project.
 */

#include "libsauvegarde.h"

static json_t *get_json_value_from_json_root(json_t *root, gchar *keyname);
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
static json_t *get_json_value_from_json_root(json_t *root, gchar *keyname)
{
    json_t *value = NULL;

    if (root != NULL && keyname != NULL)
        {
            value = json_object_get(root, keyname);

            if (value == NULL)
                {
                    fprintf(stderr, _("[%s, %d] Error while converting to JSON from keyname %s\n"), __FILE__, __LINE__, keyname);
                    exit(EXIT_FAILURE); /* An error here means that we will do nothing good */
                }
        }

    return value;
}


/**
 * returns the string with key keyname from the json tree root. It is used
 * by serveur to get the hostname from the json received message.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we seek the string value.
 * @returns a newlly allocated gchar * string that is the value associated
 *          with key keyname. It can be freed with g_free() when no longer
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
 * This function should return a newly allocated serveur_meta_data_t *
 * structure with all informations included from the json string.
 * @param json_str is a gchar * contianing the JSON formated string.
 * @returns a newly_allocated serveur_meta_data_t * structure that can be
 *         freed when no longer needed with free_smeta_data_t() function.
 *         This function can return NULL if json_str is NULL itself.
 */
serveur_meta_data_t *convert_json_to_smeta_data(gchar *json_str)
{
    json_t *root = NULL;                 /**< the json tree from which we will extract everything */
    json_error_t *error = NULL;          /**< json error handling                                 */
    meta_data_t *meta = NULL;            /**< meta_data that will be returned  in smeta           */
    serveur_meta_data_t *smeta = NULL;
    json_t *array =  NULL;               /**< retrieved array */
    size_t index = 0;                    /**< index to iter over the array */
    json_t *value = NULL;                /**< value = array[index] when iterating with json_array_foreach */
    GSList *head = NULL;                 /**< the list to build */
    guchar *a_hash = NULL;               /**< one base64 decoded hash */
    gsize hash_len = 0;                  /**< length of the decoded hash */


    if (json_str != NULL)
        {

            root = json_loads(json_str, 0, error);
            g_free(json_str);

            if (root != NULL)
                {
                    smeta = new_smeta_data_t();
                    meta = new_meta_data_t();

                    meta->file_type = get_guint8_from_json_root(root, "filetype");
                    meta->mode = get_guint32_from_json_root(root, "mode");

                    meta->atime = get_guint64_from_json_root(root, "atime");
                    meta->ctime = get_guint64_from_json_root(root, "ctime");
                    meta->mtime = get_guint64_from_json_root(root, "mtime");

                    meta->owner = get_string_from_json_root(root, "owner");
                    meta->group = get_string_from_json_root(root, "group");

                    meta->uid = get_guint32_from_json_root(root, "uid");
                    meta->gid = get_guint32_from_json_root(root, "gid");

                    meta->name = get_string_from_json_root(root, "name");

                    /* creating a list with JSON array */
                    array = get_json_value_from_json_root(root, "hash_list");

                    /* This is a loop from jansson library for the array */
                    json_array_foreach(array, index, value)
                        {
                            a_hash = g_base64_decode(json_string_value(value), &hash_len);
                            head = g_slist_append(head, a_hash);
                        }

                    meta->hash_list = head;

                    smeta->meta = meta;
                    smeta->hostname =  get_string_from_json_root(root, "hostname");
                }
            else
                {
                    fprintf(stderr, _("[%s, %d] Error while trying to load JSON : %s\n%s\nline: %d, column: %d, position: %d"), __FILE__, __LINE__, error->text, error->source, error->line, error->column, error->position);
                    exit(EXIT_FAILURE); /* An error here means that we will do nothing good */
                }
        }


    return smeta;
}

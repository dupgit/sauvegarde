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
static gchar *get_string_from_json_root(json_t *root, gchar *keyname);


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
                    fprintf(stderr, _("Error while converting to JSON from keyname %s\n"), keyname);
                    exit(EXIT_FAILURE); /* An error here means that we will do nothing good */
                }
        }

    return value;
}


/**
 * returns the string with key keyname from the json tree root.
 * @param[in,out] root is the main json tree
 * @param keyname is the key for which we seek the string value.
 * @returns a newlly allocated gchar * string that is the value associated
 *          with key keyname. It can be freed with g_free() when no longer
 *          needed.
 */
static gchar *get_string_from_json_root(json_t *root, gchar *keyname)
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
 * This function should return a newly allocated meta_data_t * structure
 * with all informations included from the json string.
 * @param json_str is a gchar * contianing the JSON formated string.
 * @returns a newly_allocated meta_data_t * structure that can be freed
 *          with free_meta_data_t() function when no longer needed. This
 *          function can return NULL if json_str is NULL itself.
 */
meta_data_t *convert_json_to_meta_data(gchar *json_str)
{
    json_t *root = NULL;        /**< the json tree from which we will extract everything */
    json_error_t *error = NULL; /**< json error handling                                 */
    meta_data_t *meta = NULL;   /**< meta_data that will be returned                     */

    if (json_str != NULL)
        {

            root = json_loads(json_str, 0, error);
            g_free(json_str);

            if (root != NULL)
                {
                    meta = new_meta_data_t();

                    meta->name = get_string_from_json_root(root, "name");

                }
            else
                {
                    fprintf(stderr, _("Error while trying to load JSON : %s\n%s\nline: %d, column: %d, position: %d"), error->text, error->source, error->line, error->column, error->position);
                    exit(EXIT_FAILURE); /* An error here means that we will do nothing good */
                }
        }


    return meta;
}

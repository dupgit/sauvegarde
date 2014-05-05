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


/**
 * This function should return a JSON string with all informations from
 * the meta_data_t structure.
 * @param meta is the structure that contains all meta data for a file or
 *        a directory.
 * @returns a JSON formated string
 */
gchar *convert_meta_data_to_json(meta_data_t *meta)
{
    json_t *root = NULL;
    json_t *string = NULL;
    gchar *json_string = NULL;
    int result = 0;

    if (meta != NULL)
        {

            root = json_object();
            string = json_string_nocheck((const char *)meta->name);
            result = json_object_set(root, "name", string);

            if (result != JANSSON_SUCCESS)
                {
                    fprintf(stderr, _("Error while converting to JSON\n"));
                    exit(EXIT_FAILURE); /* An error here means that we will do nothing good */
                }

        }

    return json_string;
}






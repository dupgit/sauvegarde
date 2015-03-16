/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *    file_backend.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 Olivier Delhomme
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
 * @file serveur/file_backend.c
 *
 * This file contains all the functions for the file backend that saves
 * everything to some flat files somewhere into the filesystem.
 */

#include "serveur.h"

static gchar *convert_hash_list_to_gchar(GSList *hash_list);

/**
 * Converts the hash list to a list of comma separated hashs in one gchar *
 * string. Hashs are base64 encoded
 * @param hash_list à GSList of hashs
 * @returns a list of comma separated hashs in one gchar * string.
 * @todo free memory a bit !
 */
static gchar *convert_hash_list_to_gchar(GSList *hash_list)
{
    GSList *head = hash_list;
    gchar *encoded_hash = NULL;
    gchar *list = NULL;
    gchar *old_list = NULL;


    while (head != NULL)
        {
            encoded_hash = g_strdup_printf("\"%s\"", g_base64_encode(head->data, HASH_LEN));

            if (old_list == NULL)
                {
                    list = g_strdup_printf("%s", encoded_hash);
                    old_list = list;
                }
            else
                {
                    list = g_strdup_printf("%s, %s", old_list, encoded_hash);
                    free_variable(old_list);
                    old_list = list;
                }

            head = g_slist_next(head);
        }

    list = old_list;

    return list;
}


/**
 * Stores meta data into a flat file .
 */
void file_store_smeta(serveur_struct_t *serveur_struct, serveur_meta_data_t *smeta)
{
    GFile *meta_file = NULL;
    gchar *filename = NULL;
    GFileOutputStream *stream = NULL;
    GError *error = NULL;
    gsize count = 0;
    gssize written = 0;
    gchar *buffer = NULL;
    gchar *hash_list = NULL;
    meta_data_t *meta = smeta->meta;

    if (smeta != NULL && smeta->hostname != NULL && meta != NULL)
        {
            print_debug("file_store_smeta: Going to store meta-datas for file %s\n", meta->name);

            filename = g_build_filename("/var/tmp/sauvegarde/serveur/metas", smeta->hostname, NULL);

            meta_file = g_file_new_for_path(filename);

            stream = g_file_append_to(meta_file, G_FILE_CREATE_NONE, NULL, &error);

            if (stream != NULL)
                {
                    hash_list = convert_hash_list_to_gchar(meta->hash_list);

                    if (hash_list != NULL)
                        {
                            buffer = g_strdup_printf("%d, %d, %ld, %ld, %ld, %ld, \"%s\", \"%s\", %d, %d, \"%s\", %s\n", meta->file_type, meta->mode, meta->atime, meta->ctime, meta->mtime, meta->size, meta->owner, meta->group, meta->uid, meta->gid, meta->name, hash_list);
                            free_variable(hash_list);
                        }
                    else
                        {
                            buffer = g_strdup_printf("%d, %d, %ld, %ld, %ld, %ld, \"%s\", \"%s\", %d, %d, \"%s\"\n", meta->file_type, meta->mode, meta->atime, meta->ctime, meta->mtime, meta->size, meta->owner, meta->group, meta->uid, meta->gid, meta->name);
                        }

                    count = strlen(buffer);
                    written = g_output_stream_write((GOutputStream *) stream, buffer, count, NULL, &error);

                    if (written != count)
                        {
                            print_error(__FILE__, __LINE__, _("Error: unable to write to file %s.\n"));
                        }

                    g_output_stream_close((GOutputStream *) stream, NULL, &error);
                    g_object_unref(meta_file)   ;
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error: unable to open file %s to append meta-datas in it.\n"), filename);
                }
        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error: no serveur_meta_data_t structure or missing hostname or missing meta_data_t * structure.\n"));
        }
}


/**
 * Inits the backend : takes care of the directories we want to write to
 */
void file_init_backend(serveur_struct_t *serveur_struct)
{
    create_directory("/var/tmp/sauvegarde/serveur/metas");
    create_directory("/var/tmp/sauvegarde/serveur/datas");
}


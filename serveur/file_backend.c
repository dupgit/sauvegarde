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
 * @todo move hash specific functions to libsauvegarde's hash.c library as
 *       it may be usefull for other programs.
 */

#include "serveur.h"


/**
 * Stores meta data into a flat file. A file is created for each host that
 * sends meta datas. This code is not thread safe (it means that this is
 * not safe to call it from different threads unless some mechanism
 * garantees that a write will never occur in the same file at the same
 * time.
 * @param serveur_struct is the serveur main structure where all
 *        informations needed by the program are stored.
 * @param smeta the serveur structure for file meta data. It contains the
 *        hostname that sent it. This structure MUST be freed by this
 *        function.
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

                    if (error != NULL)
                        {
                            print_error(__FILE__, __LINE__, _("Error: unable to write to file %s (%ld bytes written).\n"), filename, written);
                        }

                    g_output_stream_close((GOutputStream *) stream, NULL, &error);
                    free_variable(buffer);

                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error: unable to open file %s to append meta-datas in it.\n"), filename);
                }

            g_object_unref(meta_file);
            free_variable(filename);
        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error: no serveur_meta_data_t structure or missing hostname or missing meta_data_t * structure.\n"));
        }
}


/**
 * Stores data into a flat file. The file is named by its hash in hex
 * representation (one should easily check that the sha256sum of such a
 * file gives its name !).
 * @param serveur_struct is the serveur main structure where all
 *        informations needed by the program are stored.
 * @param hash_data is a hash_data_t * structure that contains the hash and
 *        the corresponding data in a binary form and a 'read' field that
 *        contains the number of bytes in 'data' field.
 */
void file_store_data(serveur_struct_t *serveur_struct, hash_data_t *hash_data)
{
    GFile *data_file = NULL;
    gchar *filename = NULL;
    GFileOutputStream *stream = NULL;
    GError *error = NULL;
    gssize written = 0;
    gchar *hex_hash = NULL;
    gchar *path = NULL;

    if (hash_data != NULL && hash_data->hash != NULL && hash_data->data != NULL)
        {
            /** @todo create directories at init time beacause
             * creating directories like that is time consuming
             * for nothing
             */
            path = make_path_from_hash("/var/tmp/sauvegarde/serveur/datas", hash_data->hash, 3);
            create_directory(path);
            hex_hash = hash_to_string(hash_data->hash);
            filename = g_build_filename(path, hex_hash, NULL);

            data_file = g_file_new_for_path(filename);
            stream = g_file_replace(data_file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &error);

            if (stream != NULL)
                {
                    written = g_output_stream_write((GOutputStream *) stream, hash_data->data, hash_data->read, NULL, &error);

                    if (error != NULL)
                        {
                            print_error(__FILE__, __LINE__, _("Error: unable to write to file %s (%ld bytes written).\n"), filename, written);
                        }

                    g_output_stream_close((GOutputStream *) stream, NULL, &error);

                    free_variable(hash_data->data);
                    free_variable(hash_data->hash);
                    free_variable(hash_data);
                }
            else
                {
                     print_error(__FILE__, __LINE__, _("Error: unable to open file %s to write datas in it.\n"), filename);
                }

            g_object_unref(data_file);
            free_variable(filename);
        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error: no hash_data_t structure or hash in it or missing data in it.\n"));
        }
}


/**
 * Inits the backend : takes care of the directories we want to write to.
 * @param serveur_struct is the serveur main structure where all
 *        informations needed by the program are stored.
 */
void file_init_backend(serveur_struct_t *serveur_struct)
{
    create_directory("/var/tmp/sauvegarde/serveur/metas");
    create_directory("/var/tmp/sauvegarde/serveur/datas");
}


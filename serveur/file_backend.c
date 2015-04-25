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
 * @todo make a more complex structure to store directory prefix and level
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
 * @todo prefix should be set as a configuration's option.
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
    gchar *prefix = NULL;
    file_backend_t *file_backend = NULL;


    if (serveur_struct != NULL && serveur_struct->backend != NULL && serveur_struct->backend->user_data != NULL)
        {
            file_backend = serveur_struct->backend->user_data;
            prefix = g_build_filename((gchar *) file_backend->prefix, "metas", NULL);

            if (smeta != NULL && smeta->hostname != NULL && meta != NULL)
                {
                    /* print_debug("file_store_smeta: Going to store meta-datas for file %s\n", meta->name); */

                    filename = g_build_filename(prefix, smeta->hostname, NULL);

                    meta_file = g_file_new_for_path(filename);

                    stream = g_file_append_to(meta_file, G_FILE_CREATE_NONE, NULL, &error);

                    if (stream != NULL)
                        {
                            hash_list = convert_hash_list_to_gchar(meta->hash_list);

                            if (hash_list != NULL)
                                {
                                    buffer = g_strdup_printf("%d, %ld, %d, %ld, %ld, %ld, %ld, \"%s\", \"%s\", %d, %d, \"%s\", %s\n", meta->file_type, meta->inode, meta->mode, meta->atime, meta->ctime, meta->mtime, meta->size, meta->owner, meta->group, meta->uid, meta->gid, meta->name, hash_list);
                                    free_variable(hash_list);
                                }
                            else
                                {
                                    buffer = g_strdup_printf("%d, %ld, %d, %ld, %ld, %ld, %ld, \"%s\", \"%s\", %d, %d, \"%s\"\n", meta->file_type, meta->inode, meta->mode, meta->atime, meta->ctime, meta->mtime, meta->size, meta->owner, meta->group, meta->uid, meta->gid, meta->name);
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

            free_variable(prefix);
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
 * @todo prefix should be set as a configuration's option.
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
    gchar *prefix = NULL;
    file_backend_t *file_backend = NULL;

    if (serveur_struct != NULL && serveur_struct->backend != NULL && serveur_struct->backend->user_data != NULL)
        {
            file_backend = serveur_struct->backend->user_data;
            prefix = g_build_filename((gchar *) file_backend->prefix, "datas", NULL);

            if (hash_data != NULL && hash_data->hash != NULL && hash_data->data != NULL)
                {
                    path = make_path_from_hash(prefix, hash_data->hash, file_backend->level);
                    /* create_directory(path); */
                    hex_hash = hash_to_string(hash_data->hash);
                    filename = g_build_filename(path, hex_hash, NULL);

                    /* print_debug("file_store_data: Going to store datas for hash %s\n", hex_hash); */

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
                    free_variable(hex_hash);
                    free_variable(path);
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error: no hash_data_t structure or hash in it or missing data in it.\n"));
                }

            free_variable(prefix);
        }
}


/**
 * Builds a list of hashs that serveur's server need.
 * @param serveur_struct is the serveur main structure where all
 *        informations needed by the program are stored.
 * @param hash_list is the list of hashs that we have to check for.
 * @returns to the client a list of hashs in no specific order for which
 *          the server needs the datas.
 */
GSList *build_needed_hash_list(serveur_struct_t *serveur_struct, GSList *hash_list)
{
    GFile *data_file = NULL;
    GSList *head = hash_list;
    GSList *needed = NULL;
    gchar *hex_hash = NULL;
    gchar *filename = NULL;
    gchar *path = NULL;
    gchar *prefix = NULL;
    file_backend_t *file_backend = NULL;


    if (serveur_struct != NULL && serveur_struct->backend != NULL && serveur_struct->backend->user_data != NULL)
        {
            file_backend = serveur_struct->backend->user_data;

            prefix = g_build_filename((gchar *) file_backend->prefix, "datas", NULL);


            while (head != NULL)
                {
                    path = make_path_from_hash(prefix, head->data, file_backend->level);
                    hex_hash = hash_to_string(head->data);
                    filename = g_build_filename(path, hex_hash, NULL);

                    data_file = g_file_new_for_path(filename);

                    if (g_file_query_exists(data_file, NULL) == FALSE)
                        {
                            /* file does not exists and is needed thus putting it it the needed list */
                            needed = g_slist_prepend(needed, head->data);
                        }

                    free_variable(filename);
                    free_variable(hex_hash);
                    free_variable(path);

                    g_object_unref(data_file);

                    head = g_slist_next(head);
                }

            free_variable(prefix);

        }

    return needed;
}


/**
 * Creates sub_dir subdirectory into save_dir path
 * @param save_dir prefix directory where to create sub_dir
 * @param sub_dir name of the sub directory to be created under save_dir
 */
static void file_create_directory(gchar *save_dir, gchar *sub_dir)
{
    gchar *a_directory = NULL;

    a_directory = g_build_filename(save_dir, sub_dir, NULL);
    create_directory(a_directory);
    free_variable(a_directory);
}


/**
 * Makes all subdirectories into the "datas" directory.
 * @note creating subdirectories for a level of 3 may take a long time and
 *       an even empty structure will consume something like 64 Gb of space
 *       on an ext4 filesystem (expect 16 Tb with level 4).
 * @param file_backend the structure that contains the prefix path and the
 *        level in which we want to create the subdirectories.
 */
static void make_all_subdirectories(file_backend_t *file_backend)
{
    gchar *path = NULL;
    gchar *path2 = NULL;
    gchar *octet = NULL;
    guint number = 0;
    double i = 0;
    double p = 0;
    double total = 0;

    if (file_backend != NULL && file_backend->level < 5 && file_backend->level > 1)
        {
            total = pow(256, file_backend->level);

            for (i = 0; i < total; i++)
                {
                    path = g_strdup_printf("");

                    for (p = file_backend->level-1; p >= 0; p--)
                        {
                            number = i / (pow(256, p));


                            if (number > 255)
                                {
                                    number = fmod(number, 256);
                                }


                            octet = g_strdup_printf("%02x", number);
                            path2 = g_strconcat(path, octet, "/", NULL);
                            free_variable(path);
                            path = path2;
                            free_variable(octet);
                        }

                    path[strlen(path)-1] = '\0';
                    path2 = g_strconcat(file_backend->prefix, "/datas/", path, NULL);

                    create_directory(path2);

                    free_variable(path);
                    free_variable(path2);

                }
    }
}


/**
 * Inits the backend : takes care of the directories we want to write to.
 * user_data of the backend structure is a file_backend_t structure that
 * contains the prefix path where to store datas and the level of
 * indirections
 * @param serveur_struct is the serveur main structure where all
 *        informations needed by the program are stored.
 */
void file_init_backend(serveur_struct_t *serveur_struct)
{
    file_backend_t *file_backend = NULL;

    if (serveur_struct != NULL && serveur_struct->backend != NULL)
        {
            file_backend = (file_backend_t *) g_malloc0(sizeof(file_backend_t));

            file_backend->prefix = g_strdup("/home/dup/sauvegarde/serveur");
            file_backend->level = 3;

            serveur_struct->backend->user_data = file_backend;

            file_create_directory(file_backend->prefix, "metas");
            file_create_directory(file_backend->prefix, "datas");

            /**
             * @todo : store somewhere that this as already been done once
             */
            fprintf(stdout, _("Please wait while creating directories\n"));
            /* make_all_subdirectories(file_backend); */
            fprintf(stdout, _("Finished !\n"));
        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error: no serveur structure or no backend structure.\n"));
        }
}


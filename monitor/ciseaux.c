/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    ciseaux.c
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
 * @file ciseaux.c
 * This program should receive filenames on which it has to calculate their
 * SHA256 checksums : checksums are calculated for a defined block size
 * (CISEAUX_BLOCK_SIZE).
 */

#include "monitor.h"

static void do_checksum(main_struct_t *main_struct, GFileInputStream *stream, gchar *filename, meta_data_t *meta);
static void it_is_a_directory(main_struct_t *main_struct, gchar *dirname, GFileInfo *fileinfo, meta_data_t *meta);
static void it_is_a_file(main_struct_t *main_struct, GFile *a_file, gchar *filename, GFileInfo *fileinfo, meta_data_t *meta);
static gpointer calculate_hashs_on_a_file(gpointer data);

/**
 * Does the checksum on the opened stream.
 * @param main_struct is the mains structure fro the program
 * @param stream the stream on which we want to calculate the checksum
 * @param filename is the filename of the opened stream (to be displayed in
 *        case of error.
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 */
static void do_checksum(main_struct_t *main_struct, GFileInputStream *stream, gchar *filename, meta_data_t *meta)
{
    gssize read = 1;
    guchar *buffer = NULL;
    guint64 i = 0;
    GChecksum *checksum = NULL;
    GError *error = NULL;
    guint8 *a_hash = NULL;
    gsize digest_len = HASH_LEN;

    checksum = g_checksum_new(G_CHECKSUM_SHA256);
    buffer = (guchar *) g_malloc0 (main_struct->opt->blocksize + 1);
    a_hash = (guint8 *) g_malloc0 (digest_len);

    while (read != 0)
        {

            read = g_input_stream_read((GInputStream *) stream, buffer, main_struct->opt->blocksize, NULL, &error);

            if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, _("Error while reading %s file: %s\n"),  filename, error->message);
                    error = free_error(error);
                    read = 0;
                }
            else
                {

                    if (read != 0)
                        {
                            g_checksum_update(checksum, buffer, read);
                            g_checksum_get_digest(checksum, a_hash, &digest_len);

                            insert_into_tree(main_struct->hashs, a_hash, buffer, read, meta);

                            g_checksum_reset(checksum);
                            i = i + 1;
                            digest_len = HASH_LEN;
                        }
                }
        }

    meta->hash_list = g_slist_reverse(meta->hash_list);

    free_variable(a_hash);
    free_variable(buffer);
    g_checksum_free(checksum);
}


/**
 * This function transmits all information about the directory.
 * @param main_struct : main structure (needed to know the queue to
 *        send the message.
 * @param dirname : a gchar * directory name.
 * @param fileinfo is a pointer to a structure that contains all meta
 *        information about the file (such as what a stat call gives back).
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 */
static void it_is_a_directory(main_struct_t *main_struct, gchar *dirname, GFileInfo *fileinfo, meta_data_t *meta)
{
    gchar *owner = NULL;
    gchar *dates = NULL;
    gchar *mode = NULL;
    gchar *size = NULL;
    guint64 inode = 0;
    capsule_t *capsule = NULL;


    if (main_struct != NULL && fileinfo != NULL && meta != NULL && dirname != NULL)
        {
            owner = get_username_owner_from_gfile(fileinfo, meta);
            dates = get_dates_from_gfile(fileinfo, meta);
            mode = get_file_mode_from_gfile(fileinfo, meta);
            size = get_file_size_from_gfile(fileinfo, meta);
            inode = get_inode_from_gfile(fileinfo, meta);

            /**
             * @todo Modify this part in order to avoid sending duplicate metas datas.
             *  We assume that we are using the cache (and this may not be the case in the future)
             */
            if (is_file_in_cache(main_struct->database, meta) == FALSE)
                {
                    print_debug("%d\t%ld\t%s\t%s\t%s\t%s\t%s\n", G_FILE_TYPE_DIRECTORY, inode, owner, dates, mode, size, dirname);

                    capsule = encapsulate_meta_data_t(ENC_META_DATA, meta);
                    g_async_queue_push(main_struct->store_queue, capsule);

                    print_debug(_("%s passed to store's thread\n"), dirname);
                }

            free_variable(owner);
            free_variable(dates);
            free_variable(mode);
            free_variable(size);
        }
}


/**
 * This functions is calling everything that is needed to calculate
 * the checksums of a file.
 * @param main_struct : main structure that contains everything
 * @param a_file : the GFile opened
 * @param filename the filename of the opened GFile.
 * @param fileinfo is a pointer to a structure that contains all meta
 *        information about the file (such as what a stat call gives back).
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 */
static void it_is_a_file(main_struct_t *main_struct, GFile *a_file, gchar *filename, GFileInfo *fileinfo, meta_data_t *meta)
{
    GFileInputStream *stream = NULL;
    GError *error = NULL;
    gchar *owner = NULL;
    gchar *dates = NULL;
    gchar *mode = NULL;
    gchar *size = NULL;
    guint64 inode = 0;
    capsule_t *capsule = NULL;

    if (a_file != NULL && main_struct != NULL && fileinfo != NULL && meta != NULL && filename != NULL)
        {
            stream = g_file_read(a_file, NULL, &error);

            if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, _("Error while opening %s file: %s\n"),  filename, error->message);
                    free_error(error);
                }
            else
                {
                    owner = get_username_owner_from_gfile(fileinfo, meta);
                    dates = get_dates_from_gfile(fileinfo, meta);
                    mode = get_file_mode_from_gfile(fileinfo, meta);
                    size = get_file_size_from_gfile(fileinfo, meta);
                    inode = get_inode_from_gfile(fileinfo, meta);

                    if (is_file_in_cache(main_struct->database, meta) == FALSE)
                        {
                            /**
                             * 1. The file is not at all in the cache or
                             * 2. Something has changed in meta datas
                             *    (atime, ctime, mode, size...) at least N minutes ago.
                             */
                            print_debug("%d\t%ld\t%s\t%s\t%s\t%s\t%s\n", G_FILE_TYPE_REGULAR, inode, owner, dates, mode, size, filename);

                            do_checksum(main_struct, stream, filename, meta);
                            g_input_stream_close((GInputStream *) stream, NULL, NULL);

                            capsule = encapsulate_meta_data_t(ENC_META_DATA, meta);
                            g_async_queue_push(main_struct->store_queue, capsule);

                            print_debug(_("%s passed to store's thread\n"), filename);
                        }
                    else
                        {
                           meta = free_meta_data_t(meta);
                        }

                    free_variable(owner);
                    free_variable(dates);
                    free_variable(mode);
                    free_variable(size);
                }

            free_object(stream);
        }
}


/**
 * The main function that will calculate the hashs on a file. This function
 * is a thread itself. It waits for messages in the main queue.
 * Messages are sent by the following functions :
 * . traverse_directory.
 * @param data is main_struct_t * pointer
 * @returns NULL to fullfill the template needed to create a GThread
 */
static gpointer calculate_hashs_on_a_file(gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    gchar *filename = NULL;
    GFile *a_file = NULL;

    GFileInfo *fileinfo = NULL;
    GError *error = NULL;
    GFileType filetype = G_FILE_TYPE_UNKNOWN;
    meta_data_t *meta = NULL;


    if (main_struct != NULL)
        {

            do
                {
                    filename = g_async_queue_pop(main_struct->queue);

                    if (g_strcmp0(filename, "$END$") != 0)
                        {
                            meta = new_meta_data_t();
                            meta->name = g_strdup(filename);
                            a_file = g_file_new_for_path(filename);

                            if (a_file != NULL)
                                {
                                    fileinfo = g_file_query_info(a_file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

                                    if (error != NULL)
                                        {
                                            print_error(__FILE__, __LINE__, _("Can't get informations on %s file: %s\n"),  filename, error->message);
                                            error = free_error(error);
                                        }
                                    else if (fileinfo != NULL)
                                        {
                                            filetype = g_file_info_get_file_type(fileinfo);
                                            meta->file_type = filetype;

                                            if (g_file_info_get_is_symlink(fileinfo))
                                                {
                                                    print_debug(_("%s is a symbolic link.\n"), filename);
                                                }
                                            else if (filetype == G_FILE_TYPE_REGULAR)
                                                {
                                                    it_is_a_file(main_struct, a_file, filename, fileinfo, meta);
                                                }
                                            else if (filetype == G_FILE_TYPE_DIRECTORY)
                                                {
                                                    it_is_a_directory(main_struct, filename, fileinfo, meta);
                                                }
                                            else
                                                {
                                                    print_error(__FILE__, __LINE__, _("%s is not a regular file\n"), filename);
                                                }

                                            fileinfo = free_object(fileinfo);
                                        }

                                    a_file = free_object(a_file);

                                }

                            filename = free_variable(filename);

                            /* wait_for_queue_to_flush(main_struct->store_queue, 16, 100); */

                        }

                }
            while (g_strcmp0(filename, "$END$") != 0);

            /*filename = free_variable(filename); */

            /* Ending the queue with END command */
            g_async_queue_push(main_struct->store_queue, encapsulate_end());

            free_variable(filename);
        }

    return NULL;
}


/**
 * This function creates one thread to print things and
 * one other thread to calculate the checksums. This function
 * is a thread itself.
 * It waits until the end of the calc_thread thread (this will change
 * as in the future thoses functions should have an end unless the program
 * itself ends.
 * @param data : main_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
gpointer ciseaux(gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    GThread *calc_thread = NULL;

    if (main_struct != NULL)
        {
            calc_thread = g_thread_new("hashs", calculate_hashs_on_a_file, main_struct);

            g_thread_join(calc_thread);
        }

    return NULL;
}



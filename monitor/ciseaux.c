/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    ciseaux.c
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
 * @file ciseaux.c
 * This program should receive filenames on which it has to calculate their
 * SHA256 checksums : checksums are calculated for a defined block size.
 */

#include "monitor.h"

static void do_checksum(main_struct_t *main_struct, GFileInputStream *stream, gchar *filename);
static void it_is_a_directory(main_struct_t *main_struct, gchar *dirname, GFileInfo *fileinfo);
static void it_is_a_file(main_struct_t *main_struct, GFile *a_file, gchar *filename, GFileInfo *fileinfo);
static gpointer calculate_hashs_on_a_file(gpointer data);
static gpointer print_things(gpointer data);

/**
 * Does the checksum on the opened stream.
 * @param main_struct is the mains structure fro the program
 * @param stream the stream on which we want to calculate the checksum
 * @param filename is the filename of the opened stream (to be displayed in
 *        case of error.
 */
static void do_checksum(main_struct_t *main_struct, GFileInputStream *stream, gchar *filename)
{
    gssize read = 1;
    guchar *buffer = NULL;
    guint64 i = 0;
    GChecksum *checksum = NULL;
    GError *error = NULL;
    gchar *to_print = NULL;
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
                    fprintf(stderr, _("Error while reading %s file: %s\n"), filename, error->message);
                    error = free_error(error);
                    read = 0;
                }
            else
                {

                    if (read != 0)
                        {
                            g_checksum_update(checksum, buffer, read);
                            g_checksum_get_digest(checksum, a_hash, &digest_len);
                            to_print = g_strdup_printf("-> %ld\n%ld\n%s", i, read, g_checksum_get_string(checksum));
                            insert_into_tree(main_struct->hashs, a_hash, buffer, read);
                            g_async_queue_push(main_struct->print_queue, to_print);

                            g_checksum_reset(checksum);
                            i = i + 1;
                            digest_len = HASH_LEN;
                        }
                }
        }

    g_free(a_hash);
    g_free(buffer);
    g_checksum_free(checksum);
}


/**
 * This function transmits all information about the directory.
 * @note Do not free to_print variable here because it is transmited to
 *       the thread that will print it and is freed there.
 * @param main_struct : main structure (needed to know the queue to
 *        send the message.
 * @param dirname : a gchar * directory name.
 */
static void it_is_a_directory(main_struct_t *main_struct, gchar *dirname, GFileInfo *fileinfo)
{
    gchar *to_print = NULL;
    gchar *owner = NULL;
    gchar *dates = NULL;
    gchar *mode = NULL;

    if (main_struct != NULL && main_struct->print_queue != NULL)
        {
            owner = get_username_owner_from_gfile(fileinfo);
            dates = get_dates_from_gfile(fileinfo);
            mode = get_file_mode_from_gfile(fileinfo);

            to_print = g_strdup_printf("%d\n%s\n%s\n%s\n%s\x0", G_FILE_TYPE_DIRECTORY, owner, dates, mode, dirname);

            g_async_queue_push(main_struct->print_queue, to_print);

            free_variable(owner);
            free_variable(dates);
            free_variable(mode);
        }
}


/**
 * This functions is calling everything that is needed to calculate
 * the checksums of a file.
 * @note Do not free to_print variable here because it is transmited to
 *       the thread that will print it and is freed there.
 * @param main_struct : main structure that contains everything
 * @param a_file : the GFile opened
 * @param filename the filename of the opened GFile
 */
static void it_is_a_file(main_struct_t *main_struct, GFile *a_file, gchar *filename, GFileInfo *fileinfo)
{
    GFileInputStream *stream = NULL;
    GError *error = NULL;
    gchar *to_print = NULL;
    gchar *owner = NULL;
    gchar *dates = NULL;
    gchar *mode = NULL;

    if (a_file != NULL && main_struct != NULL && main_struct->print_queue != NULL)
        {
            stream = g_file_read(a_file, NULL, &error);

            if (error != NULL)
                {
                    fprintf(stderr, _("Error while opening %s file: %s\n"), filename, error->message);
                    free_error(error);
                }
            else
                {
                    owner = get_username_owner_from_gfile(fileinfo);
                    dates = get_dates_from_gfile(fileinfo);
                    mode = get_file_mode_from_gfile(fileinfo);

                    to_print = g_strdup_printf("%d\n%s\n%s\n%s\n%s\x0", G_FILE_TYPE_REGULAR, owner, dates, mode, filename);
                    g_async_queue_push(main_struct->print_queue, to_print);

                    do_checksum(main_struct, stream, filename);
                    g_input_stream_close((GInputStream *) stream, NULL, NULL);

                    free_object(stream);
                    free_variable(owner);
                    free_variable(dates);
                    free_variable(mode);
                }
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


    if (main_struct != NULL)
        {

            do
                {
                    filename = free_variable(filename);

                    filename = g_async_queue_pop(main_struct->queue);

                    if (g_strcmp0(filename, "$END$") != 0)
                        {

                            a_file = g_file_new_for_path(filename);

                            if (a_file != NULL)
                                {
                                    fileinfo = g_file_query_info(a_file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

                                    if (error != NULL)
                                        {
                                            fprintf(stderr, _("Can't get informations on %s file: %s\n"), filename, error->message);
                                            error = free_error(error);
                                        }
                                    else if (fileinfo != NULL)
                                        {
                                            filetype = g_file_info_get_file_type(fileinfo);

                                            if (filetype == G_FILE_TYPE_REGULAR)
                                                {
                                                    it_is_a_file(main_struct, a_file, filename, fileinfo);
                                                }
                                            else if (filetype == G_FILE_TYPE_DIRECTORY)
                                                {
                                                    it_is_a_directory(main_struct, filename, fileinfo);
                                                }
                                            else
                                                {
                                                    fprintf(stderr, _("%s is not a regular file\n"), filename);
                                                }
                                            fileinfo = free_object(fileinfo);
                                        }

                                    a_file = free_object(a_file);
                                }
                        }
                }
            while (g_strcmp0(filename, "$END$") != 0);

            free_variable(filename);
        }

    return NULL;
}


/**
 * This function waits for messages in the queue and then prints them to
 * screen. Messages are sent by do checksum function via the print_queue
 * queue. This function is running in a thread.
 * @param data : main_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
static gpointer print_things(gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    gchar *to_print = NULL;

    if (main_struct != NULL && main_struct->opt != NULL && main_struct->opt->noprint == FALSE)
        {
            do
                {
                    to_print = free_variable(to_print);

                    to_print = g_async_queue_pop(main_struct->print_queue);

                    if (g_strcmp0(to_print, "$END$") != 0)
                        {
                            fprintf(stdout, "%s\n", to_print);
                        }
                }
            while (g_strcmp0(to_print, "$END$") != 0);
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
    GThread *print_thread = NULL;
    GThread *calc_thread = NULL;

    if (main_struct != NULL)
        {
            print_thread = g_thread_create(print_things, main_struct, TRUE, NULL);
            calc_thread = g_thread_create(calculate_hashs_on_a_file, main_struct, TRUE, NULL);

            g_thread_join(calc_thread);
            g_async_queue_push(main_struct->print_queue, "$END$");
            g_thread_join(print_thread);

        }

    return NULL;
}



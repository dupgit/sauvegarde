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

    checksum = g_checksum_new(G_CHECKSUM_SHA256);
    buffer = (guchar *) g_malloc0 (main_struct->opt->blocksize + 1);

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

                            to_print = g_strdup_printf("%ld - %s - %ld - %s", read, filename, i, g_checksum_get_string(checksum));
                            g_async_queue_push(main_struct->print_queue, to_print);

                            g_checksum_reset(checksum);
                            i = i + 1;
                        }
                }
        }

    g_free(buffer);
    g_checksum_free(checksum);
}



/**
 * The main function that will calculate the hashs on a file. This function
 * is a thread itself. It waits for messages in the main queue.
 * Messages are sent by the following functions :
 * . traverse_directory.
 * @param data is main_struct_t * pointer
 */
static gpointer calculate_hashs_on_a_file(gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    gchar *filename = NULL;
    GFile *a_file = NULL;
    GFileInputStream *stream = NULL;
    GFileInfo *fileinfo = NULL;
    GError *error = NULL;


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

                                            if (g_file_info_get_file_type(fileinfo) == G_FILE_TYPE_REGULAR)
                                                {

                                                    stream = g_file_read(a_file, NULL, &error);

                                                    if (error != NULL)
                                                        {
                                                            fprintf(stderr, _("Error while opening %s file: %s\n"), filename, error->message);
                                                            error = free_error(error);
                                                        }
                                                    else
                                                        {
                                                            do_checksum(main_struct, stream, filename);
                                                            g_input_stream_close((GInputStream *) stream, NULL, NULL);
                                                            stream = free_object(stream);
                                                        }
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

            filename = free_variable(filename);
        }

    return NULL;
}


/**
 * This function waits for messages in the queue and then prints them to
 * screen. Messages are sent by do checksum function via the print_queue
 * queue.
 * @param data : main_struct_t * structure.
 */
static gpointer print_things(gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    gchar *to_print = NULL;

    if (main_struct != NULL)
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



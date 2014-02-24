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

#include "ciseaux.h"

/**
 * Does the checksum on the opened stream
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

    checksum = g_checksum_new(G_CHECKSUM_SHA256);
    buffer = (guchar *) g_malloc0 (main_struct->opt->blocksize + 1);

    while (read != 0)
        {

            read = g_input_stream_read((GInputStream *) stream, buffer, main_struct->opt->blocksize, NULL, &error);

            if (error != NULL)
                {
                    fprintf(stderr, "Error while reading %s file : %s\n", filename, error->message);
                    read = 0;
                }
            else
                {

                    if (read != 0)
                        {
                            g_checksum_update(checksum, buffer, read);
                            fprintf(stdout, "%s - %ld - %s\n", filename, i, g_checksum_get_string(checksum));
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
 * is called from a thread pool.
 * @param data a gchar * filename
 * @param user_data is main_struct_t * pointer
 */
static void calculate_hashs_on_a_file(gpointer data, gpointer user_data)
{
    main_struct_t *main_struct = (main_struct_t *) user_data;
    gchar *filename = (gchar *) data;
    GFile *a_file = NULL;
    GFileInputStream *stream = NULL;
    GFileInfo *fileinfo = NULL;
    GError *error = NULL;


    a_file = g_file_new_for_path(filename);

    fileinfo = g_file_query_info(a_file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

    if (error != NULL)
        {
           fprintf(stderr, "Can't get informations on %s file : %s\n", filename, error->message);
        }
    else if (fileinfo != NULL)
        {

            if (g_file_info_get_file_type(fileinfo) == G_FILE_TYPE_REGULAR)
                {

                    stream = g_file_read(a_file, NULL, &error);

                    if (error != NULL)
                        {
                            fprintf(stderr, "Error while opening %s file : %s\n", filename, error->message);
                        }
                    else
                        {
                            do_checksum(main_struct, stream, filename);
                            g_input_stream_close((GInputStream *) stream, NULL, NULL);
                        }
                }
            else
                {
                    fprintf(stderr, "%s is not a regular file\n", filename);
                }
        }
}


/**
 * Inits the thread pool and saves it into main_struct
 * @param main_struct : the structures that stores everything. Without
 *        errors, tp field contains the new thread pool.
 */
static void init_thread_pool(main_struct_t *main_struct)
{
    GThreadPool *tp = NULL;
    GError *error = NULL;
    gint max_threads =  CISEAUX_MAX_THREADS;

    if (main_struct != NULL)
        {
            if (main_struct->opt != NULL && main_struct->opt->max_threads != CISEAUX_MAX_THREADS)
                {
                    max_threads = main_struct->opt->max_threads;
                }

            tp = g_thread_pool_new(calculate_hashs_on_a_file, main_struct, max_threads, FALSE, &error);

            if (tp != NULL && error == NULL)
                {
                    main_struct->tp = tp;
                }
            else if (error != NULL)
                {
                    fprintf(stderr, "Unable to create a thread pool : %s\n", error->message);
                    exit(EXIT_FAILURE);
                }
            else
                {
                    fprintf(stderr, "Thread pool not created but no error generated !\n");
                    exit(EXIT_FAILURE);
                }
        }
    else
        {
            exit(EXIT_FAILURE);
        }
}



/**
 * Main function
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns always 0
 */
int main(int argc, char **argv)
{
    main_struct_t *main_struct = NULL;  /** Structure that contians everything needed by the program */
    GSList *head = NULL;
    gint max_threads = 0;
    GMainLoop *mainloop = NULL;

    /* Initialising GLib */
    g_type_init();

    main_struct = (main_struct_t *) g_malloc0(sizeof(main_struct_t));

    main_struct->opt = do_what_is_needed_from_command_line_options(argc, argv);
    init_thread_pool(main_struct);

    head = main_struct->opt->filename_list;



    max_threads = g_thread_pool_get_max_threads(main_struct->tp);

    while (head != NULL)
        {
            if (g_thread_pool_get_num_threads(main_struct->tp) < max_threads)
                {
                    g_thread_pool_push(main_struct->tp, head->data, NULL);
                    head = g_slist_next(head);
                }
            else
                {
                    sleep(0.1);
                }
        }

    /* infinite loop */
    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    return 0;
}

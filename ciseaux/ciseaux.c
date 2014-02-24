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
 * The main function that will calculate the hashs on a file. This function
 * is called from a thread pool.
 * @param data
 * @param user_data is main_struct_t * pointer
 */
static void calculate_hashs_on_a_file(gpointer data, gpointer user_data)
{
    main_struct_t *main_struct = (main_struct_t *) user_data;


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



/**
 * Main function
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns always 0
 */
int main(int argc, char **argv)
{
    main_struct_t *main_struct = NULL;  /** Structure that contians everything needed by the program */


    /* Initialising GLib */
    g_type_init();

    main_struct = (main_struct_t *) g_malloc0(sizeof(main_struct_t));

    main_struct->opt = do_what_is_needed_from_command_line_options(argc, argv);
    init_thread_pool(main_struct);


    return 0;
}

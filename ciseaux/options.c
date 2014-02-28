/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    options.c
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
 * @file options.c
 *
 *  This file contains all the functions to manage ciseaux program command
 *  line options
 */

#include "ciseaux.h"

/**
 * This function parses command line options.
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option
 */
options_t *manage_command_line_options(int argc, char **argv)
{
    gboolean version = FALSE;      /** version option selected ?                   */
    gint64 blocksize = 0;            /** computed block size in bytes                */
    gint64 max_threads = 0;          /** Maximum number of threads to be used        */

    GOptionEntry entries[] =
    {
        { "version", 'v', 0, G_OPTION_ARG_NONE, &version, N_("Prints program version"), NULL },
        { "blocksize", 'b', 0, G_OPTION_ARG_INT64 , &blocksize, N_("Block size used to compute hashs"), NULL},
        { "max-threads", 'm', 0, G_OPTION_ARG_INT64 , &max_threads, N_("Maximum threads we can use at once"), NULL},
        { NULL }
    };

    GError *error = NULL;
    GOptionContext *context;  /** GOption context to manage options     */
    options_t *opt = NULL;    /** Structure to manage program's options */
    gchar *bugreport = NULL;
    gchar *summary = NULL;


    opt = (options_t *) g_malloc0(sizeof(options_t));

    bugreport = g_strconcat(_("Please report bugs to: "), PACKAGE_BUGREPORT, NULL);
    summary = g_strdup(_("This program is hashing files with SHA256 algorithms from Glib."));
    context = g_option_context_new("");

    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
    g_option_context_set_help_enabled(context, TRUE);
    g_option_context_set_description(context, bugreport);
    g_option_context_set_summary(context, summary);

    if (!g_option_context_parse(context, &argc, &argv, &error))
        {
            g_print(_("Option parsing failed: %s\n"), error->message);
            exit(1);
        }

    opt->version = version;

    if (blocksize > 0)
        {
            opt->blocksize = blocksize;
        }
    else
        {
            opt->blocksize = CISEAUX_BLOCK_SIZE;
        }

    if (max_threads > 0)
        {
            opt->max_threads = max_threads;
        }
    else
        {
             opt->max_threads = CISEAUX_MAX_THREADS;
        }

    g_option_context_free(context);
    g_free(bugreport);
    g_free(summary);

    return opt;
}


/**
 * Frees the options structure if necessary
 * @param opt : the malloc'ed options_t structure
 */
void free_options_t_structure(options_t *opt)
{

    if (opt != NULL)
        {
            g_free(opt);
        }

}


/**
 * Decides what to do upon command lines options passed to the program
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option (in manage_command_line_options function).
 */
options_t *do_what_is_needed_from_command_line_options(int argc, char **argv)
{
    options_t *opt = NULL;  /** Structure to manage options from the command line can be freed when no longer needed */

    opt = manage_command_line_options(argc, argv);

    if (opt != NULL && opt->version == TRUE)
        {
            print_program_version(CISEAUX_DATE, CISEAUX_AUTHORS, CISEAUX_LICENSE);
            print_libraries_versions();
        }

    return opt;
}



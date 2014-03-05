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
 *  This file contains all the functions to manage command line options.
 */

#include "monitor.h"


/**
 * Reads from the configuration file "filename"
 * @param opt : options_t * structure to store options read from the
 *              configuration file "filename"
 * @param filename : the filename of the configuration file to read from
 */
static void read_from_configuration_file(options_t *opt, gchar *filename)
{
    GKeyFile *keyfile = NULL;      /** Configuration file parser                          */
    GError *error = NULL;          /** Glib error handling                                */
    gchar **dirname_array = NULL;  /** array of dirnames read into the configuration file */
    gint num = 0;
    gint i = 0;
    gchar *dirname = NULL;         /** A variable to help to have things more simple      */

    if (filename != NULL)
        {
            opt->configfile = g_strdup(filename);

            if (ENABLE_DEBUG == TRUE)
                {
                    fprintf(stdout, _("Reading configuration from file %s\n"), opt->configfile);
                }

            keyfile = g_key_file_new();

            if (g_key_file_load_from_file(keyfile, opt->configfile, G_KEY_FILE_KEEP_COMMENTS, &error))
                {
                    dirname_array = g_key_file_get_string_list(keyfile, GN_MONITOR, KN_DIR_LIST, NULL, &error);

                    if (dirname_array != NULL)
                        {
                            num = g_strv_length(dirname_array);

                            for (i = 0; i < num; i++)
                                {
                                    dirname = g_strdup(dirname_array[i]);
                                    opt->dirname_list = g_slist_append(opt->dirname_list, dirname);

                                    if (ENABLE_DEBUG == TRUE)
                                        {
                                            fprintf(stdout, "%s\n", dirname);
                                        }
                                }
                        }
                    else if (error != NULL &&  ENABLE_DEBUG == TRUE)
                        {
                            fprintf(stderr, _("Could not load directory list from file %s : %s\n"), filename, error->message);
                        }

                    opt->blocksize = g_key_file_get_int64(keyfile, GN_CISEAUX, KN_BLOCK_SIZE, &error);
                    if (error != NULL && ENABLE_DEBUG == TRUE)
                        {
                            fprintf(stderr, _("Could not load blocksize from file %s : %s"), filename, error->message);
                        }

                    opt->max_threads = g_key_file_get_int64(keyfile, GN_CISEAUX, KN_MAX_THREADS, &error);
                    if (error != NULL && ENABLE_DEBUG == TRUE)
                        {
                            fprintf(stderr, _("Could not load max-threads from file %s : %s"), filename, error->message);
                        }
                }
            else if (error != NULL && ENABLE_DEBUG == TRUE)
                {
                    fprintf(stderr, _("Failed to open %s configuration file : %s\n"), filename, error->message);
                }

            g_key_file_free(keyfile);
        }
}


/**
 * This function parses command line options.
 * 0) default values are set into the options_t * structure
 * 1) reads the default configuration file if any.
 * 2) reads the configuration file mentionned in the command line option
 * 3) sets the command line options (except for the list of directories,
 *    all other values are replaced by thoses in the command line)
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option
 */
options_t *manage_command_line_options(int argc, char **argv)
{
    gboolean version = FALSE;
    gchar **dirname_array = NULL;  /** array of dirnames left on the command line  */
    gchar *configfile = NULL;      /** filename for the configuration file if any  */
    gint64 blocksize = 0;          /** computed block size in bytes                */
    gint64 max_threads = 0;        /** Maximum number of threads to be used        */

    GOptionEntry entries[] =
    {
        { "version", 'v', 0, G_OPTION_ARG_NONE, &version, N_("Prints program version"), NULL },
        { "configuration", 'c', 0, G_OPTION_ARG_STRING, &configfile, N_("Specify an alternative configuration file"), NULL},
        { "blocksize", 'b', 0, G_OPTION_ARG_INT64 , &blocksize, N_("Block size used to compute hashs"), NULL},
        { "max-threads", 'm', 0, G_OPTION_ARG_INT64 , &max_threads, N_("Maximum threads we can use at once"), NULL},
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &dirname_array, "", NULL},
        { NULL }
    };

    GError *error = NULL;
    GOptionContext *context;
    options_t *opt = NULL;    /** Structure to manage program's options */
    gchar *bugreport = NULL;
    gchar *summary = NULL;
    gint num = 0;             /** number of filenames in the filename_array if any */
    gint i = 0;
    gchar *dirname = NULL;
    gchar *defaultconfigfilename = NULL;


    opt = (options_t *) g_malloc0(sizeof(options_t));

    bugreport = g_strconcat(_("Please report bugs to: "), PACKAGE_BUGREPORT, NULL);
    summary = g_strdup(_("This program is monitoring file changes in the filesystem and is hashing\nfiles with SHA256 algorithms from Glib."));
    context = g_option_context_new("");

    set_option_context_options(context, entries, TRUE, bugreport, summary);

    if (!g_option_context_parse(context, &argc, &argv, &error))
        {
            g_print(_("Option parsing failed: %s\n"), error->message);
            exit(1);
        }

    /* 0) Setting default values */
    opt->dirname_list = NULL;
    opt->blocksize = CISEAUX_BLOCK_SIZE;
    opt->max_threads = CISEAUX_MAX_THREADS;

    /* 1) Reading from the default configuration file */
    defaultconfigfilename = get_probable_etc_path("client");
    read_from_configuration_file(opt,  defaultconfigfilename);
    g_free(defaultconfigfilename);

    opt->version = version; /* only TRUE if -v or --version was invoked */

    if (configfile != NULL)
        {
            /* 2) Reading the configuration from the configuration file */
            read_from_configuration_file(opt, configfile);
        }

    if (dirname_array != NULL)
        {
            /* 3) retrieving the filenames stored in the array to put them
             *    into a list in the options_t * structure
             */
            num = g_strv_length(dirname_array);

            for (i = 0; i < num; i++)
                {
                    dirname = g_strdup(dirname_array[i]);
                    opt->dirname_list = g_slist_append(opt->dirname_list, dirname);
                }
        }

    if (blocksize > 0)
        {
            opt->blocksize = blocksize;
        }
    if (max_threads > 0)
        {
            opt->max_threads = max_threads;
        }

    g_option_context_free(context);
    g_free(bugreport);

    return opt;
}


/**
 * Frees the options structure if necessary
 * @param opt : the malloc'ed options_t structure
 */
void free_options_t_structure(options_t *opt)
{
    GSList *head = NULL;
    GSList *next = NULL;


    if (opt != NULL)
        {
            /* free the list */
            head = opt->dirname_list;

            while (head != NULL)
                {
                    g_free(head->data);
                    next = g_slist_next(head);
                    g_slist_free_1(head);
                    head = next;
                }

            g_free(opt->configfile);

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

    if (opt != NULL)
        {
            if (opt->version == TRUE)
                {
                    print_program_version(MONITOR_DATE, MONITOR_AUTHORS, MONITOR_LICENSE);
                    print_libraries_versions();
                    exit(EXIT_SUCCESS);
                }
        }

    return opt;
}



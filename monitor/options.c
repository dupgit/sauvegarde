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

static void print_selected_options(options_t *opt)
{
    GSList *head = NULL;

    if (opt != NULL)
        {
            fprintf(stdout, _("\nOptions are :\n"));

            if (opt->dirname_list != NULL)
                {
                    fprintf(stdout, _("Directory list :\n"));
                    head = opt->dirname_list;
                    while (head != NULL)
                        {
                            fprintf(stdout, "\t%s\n", (char *) head->data);
                            head = g_slist_next(head);
                        }
                }

            fprintf(stdout, _("Blocksize : %ld\n"), opt->blocksize);

            if (opt->configfile != NULL)
                {
                    fprintf(stdout, _("Configuration file : %s\n"), opt->configfile);
                }
        }
}



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
                            error = free_error(error);
                        }

                    opt->blocksize = g_key_file_get_int64(keyfile, GN_CISEAUX, KN_BLOCK_SIZE, &error);
                    if (error != NULL && ENABLE_DEBUG == TRUE)
                        {
                            fprintf(stderr, _("Could not load blocksize from file %s : %s"), filename, error->message);
                            error = free_error(error);
                        }
                }
            else if (error != NULL && ENABLE_DEBUG == TRUE)
                {
                    fprintf(stderr, _("Failed to open %s configuration file : %s\n"), filename, error->message);
                    error = free_error(error);
                }

            g_key_file_free(keyfile);
        }
}


/**
 * This function parses command line options. It sets the options in this
 * order. It means that the value used for an option is the one set in the
 * lastest step.
 * 0) default values are set into the options_t * structure
 * 1) reads the default configuration file if any.
 * 2) reads the configuration file mentionned on the command line.
 * 3) sets the command line options (except for the list of directories,
 *    all other values are replaced by thoses in the command line)
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option
 */
options_t *manage_command_line_options(int argc, char **argv)
{
    gboolean version = FALSE;      /** True if -v was selected on the command line */
    gchar **dirname_array = NULL;  /** array of dirnames left on the command line  */
    gchar *configfile = NULL;      /** filename for the configuration file if any  */
    gint64 blocksize = 0;          /** computed block size in bytes                */

    GOptionEntry entries[] =
    {
        { "version", 'v', 0, G_OPTION_ARG_NONE, &version, N_("Prints program version"), NULL },
        { "configuration", 'c', 0, G_OPTION_ARG_STRING, &configfile, N_("Specify an alternative configuration file"), NULL},
        { "blocksize", 'b', 0, G_OPTION_ARG_INT64 , &blocksize, N_("Block size used to compute hashs"), NULL},
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &dirname_array, "", NULL},
        { NULL }
    };

    GError *error = NULL;
    GOptionContext *context;
    options_t *opt = NULL;    /** Structure to manage program's options            */
    gchar *bugreport = NULL;  /** Bug Report message                               */
    gchar *summary = NULL;    /** Abstract for the program                         */
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
            exit(EXIT_FAILURE);
        }

    /* 0) Setting default values */
    opt->dirname_list = NULL;
    opt->blocksize = CISEAUX_BLOCK_SIZE;

    /* 1) Reading options from default configuration file */
    defaultconfigfilename = get_probable_etc_path(PROGRAM_NAME);
    read_from_configuration_file(opt,  defaultconfigfilename);
    defaultconfigfilename = free_variable(defaultconfigfilename);

    opt->version = version; /* only TRUE if -v or --version was invoked */

    /* 2) Reading the configuration from the configuration file specified
     *    on the command line
     */
    if (configfile != NULL)
        {
            read_from_configuration_file(opt, configfile);
        }


    /* 3) retrieving other options from the command line. Directories are
     *    added to the existing directory list
     */
    if (dirname_array != NULL)
        {
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

    g_option_context_free(context);
    free_variable(bugreport);
    free_variable(summary);

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
                    head->data = free_variable(head->data);
                    next = g_slist_next(head);
                    g_slist_free_1(head);
                    head = next;
                }

            free_variable(opt->configfile);
            free_variable(opt);
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
                    print_selected_options(opt);
                    exit(EXIT_SUCCESS);
                }
        }

    return opt;
}



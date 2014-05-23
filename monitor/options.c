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
 * @file monitor/options.c
 *
 *  This file contains all the functions to manage command line options.
 */

#include "monitor.h"

static void print_selected_options(options_t *opt);
static void read_from_group_monitor(options_t *opt, GKeyFile *keyfile, gchar *filename);
static void read_from_group_ciseaux(options_t *opt, GKeyFile *keyfile, gchar *filename);
static void read_from_group_antememoire(options_t *opt, GKeyFile *keyfile, gchar *filename);
static void read_from_configuration_file(options_t *opt, gchar *filename);
static void print_string_option(gchar *description, gchar *string);


/**
 * Prints if string is not NULL then prints it to stdout right in the
 * 'description' printf format.
 * @param description is a fprintf format string that must contain a %s
 *        in order to include the string 'string'
 * @param string is the string to be printed.
 */
static void print_string_option(gchar *description, gchar *string)
{
    if (string != NULL)
    {
        fprintf(stdout, description, string);
    }
}


/**
 * Prints options as selected when invoking the program with -v option
 * @param opt the options_t * structure that contains all selected options
 *        from the command line and that will be used by the program.
 */
static void print_selected_options(options_t *opt)
{
    GSList *head = NULL;

    if (opt != NULL)
        {
            fprintf(stdout, _("\n%s options are:\n"), PROGRAM_NAME);

            if (opt->dirname_list != NULL)
                {
                    fprintf(stdout, _("Directory list:\n"));
                    head = opt->dirname_list;
                    while (head != NULL)
                        {
                            fprintf(stdout, "\t%s\n", (char *) head->data);
                            head = g_slist_next(head);
                        }
                }

            fprintf(stdout, _("Blocksize: %ld\n"), opt->blocksize);

            print_string_option(N_("Configuration file: %s\n"), opt->configfile);
            print_string_option(N_("Cache directory: %s\n"), opt->dircache);
            print_string_option(N_("Cache database name: %s\n"), opt->dbname);
            print_string_option(N_("serveur's IP address: %s\n"), opt->ip);
        }
}


/**
 * Reads keys in keyfile if group GN_MONITOR is in that keyfile and fills
 * options_t *opt structure accordingly.
 * @param[in,out] opt : options_t * structure to store options read from the
 *                configuration file "filename".
 * @param keyfile is the GKeyFile structure that is used by glib to read
 *        groups and keys from.
 * @param filename : the filename of the configuration file to read from
 */
static void read_from_group_monitor(options_t *opt, GKeyFile *keyfile, gchar *filename)
{
    if (keyfile != NULL && filename != NULL && g_key_file_has_group(keyfile, GN_MONITOR) == TRUE)
        {
            /* Reading the directory list */
            opt->dirname_list = read_list_from_file(keyfile, filename, GN_MONITOR, KN_DIR_LIST, N_("Could not load directory list from file"));
        }
}


/**
 * Reads keys in keyfile if group GN_CISEAUX is in that keyfile and fills
 * options_t *opt structure accordingly.
 * @param[in,out] opt : options_t * structure to store options read from the
 *                configuration file "filename".
 * @param keyfile is the GKeyFile structure that is used by glib to read
 *        groups and keys from.
 * @param filename : the filename of the configuration file to read from
 */
static void read_from_group_ciseaux(options_t *opt, GKeyFile *keyfile, gchar *filename)
{
    if (keyfile != NULL && filename != NULL && g_key_file_has_group(keyfile, GN_CISEAUX) == TRUE)
        {
            /* Reading the blocksize if any */
            opt->blocksize = read_int64_from_file(keyfile, filename, GN_CISEAUX, KN_BLOCK_SIZE, N_("Could not load blocksize from file"));
        }
}


/**
 * Reads keys in keyfile if group GN_ANTEMEMOIRE is in that keyfile and fills
 * options_t *opt structure accordingly.
 * @param[in,out] opt : options_t * structure to store options read from the
 *                configuration file "filename".
 * @param keyfile is the GKeyFile structure that is used by glib to read
 *        groups and keys from.
 * @param filename : the filename of the configuration file to read from
 */
static void read_from_group_antememoire(options_t *opt, GKeyFile *keyfile, gchar *filename)
{
    if (keyfile != NULL && filename != NULL && g_key_file_has_group(keyfile, GN_ANTEMEMOIRE) == TRUE)
        {
            /* Reading the cache directory if any */
            opt->dircache = read_string_from_file(keyfile, filename, GN_ANTEMEMOIRE, KN_CACHE_DIR, N_("Could not load directory name"));

            /* Reading filename of the database if any */
            opt->dbname = read_string_from_file(keyfile, filename, GN_ANTEMEMOIRE, KN_DB_NAME, N_("Could not load cache database name"));

            /* Reading IP address of serveur's host if any */
            opt->ip = read_string_from_file(keyfile, filename, GN_ANTEMEMOIRE, KN_SERVEUR_IP, N_("Could not load cache database name"));
        }
}


/**
 * Reads from the configuration file "filename"
 * @param[in,out] opt : options_t * structure to store options read from the
 *                configuration file "filename"
 * @param filename : the filename of the configuration file to read from
 */
static void read_from_configuration_file(options_t *opt, gchar *filename)
{
    GKeyFile *keyfile = NULL;      /** Configuration file parser                          */
    GError *error = NULL;          /** Glib error handling                                */

    if (filename != NULL)
        {

            if (opt->configfile != NULL)
                {
                    free_variable(opt->configfile);
                }
            opt->configfile = g_strdup(filename);

            print_debug(stdout, _("Reading configuration from file %s\n"), filename);

            keyfile = g_key_file_new();

            if (g_key_file_load_from_file(keyfile, filename, G_KEY_FILE_KEEP_COMMENTS, &error))
                {
                    read_from_group_monitor(opt, keyfile, filename);
                    read_from_group_ciseaux(opt, keyfile, filename);
                    read_from_group_antememoire(opt, keyfile, filename);
                }
            else if (error != NULL)
                {
                    print_debug(stderr, _("Failed to open %s configuration file : %s\n"), filename, error->message);
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
    gboolean version = FALSE;      /** True if -v was selected on the command line           */
    gchar **dirname_array = NULL;  /** array of dirnames left on the command line            */
    gchar *configfile = NULL;      /** filename for the configuration file if any            */
    gint64 blocksize = 0;          /** computed block size in bytes                          */
    gboolean noprint = FALSE;      /** True if we do not want to print will checksuming      */
    gchar *dircache = NULL;        /** Directory used to store cache files                   */
    gchar *dbname = NULL;          /** Database filename where data and meta data are cached */
    gchar *ip =  NULL;             /** IP address where is located serveur's program         */

    GOptionEntry entries[] =
    {
        { "version", 'v', 0, G_OPTION_ARG_NONE, &version, N_("Prints program version"), NULL },
        { "configuration", 'c', 0, G_OPTION_ARG_STRING, &configfile, N_("Specify an alternative configuration file"), NULL},
        { "blocksize", 'b', 0, G_OPTION_ARG_INT64 , &blocksize, N_("Block size used to compute hashs"), NULL},
        { "noprint", 'n', 0, G_OPTION_ARG_NONE, &noprint, N_("Quiets the program while calculating checksum"), NULL},
        { "dircache", 'd', 0, G_OPTION_ARG_STRING, &dircache, N_("Directory where to cache files"), NULL},
        { "dbname", 'f', 0, G_OPTION_ARG_STRING, &dbname, N_("Database filename"), NULL},
        { "ip", 'i', 0, G_OPTION_ARG_STRING, &ip, N_("IP address where serveur program is."), NULL},
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &dirname_array, "", NULL},
        { NULL }
    };

    GError *error = NULL;
    GOptionContext *context;
    options_t *opt = NULL;    /** Structure to manage program's options            */
    gchar *bugreport = NULL;  /** Bug Report message                               */
    gchar *summary = NULL;    /** Abstract for the program                         */
    gchar *defaultconfigfilename = NULL;

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

    opt = (options_t *) g_malloc0(sizeof(options_t));

    opt->dirname_list = NULL;
    opt->blocksize = CISEAUX_BLOCK_SIZE;
    opt->noprint = FALSE;
    opt->configfile = NULL;
    opt->dircache = g_strdup("/var/tmp/sauvegarde");
    opt->dbname = g_strdup("filecache.db");
    opt->ip = g_strdup("localhost:5468");

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
     *    added to the existing directory list and then the array is freed
     *    as every string has been copied with g_strdup().
     */
    opt->dirname_list = convert_gchar_array_to_GSList(dirname_array, opt->dirname_list);
    g_strfreev(dirname_array);

    if (blocksize > 0)
        {
            opt->blocksize = blocksize;
        }

    if (noprint == TRUE)
        {
            opt->noprint = TRUE;
        }

    if (dircache != NULL)
        {
            free_variable(opt->dircache);
            opt->dircache = g_strdup(dircache);
        }

    if (dbname != NULL)
        {
            free_variable(opt->dbname);
            opt->dbname = g_strdup(dbname);
        }

    if (ip != NULL)
        {
            free_variable(opt->ip);
            opt->ip = g_strdup(ip);
        }

    g_option_context_free(context);
    free_variable(dbname);
    free_variable(dircache);
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

            free_variable(opt->dircache);
            free_variable(opt->configfile);
            free_variable(opt->dbname);
            free_variable(opt->ip);
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



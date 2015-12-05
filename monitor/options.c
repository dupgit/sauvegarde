/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    options.c
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
 * @file monitor/options.c
 *
 *  This file contains all the functions to manage command line options.
 * @todo review file configuration sections to make it more consistant
 *       with actual program.
 */

#include "monitor.h"

static void print_selected_options(options_t *opt);
static void read_from_group_client(options_t *opt, GKeyFile *keyfile, gchar *filename);
static void read_from_group_serveur(options_t *opt, GKeyFile *keyfile, gchar *filename);
static void read_from_configuration_file(options_t *opt, gchar *filename);
static void print_filelist(GSList *filelist, gchar *title);


/**
 * Prints filenames contained in the list.
 * @param filelist is a list containing file list (directories to be
 *        savec or to be excluded for instance).
 * @param title is a gchar * string that is printed at the top of the
 *        list.
 */
static void print_filelist(GSList *filelist, gchar *title)
{

    if (filelist != NULL)
        {
            fprintf(stdout, title);
            while (filelist != NULL)
                {
                    fprintf(stdout, "\t%s\n", (char *) filelist->data);
                    filelist = g_slist_next(filelist);
                }
        }
}


/**
 * Prints options as selected when invoking the program with -v option
 * @param opt the options_t * structure that contains all selected options
 *        from the command line and that will be used by the program.
 */
static void print_selected_options(options_t *opt)
{
    gchar *blocksize = NULL;

    if (opt != NULL)
        {
            fprintf(stdout, _("\n%s options are:\n"), PROGRAM_NAME);

            print_filelist(opt->dirname_list, _("Directory list:\n"));
            print_filelist(opt->exclude_list, _("Exclude list:\n"));

            if (opt->adaptative == FALSE)
                {
                    /**
                     * We need to translated this number into a string before
                     * inserting it into the final string in order to allow
                     * this final string to be translated in an other language.
                     */
                    blocksize = g_strdup_printf("%" G_GINT64_FORMAT, opt->blocksize);
                    fprintf(stdout, _("Blocksize: %s\n"), blocksize);
                    free_variable(blocksize);
                }
            else
                {
                    fprintf(stdout, _("Blocksize: adaptative mode\n"));
                }

            print_string_option(_("Configuration file: %s\n"), opt->configfile);
            print_string_option(_("Cache directory: %s\n"), opt->dircache);
            print_string_option(_("Cache database name: %s\n"), opt->dbname);
            print_string_option(_("Server's IP address: %s\n"), opt->ip);
            fprintf(stdout, _("Server's port number: %d\n"), opt->port);
            fprintf(stdout, _("Buffersize: %d\n"), opt->buffersize);
        }
}


/**
 * Reads keys in keyfile if group GN_CLIENT is in that keyfile and fills
 * options_t *opt structure accordingly.
 * @param[in,out] opt : options_t * structure to store options read from the
 *                configuration file "filename".
 * @param keyfile is the GKeyFile structure that is used by glib to read
 *        groups and keys from.
 * @param filename : the filename of the configuration file to read from
 */
static void read_from_group_client(options_t *opt, GKeyFile *keyfile, gchar *filename)
{
    gchar *dircache = NULL;

    if (keyfile != NULL && filename != NULL && g_key_file_has_group(keyfile, GN_CLIENT) == TRUE)
        {
            /* Reading the directory list */
            opt->dirname_list = read_list_from_file(keyfile, filename, GN_CLIENT, KN_DIR_LIST, _("Could not load directory list from file"));
            opt->exclude_list = read_list_from_file(keyfile, filename, GN_CLIENT, KN_EXC_LIST, _("Could not load exclude file list from file"));

            /* Reading blocksize */
            opt->blocksize = read_int64_from_file(keyfile, filename, GN_CLIENT, KN_BLOCK_SIZE, _("Could not load blocksize from file"));

            /* Reading the cache directory if any */
            dircache = read_string_from_file(keyfile, filename, GN_CLIENT, KN_CACHE_DIR, _("Could not load directory name"));
            opt->dircache = normalize_directory(dircache);
            free_variable(dircache);

            /* Reading filename of the database if any */
            opt->dbname = read_string_from_file(keyfile, filename, GN_CLIENT, KN_DB_NAME, _("Could not load cache database name"));

            /* Adaptative mode for blocksize ? */
            opt->adaptative = read_boolean_from_file(keyfile, filename, GN_CLIENT, KN_ADAPTATIVE, _("Could not load adaptative configuration from file."));

            /* Buffer size to be used to send data to server */
            opt->buffersize = read_int_from_file(keyfile, filename, GN_CLIENT, KN_BUFFER_SIZE, _("Could not load buffersize from file"));
        }

   read_debug_mode_from_file(keyfile, filename);
}

/**
 * Reads keys in keyfile if groupname is in that keyfile and fills
 * options_t *opt structure accordingly.
 * @param[in,out] opt : options_t * structure to store options read from the
 *                configuration file "filename".
 * @param keyfile is the GKeyFile structure that is used by glib to read
 *        groups and keys from.
 * @param filename : the filename of the configuration file to read from
 */
static void read_from_group_serveur(options_t *opt, GKeyFile *keyfile, gchar *filename)
{
    gint port = 0;

    if (opt != NULL && keyfile != NULL && filename != NULL && g_key_file_has_group(keyfile, GN_SERVEUR) == TRUE)
        {
            /* Reading the port number if any */
            port = read_int_from_file(keyfile, filename, GN_SERVEUR, KN_SERVEUR_PORT, _("Could not load serveur port number from file."));

            if (port > 1024 && port < 65535)
                {
                    opt->port = port;
                }

            /* Reading IP address of server's host if any */
            opt->ip = read_string_from_file(keyfile, filename, GN_SERVEUR, KN_SERVEUR_IP, _("Could not load cache database name"));
        }
}


/**
 * Reads from the configuration file "filename" and fills the options_t *
 * opt structure.
 * @param[in,out] opt : options_t * structure to store options read from
 *                the configuration file "filename"
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

            print_debug(_("Reading configuration from file %s\n"), filename);

            keyfile = g_key_file_new();

            if (g_key_file_load_from_file(keyfile, filename, G_KEY_FILE_KEEP_COMMENTS, &error))
                {
                    read_from_group_client(opt, keyfile, filename);
                    read_from_group_serveur(opt, keyfile, filename);
                }
            else if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, _("Failed to open %s configuration file: %s\n"), filename, error->message);
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
    gint debug = -4;               /** 0 == FALSE and other values == TRUE                   */
    gint adaptative = -1;          /** 0 == FALSE and other positive values == TRUE          */
    gchar **dirname_array = NULL;  /** array of dirnames left on the command line            */
    gchar **exclude_array = NULL;  /** array of dirnames and filenames to be excluded        */
    gchar *configfile = NULL;      /** filename for the configuration file if any            */
    gint64 blocksize = 0;          /** computed block size in bytes                          */
    gint buffersize = 0;           /** buffer size used to send data to server               */
    gchar *dircache = NULL;        /** Directory used to store cache files                   */
    gchar *dbname = NULL;          /** Database filename where data and meta data are cached */
    gchar *ip =  NULL;             /** IP address where is located server's program          */
    gint port = 0;                 /** Port number on which to send things to the server     */

    GOptionEntry entries[] =
    {
        { "version", 'v', 0, G_OPTION_ARG_NONE, &version, N_("Prints program version"), NULL },
        { "debug", 'd', 0,  G_OPTION_ARG_INT, &debug, N_("Activates (1) or desactivates (0) debug mode."), N_("BOOLEAN")},
        { "configuration", 'c', 0, G_OPTION_ARG_STRING, &configfile, N_("Specify an alternative configuration file."), N_("FILENAME")},
        { "blocksize", 'b', 0, G_OPTION_ARG_INT64, &blocksize, N_("Fixed block SIZE used to compute hashs."), N_("SIZE")},
        { "adaptative", 'a', 0, G_OPTION_ARG_INT, &adaptative, N_("Adapative block size used to compute hashs."), N_("BOOLEAN")},
        { "buffersize", 's', 0, G_OPTION_ARG_INT, &buffersize, N_("SIZE of the cache used to send data to server."), N_("SIZE")},
        { "dircache", 'r', 0, G_OPTION_ARG_STRING, &dircache, N_("Directory DIRNAME where to cache files."), N_("DIRNAME")},
        { "dbname", 'f', 0, G_OPTION_ARG_STRING, &dbname, N_("Database FILENAME."), N_("FILENAME")},
        { "ip", 'i', 0, G_OPTION_ARG_STRING, &ip, N_("IP address where server program is."), "IP"},
        { "port", 'p', 0, G_OPTION_ARG_INT, &port, N_("Port NUMBER on which to listen."), N_("NUMBER")},
        { "exclude", 'x', 0, G_OPTION_ARG_FILENAME_ARRAY, &exclude_array, N_("Exclude FILENAME from being saved."), N_("FILENAME")},
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

    set_debug_mode(ENABLE_DEBUG);

    set_option_context_options(context, entries, TRUE, bugreport, summary);

    if (!g_option_context_parse(context, &argc, &argv, &error))
        {
            g_print(_("Option parsing failed: %s\n"), error->message);
            exit(EXIT_FAILURE);
        }

    /* 0) Setting default values */

    opt = (options_t *) g_malloc0(sizeof(options_t));

    opt->dirname_list = NULL;
    opt->exclude_list = NULL;
    opt->blocksize = CLIENT_BLOCK_SIZE;
    opt->configfile = NULL;
    opt->dircache = g_strdup("/var/tmp/cdpfgl");
    opt->dbname = g_strdup("filecache.db");
    opt->ip = g_strdup("localhost");
    opt->port = 5468;
    opt->buffersize = -1;
    opt->adaptative = FALSE;

    /* 1) Reading options from default configuration file */
    defaultconfigfilename = get_probable_etc_path(PROGRAM_NAME, "client.conf");
    read_from_configuration_file(opt,  defaultconfigfilename);
    defaultconfigfilename = free_variable(defaultconfigfilename);

    opt->version = version; /* only TRUE if -v or --version was invoked */


    /* 2) Reading the configuration from the configuration file specified
     *    on the command line (if any).
     */
    if (configfile != NULL)
        {
            read_from_configuration_file(opt, configfile);
        }


    /* 3) retrieving other options from the command line. Directories are
     *    added to the existing directory list and then the array is freed
     *    as every string has been copied with g_strdup().
     */
    set_debug_mode_upon_cmdl(debug);

    opt->dirname_list = convert_gchar_array_to_GSList(dirname_array, opt->dirname_list);
    opt->exclude_list = convert_gchar_array_to_GSList(exclude_array, opt->exclude_list);

    g_strfreev(dirname_array);
    g_strfreev(exclude_array);

    if (blocksize > 0)
        {
            opt->blocksize = blocksize;
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

    if (port > 1024 && port < 65535)
        {
            opt->port = port;
        }

    if (adaptative > 0)
        {
            opt->adaptative = TRUE;
        }
    else if (adaptative == 0)
        {
            opt->adaptative = FALSE;
        }

    if (buffersize > 0)
        {
            opt->buffersize = buffersize;
        }
    else if (opt->buffersize <= 0)
        {
            opt->buffersize = CLIENT_MIN_BUFFER;
        }

    g_option_context_free(context);
    free_variable(ip);
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
    if (opt != NULL)
        {
            free_list(opt->dirname_list);
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
                    print_program_version(PROGRAM_NAME, MONITOR_DATE, MONITOR_VERSION, MONITOR_AUTHORS, MONITOR_LICENSE);
                    print_libraries_versions(PROGRAM_NAME);
                    print_selected_options(opt);
                    exit(EXIT_SUCCESS);
                }
        }

    return opt;
}



/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    options.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 - 2017 Olivier Delhomme
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
 * @file restore/options.c
 *
 *  This file contains all the functions to manage command line options for
 *  'cdpfglrestore' program.
 */

#include "restore.h"

static void print_selected_options(options_t *opt);
static void read_from_group_all(GKeyFile *keyfile, gchar *filename);
static void read_from_group_server(options_t *opt, GKeyFile *keyfile, gchar *filename);
static options_t *manage_command_line_options(int argc, char **argv);


/**
 * Prints options as selected when invoking the program with -v option
 * @param opt the options_t * structure that contains all selected options
 *        from the command line and that will be used by the program.
 */
static void print_selected_options(options_t *opt)
{
    if (opt != NULL)
        {
            fprintf(stdout, _("\n%s options are:\n"), PROGRAM_NAME);
            print_string_option(_("Configuration file: %s\n"), opt->configfile);
            print_string_option(_("server's IP address: %s\n"), opt->ip);
            fprintf(stdout, _("server's port number: %d\n"), opt->port);
        }
}


/**
 * Reads keys in keyfile if group GN_ALL is in that keyfile.
 * @param keyfile is the GKeyFile structure that is used by glib to read
 *        groups and keys from.
 * @param filename : the filename of the configuration file to read from
 */
static void read_from_group_all(GKeyFile *keyfile, gchar *filename)
{
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
static void read_from_group_server(options_t *opt, GKeyFile *keyfile, gchar *filename)
{
    gint port = 0;

    if (opt != NULL && keyfile != NULL && filename != NULL && g_key_file_has_group(keyfile, GN_SERVER) == TRUE)
        {
            /* Reading the port number if any */
            port = read_int_from_file(keyfile, filename, GN_SERVER, KN_SERVER_PORT, _("Could not load server port number from file."), SERVER_PORT);

            if (port > 1024 && port < 65535)
                {
                    opt->port = port;
                }

            /* Reading IP address of server's host if any */
            opt->ip = read_string_from_file(keyfile, filename, GN_SERVER, KN_SERVER_IP, _("Could not load cache database name"));
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
    GKeyFile *keyfile = NULL;      /** Configuration file parser   */
    GError *error = NULL;          /** Glib error handling         */

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
                    read_from_group_all(keyfile, filename);
                    read_from_group_server(opt, keyfile, filename);
                }
            else if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, _("Failed to open %s configuration file: %s\n"), filename, error->message);
                    free_error(error);
                }

            g_key_file_free(keyfile);
        }
}



/**
 * This function parses command line options. It sets the options in this
 * order. It means that the value used for an option is the one set in the
 * latest step.
 * 0) default values are set into the options_t * structure
 * 1) reads the default configuration file if any.
 * 2) reads the configuration file mentioned on the command line.
 * 3) sets the command line options (except for the list of directories,
 *    all other values are replaced by those in the command line)
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option
 */
static options_t *manage_command_line_options(int argc, char **argv)
{
    options_t *opt = NULL;         /** Structure to manage program's options                                             */
    gchar *defaultconfigfilename = NULL;
    gchar *summary = NULL;         /** Abstract for the program                                                          */

    gboolean version = FALSE;      /** True if -v was selected on the command line                                       */
    gint debug = -4;               /** 0 == FALSE and other values == TRUE                                               */
    gchar *configfile = NULL;      /** filename for the configuration file if any                                        */
    gchar *ip =  NULL;             /** IP address where is located server's program                                      */
    gint port = 0;                 /** Port number on which to send things to the server                                 */
    gchar *list = NULL;            /** Should contain a filename or a directory to filter out                            */
    gchar *r_hostname = NULL;      /** r_hostname is the name fo the host where the file to be restored belung.          */
    gchar *restore = NULL;         /** Must contain a filename or a directory name to be restored                        */
    gchar *date = NULL;            /** date at which we want to restore a file or directory                              */
    gchar *where = NULL;           /** Contains the directory where to restore a file / directory                        */
    gchar *afterdate = NULL;       /** afterdate: we want to restore a file that has its mtime after this date           */
    gchar *beforedate = NULL;      /** beforedate:  we want to restore a file that has its mtime before this date        */
    gboolean all_versions = FALSE; /** all_version: True if we want to restore all version FALSE otherwise (default)     */
    gboolean all_files = FALSE;    /** all_files: True if we want to restore all files found by REGEX (-r or -l options) */
    gboolean latest = FALSE;       /** latest: True if we only want to get the latest version of a file                  */
    gboolean parents = FALSE;      /** parents: True if restore has to create / restore files with the whole path        */
    GOptionEntry entries[] =
    {
        { "version", 'v', 0, G_OPTION_ARG_NONE, &version, N_("Prints program version."), NULL},
        { "debug", 'd', 0,  G_OPTION_ARG_INT, &debug, N_("Activates (1) or deactivates (0) debug mode."), N_("BOOLEAN")},
        { "configuration", 'c', 0, G_OPTION_ARG_STRING, &configfile, N_("Specify an alternative configuration file."), N_("FILENAME")},
        { "list", 'l', 0, G_OPTION_ARG_FILENAME, &list, N_("Lists saved files that correspond to the given REGEX."), "REGEX"},
        { "restore", 'r', 0, G_OPTION_ARG_FILENAME, &restore, N_("Restores requested filename (REGEX) (by default latest version)."), "REGEX"},
        { "hostname", 'n', 0, G_OPTION_ARG_STRING, &r_hostname, N_("Specifies a hostname (HOSTNAME) that owned the file to be restored."), "HOSTNAME"},
        { "date", 't', 0, G_OPTION_ARG_STRING, &date, N_("Selects file with that specific DATE (YYYY-MM-DD HH:MM:SS format)."), "DATE"},
        { "after", 'a', 0, G_OPTION_ARG_STRING, &afterdate, N_("Selects file with mtime after DATE (YYYY-MM-DD HH:MM:SS format)."), "DATE"},
        { "before", 'b', 0, G_OPTION_ARG_STRING, &beforedate, N_("Selects file with mtime before DATE (YYYY-MM-DD HH:MM:SS format)."), "DATE"},
        { "all-versions", 'e', 0, G_OPTION_ARG_NONE, &all_versions, N_("Selects all versions of a file."), NULL},
        { "all-files", 'f', 0, G_OPTION_ARG_NONE, &all_files, N_("Forces -r to restore all files found (not the latest one)"), NULL},
        { "latest", 'g', 0, G_OPTION_ARG_NONE, &latest, N_("Selects only latest version of each file."), NULL},
        { "parents", 'P', 0, G_OPTION_ARG_NONE, &parents, N_("Creates directories if needed: ie restore with the whole path"), NULL},
        { "where", 'w', 0, G_OPTION_ARG_STRING, &where, N_("Specify a DIRECTORY where to restore a file."), N_("DIRECTORY")},
        { "ip", 'i', 0, G_OPTION_ARG_STRING, &ip, N_("IP address where server program is."), "IP"},
        { "port", 'p', 0, G_OPTION_ARG_INT, &port, N_("Port NUMBER on which server program is listening."), N_("NUMBER")},
        { NULL }
    };

    set_debug_mode(ENABLE_DEBUG);

    summary = g_strdup(_("This program is restoring files from cdpfglserver's server."));
    parse_command_line(argc, argv, entries, summary);

    /* 0) Setting some default values */

    opt = (options_t *) g_malloc0(sizeof(options_t));

    opt->configfile = NULL;
    opt->list = NULL;
    opt->restore = NULL;
    opt->ip = g_strdup("localhost");
    opt->port = SERVER_PORT;
    opt->where = NULL;
    opt->r_hostname = NULL;



    /* 1) Reading options from default configuration file
     *    note: restore option will never be read into the configuration
     *          file.
     */
    defaultconfigfilename = get_probable_etc_path(PROGRAM_NAME, "restore.conf");
    read_from_configuration_file(opt,  defaultconfigfilename);
    free_variable(defaultconfigfilename);


    /* 2) Reading the configuration from the configuration file specified
     *    on the command line (if any).
     *    note: same note than 1) applies here too.
     */
    if (configfile != NULL)
        {
            read_from_configuration_file(opt, configfile);
        }


    /* 3) retrieving other options from the command line.
     */
    set_debug_mode_upon_cmdl(debug);
    opt->version = version;           /* only TRUE if -v or --version was invoked      */
    opt->all_versions = all_versions; /* only TRUE if -e or --all-versions was invoked */
    opt->all_files = all_files;       /* only TRUE if -f or --all-files was invoked    */
    opt->latest = latest;             /* only TRUE if -r or --latest was invoked       */
    opt->parents = parents;           /* only TRUE if -p or --parents was invoked      */

    opt->date = set_option_str(date, opt->date);
    opt->afterdate = set_option_str(afterdate, opt->afterdate);
    opt->beforedate = set_option_str(beforedate, opt->beforedate);
    opt->list = set_option_str(list, opt->list);
    opt->restore = set_option_str(restore, opt->restore);
    opt->where = set_option_str(where, opt->where);
    opt->ip = set_option_str(ip, opt->ip);
    opt->r_hostname = set_option_str(r_hostname, opt->r_hostname);

    if (port > 1024 && port < 65535)
        {
            opt->port = port;
        }

    free_variable(summary);
    free_variable(r_hostname);
    free_variable(ip);
    free_variable(list);
    free_variable(restore);
    free_variable(date);
    free_variable(afterdate);
    free_variable(beforedate);
    free_variable(where);

    return opt;
}


/**
 * Decides what to do upon command lines options passed to the program
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon chosen command
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
                    print_program_version(PROGRAM_NAME, RESTORE_DATE, RESTORE_VERSION, RESTORE_AUTHORS, RESTORE_LICENSE);
                    print_libraries_versions(PROGRAM_NAME);
                    print_selected_options(opt);
                    exit(EXIT_SUCCESS);
                }
        }

    return opt;
}


/**
 * Frees the option structure
 * @param opt is the structure to be freed
 */
void free_options_t(options_t *opt)
{
    if (opt != NULL)
        {
            /* list, restore, date, ip, configfile, afterdate, beforedate and where are 'gchar *' strings */
            free_variable(opt->list);
            free_variable(opt->restore);
            free_variable(opt->date);
            free_variable(opt->configfile);
            free_variable(opt->ip);
            free_variable(opt->afterdate);
            free_variable(opt->beforedate);
            free_variable(opt->where);
            free_variable(opt->r_hostname);
            free_variable(opt);
        }
}

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    monitor.c
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
 * @file monitor.c
 *
 * This file is the main file for the monitor program. This monitor
 * program has to monitor file changes onto filesystems. It should notice
 * when a file is created, deleted or changed
 */

#include "monitor.h"

static void print_libraries_versions(void);


/**
 * Prints version of the libraries we are using.
 */
static void print_libraries_versions(void)
{
    fprintf(stdout, "%s was compiled with the following libraries :\n", PACKAGE_NAME);
    fprintf(stdout, "\t. GLIB version : %d.%d.%d\n", glib_major_version, glib_minor_version, glib_micro_version);

}


/**
 * Prints the version of the program.
 */
static void print_program_version(void)
{

    fprintf(stdout, "%s version : %s (%s)\n", PACKAGE_NAME, PACKAGE_VERSION, MONITOR_DATE);
    fprintf(stdout, "Author(s) : %s\n", MONITOR_AUTHORS);
    fprintf(stdout, "License : %s\n", MONITOR_LICENSE);
    fprintf(stdout, "\n");

}


/**
 * Decides what to do upon command lines options passed to the program
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option (in manage_command_line_options function).
 */
static options_t *do_what_is_needed_from_command_line_options(int argc, char **argv)
{
    options_t *opt = NULL;  /** Structure to manage options from the command line can be freed when no longer needed */

    opt = manage_command_line_options(argc, argv);

    if (opt != NULL && opt->version == TRUE)
        {
            print_program_version();
            print_libraries_versions();
        }

    return opt;
}


/**
 * Gets the filename of a  GFile
 * @param a_file : the GFile to get the filename from.
 * @returns the name of the GFile if any or "--" gchar * string that may be
 *          freed when no longer needed
 */
gchar *get_filename_from_gfile(GFile *a_file)
{
    gchar *filename = NULL;

    if (a_file != NULL)
        {
            filename = g_file_get_basename(a_file);
        }
    else
        {
            filename = g_strdup("--");
        }

    return filename;
}


static void monitor_changed(GFileMonitor *monitor, GFile *first_file, GFile *second_file, GFileMonitorEvent event, gpointer user_data)
{
    gchar *message = NULL;
    gchar *first_filename = NULL;
    gchar *second_filename = NULL;

    switch (event)
        {
            case G_FILE_MONITOR_EVENT_CHANGED:
                message = g_strdup("changed");
            break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
                message = g_strdup("changes done hint");
            break;
            case G_FILE_MONITOR_EVENT_DELETED:
                message = g_strdup("deleted");
            break;
            case G_FILE_MONITOR_EVENT_CREATED:
                message = g_strdup("created");
            break;
            case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
                message = g_strdup("attribute changed");
            break;
            case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
                message = g_strdup("pre unmount");
            break;
            case G_FILE_MONITOR_EVENT_UNMOUNTED:
                message = g_strdup("unmounted");
            break;
            case G_FILE_MONITOR_EVENT_MOVED:
                message = g_strdup("moved");
            break;
            default:
                message = g_strdup("unknown event");
            break;
        }

    first_filename = get_filename_from_gfile(first_file);
    second_filename = get_filename_from_gfile(second_file);

    fprintf(stdout, "Event : %s ; first file is %s, second file is %s\n", message, first_filename, second_filename);

    g_free(message);
    g_free(first_filename);
    g_free(second_filename);
}


/**
 * Main function
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns always 0
 */
int main(int argc, char **argv)
{
    options_t *opt = NULL;  /** Structure to manage options from the command line can be freed when no longer needed */
    GFileMonitor *monitor = NULL;
    GError *error = NULL;
    GFile *a_file = NULL;
    GMainLoop *mainloop = NULL;

    g_type_init();

    opt = do_what_is_needed_from_command_line_options(argc, argv);

    a_file = g_file_new_for_path("/home/dup/Dossiers_Perso/projets/sauvegarde/monitor");

    monitor = g_file_monitor(a_file, G_FILE_MONITOR_SEND_MOVED, NULL, &error);
    /* g_file_monitor_set_rate_limit(monitor, 5000); */

    g_signal_connect(monitor, "changed", G_CALLBACK(monitor_changed), NULL);

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    /* when leaving, we have to free memory */
    free_options_t_structure(opt);
    g_object_unref(monitor);

    return 0;
}

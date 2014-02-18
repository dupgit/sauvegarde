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
static void print_program_version(void);
static options_t *do_what_is_needed_from_command_line_options(int argc, char **argv);
static gchar *get_filename_from_gfile(GFile *a_file);
static void monitor_changed(GFileMonitor *monitor, GFile *first_file, GFile *second_file, GFileMonitorEvent event, gpointer user_data);
static GFileMonitor *add_a_path_to_monitor(main_struct_t *main_struct, path_t *a_path);
static path_t *new_path_t(main_struct_t *main_struct, gchar *path, gint64 rate);
static void free_path_t(path_t *a_path);
static main_struct_t *init_main_structure(options_t *opt);


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
static gchar *get_filename_from_gfile(GFile *a_file)
{
    gchar *filename = NULL;

    if (a_file != NULL)
        {
            filename = g_file_get_parse_name(a_file);
        }
    else
        {
            filename = g_strdup("--");
        }

    return filename;
}


/**
 * A new file has been created and we create a new monitor and insert the
 * file filename in the list only if this file is a directory.
 * @param main_struct is the main structure of the program
 * @param a_file is the GFile * file that was created
 */
static void file_created(main_struct_t *main_struct, GFile *a_file)
{
    path_t *a_path = NULL;
    gint filetype = 0;

    if (a_file != NULL)
        {

            filetype = g_file_query_file_type(a_file, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);

            if (filetype == G_FILE_TYPE_DIRECTORY)
                {
                    a_path = new_path_t(main_struct, get_filename_from_gfile(a_file), 60);
                    main_struct->path_list = g_slist_prepend(main_struct->path_list, a_path);
                }

        }
}


/**
 *
 *
 */
static void file_deleted(main_struct_t *main_struct, GFile *a_file)
{
    path_t *a_path = NULL;
    gchar *filename = NULL;
    gboolean done = FALSE;
    GSList *head = NULL;    /** used to iter over the path list */

    if (a_file != NULL)
        {

            filename = get_filename_from_gfile(a_file);

            head = main_struct->path_list;

            while (head != NULL && done == FALSE)
                {

                    a_path = (path_t *) head->data;

                    if (g_strcmp0(filename, a_path->path) == 0) /* both strings are equal */
                        {
                            main_struct->path_list = g_slist_remove_link(main_struct->path_list, head);
                            free_path_t(a_path);
                        }
                    else
                        {
                            head = g_slist_next(head);
                        }
                }

            g_free(filename);

        }
}


/**
 * Callback function called upon change on a monitored file
 * @param monitor : the monitor which detected a change in the monitored
 *                  files
 * @param first_file : is the file which has changed (original one)
 * @param second_file : is the named of the file if it has been moved (renamed)
 * @param event : the event that called this callback function
 * @param user_data:  options_t * structure that contains all the options for the monitor program
 */
static void monitor_changed(GFileMonitor *monitor, GFile *first_file, GFile *second_file, GFileMonitorEvent event, gpointer user_data)
{

    main_struct_t *main_struct = (main_struct_t *) user_data;

    gchar *message = NULL;
    gchar *first_filename = NULL;
    gchar *second_filename = NULL;

    options_t *opt = NULL;


    if (main_struct != NULL)
        {

            /* We are suposed to be sure that main_struct exists ! */
            opt = main_struct->opt;

            switch (event)
                {
                    case G_FILE_MONITOR_EVENT_CHANGED:
                        message = g_strdup("changed          ");
                    break;

                    case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
                        message = g_strdup("changes          ");
                    break;

                    case G_FILE_MONITOR_EVENT_DELETED:

                        message = g_strdup("deleted          ");
                        file_deleted(main_struct, first_file);

                    break;

                    case G_FILE_MONITOR_EVENT_CREATED:

                        message = g_strdup("created          ");
                        file_created(main_struct, first_file);

                    break;

                    case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
                        message = g_strdup("attribute changed");
                    break;

                    case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
                        message = g_strdup("pre unmount      ");
                    break;

                    case G_FILE_MONITOR_EVENT_UNMOUNTED:
                        message = g_strdup("unmounted        ");
                    break;

                    case G_FILE_MONITOR_EVENT_MOVED:
                        message = g_strdup("moved            ");
                    break;

                    default:
                        message = g_strdup("unknown event    ");
                    break;
                }

            first_filename = get_filename_from_gfile(first_file);
            second_filename = get_filename_from_gfile(second_file);

            fprintf(stdout, "%s ; Event : %s ; first file is %s, second file is %s\n", main_struct->hostname, message, first_filename, second_filename);

            g_free(message);
            g_free(first_filename);
            g_free(second_filename);

        }
}


/**
 * Creates a new monitor with the given path.
 * @param main_struct is the main structure for the program and we need to
 *        pass it to the callback function.
 * @param a_path : path_t structure containing everything we need to
 *        monitor a path.
 * @returns a newly allocated monitor. Free the returned object with
 *          g_object_unref().
 */
static GFileMonitor *add_a_path_to_monitor(main_struct_t *main_struct, path_t *a_path)
{
    GFileMonitor *monitor = NULL;
    GError *error = NULL;
    GFile *a_file = NULL;

    a_file = g_file_new_for_path(a_path->path);

    monitor = g_file_monitor(a_file, G_FILE_MONITOR_SEND_MOVED, NULL, &error);
    g_file_monitor_set_rate_limit(monitor, (1000 * a_path->rate)); /* The value in this function is expressed in milliseconds */

    g_signal_connect(monitor, "changed", G_CALLBACK(monitor_changed), main_struct);

    return monitor;

}


/**
 * Allocate a new structure path_t containing a path to monitor and a rate
 * limit for notifications.
 * @param main_struct : the main structure for the program
 * @param path : the path to be monitored
 * @param rate : the rate in seconds under which a new notification will not
 *        occur.
 * @returns a newly allocated path_t structure that may be freed when no
 *          longer needed (do not forget to free 'path' in it).
 */
static path_t *new_path_t(main_struct_t *main_struct, gchar *path, gint64 rate)
{
    path_t *a_path = NULL;

    a_path = (path_t *) g_malloc0(sizeof(path_t));

    a_path->path = g_strdup(path);
    a_path->rate = rate;
    a_path->monitor = add_a_path_to_monitor(main_struct, a_path);

    return a_path;
}

/**
 * Free the memory for path_t * structure
 * @param a path_t * pointer to be freed from memory
 */
static void free_path_t(path_t *a_path)
{
    if (a_path != NULL)
        {
            g_free(a_path->path);
            g_object_unref(a_path->monitor);
            g_free(a_path);
        }
}


/**
 * Inits the main structure
 * @returns a main_struct_t * pointer to the main structure
 */
static main_struct_t *init_main_structure(options_t *opt)
{
    main_struct_t *main_struct = NULL;


    main_struct = (main_struct_t *) g_malloc0(sizeof(main_struct_t));

    main_struct->opt = opt;
    main_struct->path_list = NULL;
    main_struct->hostname = g_get_host_name();

    return main_struct;

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
    path_t *a_path = NULL;
    main_struct_t *main_struct = NULL;
    GMainLoop *mainloop = NULL;



    g_type_init();



    opt = do_what_is_needed_from_command_line_options(argc, argv);
    main_struct = init_main_structure(opt);

    a_path = new_path_t(main_struct, "/home/dup/Dossiers_Perso/projets/sauvegarde/monitor", 60);
    main_struct->path_list = g_slist_prepend(main_struct->path_list, a_path);



    /* infinite loop */
    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    /* when leaving, we have to free memory... but this is not going to happen here ! */
    /* free_options_t_structure(main_struct->opt); */

    return 0;
}

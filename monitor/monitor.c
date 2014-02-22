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


static gchar *get_filename_from_gfile(GFile *a_file);
static void monitor_changed(GFileMonitor *monitor, GFile *first_file, GFile *second_file, GFileMonitorEvent event, gpointer user_data);
static int add_a_path_to_monitor(main_struct_t *main_struct, path_t *a_path);
static void add_path_to_tree(main_struct_t *main_struct, path_t *a_path);
static main_struct_t *init_main_structure(options_t *opt);



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
            filename = NULL;
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
                    a_path = new_path_t(get_filename_from_gfile(a_file), 60);
                    a_path->wd = add_a_path_to_monitor(main_struct, a_path);
                    add_path_to_tree(main_struct, a_path);
                }

        }
}


/**
 * Function called when a deleted file was monitored.
 * We get the GFile back but we cannot do any test on the type of the
 * deleted file (as this file IS already deleted!). So we are unable to
 * know if it was a directory or not. We need to lookup into the tree to
 * know if this file was monitored or not (is so we have to delete the
 * monitor).
 * @param main_struct is the main structure
 * @param a_file is the GFile structure of the monitored file that was found
 *        deleted !
 */
static void file_deleted(main_struct_t *main_struct, GFile *a_file)
{
    path_t *cmp_path = NULL;
    path_t *a_path = NULL;
    gchar *filename = NULL;

    if (a_file != NULL)
        {

            filename = get_filename_from_gfile(a_file);

            cmp_path = new_path_t(filename, 60);

            a_path = g_tree_lookup(main_struct->path_tree, cmp_path);

            if (a_path != NULL)
                {
                    if (g_tree_remove(main_struct->path_tree, a_path) == TRUE) /* a_path was removed */
                        {
                            free_path_t(a_path);
                        }
                }

            g_free(filename);
            free_path_t(cmp_path);

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
    options_t *opt = NULL;

    gchar *message = NULL;
    gchar *first_filename = NULL;
    gchar *second_filename = NULL;
    gchar *file_type = NULL;
    GFileInfo *fileinfo = NULL;
    guint64 size = 0;
    GError *error = NULL;


    if (main_struct != NULL)
        {

            /* We are suposed to be sure that main_struct exists ! */
            opt = main_struct->opt;

            switch (event)
                {
                    case G_FILE_MONITOR_EVENT_CHANGED:
                        message = g_strdup("event=changed");
                    break;

                    case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
                        message = g_strdup("event=changes");
                    break;

                    case G_FILE_MONITOR_EVENT_DELETED:
                        message = g_strdup("event=deleted");
                        file_deleted(main_struct, first_file);
                    break;

                    case G_FILE_MONITOR_EVENT_CREATED:
                        message = g_strdup("event=created");
                        file_created(main_struct, first_file);
                    break;

                    case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
                        message = g_strdup("event=attribute changed");
                    break;

                    case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
                        message = g_strdup("event=pre unmount");
                    break;

                    case G_FILE_MONITOR_EVENT_UNMOUNTED:
                        message = g_strdup("event=unmounted");
                    break;

                    case G_FILE_MONITOR_EVENT_MOVED:
                        message = g_strdup("event=moved");
                    break;

                    default:
                        message = g_strdup("event=unknown");
                    break;
                }

                if (first_file != NULL)
                    {
                        fileinfo = g_file_query_info(first_file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

                        if (error == NULL)
                            {
                                size = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_STANDARD_SIZE);

                                switch (g_file_info_get_file_type(fileinfo))
                                    {
                                        case G_FILE_TYPE_UNKNOWN:
                                            file_type = g_strdup("unknown");
                                        break;

                                        case G_FILE_TYPE_REGULAR:
                                            file_type = g_strdup("file");
                                        break;

                                        case G_FILE_TYPE_DIRECTORY:
                                            file_type = g_strdup("directory");
                                        break;

                                        case G_FILE_TYPE_SYMBOLIC_LINK:
                                            file_type = g_strdup("link");
                                        break;

                                        case G_FILE_TYPE_SPECIAL:        /* socket, fifo, blockdev, chardev */
                                            file_type = g_strdup("special");
                                        break;

                                        case G_FILE_TYPE_SHORTCUT:
                                            file_type = g_strdup("shortcut");
                                        break;

                                        default:
                                            file_type = g_strdup("other");
                                        break;
                                    }
                            }
                        else
                            {
                                file_type = g_strdup("?");
                            }
                    }


            first_filename = get_filename_from_gfile(first_file);
            second_filename = get_filename_from_gfile(second_file);


            if (first_filename != NULL && second_filename == NULL)
                {
                    fprintf(stdout, "host=%s ; %s ; type=%s ; size=%llu ; file1=%s\n", main_struct->hostname, message, file_type, (long long unsigned int) size, first_filename);
                }
            else if (first_filename != NULL && second_filename != NULL)
                {
                    fprintf(stdout, "host=%s ; %s ; type=%s ; file1=%s ; file2=%s\n", main_struct->hostname, message, file_type, first_filename, second_filename);
                }
            else if (first_filename == NULL && second_filename != NULL)
                {
                    fprintf(stdout, "host=%s ; %s ; file2=%s\n", main_struct->hostname, message, second_filename);
                }
            else  /* both filenames are NULL */
                {
                    fprintf(stdout, "host=%s ; %s\n", main_struct->hostname, message);
                }

            g_free(message);
            g_free(first_filename);    /* g_free() handles NULL pointers */
            g_free(second_filename);
            g_free(file_type);

            if (fileinfo != NULL)
                {
                    g_object_unref(fileinfo);
                }

        }
}


/**
 * Creates a new monitor with the given path.
 * @param main_struct is the main structure for the program and we need to
 *        pass it to the callback function.
 * @param a_path : path_t structure containing everything we need to
 *        monitor a path.
 * @returns
 */
static int add_a_path_to_monitor(main_struct_t *main_struct, path_t *a_path)
{
    int result = 0;


    if (main_struct != NULL && a_path != NULL && a_path->path != NULL)
        {
            fprintf(stdout, "-> %s\n", a_path->path);

            result = inotify_add_watch(main_struct->fd, a_path->path, IN_ATTRIB | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_DONT_FOLLOW);

            if (result == -1)
                {
                    perror("Error while adding a new watch descriptor");
                }

            return result;
        }
    else
        {
            return -1;
        }
}


/**
 * Adds a path to the GTree structure
 * @param main_struct : the main structure (path_tree field contains the
 *        GTree * balanced binary tree).
 * @param path : a path_t * to be inserted in the balanced binary tree.
 */
void add_path_to_tree(main_struct_t *main_struct, path_t *a_path)
{
    if (a_path != NULL)
        {

            g_tree_insert(main_struct->path_tree, a_path, NULL);

        }
}


/**
 * Traverse all sub-directories of a directory and adds each directory into
 * path_t * structure in main_struct->path_tree balanced binary tree.
 * @param main_struct : main structure with everything needed
 * @param directory : the directory that we want to traverse
 */
static void traverse_directory(main_struct_t *main_struct, gchar *directory)
{
    GFile *a_dir = NULL;
    GFileEnumerator *file_enum = NULL;
    GError *error = NULL;
    GFileInfo *fileinfo = NULL;
    gchar *dirname = NULL;
    path_t *a_path = NULL;


    /* Adding "directory " path to be monitored */
    a_path = new_path_t(directory, MONITOR_TIME);
    a_path->wd = add_a_path_to_monitor(main_struct, a_path);
    add_path_to_tree(main_struct, a_path);

    a_dir = g_file_new_for_path(directory);

    file_enum = g_file_enumerate_children(a_dir, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

    if (error == NULL)
        {
            fileinfo = g_file_enumerator_next_file(file_enum, NULL, &error);

            while (error == NULL && fileinfo != NULL)
                {
                    if (g_file_info_get_file_type(fileinfo) == G_FILE_TYPE_DIRECTORY)
                        {
                            dirname = g_build_path(G_DIR_SEPARATOR_S, directory, g_file_info_get_name(fileinfo), NULL);

                            traverse_directory(main_struct, dirname);

                            g_free(dirname);
                        }

                    fileinfo = g_file_enumerator_next_file(file_enum, NULL, &error);
                }

            g_file_enumerator_close(file_enum, NULL, NULL);
        }
    else
    {
        fprintf(stderr, "Unable to enumerate directory %s : %s\n", directory, error->message);
        g_error_free(error);
    }
}


/**
 * This function is a wrapper to directory_traverse function to allow
 * the directory traversal to be threaded
 */
static gpointer threaded_directory_traversal(gpointer data)
{
    thread_data_t *a_thread_data = (thread_data_t *) data;
    main_struct_t *main_struct = a_thread_data->main_struct;
    GSList *dir_list = a_thread_data->dir_list;

    while (dir_list != NULL)
        {
            traverse_directory(main_struct, dir_list->data);

            dir_list = g_slist_next(dir_list);
        }

    fflush(NULL);

    return NULL;
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
    main_struct->path_tree = g_tree_new(compare_path);
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
    main_struct_t *main_struct = NULL;
    GMainLoop *mainloop = NULL;
    thread_data_t *a_thread_data = NULL;
    GThread *a_thread = NULL;
    long max_files = 0;
    int fd = 0;
    int result = 0;
    char buffer[16384];
    struct inotify_event *event = NULL;
    gchar *filename = NULL;

    g_type_init();

    max_files = sysconf(_SC_OPEN_MAX);
    fprintf(stdout, "Maximum number of opened files : %ld\n", max_files);

    opt = do_what_is_needed_from_command_line_options(argc, argv);
    main_struct = init_main_structure(opt);

     main_struct->fd = inotify_init();

    /* Adding paths to be monitored in a threaded way */
    a_thread_data = (thread_data_t *) g_malloc0(sizeof(thread_data_t));

    a_thread_data->main_struct = main_struct;
    a_thread_data->dir_list = g_slist_prepend(a_thread_data->dir_list, "/home/dup/Dossiers_Perso/projets");

    a_thread = g_thread_create(threaded_directory_traversal, a_thread_data, FALSE, NULL);


    while(1)
        {
            read(main_struct->fd, buffer, sizeof(buffer));
            event = (struct inotify_event *) buffer;

            filename = g_strndup(event->name, event->len);

            printf("Fichier surveillé : %s\n", filename);

            g_free(filename);
        }


    /* infinite loop */
    /* mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop); */

    /* when leaving, we have to free memory... but this is not going to happen here ! */
    /* free_options_t_structure(main_struct->opt); */

    return 0;
}

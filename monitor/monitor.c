/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    monitor.c
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
 * @file monitor.c
 *
 * This file is the main file for the monitor program. This monitor
 * program has to monitor file changes onto filesystems. It should notice
 * when a file is created, deleted or changed
 */

#include "monitor.h"


static void traverse_directory(main_struct_t *main_struct, gchar *directory);
static gpointer first_directory_traversal(gpointer data);
static main_struct_t *init_main_structure(options_t *opt);
static gchar *make_connexion_string(gchar *ip, gint port);


/**
 * Traverse all sub-directories of a directory and adds each directory into
 * path_t * structure in main_struct->path_tree balanced binary tree.
 * @param main_struct : main structure with everything needed
 * @param directory : the directory that we want to traverse
 * @todo do something with symbolic links that must be saved as is.
 */
static void traverse_directory(main_struct_t *main_struct, gchar *directory)
{
    GFile *a_dir = NULL;
    GFileEnumerator *file_enum = NULL;
    GError *error = NULL;
    GFileInfo *fileinfo = NULL;
    gchar *dirname = NULL;
    gchar *filename = NULL;
    GFileType filetype = G_FILE_TYPE_UNKNOWN;

    a_dir = g_file_new_for_path(directory);

    file_enum = g_file_enumerate_children(a_dir, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

    if (error == NULL)
        {
            fileinfo = g_file_enumerator_next_file(file_enum, NULL, &error);

            while (error == NULL && fileinfo != NULL)
                {
                    filetype = g_file_info_get_file_type(fileinfo);

                    if ( filetype == G_FILE_TYPE_DIRECTORY)
                        {
                            /* We've got a directory : we must transmit to ciseaux and dig into it ! */
                            dirname = g_build_path(G_DIR_SEPARATOR_S, directory, g_file_info_get_name(fileinfo), NULL);

                            if (dirname != NULL)
                                {
                                    g_async_queue_push(main_struct->queue, g_strdup(dirname));
                                    traverse_directory(main_struct, dirname);
                                    dirname = free_variable(dirname);
                                }
                        }
                    else if (filetype == G_FILE_TYPE_REGULAR || filetype == G_FILE_TYPE_SYMBOLIC_LINK)
                        {
                            /* We've got a regular file or a symlink : we have to transmit it to ciseaux */

                            filename = g_build_path(G_DIR_SEPARATOR_S, directory, g_file_info_get_name(fileinfo), NULL);

                            if (filename != NULL)
                                {
                                    g_async_queue_push(main_struct->queue, g_strdup(filename));
                                    print_debug(_("%s passed to 'cut' thread\n"), filename);
                                    filename = free_variable(filename);
                                }
                        }

                    fileinfo = free_object(fileinfo);

                    /* wait_for_queue_to_flush(main_struct->queue, 16, 100); */

                    fileinfo = g_file_enumerator_next_file(file_enum, NULL, &error);
                }

            fileinfo = free_object(fileinfo);
            g_file_enumerator_close(file_enum, NULL, NULL);
            file_enum = free_object(file_enum);
        }
    else
        {
            print_error(__FILE__, __LINE__, _("Unable to enumerate directory %s: %s\n"), directory, error->message);
            error = free_error(error);
        }

    free_object(a_dir);

}


/**
 * This function is a wrapper to directory_traverse function to allow
 * the directory traversal to be threaded. We need to traverse at least
 * once to be sure that every file has been saved at least once.
 * @param data : thread_data_t * structure.
 */
static gpointer first_directory_traversal(gpointer data)
{
    thread_data_t *a_thread_data = (thread_data_t *) data;
    main_struct_t *main_struct = NULL;
    GSList *head = NULL;

    if (a_thread_data != NULL)
        {
            main_struct = a_thread_data->main_struct;

            if (main_struct != NULL)
                {
                    head = a_thread_data->dir_list;

                    while (head != NULL)
                        {
                            traverse_directory(main_struct, head->data);
                            head = g_slist_next(head);
                        }
                }
        }

    return NULL;
}


/**
 * Makes the connexion string that is used by ZMQ to create a new socket
 * and verifies that port number is between 1025 and 65534 included.
 * @param ip : a gchar * that contains either an ip address or a hostname
 * @param port : a gint that is comprised between 1025 and 65534 included
 * @returns a newly allocated string that may be freed with free_variable()
 *          function.
 */
static gchar *make_connexion_string(gchar *ip, gint port)
{
    /**
     * @todo check the ip string to be sure that it correspond to something that
     *       we can join (IP or hostname).
     */
    gchar *conn = NULL;

    if (ip != NULL && port > 1024 && port < 65535)
        {
            /* We must ensure that ip is correct before doing this ! */
            conn = g_strdup_printf("http://%s:%d", ip, port);
        }

    return conn;
}


/**
 * Inits the main structure.
 * @note With sqlite version > 3.7.7 we should use URI filename.
 * @param opt : a filled options_t * structure that contains all options
 *        by default, read into the file or selected in the command line.
 * @returns a main_struct_t * pointer to the main structure
 */
static main_struct_t *init_main_structure(options_t *opt)
{
    main_struct_t *main_struct = NULL;
    gchar *db_uri = NULL;
    gchar *conn = NULL;

    if (opt != NULL)
        {

            print_debug(_("Please wait while initializing main structure...\n"));

            main_struct = (main_struct_t *) g_malloc0(sizeof(main_struct_t));

            create_directory(opt->dircache);
            db_uri = g_build_filename(opt->dircache, opt->dbname , NULL);
            main_struct->database = open_database(db_uri);

            main_struct->opt = opt;
            main_struct->hostname = g_get_host_name();
            main_struct->queue = g_async_queue_new();
            main_struct->store_queue = g_async_queue_new();

            main_struct->hashs = get_all_inserted_hashs(main_struct->database);


            if (opt != NULL && opt->ip != NULL)
                {
                    conn = make_connexion_string(opt->ip, opt->port);
                    main_struct->comm = init_comm_struct(conn);
                }
            else
                {
                    /* This should never happen because we have default values */
                    main_struct->comm = NULL;
                }

            main_struct->signal_fd = start_signals();
            main_struct->fanotify_fd = start_fanotify(opt);

            print_debug(_("Main structure initialized !\n"));

        }

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
    thread_data_t *a_thread_data = NULL;
    GThread *cut_thread = NULL;
    GThread *store_thread = NULL;
    GThread *dir_thread = NULL;

    #if !GLIB_CHECK_VERSION(2, 36, 0)
        g_type_init();  /** g_type_init() is deprecated since glib 2.36 */
    #endif

    init_international_languages();
    curl_global_init(CURL_GLOBAL_ALL);

    opt = do_what_is_needed_from_command_line_options(argc, argv);

    if (opt != NULL)
        {

            main_struct = init_main_structure(opt);

            /* Adding paths to be monitored in a threaded way */
            a_thread_data = (thread_data_t *) g_malloc0(sizeof(thread_data_t));

            a_thread_data->main_struct = main_struct;
            a_thread_data->dir_list = opt->dirname_list;

            store_thread = g_thread_new("store", store_buffer_data, main_struct);
            cut_thread   = g_thread_new("cut", ciseaux, main_struct);
            dir_thread   = g_thread_new("dir_traversal", first_directory_traversal, a_thread_data);


            /** Launching an infinite loop to get modifications done on
             * the filesystem (on directories we watch).
             * @note fanotify's kernel interface does not provide the events
             * needed to know if a file has been deleted or it's attributes
             * changed. Disabling this for now.
             * fanotify_loop(main_struct);
             */

            /* There is no need to send the $END$ command as we use
             * cut and store thread in the loop above.
             */
             /* Waiting for the directory traversal to finish. */

             g_thread_join(dir_thread);

             g_async_queue_push(main_struct->queue, g_strdup("$END$"));

             g_thread_join(cut_thread);

             g_async_queue_push(main_struct->store_queue, encapsulate_end());
             g_thread_join(store_thread);

             print_tree_hashs_stats(main_struct->hashs);

             free_options_t_structure(main_struct->opt);
        }

    return 0;
}

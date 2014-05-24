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


static void traverse_directory(main_struct_t *main_struct, gchar *directory);
static gpointer first_directory_traversal(gpointer data);
static main_struct_t *init_main_structure(options_t *opt);
static void print_tree_hashs_stats(hashs_t *hashs);


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
                            /* We've got a directory : we must go into it ! */
                            dirname = g_build_path(G_DIR_SEPARATOR_S, directory, g_file_info_get_name(fileinfo), NULL);
                            if (dirname != NULL)
                                {
                                    g_async_queue_push(main_struct->queue, g_strdup(dirname));
                                    traverse_directory(main_struct, dirname);
                                    dirname = free_variable(dirname);
                                }
                        }
                    else if (filetype == G_FILE_TYPE_REGULAR)
                        {
                            /* We've got a regular file : we have to transmit it to ciseaux */

                            filename = g_build_path(G_DIR_SEPARATOR_S, directory, g_file_info_get_name(fileinfo), NULL);

                            if (filename != NULL)
                                {
                                    g_async_queue_push(main_struct->queue, g_strdup(filename));
                                }

                            print_debug(stdout, _("%s passed to checksum's thread\n"), filename);

                            filename = free_variable(filename);
                        }

                    fileinfo = free_object(fileinfo);

                    fileinfo = g_file_enumerator_next_file(file_enum, NULL, &error);
                }

            fileinfo = free_object(fileinfo);
            g_file_enumerator_close(file_enum, NULL, NULL);
            file_enum = free_object(file_enum);
        }
    else
        {
            fprintf(stderr, _("[%s, %d] Unable to enumerate directory %s: %s\n"), __FILE__, __LINE__, directory, error->message);
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
 * Inits the main structure.
 * @note : With sqlite version > 3.7.7 we should use URI filename.
 * @returns a main_struct_t * pointer to the main structure
 */
static main_struct_t *init_main_structure(options_t *opt)
{
    main_struct_t *main_struct = NULL;
    gchar *db_uri = NULL;

    print_debug(stdout, _("Please wait while initializing main structure...\n"));

    main_struct = (main_struct_t *) g_malloc0(sizeof(main_struct_t));

    db_uri = g_build_filename(opt->dircache, opt->dbname , NULL);
    main_struct->database = open_database(db_uri);

    main_struct->opt = opt;
    main_struct->hostname = g_get_host_name();
    main_struct->queue = g_async_queue_new();
    main_struct->print_queue = g_async_queue_new();
    main_struct->store_queue = g_async_queue_new();

    main_struct->hashs = get_all_inserted_hashs(main_struct->database);

    /* Testing things */
    if (opt != NULL && opt->ip != NULL)
        {
            /* We must ensure that opt->ip is correct before doing this ! */
            main_struct->comm = create_push_socket(g_strconcat("tcp://", opt->ip, NULL));
        }
    else
        {
            main_struct->comm = create_push_socket("tcp://localhost:5468");
        }

    print_debug(stdout, _("Main structure initialized !\n"));

    return main_struct;
}


/**
 * Prints statistics from the binary tree hash
 * @param hashs : the structure that contains all hashs and some values
 *        that may give some stats about the datas
 */
static void print_tree_hashs_stats(hashs_t *hashs)
{
    if (hashs != NULL)
        {
            /* printing some stats of the GTree */
            fprintf(stdout, _("Number of unique hash : %d\n"), g_tree_nnodes(hashs->tree_hash));
            fprintf(stdout, _("Tree height           : %d\n"), g_tree_height(hashs->tree_hash));
            fprintf(stdout, _("Total size in bytes   : %ld\n"), hashs->total_bytes);
            fprintf(stdout, _("Dedup size in bytes   : %ld\n"), hashs->total_bytes - hashs->in_bytes);

            if (hashs->total_bytes != 0)
                {
                    fprintf(stdout, _("Deduplication %%       : %.2f\n"), 100*(hashs->total_bytes - hashs->in_bytes)/ (float) hashs->total_bytes);
                }
            else
                {
                    fprintf(stdout, _("Deduplication in bytes : %.2ld\n"), hashs->in_bytes);
                }
        }
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
    GThread *a_thread = NULL;     /* thread used to do the directory traversal */
    GThread *cut_thread = NULL;
    GThread *store_thread = NULL;


    g_type_init();

    init_international_languages();

    opt = do_what_is_needed_from_command_line_options(argc, argv);

    if (opt != NULL)
        {
            main_struct = init_main_structure(opt);

            /* Adding paths to be monitored in a threaded way */
            a_thread_data = (thread_data_t *) g_malloc0(sizeof(thread_data_t));

            a_thread_data->main_struct = main_struct;
            a_thread_data->dir_list = opt->dirname_list;

            a_thread = g_thread_create(first_directory_traversal, a_thread_data, TRUE, NULL);
            cut_thread = g_thread_create(ciseaux, main_struct, TRUE, NULL);
            store_thread = g_thread_create(store_buffer_data, main_struct, TRUE, NULL);

            /* As we are only testing things for now, we just wait for the
             * threads to join and then exits.
             */
            g_thread_join(a_thread);

            g_async_queue_push(main_struct->queue, g_strdup("$END$"));

            g_thread_join(cut_thread);

            g_async_queue_push(main_struct->store_queue, g_strdup("$END$"));
            g_thread_join(store_thread);

            print_tree_hashs_stats(main_struct->hashs);

            /* when leaving, we have to free memory... but this is not going to happen here ! */
            /* free_options_t_structure(main_struct->opt); */
        }

    return 0;
}

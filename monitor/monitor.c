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

static main_struct_t *init_main_structure(options_t *opt);
static GSList *calculate_hash_data_list_for_file(GFile *a_file, gint64 blocksize);
static meta_data_t *get_meta_data_from_fileinfo(gchar *directory, GFileInfo *fileinfo, gint64 blocksize, db_t *database);
static gchar *send_meta_data_to_serveur(main_struct_t *main_struct, meta_data_t *meta);
static hash_data_t *find_hash_in_list(GSList *hash_data_list, guint8 *hash);
static gint send_data_to_serveur(main_struct_t *main_struct, meta_data_t *meta, gchar *answer);
static void iterate_over_enum(main_struct_t *main_struct, gchar *directory, GFileEnumerator *file_enum);
static void carve_one_directory(gpointer data, gpointer user_data);
static void carve_all_directories(main_struct_t *main_struct);


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
            db_uri = free_variable(db_uri);

            main_struct->opt = opt;
            main_struct->hostname = g_get_host_name();

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
 * Calculates hashs for each block of blocksize bytes long on the file
 * and returns a list of all hashs in correct order stored in a binary
 * form to save space.
 * @note This technique has some limits in term of memory footprint
 *       because one file is entirely in memory at a time. Saving huge
 *       files may not be possible with this, depending on the size of
 *       the file and the size of the memory.
 * @todo Imagine a new way to checksum huge files because of limitations.
 *       May be with the local sqlite database ?
 * @param a_file is the file from which we want the hashs.
 * @param blocksize is the blocksize to be used to calculate hashs upon.
 * @returns a GSList * list of hashs stored in a binary form.
 */
static GSList *calculate_hash_data_list_for_file(GFile *a_file, gint64 blocksize)
{
    GFileInputStream *stream = NULL;
    GError *error = NULL;
    GSList *hash_data_list = NULL;
    hash_data_t *hash_data = NULL;
    gssize read = 0;
    guchar *buffer = NULL;
    GChecksum *checksum = NULL;
    guint8 *a_hash = NULL;
    gsize digest_len = HASH_LEN;

    if (a_file != NULL)
        {
            stream = g_file_read(a_file, NULL, &error);

            if (stream != NULL && error == NULL)
                {

                    checksum = g_checksum_new(G_CHECKSUM_SHA256);
                    buffer = (guchar *) g_malloc0(blocksize);
                    a_hash = (guint8 *) g_malloc0(digest_len);

                    read = g_input_stream_read((GInputStream *) stream, buffer, blocksize, NULL, &error);

                    while (read != 0 && error == NULL)
                        {
                            g_checksum_update(checksum, buffer, read);
                            g_checksum_get_digest(checksum, a_hash, &digest_len);

                            /* Need to save data and read in hash_data_t structure */
                            hash_data = new_hash_data_t(buffer, read, a_hash);

                            hash_data_list = g_slist_prepend(hash_data_list, hash_data);
                            g_checksum_reset(checksum);
                            digest_len = HASH_LEN;

                            buffer = (guchar *) g_malloc0(blocksize);
                            a_hash = (guint8 *) g_malloc0(digest_len);
                            read = g_input_stream_read((GInputStream *) stream, buffer, blocksize, NULL, &error);
                        }

                    if (error != NULL)
                        {
                            print_error(__FILE__, __LINE__, _("Error while reading file: %s\n"), error->message);
                            error = free_error(error);
                            g_slist_free_full(hash_data_list, free_hdt_struct);
                            hash_data_list =  NULL;
                        }
                    else
                        {
                            /* get the list in correct order (because we prepended the hashs to get speed when inserting hashs in the list) */
                            hash_data_list = g_slist_reverse(hash_data_list);
                        }

                    g_checksum_free(checksum);
                    g_input_stream_close((GInputStream *) stream, NULL, NULL);
                    free_object(stream);
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Unable to open file for reading: %s\n"), error->message);
                    error = free_error(error);
                }
        }

    return hash_data_list;
}


/**
 * Gets all meta data for a file and returns a filled meta_data_t *
 * structure.
 * @param directory is the directory we are iterating over it is used
 *        here to build the filename name.
 * @param fileinfo is a glib structure that contains all meta data and
 *        more for a file.
 * @param blocksize is the blocksize to be used to calculate hashs upon.
 * @param database is the db_t * structure to access thhe local database
 *        cache in order to know if we already know this file (and thus
 *        not process it).
 * @returns a newly allocated and filled meta_data_t * structure.
 */
static meta_data_t *get_meta_data_from_fileinfo(gchar *directory, GFileInfo *fileinfo, gint64 blocksize, db_t *database)
{
    meta_data_t *meta = NULL;
    GFile *a_file = NULL;

    if (directory != NULL && fileinfo != NULL && database != NULL)
        {
            /* filling meta data for the file represented by fileinfo */
            meta = new_meta_data_t();

            meta->file_type = g_file_info_get_file_type(fileinfo);
            meta->name = g_build_path(G_DIR_SEPARATOR_S, directory, g_file_info_get_name(fileinfo), NULL);
            meta->inode = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_UNIX_INODE);
            meta->owner = g_file_info_get_attribute_as_string(fileinfo, G_FILE_ATTRIBUTE_OWNER_USER);
            meta->group = g_file_info_get_attribute_as_string(fileinfo, G_FILE_ATTRIBUTE_OWNER_GROUP);
            meta->uid = g_file_info_get_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_UID);
            meta->gid = g_file_info_get_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_GID);
            meta->atime = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_ACCESS);
            meta->ctime = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_CHANGED);
            meta->mtime = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_MODIFIED);
            meta->mode = g_file_info_get_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_MODE);
            meta->size = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_STANDARD_SIZE);

             /* Do the right things with specific cases */
            if (meta->file_type == G_FILE_TYPE_SYMBOLIC_LINK)
                {
                    meta->link = (gchar *) g_file_info_get_attribute_byte_string(fileinfo, G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET);
                }
            else
                {
                    meta->link = g_strdup("");
                }


            /* We need to determine if the file has already been saved by looking into the local database */
            meta->in_cache = is_file_in_cache(database, meta);


            if (meta->in_cache == FALSE && meta->file_type == G_FILE_TYPE_REGULAR)
                {
                    /* Calculates hashs and takes care of data */
                    a_file = g_file_new_for_path(meta->name);
                    meta->hash_data_list = calculate_hash_data_list_for_file(a_file, blocksize);
                    a_file = free_object(a_file);
                }
        }

    return meta;
}


/**
 * Sends meta data to the serveur and returns it's answer or NULL in
 * case of an error.
 * @param main_struct : main structure of the program (contains pointers
 *        to the communication socket.
 * @param meta : the meta_data_t * structure to be saved.
 */
static gchar *send_meta_data_to_serveur(main_struct_t *main_struct, meta_data_t *meta)
{
    gchar *json_str = NULL;
    gchar *answer = NULL;
    gint success = CURLE_FAILED_INIT;

    if (main_struct != NULL && meta != NULL && main_struct->hostname != NULL)
        {
            json_str = convert_meta_data_to_json_string(meta, main_struct->hostname);

            /* Sends meta data here */
            print_debug(_("Sending meta data: %s\n"), json_str);
            main_struct->comm->buffer = json_str;
            success = post_url(main_struct->comm, "/Meta.json");

            free_variable(json_str);

            if (success == CURLE_OK)
                {
                    answer = g_strdup(main_struct->comm->buffer);
                    main_struct->comm->buffer = free_variable(main_struct->comm->buffer);
                }
            else
                {
                    /* Need to manage HTTP errors ? */
                }
        }

    return answer;
}


/**
 * Finds a hash in the hash data list and returns the hash_data_t that
 * corresponds to it. In normal operations it should always find
 * something.
 * @param hash_data_list is the list to look into for the hash 'hash'
 * @param hash is the hash to look for.
 * @returns the hash_data_t  structure that corresponds to the hash 'hash'.
 */
static hash_data_t *find_hash_in_list(GSList *hash_data_list, guint8 *hash)
{
    GSList *iter = hash_data_list;
    hash_data_t *found = NULL;
    gboolean ok = FALSE;

    while (iter != NULL && ok == FALSE)
        {
            found = iter->data;

            if (compare_two_hashs(hash, found->hash) == 0)
                {
                    ok = TRUE;
                }
            else
                {
                   iter = g_slist_next(iter);
                }
        }

    if (ok == TRUE)
        {
            return found;
        }
    else
        {
            return NULL;
        }
}


/**
 * Sends data as requested by the server 'serveur' in a buffered way.
 * @param main_struct : main structure of the program.
 * @param meta : the meta_data_t * structure to be saved and that
 *        contains the data.
 * @param answer is the request sent back by serveur when we had send
 *        meta data.
 * @note using directly main_struct->comm->buffer -> not threadable as is.
 */
static gint send_all_data_to_serveur(main_struct_t *main_struct, meta_data_t *meta, gchar *answer)
{
    json_t *root = NULL;
    json_t *array = NULL;
    GSList *hash_list = NULL;         /** hash_list is local to this function */
    GSList *head = NULL;
    gint success = CURLE_FAILED_INIT;
    hash_data_t *found = NULL;
    hash_data_t *hash_data = NULL;
    gint all_ok = CURLE_OK;
    gint i = 0;
    json_t *to_insert = NULL;
    gint limit = 0;

    if (answer != NULL && meta != NULL && main_struct != NULL && main_struct->opt != NULL)
        {
            root = load_json(answer);

            limit = (1048576) / main_struct->opt->blocksize;

            if (root != NULL)
                {
                    /* This hash_list is the needed hashs from serveur */
                    hash_list = extract_gslist_from_array(root, "hash_list", TRUE);
                    json_decref(root);

                    array = json_array();
                    root = json_object();


                    head = hash_list;

                    while (hash_list != NULL && all_ok == CURLE_OK)
                        {
                            hash_data = hash_list->data;
                            /* hash_data_list contains all hashs and their associated data for the file
                             * being processed */
                            found = find_hash_in_list(meta->hash_data_list, hash_data->hash);

                            to_insert = convert_hash_data_t_to_json(found);
                            json_array_append_new(array, to_insert);

                            i = i + 1;

                            if (i >= limit)
                                {
                                    /* when we've got 1M bytes of data send them ! */
                                    /* main_struct->comm->buffer is the buffer sent to serveur */
                                    insert_json_value_into_json_root(root, "data_array", array);
                                    main_struct->comm->buffer = json_dumps(root, 0);
                                    success = post_url(main_struct->comm, "/Data_Array.json");
                                    json_decref(array);
                                    json_decref(root);
                                    array = json_array();
                                    root = json_object();
                                    all_ok = success;
                                    main_struct->comm->buffer = free_variable(main_struct->comm->buffer);
                                    i = 0;
                                }

                            hash_list = g_slist_next(hash_list);
                        }

                    if (i > 0)
                        {
                            /* Send the rest of the data (less than 1M) */
                            /* main_struct->comm->buffer is the buffer sent to serveur */
                            insert_json_value_into_json_root(root, "data_array", array);
                            main_struct->comm->buffer = json_dumps(root, 0);
                            success = post_url(main_struct->comm, "/Data_Array.json");
                            json_decref(array);
                            json_decref(root);
                            all_ok = success;
                            main_struct->comm->buffer = free_variable(main_struct->comm->buffer);
                        }

                    if (head != NULL)
                        {
                            g_slist_free_full(head, free_hdt_struct);
                        }
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error while loading JSON answer from serveur\n"));
                }
        }

   return all_ok;
}


/**
 * Sends data as requested by the server 'serveur'.
 * @param main_struct : main structure of the program.
 * @param meta : the meta_data_t * structure to be saved and that
 *        contains the data.
 * @param answer is the request sent back by serveur when we had send
 *        meta data.
 * @note using directly main_struct->comm->buffer -> not threadable as is.
 */
static gint send_data_to_serveur(main_struct_t *main_struct, meta_data_t *meta, gchar *answer)
{
    json_t *root = NULL;
    GSList *hash_list = NULL;         /** hash_list is local to this function */
    GSList *head = NULL;
    gint success = CURLE_FAILED_INIT;
    hash_data_t *found = NULL;
    hash_data_t *hash_data = NULL;
    gint all_ok = CURLE_OK;

    if (answer != NULL && meta != NULL)
        {
            root = load_json(answer);

            if (root != NULL)
                {
                    /* This hash_list is the needed hashs from serveur */
                    hash_list = extract_gslist_from_array(root, "hash_list", TRUE);
                    json_decref(root);
                    head = hash_list;

                    while (hash_list != NULL && all_ok == CURLE_OK)
                        {
                            hash_data = hash_list->data;
                            /* hash_data_list contains all hashs and their associated data */
                            found = find_hash_in_list(meta->hash_data_list, hash_data->hash);

                            /* main_struct->comm->buffer is the buffer sent to serveur */
                            main_struct->comm->buffer = convert_hash_data_t_to_string(found);
                            success = post_url(main_struct->comm, "/Data.json");

                            all_ok = success;

                            main_struct->comm->buffer = free_variable(main_struct->comm->buffer);
                            hash_list = g_slist_next(hash_list);
                        }

                    if (head != NULL)
                        {
                            g_slist_free_full(head, free_hdt_struct);
                        }
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error while loading JSON answer from serveur\n"));
                }
        }

   return all_ok;
}


/**
 * This function gets meta data and data from a file and sends them
 * to the serveur in order to save the file located in the directory
 * 'directory' and represented by 'fileinfo' variable.
 * @param main_struct : main structure of the program
 * @param directory is the directory we are iterating over
 * @param fileinfo is a glib structure that contains all meta data and
 *        more for a file.
 * @note This function is not threadable as is. One may have problems
 *       when writing to the database for instance.
 */
void save_one_file(main_struct_t *main_struct, gchar *directory, GFileInfo *fileinfo)
{
    meta_data_t *meta = NULL;
    gchar *answer = NULL;
    gint success = 0;
    a_clock_t *my_clock = NULL;
    gchar *message = NULL;

    if (main_struct != NULL && main_struct->opt != NULL && directory != NULL && fileinfo != NULL)
        {

            my_clock = new_clock_t();

            /* Get data and meta_data for a file. */
            meta = get_meta_data_from_fileinfo(directory, fileinfo, main_struct->opt->blocksize, main_struct->database);

            if (meta->in_cache == FALSE)
                {
                    /* Send data and meta data only if the file isn't already in our local database */
                    answer = send_meta_data_to_serveur(main_struct, meta);

                    if (answer != NULL)
                        {
                            /* success = send_data_to_serveur(main_struct, meta, answer); */
                            success = send_all_data_to_serveur(main_struct, meta, answer);
                            free_variable(answer);

                            if (success == TRUE)
                                {
                                    /* Everything has been transmitted so we can save meta data into the local db cache */
                                    db_save_meta_data(main_struct->database, meta, TRUE);
                                }
                            else
                                {
                                    /* Something went wrong when sending data */
                                    /* Need to save data and meta data because an error occured. */
                                }
                        }
                    else
                        {
                            /* Something went wrong when sending meta-data */
                            /* Need to save data and meta data because an error occured when transmitting. */
                        }
                }

            message = g_strdup_printf(_("processing file %s"), meta->name);
            end_clock(my_clock, message);
            free_variable(message);

            if (meta->file_type == G_FILE_TYPE_DIRECTORY)
                {
                    /* This is a recursive call */
                    carve_one_directory(meta->name, main_struct);
                }

            meta = free_meta_data_t(meta);
        }


}


/**
 * Iterates over an enumerator obtained from a directory.
 * @param main_struct : main structure of the program
 * @param directory is the directory we are iterating over
 * @param file_enum is the enumerator obtained when opening a directory
 *        to carve it.
 */
static void iterate_over_enum(main_struct_t *main_struct, gchar *directory, GFileEnumerator *file_enum)
{
    GError *error = NULL;
    GFileInfo *fileinfo = NULL;

    if (main_struct != NULL && file_enum != NULL)
        {
            fileinfo = g_file_enumerator_next_file(file_enum, NULL, &error);

            while (error == NULL && fileinfo != NULL)
                {
                    save_one_file(main_struct, directory, fileinfo);

                    fileinfo = free_object(fileinfo);

                    fileinfo = g_file_enumerator_next_file(file_enum, NULL, &error);
                }
        }
}


/**
 * Call back for the g_slist_foreach function that carves one directory
 * and sub directories in a recursive way.
 * @param data is an element of opt->list ie: a gchar * that represents
 *        a directory name
 * @param user_data is the main_struct_t * pointer to the main structure.
 */
static void carve_one_directory(gpointer data, gpointer user_data)
{
    gchar *directory = (gchar *) data;
    main_struct_t *main_struct = (main_struct_t *) user_data;

    GFile *a_dir = NULL;
    GFileEnumerator *file_enum = NULL;
    GError *error = NULL;

    if (directory != NULL && main_struct != NULL)
        {
            a_dir = g_file_new_for_path(directory);
            file_enum = g_file_enumerate_children(a_dir, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

            if (error == NULL && file_enum != NULL)
                {
                    iterate_over_enum(main_struct, directory, file_enum);
                    g_file_enumerator_close(file_enum, NULL, NULL);
                    file_enum = free_object(file_enum);
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Unable to enumerate directory %s: %s\n"), directory, error->message);
                    error = free_error(error);
                }

            a_dir = free_object(a_dir);
        }
}


/**
 * Does carve all directories from the list in the option list
 * @param main_struct : main structure of the program that contains also
 *        the options structure that should have a list of directories
 *        to save.
 */
static void carve_all_directories(main_struct_t *main_struct)
{
    if (main_struct != NULL && main_struct->opt != NULL)
        {
            g_slist_foreach(main_struct->opt->dirname_list, carve_one_directory, main_struct);
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

    #if !GLIB_CHECK_VERSION(2, 36, 0)
        g_type_init();  /** g_type_init() is deprecated since glib 2.36 */
    #endif

    init_international_languages();
    curl_global_init(CURL_GLOBAL_ALL);

    opt = do_what_is_needed_from_command_line_options(argc, argv);

    if (opt != NULL)
        {
            main_struct = init_main_structure(opt);

            carve_all_directories(main_struct);

            /** Launching an infinite loop to get modifications done on
             * the filesystem (on directories we watch).
             * @note fanotify's kernel interface does not provide the events
             * needed to know if a file has been deleted or it's attributes
             * changed. Enabling this feature even if we know that files
             * will never get deleted in our database.
             */
            fanotify_loop(main_struct);

            free_options_t_structure(main_struct->opt);
        }

    return 0;
}

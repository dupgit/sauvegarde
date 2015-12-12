/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    client.c
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
 * @file client.c
 *
 * This file is the main file for the monitor program. This monitor
 * program has to monitor file changes onto filesystems. It should notice
 * when a file is created, deleted or changed
 */

#include "client.h"

static GSList *make_regex_exclude_list(GSList *exclude_list);
static gboolean exclude_file(GSList *regex_exclude_list, gchar *filename);
static main_struct_t *init_main_structure(options_t *opt);
static GList *calculate_hash_data_list_for_file(GFile *a_file, gint64 blocksize);
static meta_data_t *get_meta_data_from_fileinfo(gchar *directory, GFileInfo *fileinfo, main_struct_t *main_struct);
static gchar *send_meta_data_to_server(main_struct_t *main_struct, meta_data_t *meta, gboolean data_sent);
static GList *find_hash_in_list(GList *hash_data_list, guint8 *hash);
static void send_data_to_server(main_struct_t *main_struct, meta_data_t *meta, gchar *answer);
static void send_all_data_to_server(main_struct_t *main_struct, meta_data_t *meta, gchar *answer);
static void iterate_over_enum(main_struct_t *main_struct, gchar *directory, GFileEnumerator *file_enum);
static void carve_one_directory(gpointer data, gpointer user_data);
static gpointer carve_all_directories(gpointer data);
static gpointer save_one_file_threaded(gpointer data);
static gpointer free_file_event_t(file_event_t *file_event);
static gint insert_array_in_root_and_send(main_struct_t *main_struct, json_t *array);
static void process_small_file_not_in_cache(main_struct_t *main_struct, meta_data_t *meta);
static gint64 calculate_file_blocksize(main_struct_t *main_struct, gint64 size);


/**
 * Make a list of precompiled GRegex to be used to filter out directories
 * and filenames from being saved.
 * @param exclude_list is the list of gchar * string that should contain
 *        filenames and/or dirnames with basic regex notation.
 * @returns A newly allocated list of compiled GRegex * pointers.
 */
static GSList *make_regex_exclude_list(GSList *exclude_list)
{
    GRegex *a_regex = NULL;
    GError *error = NULL;
    GSList *regex_exclude_list = NULL;

    while (exclude_list != NULL)
        {
            a_regex = g_regex_new(exclude_list->data, G_REGEX_CASELESS, 0, &error);

            if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, _("Error while compiling %s to a regular expression: %s"), exclude_list->data, error->message);
                }
            else if (a_regex != NULL)
                {
                    regex_exclude_list = g_slist_prepend(regex_exclude_list, a_regex);
                }

            exclude_list = g_slist_next(exclude_list);
        }

    return regex_exclude_list;
}


/**
 * Says if 'filename' may correspond to one of the regex of the list.
 * @param regex_exclude_list is the list of precompiled Regex of files
 *        to be excluded from being saved.
 * @param filename is the name pf the file that we want to know if we
 *        have to exclude it or not.
 * @returns TRUE if the file has to be excluded, FALSE otherwise
 */
static gboolean exclude_file(GSList *regex_exclude_list, gchar *filename)
{
    gboolean result = FALSE;

    while (regex_exclude_list != NULL && result == FALSE)
        {
            result = g_regex_match(regex_exclude_list->data, filename, 0, NULL);
            regex_exclude_list = g_slist_next(regex_exclude_list);
        }

    return result;
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

            /* inits the queue that will wait for events on files */
            main_struct->save_queue = g_async_queue_new();
            main_struct->dir_queue = g_async_queue_new();
            main_struct->regex_exclude_list = make_regex_exclude_list(opt->exclude_list);

            /* Thread initialization */
            main_struct->save_one_file = g_thread_new("save_one_file", save_one_file_threaded, main_struct);
            main_struct->carve_all_directories = g_thread_new("carve_all_directories", carve_all_directories, main_struct);

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
static GList *calculate_hash_data_list_for_file(GFile *a_file, gint64 blocksize)
{
    GFileInputStream *stream = NULL;
    GError *error = NULL;
    GList *hash_data_list = NULL;
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
                    buffer = (guchar *) g_malloc(blocksize);
                    a_hash = (guint8 *) g_malloc(digest_len);

                    read = g_input_stream_read((GInputStream *) stream, buffer, blocksize, NULL, &error);

                    while (read != 0 && error == NULL)
                        {
                            g_checksum_update(checksum, buffer, read);
                            g_checksum_get_digest(checksum, a_hash, &digest_len);

                            /* Need to save data and read in hash_data_t structure */
                            hash_data = new_hash_data_t(buffer, read, a_hash);

                            hash_data_list = g_list_prepend(hash_data_list, hash_data);
                            g_checksum_reset(checksum);
                            digest_len = HASH_LEN;

                            buffer = (guchar *) g_malloc(blocksize);
                            a_hash = (guint8 *) g_malloc(digest_len);
                            read = g_input_stream_read((GInputStream *) stream, buffer, blocksize, NULL, &error);
                        }

                    if (error != NULL)
                        {
                            print_error(__FILE__, __LINE__, _("Error while reading file: %s\n"), error->message);
                            error = free_error(error);
                            g_list_free_full(hash_data_list, free_hdt_struct);
                            hash_data_list =  NULL;
                        }
                    else
                        {
                            /* get the list in correct order (because we prepended the hashs to get speed when inserting hashs in the list) */
                            hash_data_list = g_list_reverse(hash_data_list);
                        }

                    free_variable(buffer);
                    free_variable(a_hash);

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
 * @param database is the db_t * structure to access thhe local database
 *        cache in order to know if we already know this file (and thus
 *        not process it).
 * @returns a newly allocated and filled meta_data_t * structure.
 */
static meta_data_t *get_meta_data_from_fileinfo(gchar *directory, GFileInfo *fileinfo, main_struct_t *main_struct)
{
    meta_data_t *meta = NULL;

    if (directory != NULL && fileinfo != NULL &&  main_struct != NULL && main_struct->database != NULL)
        {
            /* filling meta data for the file represented by fileinfo */
            meta = new_meta_data_t();

            meta->file_type = g_file_info_get_file_type(fileinfo);
            meta->name = g_build_path(G_DIR_SEPARATOR_S, directory, g_file_info_get_name(fileinfo), NULL);

            if (exclude_file(main_struct->regex_exclude_list, meta->name) == FALSE)
                {
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
                    meta->blocksize = calculate_file_blocksize(main_struct, meta->size);

                     /* Do the right things with specific cases */
                    if (meta->file_type == G_FILE_TYPE_SYMBOLIC_LINK)
                        {
                            meta->link = (gchar *) g_file_info_get_attribute_byte_string(fileinfo, G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET);
                        }
                    else
                        {
                            meta->link = g_strdup("");
                        }


                    /* We need to determine if the file has already been saved by looking into the local database
                     * This is usefull only when carving directories at the begining of the process as when called
                     * by m_fanotify we already know that the file was written and that something changed.
                     */
                    meta->in_cache = is_file_in_cache(main_struct->database, meta);
                }
            else
                {
                    free_variable(meta->name);
                    meta = free_variable(meta);
                }
        }

    return meta;
}


/**
 * Sends meta data to the server and returns it's answer or NULL in
 * case of an error.
 * @param main_struct : main structure of the program (contains pointers
 *        to the communication socket.
 * @param meta : the meta_data_t * structure to be saved.
 * @returns a newly allocated gchar * string that may be freed when no
 *          longer needed.
 */
static gchar *send_meta_data_to_server(main_struct_t *main_struct, meta_data_t *meta, gboolean data_sent)
{
    gchar *json_str = NULL;
    gchar *answer = NULL;
    gint success = CURLE_FAILED_INIT;
    json_t *root = NULL;
    json_t *array = NULL;

    if (main_struct != NULL && meta != NULL && main_struct->hostname != NULL)
        {
            json_str = convert_meta_data_to_json_string(meta, main_struct->hostname, data_sent);

            /* Sends meta data here: readbuffer is the buffer sent to server */
            print_debug(_("Sending meta data: %s\n"), json_str);
            main_struct->comm->readbuffer = json_str;
            success = post_url(main_struct->comm, "/Meta.json");

            if (success == CURLE_OK)
                {
                    answer = g_strdup(main_struct->comm->buffer);
                    main_struct->comm->buffer = free_variable(main_struct->comm->buffer);
                }
            else
                {
                    /* Need to manage HTTP errors ? */
                    /* Saving meta data that should have been sent to sqlite database */
                    db_save_buffer(main_struct->database, "/Meta.json", main_struct->comm->readbuffer);

                    /* An error occured -> we need the whole hash list to be saved
                     * we are building a 'fake' answer with the whole hash list.
                     */
                    array = convert_hash_list_to_json(meta->hash_data_list);
                    root = json_object();
                    insert_json_value_into_json_root(root, "hash_list", array);
                    answer = json_dumps(root, 0);
                    json_decref(root);
                }

            free_variable(main_struct->comm->readbuffer);
        }

    return answer;
}


/**
 * Finds a hash in the hash data list and returns the hash_data_t that
 * corresponds to it. In normal operations it should always find
 * something.
 * @param hash_data_list is the list to look into for the hash 'hash'
 * @param hash is the hash to look for.
 * @returns the GList * pointer that contains hash_data_t structure that
 *          corresponds to the hash 'hash'.
 */
static GList *find_hash_in_list(GList *hash_data_list, guint8 *hash)
{
    GList *iter = hash_data_list;
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
                   iter = g_list_next(iter);
                }
        }

    if (ok == TRUE)
        {
            return iter;
        }
    else
        {
            return NULL;
        }
}


/**
 * Inserts the array into a root json_t * structure and dumps it into a
 * buffer that is send to the server and then freed.
 * @param main_struct : main structure of the program.
 * @param array is the json_t * array to be sent to the server
 */
static gint insert_array_in_root_and_send(main_struct_t *main_struct, json_t *array)
{
    json_t *root = NULL;
    gint success = CURLE_FAILED_INIT;


    if (main_struct != NULL && main_struct->comm != NULL && array != NULL)
        {

            root = json_object();
            insert_json_value_into_json_root(root, "data_array", array);

            /* readbuffer is the buffer sent to server */
            main_struct->comm->readbuffer = json_dumps(root, 0);

            success = post_url(main_struct->comm, "/Data_Array.json");

            if (success != CURLE_OK)
                {
                    db_save_buffer(main_struct->database, "/Data_Array.json", main_struct->comm->readbuffer);
                }

            free_variable(main_struct->comm->readbuffer);
            json_decref(root);
            main_struct->comm->buffer = free_variable(main_struct->comm->buffer);

        }

    return success;
}


/**
 * Sends data as requested by the server 'cdpfglserver' in a buffered way.
 * @param main_struct : main structure of the program.
 * @param meta : meta is a pointer to meta_data_t strcuture that contains
 *               hash_data_list the list of hash_data_t * pointers
 *               containing all all the data to be saved.
 * @param answer is the request sent back by server when we had send
 *        meta data.
 * @note using directly main_struct->comm->buffer -> not threadable as is.
 */
static void send_all_data_to_server(main_struct_t *main_struct, meta_data_t *meta, gchar *answer)
{
    json_t *root = NULL;
    json_t *array = NULL;
    GList *hash_list = NULL;      /** hash_list is local to this function and contains the needed hashs as answer by server */
    GList *head = NULL;
    GList *iter = NULL;
    hash_data_t *found = NULL;
    hash_data_t *hash_data = NULL;
    gint bytes = 0;
    json_t *to_insert = NULL;
    gint64 limit = 0;
    a_clock_t *elapsed = NULL;

    if (answer != NULL && meta != NULL && meta->hash_data_list != NULL && main_struct != NULL && main_struct->opt != NULL)
        {
            root = load_json(answer);

            limit = main_struct->opt->buffersize;

            if (root != NULL)
                {
                    /* This hash_list is the needed hashs from server */
                    hash_list = extract_glist_from_array(root, "hash_list", TRUE);
                    json_decref(root);

                    array = json_array();

                    head = hash_list;

                    while (hash_list != NULL)
                        {
                            hash_data = hash_list->data;
                            /* hash_data_list contains all hashs and their associated data for the file
                             * being processed */
                            iter = find_hash_in_list(meta->hash_data_list, hash_data->hash);
                            found = iter->data;

                            to_insert = convert_hash_data_t_to_json(found);
                            json_array_append_new(array, to_insert);

                            bytes = bytes + found->read;

                            meta->hash_data_list = g_list_remove_link(meta->hash_data_list, iter);
                            g_list_free_full(iter, free_hdt_struct);

                            if (bytes >= limit)
                                {
                                    /* when we've got opt->buffersize bytes of data send them ! */

                                    elapsed = new_clock_t();
                                    insert_array_in_root_and_send(main_struct, array);
                                    array = json_array();
                                    bytes = 0;
                                    end_clock(elapsed, "insert_array_in_root_and_send");
                                }

                            hash_list = g_list_next(hash_list);
                        }

                    if (bytes > 0)
                        {
                            /* Send the rest of the data (less than opt->buffersize bytes) */
                            elapsed = new_clock_t();
                            insert_array_in_root_and_send(main_struct, array);
                            end_clock(elapsed, "insert_array_in_root_and_send");
                        }
                    else
                        {
                            json_decref(array);
                        }

                    if (head != NULL)
                        {
                            g_list_free_full(head, free_hdt_struct);
                        }
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error while loading JSON answer from server\n"));
                }
        }
}


/**
 * Sends data as requested by the server 'cdpfglserver'.
 * @param main_struct : main structure of the program.
 * @param hash_data_list : list of hash_data_t * pointers containing
 *                          all the data to be saved.
 * @param answer is the request sent back by server when we had send
 *        meta data.
 * @note using directly main_struct->comm->buffer -> not threadable as is.
 */
static void send_data_to_server(main_struct_t *main_struct, meta_data_t *meta, gchar *answer)
{
    json_t *root = NULL;
    GList *hash_list = NULL;         /** hash_list is local to this function */
    GList *head = NULL;
    gint success = CURLE_FAILED_INIT;
    GList *iter = NULL;
    hash_data_t *found = NULL;
    hash_data_t *hash_data = NULL;

    if (main_struct != NULL && main_struct->comm != NULL && answer != NULL &&  meta != NULL && meta->hash_data_list!= NULL)
        {
            root = load_json(answer);

            if (root != NULL)
                {
                    /* This hash_list is the needed hashs from server */
                    hash_list = extract_glist_from_array(root, "hash_list", TRUE);
                    json_decref(root);
                    head = hash_list;

                    while (hash_list != NULL)
                        {
                            hash_data = hash_list->data;
                            /* hash_data_list contains all hashs and their associated data */
                            iter = find_hash_in_list(meta->hash_data_list, hash_data->hash);
                            found = iter->data;

                            /* readbuffer is the buffer sent to server  */
                            main_struct->comm->readbuffer = convert_hash_data_t_to_string(found);
                            success = post_url(main_struct->comm, "/Data.json");

                            if (success != CURLE_OK)
                                {
                                    db_save_buffer(main_struct->database, "/Data.json", main_struct->comm->readbuffer);
                                }

                            free_variable(main_struct->comm->readbuffer);

                            meta->hash_data_list = g_list_remove_link(meta->hash_data_list, iter);
                            g_list_free_full(iter, free_hdt_struct);

                            main_struct->comm->buffer = free_variable(main_struct->comm->buffer);
                            hash_list = g_list_next(hash_list);
                        }

                    if (head != NULL)
                        {
                            g_list_free_full(head, free_hdt_struct);
                        }
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error while loading JSON answer from server\n"));
                }
        }
}


/**
 * @returns a newly alloacted file_event_t * structure that must be freed
 * when no longer needed
 * @param directory is the directory where the event occured
 * @param fileinfo is fileinfo of the file on which the event occured
 */
file_event_t *new_file_event_t(gchar *directory, GFileInfo *fileinfo)
{
    file_event_t *file_event = NULL;


    file_event = (file_event_t *) g_malloc(sizeof(file_event_t));

    file_event->directory = g_strdup(directory);
    file_event->fileinfo = g_file_info_dup(fileinfo);

    return file_event;
}


/**
 * Frees file_event_t's memory
 * @param file_event is the event to be freed
 * @returns NULL
 */
static gpointer free_file_event_t(file_event_t *file_event)
{
    if (file_event != NULL)
        {
            free_variable(file_event->directory);
            free_object(file_event->fileinfo);
            free_variable(file_event);
        }

    return NULL;
}


/**
 * Threaded function that saves one file by getting it's meta-data and
 * it's data and sends them to the server in order to be saved.
 * @param data must be main_struct_t * pointer.
 */
static gpointer save_one_file_threaded(gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    file_event_t *file_event = NULL;

    if (main_struct != NULL && main_struct->save_queue != NULL)
        {
            while (1)
                {
                    file_event = g_async_queue_pop(main_struct->save_queue);
                    save_one_file(main_struct, file_event->directory, file_event->fileinfo);
                    free_file_event_t(file_event);
                }
        }

    return NULL;
}


/**
 * Calculates the block size to be used upon a file
 * @param main_struct : main structure of the program
 * @param size is the size of the considered file.
 */
static gint64 calculate_file_blocksize(main_struct_t *main_struct, gint64 size)
{

    if (main_struct != NULL && main_struct->opt != NULL)
        {
            if (main_struct->opt->adaptative == TRUE)
                {
                    if (size < 32768)            /* max 64 blocks       */
                        {
                            return 512;
                        }
                    else if (size < 262144)      /* max 128 blocks      */
                        {
                            return 2048;
                        }
                    else if (size < 1048576)     /* max 128 blocks      */
                        {
                            return 8192;
                        }
                    else if (size < 8388608)     /* max 512 blocks      */
                        {
                            return 16384;
                        }
                    else if (size < 67108864)    /* max 1024 blocks     */
                        {
                            return 65536;
                        }
                    else if (size < 134217728)   /* max 1024 blocks     */
                        {
                            main_struct->opt->buffersize = (CLIENT_MIN_BUFFER) * 2;
                            return 131072;
                        }
                    else                         /* at least 512 blocks */
                        {
                            main_struct->opt->buffersize = (CLIENT_MIN_BUFFER) * 4;
                            return 262144;
                        }
                }
        }

    /* default case */
    return main_struct->opt->blocksize;
}


/**
 * Process the file that is not already in our local cache
 * @param main_struct : main structure of the program
 * @param meta is the meta data of the file to be processed (it does
 *             not contain any hashs at that point).
 */
static void process_small_file_not_in_cache(main_struct_t *main_struct, meta_data_t *meta)
{
    GFile *a_file = NULL;
    gchar *answer = NULL;
    gint success = 0;      /** success returns a CURL Error status such as CURLE_OK for instance */


    if (main_struct != NULL && main_struct->opt != NULL && meta != NULL)
        {

            if (meta->file_type == G_FILE_TYPE_REGULAR)
                {
                    /* Calculates hashs and takes care of data */
                    a_file = g_file_new_for_path(meta->name);
                    meta->hash_data_list = calculate_hash_data_list_for_file(a_file, meta->blocksize);
                    a_file = free_object(a_file);
                }

            answer = send_meta_data_to_server(main_struct, meta, FALSE);

            if (meta->size < meta->blocksize)
                {
                    /* Only one block to send (size is less than blocksize's value) */
                    send_data_to_server(main_struct, meta, answer);
                }
            else
                {
                    /* A least 2 blocks to send */
                    send_all_data_to_server(main_struct, meta, answer);
                }

            free_variable(answer); /* Not used by now */

            if (success == CURLE_OK)
                {
                    /* Everything has been transmitted so we can save meta data into the local db cache */
                    /* This is usefull for file carving to avoid sending too much things to the server  */
                    db_save_meta_data(main_struct->database, meta, TRUE);
                }
        }
}


/**
 * Process the file that is not already in our local cache
 * @param main_struct : main structure of the program
 * @param meta is the meta data of the file to be processed (it does
 *             not contain any hashs at that point).
 */
static void process_big_file_not_in_cache(main_struct_t *main_struct, meta_data_t *meta)
{
    GFile *a_file = NULL;
    gchar *answer = NULL;
    GFileInputStream *stream = NULL;
    GError *error = NULL;
    GList *hash_data_list = NULL;
    hash_data_t *hash_data = NULL;
    gssize read = 0;
    guchar *buffer = NULL;
    GChecksum *checksum = NULL;
    guint8 *a_hash = NULL;
    gsize digest_len = HASH_LEN;
    gsize read_bytes = 0;
    json_t *array = NULL;
    json_t *to_insert = NULL;
    a_clock_t *elapsed = NULL;

    if (main_struct != NULL && main_struct->opt != NULL && meta != NULL)
        {
            a_file = g_file_new_for_path(meta->name);
            print_debug(_("Processing file: %s\n"), meta->name);

            if (a_file != NULL)
                {
                    stream = g_file_read(a_file, NULL, &error);

                    if (stream != NULL && error == NULL)
                        {

                            checksum = g_checksum_new(G_CHECKSUM_SHA256);
                            buffer = (guchar *) g_malloc(meta->blocksize);
                            a_hash = (guint8 *) g_malloc(digest_len);

                            read = g_input_stream_read((GInputStream *) stream, buffer, meta->blocksize, NULL, &error);
                            read_bytes = read_bytes + read;
                            array = json_array();

                            while (read != 0 && error == NULL)
                                {
                                    g_checksum_update(checksum, buffer, read);
                                    g_checksum_get_digest(checksum, a_hash, &digest_len);

                                    /* Need to save data and read in hash_data_t structure */
                                    hash_data = new_hash_data_t(buffer, read, a_hash);

                                    to_insert = convert_hash_data_t_to_json(hash_data);
                                    json_array_append_new(array, to_insert);

                                    /* Only keeping hashs in the list */
                                    hash_data->data = free_variable(hash_data->data);
                                    hash_data_list = g_list_prepend(hash_data_list, hash_data);

                                    g_checksum_reset(checksum);
                                    digest_len = HASH_LEN;

                                    if (read_bytes >= main_struct->opt->buffersize)
                                        {
                                            /* sending datas naïvely */
                                            elapsed = new_clock_t();
                                            print_debug(_("Sending data: %d bytes\n"), read_bytes);
                                            insert_array_in_root_and_send(main_struct, array);
                                            array = json_array();
                                            read_bytes = 0;
                                            end_clock(elapsed, "insert_array_in_root_and_send");
                                        }

                                    buffer = (guchar *) g_malloc(meta->blocksize);
                                    a_hash = (guint8 *) g_malloc(digest_len);
                                    read = g_input_stream_read((GInputStream *) stream, buffer, meta->blocksize, NULL, &error);
                                    read_bytes = read_bytes + read;
                                }

                            if (error != NULL)
                                {
                                    print_error(__FILE__, __LINE__, _("Error while reading file: %s\n"), error->message);
                                    error = free_error(error);
                                    g_list_free_full(hash_data_list, free_hdt_struct);
                                    hash_data_list =  NULL;
                                }
                            else
                                {

                                    /* sending datas naïvely */
                                    if (read_bytes > 0)
                                        {
                                            elapsed = new_clock_t();
                                            print_debug(_("Sending data: %d bytes\n"), read_bytes);
                                            insert_array_in_root_and_send(main_struct, array);
                                            end_clock(elapsed, "insert_array_in_root_and_send");
                                        }

                                    /* get the list in correct order (because we prepended the hashs to get speed when inserting hashs in the list) */
                                    hash_data_list = g_list_reverse(hash_data_list);
                                }

                            free_variable(buffer);
                            free_variable(a_hash);
                            g_checksum_free(checksum);
                            g_input_stream_close((GInputStream *) stream, NULL, NULL);
                            free_object(stream);
                        }
                    else
                        {
                            print_error(__FILE__, __LINE__, _("Unable to open file for reading: %s\n"), error->message);
                            error = free_error(error);
                        }

                    meta->hash_data_list = hash_data_list;
                    answer = send_meta_data_to_server(main_struct, meta, TRUE);

                    if (answer != NULL)
                        {   /** @todo may be we should check that answer is something that tells that everything went Ok. */
                            /* Everything has been transmitted so we can save meta data into the local db cache */
                            /* This is usefull for file carving to avoid sending too much things to the server  */
                            db_save_meta_data(main_struct->database, meta, TRUE);
                        }

                    a_file = free_object(a_file);
                }
        }
}


/**
 * This function gets meta data and data from a file and sends them
 * to the server in order to save the file located in the directory
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
    a_clock_t *my_clock = NULL;
    gchar *message = NULL;
    gchar *another_dir = NULL;

    if (main_struct != NULL && main_struct->opt != NULL && directory != NULL && fileinfo != NULL)
        {

            my_clock = new_clock_t();

            /* Get data and meta_data for a file. */
            meta = get_meta_data_from_fileinfo(directory, fileinfo, main_struct);

            /* We want to save all files that are not excluded (meta != NULL) */
            if (meta != NULL)
                {
                    /* File is not in cache thus unknown thus we need to save it */
                    if (meta->in_cache == FALSE)
                        {
                            if (meta->size < CLIENT_SMALL_FILE_SIZE)
                                {
                                    process_small_file_not_in_cache(main_struct, meta);
                                }
                            else
                                {
                                    process_big_file_not_in_cache(main_struct, meta);
                                }
                        }

                    if (meta->file_type == G_FILE_TYPE_DIRECTORY)
                        {
                            /* This is a recursive call */
                            another_dir = g_strdup(meta->name);
                            g_async_queue_push(main_struct->dir_queue, another_dir);

                        }
                    message = g_strdup_printf(_("processing file %s"), meta->name);
                    free_meta_data_t(meta, FALSE);
                }
            else
                {
                    message = g_strdup_printf(_("processing excluded file"));
                }

            end_clock(my_clock, message);
            free_variable(message);
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
    file_event_t *file_event = NULL;

    if (main_struct != NULL && file_enum != NULL)
        {
            fileinfo = g_file_enumerator_next_file(file_enum, NULL, &error);

            while (error == NULL && fileinfo != NULL)
                {
                    /* file_event is used and freed in the thread
                     * save_one_file_threaded where the queue save_queue
                     * is used
                     */
                    file_event = new_file_event_t(directory, fileinfo);
                    g_async_queue_push(main_struct->save_queue, file_event);

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
 * Does carve all directories from the list in the option list.
 * This function is a thread that is run at the end of the initialisation
 * of main_struct structure.
 * @param main_struct : main structure of the program that contains also
 *        the options structure that should have a list of directories
 *        to save.
 */
static gpointer carve_all_directories(gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    gchar *directory = NULL;

    if (main_struct != NULL && main_struct->opt != NULL)
        {
            g_slist_foreach(main_struct->opt->dirname_list, carve_one_directory, main_struct);

            directory = g_async_queue_pop(main_struct->dir_queue);

            while (directory != NULL)
                {
                    carve_one_directory(directory, main_struct);
                    free_variable(directory);
                    directory = g_async_queue_pop(main_struct->dir_queue);
                }
        }

    return NULL;
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
            /**
             * Inits the main structure and launches two threads one that
             * will save the files with an asynchronous queue and a second
             * that will do directory carving. Threads communicates with
             * the asynchronous queue.
             */
            main_struct = init_main_structure(opt);


            if (db_is_there_buffers_to_transmit(main_struct->database))
                {
                    print_debug(_("We have data and meta data to transmit to server\n"));
                }


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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    restore.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 Olivier Delhomme
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
 * @file restore.c
 * This program should be able to restore files.
 */

#include "restore.h"

static res_struct_t *init_res_struct(int argc, char **argv);
static gchar *encode_to_base64(gchar *string);
static query_t *get_user_infos(gchar *hostname, gchar *filename, gchar *date);
static GSList *get_files_from_server(res_struct_t *res_struct, query_t *query);
static void print_all_files(res_struct_t *res_struct, query_t *query);
static void restore_data_to_stream(res_struct_t *res_struct, GFileOutputStream *stream, GList *hash_list);
static void create_file(res_struct_t *res_struct, meta_data_t *meta);
static void restore_last_file(res_struct_t *res_struct, query_t *query);
static void free_res_struct_t(res_struct_t *res_struct);

/**
 * Inits a res_struct_t * structure. Manages the command line options.
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns a res_struct_t * structure containing all of what is needed
 *          by cdpfglrestore program.
 */
static res_struct_t *init_res_struct(int argc, char **argv)
{
    res_struct_t *res_struct = NULL;
    gchar *conn = NULL;


    res_struct = (res_struct_t *) g_malloc0(sizeof(res_struct_t));

    res_struct->hostname = (gchar *) g_get_host_name();

    res_struct->opt = do_what_is_needed_from_command_line_options(argc, argv);

    if (res_struct->opt != NULL && res_struct->opt->ip != NULL)
        {
            /* We keep conn string into comm_t structure: do not free it ! */
            conn = make_connexion_string(res_struct->opt->ip, res_struct->opt->port);
            res_struct->comm = init_comm_struct(conn);
        }
    else
        {
            /* This should never happen because we have default values... */
            res_struct->comm = NULL;
        }

    return res_struct;
}


/**
 * Encodes a gchar string into a base64 formated gchar * string.
 * @param string is the gchar string to be encoded (MUST be 0 terminated).
 * @returns a newly allocated gchar * string in base64 or NULL.
 */
static gchar *encode_to_base64(gchar *string)
{
    gchar *encoded_string = NULL;

    if (string != NULL)
        {
            encoded_string = g_base64_encode((const guchar *) string, strlen(string));
        }

    return encoded_string;
}



/**
 * Gets all user infos and fills a query_t * structure accordingly.
 * @param hostname the hostname where the program is run
 * @param filename is the name of the file we want to restore.
 * @param date is the date of the version of this file.
 * @return a pointer to a newly allocated query_t structure that may be
 *         freed when no longer needed.
 */
static query_t *get_user_infos(gchar *hostname, gchar *filename, gchar *date)
{
    uid_t uid;
    struct passwd *pass = NULL;
    struct group *grp = NULL;
    query_t *query = NULL;
    gchar *the_uid = NULL;
    gchar *the_gid = NULL;
    gchar *owner = NULL;
    gchar *group = NULL;
    gchar *encoded_date = NULL;
    gchar *encoded_filename = NULL;

    uid = geteuid();
    pass = getpwuid(uid);

    if (pass != NULL)
        {
            grp = getgrgid(pass->pw_gid);
            group = g_strdup(grp->gr_name);
            owner = g_strdup(pass->pw_name);
            the_uid = g_strdup_printf("%d", uid);
            the_gid = g_strdup_printf("%d", pass->pw_gid);

            encoded_filename = encode_to_base64(filename);
            encoded_date = encode_to_base64(date);

            query = init_query_structure(hostname, the_uid, the_gid, owner, group, encoded_filename, encoded_date);
            print_debug(_("hostname: %s, uid: %s, gid: %s, owner: %s, group: %s\n"), hostname, the_uid, the_gid, owner, group);
        }

    return query;
}


/**
 * Gets the file the SERVER_meta_data_t * file list if any
 * @param res_struct is the main structure for cdpfglrestore program.
 * @param query is the structure that contains everything needed to
 *        query the server (and filter a bit). It must not be NULL.
 * @returns a GSList * of SERVER_meta_data_t *
 */
static GSList *get_files_from_server(res_struct_t *res_struct, query_t *query)
{
    gchar *request = NULL;
    json_t *root = NULL;
    GSList *list = NULL;    /** List of SERVER_meta_data_t * returned by this function */
    gint res = CURLE_FAILED_INIT;

    if (res_struct != NULL && query != NULL)
        {
            if (query->date == NULL)
                {
                    request = g_strdup_printf("/File/List.json?hostname=%s&uid=%s&gid=%s&owner=%s&group=%s&filename=%s", query->hostname, query->uid, query->gid, query->owner, query->group, query->filename);
                }
            else
                {
                    request = g_strdup_printf("/File/List.json?hostname=%s&uid=%s&gid=%s&owner=%s&group=%s&filename=%s&date=%s", query->hostname, query->uid, query->gid, query->owner, query->group, query->filename, query->date);
                }

            print_debug(_("Query is: %s\n"), request);
            res = get_url(res_struct->comm, request);

            if (res == CURLE_OK && res_struct->comm->buffer != NULL)
                {
                    root = load_json(res_struct->comm->buffer);

                    list = extract_smeta_gslist_from_file_list(root);
                    list = g_slist_sort(list, compare_filenames);

                    json_decref(root);
                    res_struct->comm->buffer = free_variable(res_struct->comm->buffer);
                }

            free_variable(request);
        }

    return list;
}


/**
 * Prints all saved files
 * @param res_struct is the main structure for cdpfglrestore program.
 * @param query is the structure that contains everything needed to
 *        query the server (and filter a bit). It must not be NULL.
 */
static void print_all_files(res_struct_t *res_struct, query_t *query)
{
    GSList *list = NULL;   /** List of SERVER_meta_data_t * */
    GSList *head = NULL;   /** head of the list to be freed  */
    SERVER_meta_data_t *smeta = NULL;

    if (res_struct != NULL && query != NULL)
        {
            list = get_files_from_server(res_struct, query);
            head = list;

            while (list != NULL)
                {
                    smeta = (SERVER_meta_data_t *) list->data;
                    print_smeta_to_screen(smeta);
                    free_smeta_data_t(smeta);

                    list = g_slist_next(list);
                }

            g_slist_free(head);
        }
}


/**
 * Writes data obtained from the server with the hash_list hashs
 * to the stream.
 * @param stream is the stream where we are writing data (MUST be opened
 *        and not NULL)
 * @param hash_list list of hashs of the file to be restored
 * @todo error management.
 */
static void restore_data_to_stream(res_struct_t *res_struct, GFileOutputStream *stream, GList *hash_list)
{
    gchar *hash = NULL;
    hash_data_t *hash_data = NULL;
    GError *error = NULL;
    gchar *request = NULL;
    gint res = CURLE_FAILED_INIT;

    if (stream != NULL)
        {
            while (hash_list != NULL)
                {
                    hash_data = hash_list->data;
                    hash = hash_to_string(hash_data->hash);
                    request = g_strdup_printf("/Data/%s.json", hash);

                    print_debug(_("Query is: %s\n"), request);

                    /* This call fills res_struct->comm->buffer */
                    res = get_url(res_struct->comm, request);

                    if (res == CURLE_OK)
                        {
                            /** We need to save the retrieved buffer */
                            if (res_struct->comm->buffer != NULL)
                                {
                                    hash_data = convert_string_to_hash_data(res_struct->comm->buffer);
                                    res_struct->comm->buffer = free_variable(res_struct->comm->buffer);

                                    if (hash_data != NULL)
                                        {
                                            g_output_stream_write((GOutputStream *) stream, hash_data->data, hash_data->read, NULL, &error);

                                            free_hash_data_t_structure(hash_data);
                                        }
                                    else
                                        {
                                            print_error(__FILE__, __LINE__, _("Error while trying to restore %s hash\n"), hash);
                                        }
                                }
                        }
                    else
                        {
                            print_error(__FILE__, __LINE__, _("Error while getting hash %s"), hash);
                        }

                    hash_list = g_list_next(hash_list);
                    free_variable(request);
                    free_variable(hash);
                }
        }
}


/**
 * Creates the file to be restored.
 * @param res_struct is the main structure for cdpfglrestore program (used here
 *        to communicate with cdpfglserver's server).
 * @param meta is the whole meta_data file describing the file to be
 *        restored
 */
static void create_file(res_struct_t *res_struct, meta_data_t *meta)
{
    GFile *file = NULL;
    gchar *basename = NULL;    /** basename for the file to be restored     */
    gchar *where = NULL;       /** directory where to restore the file      */
    gchar *filename = NULL;    /** filename of the restored file            */
    GFileOutputStream *stream =  NULL;
    GError *error = NULL;
    options_t *opt = NULL;

    if (res_struct != NULL && meta != NULL)
        {
            /* get the basename of the file to be restored */
            basename = g_path_get_basename(meta->name);

            opt = res_struct->opt;

            if (opt != NULL && opt->where != NULL && g_file_test(opt->where, G_FILE_TEST_IS_DIR))
                {
                    where = g_strdup(opt->where);
                }

            if (where == NULL)
                {
                    /* Fall back to get the current directory to make the file to be restored in it */
                    where = g_get_current_dir();
                }

            filename = g_build_filename(where, basename, NULL);
            print_debug(_("filename to restore: %s\n"), filename);
            file = g_file_new_for_path(filename);

            if (g_strcmp0("", meta->link) == 0)
                {
                    stream = g_file_replace(file, NULL, TRUE, G_FILE_CREATE_NONE, NULL, &error);

                    if (stream != NULL)
                        {
                            restore_data_to_stream(res_struct, stream, meta->hash_data_list);

                            g_output_stream_close((GOutputStream *) stream, NULL, &error);
                            free_object(stream);
                        }
                    else if (error != NULL)
                        {
                            print_error(__FILE__, __LINE__, _("Error: unable to open file %s to write data in it (%s).\n"), filename, error->message);
                            free_variable(error);
                        }

                    /* Setting before closing the file does not alter access and modification time */
                    set_file_attributes(file, meta);
                }
            else
                {
                    make_symbolic_link(file, meta->link);
                }

            free_object(file);
            free_variable(where);
            free_variable(basename);
            free_variable(filename);
        }
}


/**
 * Restores the last file that the fetched list contains.
 * @param res_struct is the main structure for cdpfglrestore program.
 * @param query is the structure that contains everything needed to
 *        query the server (and filter a bit). It must not be NULL.
 */
static void restore_last_file(res_struct_t *res_struct, query_t *query)
{
    GSList *list = NULL;      /** List of SERVER_meta_data_t *            */
    GSList *last = NULL;      /** last element of the list                 */
    SERVER_meta_data_t *smeta = NULL;
    meta_data_t *meta = NULL;
    gchar *string_inode = NULL;
    gchar *string_atime = NULL;
    gchar *string_ctime = NULL;
    gchar *string_mtime = NULL;
    gchar *string_size = NULL;

    if (res_struct != NULL && query != NULL)
        {
            list = get_files_from_server(res_struct, query);
            last = g_slist_last(list);

            if (last != NULL)
                {
                    smeta = (SERVER_meta_data_t *) last->data;
                    meta = smeta->meta;

                    if (get_debug_mode() == TRUE)
                        {
                            string_inode = g_strdup_printf("%"G_GUINT64_FORMAT, meta->inode);
                            string_atime = g_strdup_printf("%"G_GUINT64_FORMAT, meta->atime);
                            string_ctime = g_strdup_printf("%"G_GUINT64_FORMAT, meta->ctime);
                            string_mtime = g_strdup_printf("%"G_GUINT64_FORMAT, meta->mtime);
                            string_size = g_strdup_printf("%"G_GUINT64_FORMAT, meta->size);

                            print_debug(_("File to be restored: type %d, inode: %s, mode: %d, atime: %s, ctime: %s, mtime: %s, size: %s, filename: %s, owner: %s, group: %s, uid: %d, gid: %d\n"), meta->file_type, string_inode, meta->mode, string_atime, string_ctime, string_mtime, string_size, meta->name, meta->owner, meta->group, meta->uid, meta->gid);

                            free_variable(string_inode);
                            free_variable(string_atime);
                            free_variable(string_ctime);
                            free_variable(string_mtime);
                            free_variable(string_size);
                        }

                    create_file(res_struct, meta);
                }

            g_slist_free_full(list, gslist_free_smeta);
        }
}


/**
 * Frees a previously allocated res_struct_t * structure.
 * @param res_struct is the res_struct_t * structure to be freed.
 */
static void free_res_struct_t(res_struct_t *res_struct)
{
    if (res_struct != NULL)
        {
            free_options_t_structure(res_struct->opt);
            free_comm_t(res_struct->comm);
            free_variable(res_struct->hostname);
            free_variable(res_struct);
        }
}


/**
 * Main function
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns
 */
int main(int argc, char **argv)
{
    res_struct_t *res_struct = NULL;
    query_t *query =  NULL;


    #if !GLIB_CHECK_VERSION(2, 36, 0)
        g_type_init();  /** g_type_init() is deprecated since glib 2.36 */
    #endif

    init_international_languages();

    res_struct = init_res_struct(argc, argv);

    if (res_struct != NULL && res_struct->opt != NULL && res_struct->comm != NULL)
        {

            if (res_struct->opt->list != NULL)
                {
                    query = get_user_infos(res_struct->hostname, res_struct->opt->list, res_struct->opt->date);
                    print_all_files(res_struct, query);
                    free_query_structure(query);
                }

            if (res_struct->opt->restore != NULL)
                {
                    query = get_user_infos(res_struct->hostname, res_struct->opt->restore, res_struct->opt->date);
                    restore_last_file(res_struct, query);
                    free_query_structure(query);
                }

            free_res_struct_t(res_struct);

            return EXIT_SUCCESS;
        }
    else
        {
            return EXIT_FAILURE;
        }
}

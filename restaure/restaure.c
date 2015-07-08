
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    restaure.c
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
 * @file restaure.c
 * This program should be able to restaure files.
 */

#include "restaure.h"

static res_struct_t *init_res_struct(int argc, char **argv);
static query_t *get_user_infos(gchar *hostname, gchar *filename);
static void print_smeta_to_screen(serveur_meta_data_t *smeta);
static GSList *get_files_from_serveur(res_struct_t *res_struct, gchar *filename);
static void print_all_files(res_struct_t *res_struct, gchar *filename);

/**
 * Inits a res_struct_t * structure. Manages the command line options.
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns a res_struct_t * structure containing all of what is needed
 *          by restaure program.
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
 * Gets all user infos and fills a query_t * structure accordingly.
 * @param hostname the hostname where the program is run
 */
static query_t *get_user_infos(gchar *hostname, gchar *filename)
{
    uid_t uid;
    struct passwd *pass = NULL;
    struct group *grp = NULL;
    query_t *query = NULL;
    gchar *the_uid = NULL;
    gchar *the_gid = NULL;
    gchar *owner = NULL;
    gchar *group = NULL;

    uid = geteuid();
    pass = getpwuid(uid);

    if (pass != NULL)
        {
            grp = getgrgid(pass->pw_gid);
            group = g_strdup(grp->gr_name);
            owner = g_strdup(pass->pw_name);
            the_uid = g_strdup_printf("%d", uid);
            the_gid = g_strdup_printf("%d", pass->pw_gid);

            query = init_query_structure(hostname, the_uid, the_gid, owner, group, filename);
            print_debug("hostname: %s, uid: %s, gid: %s, owner: %s, group: %s\n", hostname, the_uid, the_gid, owner, group);
        }

    return query;
}


/**
 * Prints a file ands its meta data to the screen
 * @param smeta is the serveur meta data of the file to be printed on the
 *        screen
 */
static void print_smeta_to_screen(serveur_meta_data_t *smeta)
{
    meta_data_t *meta = NULL;
    GDateTime *la_date = NULL;
    gchar *the_date = NULL;

    if (smeta !=  NULL && smeta->meta != NULL)
        {
            meta = smeta->meta;

            switch (meta->file_type)
                {
                    case 1:
                        fprintf(stdout, "[FILE] ");
                    break;
                    case 2:
                        fprintf(stdout, "[DIR ] ");
                    break;
                    default:
                        fprintf(stdout, "[    ] ");
                    break;
                }

            la_date = g_date_time_new_from_unix_utc(meta->mtime);
            the_date = g_date_time_format(la_date, "%F %T");

            fprintf(stdout, "%s ", the_date);

            fprintf(stdout, "%s\n", meta->name);

            free_variable(the_date);
            g_date_time_unref(la_date);
        }

}


/**
 * Gets the file the serveur_metat_data_t * file list if any
 * @param res_struct is the main structure for restaure program.
 * @param filename is the filename used to filter out the query. It must
 *        not be NULL.
 * @returns a GSList * of serveur_meta_data_t *
 */
static GSList *get_files_from_serveur(res_struct_t *res_struct, gchar *filename)
{
    query_t *query = NULL;
    gchar *request = NULL;
    json_t *root = NULL;
    GSList *list = NULL;    /** List of serveur_meta_data_t * */
    gint res = CURLE_FAILED_INIT;

    query = get_user_infos(res_struct->hostname, filename);

    if (query != NULL)
        {
            request = g_strdup_printf("/File/List.json?hostname=%s&uid=%s&gid=%s&owner=%s&group=%s&filename=%s", query->hostname, query->uid, query->gid, query->owner, query->group, filename);
            print_debug(_("Query is: %s\n"), request);
            res = get_url(res_struct->comm, request);

            if (res == CURLE_OK && res_struct->comm->buffer != NULL)
                {
                    root = load_json(res_struct->comm->buffer);

                    list = extract_smeta_gslist_from_file_list(root);
                    list = g_slist_sort(list, compare_filenames);

                    json_decref(root);
                    free_variable(res_struct->comm->buffer);
                }

            free_variable(request);
            free_query_structure(query);
        }

    return list;
}


/**
 * Prints all saved files
 * @param res_struct is the main structure for restaure program.
 * @param filename is the filename used to filter out the query. It must
 *        not be NULL.
 */
static void print_all_files(res_struct_t *res_struct, gchar *filename)
{
    GSList *list = NULL;   /** List of serveur_meta_data_t * */
    GSList *head = NULL;   /** head of the list to be freed  */
    serveur_meta_data_t *smeta = NULL;

    if (res_struct != NULL && filename != NULL)
        {
            list = get_files_from_serveur(res_struct, filename);
            head = list;

            while (list != NULL)
                {
                    smeta = (serveur_meta_data_t *) list->data;
                    print_smeta_to_screen(smeta);
                    free_smeta_data_t(smeta);

                    list = g_slist_next(list);
                }

            g_slist_free(head);
        }
}


/**
 * Creates the file to be restored.
 * @param res_struct is the main structure for restaure program (used here
 *        to communicate with serveur's server).
 * @param meta is the whole meta_data file describing the file to be
 *        restored
 * @todo simplify this function.
 * @todo error management.
 */
static void create_file(res_struct_t *res_struct, meta_data_t *meta)
{
    GFile *file = NULL;
    gchar *basename = NULL;    /** basename for the file to be restored     */
    gchar *cwd = NULL;         /** current working directory                */
    gchar *filename = NULL;    /** filename of the restored file            */
    GSList *hash_list = NULL;  /** list of hashs of the file to be restored */
    gchar *hash = NULL;
    gchar *request = NULL;
    gint res = CURLE_FAILED_INIT;
    GFileOutputStream *stream =  NULL;
    GError *error = NULL;
    guchar *data = NULL;
    gsize data_len = 0;

    if (meta != NULL)
        {
            file = g_file_new_for_path(meta->name);
            basename = g_file_get_basename(file);
            free_object(file);

            cwd = getcwd(NULL, 0);
            filename = g_build_filename(cwd, basename, NULL);
            print_debug("filename = %s\n", filename);
            file = g_file_new_for_path(filename);

            stream = g_file_create(file, G_FILE_CREATE_REPLACE_DESTINATION, NULL, &error);

            if (stream != NULL)
                {
                    hash_list = meta->hash_list;

                    while (hash_list != NULL)
                        {
                            hash = hash_to_string(hash_list->data);

                            request = g_strdup_printf("/Data/%s.json", hash);
                            print_debug(_("Query is: %s\n"), request);
                            res = get_url(res_struct->comm, request);

                            if (res == CURLE_OK)
                                {
                                    /** We need to save the retrieved buffer */
                                    if (res_struct->comm->buffer != NULL)
                                        {
                                            data = g_base64_decode(res_struct->comm->buffer, &data_len);
                                            g_output_stream_write((GOutputStream *) stream, data, data_len, NULL, &error);
                                            free_variable(res_struct->comm->buffer);
                                            free_variable(data);
                                        }
                                }
                            else
                                {
                                    print_error(__FILE__, __LINE__, _("Error while getting hash %s"), hash);
                                }

                            hash_list = g_slist_next(hash_list);
                            free_variable(request);
                            free_variable(hash);
                        }

                    g_output_stream_close((GOutputStream *) stream, NULL, &error);
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error: unable to open file %s to write datas in it (%s).\n"), filename, error->message);
                }

            free_object(file);
            free(cwd);
            free_variable(basename);
            free_variable(filename);
        }
}


/**
 * Restores the last file that the fetched list contains.
 * @param res_struct is the main structure for restaure program.
 * @param filename is the filename used to filter out the query. It must
 *        not be NULL.
 */
static void restore_last_file(res_struct_t *res_struct, gchar *filename)
{
    GSList *list = NULL;      /** List of serveur_meta_data_t *            */
    GSList *last = NULL;      /** last element of the list                 */
    serveur_meta_data_t *smeta = NULL;
    meta_data_t *meta = NULL;

    if (res_struct != NULL && filename != NULL)
        {
            list = get_files_from_serveur(res_struct, filename);
            last = g_slist_last(list);

            if (last != NULL)
                {
                    smeta = (serveur_meta_data_t *) last->data;
                    meta = smeta->meta;

                    print_debug(_("File to be restored: type %d, inode: %ld, mode: %d, atime: %ld, ctime: %ld, mtime: %ld, size: %ld, filename: %s, owner: %s, group: %s, uid: %d, gid: %d\n"), meta->file_type, meta->inode, meta->mode, meta->atime, meta->ctime, meta->mtime, meta->size, meta->name, meta->owner, meta->group, meta->uid, meta->gid);

                    create_file(res_struct, meta);
                }

            g_slist_free_full(list, gslist_free_smeta);
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


    #if !GLIB_CHECK_VERSION(2, 36, 0)
        g_type_init();  /** g_type_init() is deprecated since glib 2.36 */
    #endif

    init_international_languages();

    res_struct = init_res_struct(argc, argv);

    if (res_struct != NULL && res_struct->opt != NULL && res_struct->comm != NULL)
        {

            if (res_struct->opt->list != NULL)
                {
                    print_all_files(res_struct, res_struct->opt->list);
                }

            if (res_struct->opt->restore != NULL)
                {
                    fprintf(stdout, "We should restore %s!\n", res_struct->opt->restore);
                    restore_last_file(res_struct, res_struct->opt->restore);
                }

            return EXIT_SUCCESS;
        }
    else
        {
            return EXIT_FAILURE;
        }
}

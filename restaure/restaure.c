
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
 * Prints all saved files
 * @param res_struct is the main structure for restaure program.
 * @param filename is the filename used to filter out the query. It must
 *        not be NULL.
 */
static void print_all_files(res_struct_t *res_struct, gchar *filename)
{
    query_t *query = NULL;
    gchar *request = NULL;
    json_t *root = NULL;
    GSList *list = NULL;  /** List of serveur_meta_data_t * */
    serveur_meta_data_t *smeta = NULL;

    query = get_user_infos(res_struct->hostname, filename);

    if (query != NULL && filename != NULL)
        {
            request = g_strdup_printf("/File/List.json?hostname=%s&uid=%s&gid=%s&owner=%s&group=%s&filename=%s", query->hostname, query->uid, query->gid, query->owner, query->group, filename);
            get_url(res_struct->comm, request);

            if (res_struct->comm->buffer != NULL)
                {
                    root = load_json(res_struct->comm->buffer);
                    list = extract_smeta_gslist_from_file_list(root);
                    list = g_slist_sort(list, compare_filenames);

                    while (list != NULL)
                        {
                            smeta = (serveur_meta_data_t *) list->data;

                            print_smeta_to_screen(smeta);
                            free_smeta_data_t(smeta);

                            list = g_slist_next(list);
                        }
                }

            free_variable(request);
            free_query_structure(query);
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

            return EXIT_SUCCESS;
        }
    else
        {
            return EXIT_FAILURE;
        }
}

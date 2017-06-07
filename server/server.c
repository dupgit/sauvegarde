/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    server.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2017 Olivier Delhomme
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
 * @file server.c
 * This file contains all the stuff for the cdpfglserver program of "Sauvegarde"
 * project. The aim of this program is to save every checksum and data and
 * meta data of every 'client' program that is connected to.
 */

#include "server.h"

static void free_server_struct_t(server_struct_t *server_struct);
static gboolean int_signal_handler(gpointer user_data);
static server_struct_t *init_server_main_structure(int argc, char **argv);
static gchar *get_data_from_a_specific_hash(server_struct_t *server_struct, gchar *hash);
static gchar *get_argument_value_from_key(struct MHD_Connection *connection, gchar *key, gboolean encoded);
static gboolean get_boolean_argument_value_from_key(struct MHD_Connection *connection, gchar *key);
static gchar *get_a_list_of_files(server_struct_t *server_struct, struct MHD_Connection *connection);
static hash_data_t *create_one_hash_data_t_from_hash_data_list(GList *hash_data_list, guint size);
static gchar *get_data_from_a_list_of_hashs(server_struct_t *server_struct, struct MHD_Connection *connection);
static gchar *get_json_answer(server_struct_t *server_struct, struct MHD_Connection *connection, const char *url);
static gchar *get_unformatted_answer(server_struct_t *server_struct, const char *url);
static int create_MHD_response(struct MHD_Connection *connection, gchar *answer, gchar *content_type);
static int process_get_request(server_struct_t *server_struct, struct MHD_Connection *connection, const char *url, void **con_cls);
static json_t *find_needed_hashs(server_struct_t *server_struct, GList *hash_data_list);
static int answer_meta_json_post_request(server_struct_t *server_struct, struct MHD_Connection *connection, gchar *received_data);
static int answer_hash_array_post_request(server_struct_t *server_struct, struct MHD_Connection *connection, gchar *received_data);
static void print_received_data_for_hash(guint8 *hash, gssize read);
static int process_received_data(server_struct_t *server_struct, struct MHD_Connection *connection, const char *url, gchar *received_data);
static guint64 get_content_length(struct MHD_Connection *connection);
static int process_post_request(server_struct_t *server_struct, struct MHD_Connection *connection, const char *url, void **con_cls, const char *upload_data, size_t *upload_data_size);
static int print_out_key(void *cls, enum MHD_ValueKind kind, const char *key, const char *value);
static void print_headers(struct MHD_Connection *connection);
static int ahc(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls);
static gpointer meta_data_thread(gpointer user_data);
static gpointer data_thread(gpointer user_data);
static void install_server_signal_traps(server_struct_t *server_struct);


/**
 * Frees server's structure
 * @param server_struct is the structure to be freed
 */
void free_server_struct_t(server_struct_t *server_struct)
{

    if (server_struct != NULL)
        {
            MHD_stop_daemon(server_struct->d);
            print_debug(_("\tMHD daemon stopped.\n"));
            free_variable(server_struct->backend); /** we need a backend function to be called to free th backend structure */
            print_debug(_("\tbackend variable freed.\n"));
            g_thread_unref(server_struct->data_thread);
            print_debug(_("\tdata thread unreferenced.\n"));
            g_thread_unref(server_struct->meta_thread);
            print_debug(_("\tmeta thread unreferenced.\n"));
            free_options_t(server_struct->opt);
            print_debug(_("\toption structure freed.\n"));
            free_variable(server_struct);
            print_debug(_("\tmain structure freed.\n"));
        }
}


/**
 * SIGINT signal handler function
 * @param user_data is a gpointer that MUST be a pointer to the
 *        server_struct_t *
 * @returns FALSE if user_data is NULL and frees memory and exits if TRUE.
 */
static gboolean int_signal_handler(gpointer user_data)
{
    server_struct_t *server_struct = (server_struct_t *) user_data;

    if (server_struct != NULL)
        {
            print_debug(_("\nEnding the program:\n"));
            g_main_loop_quit(server_struct->loop);
            print_debug(_("\tMain loop exited.\n"));
            free_server_struct_t(server_struct);
        }

    /** we can remove the handler as we are exiting the program anyway */
    return FALSE;
}


/**
 * Inits main server's structure
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns a server_struct_t * structure that contains everything that is
 *          needed for 'cdpfglserver' program.
 */
static server_struct_t *init_server_main_structure(int argc, char **argv)
{
    server_struct_t *server_struct = NULL;  /** main structure for 'server' program. */

    server_struct = (server_struct_t *) g_malloc0(sizeof(server_struct_t));

    server_struct->data_thread = NULL;
    server_struct->meta_thread = NULL;
    server_struct->opt = do_what_is_needed_from_command_line_options(argc, argv);
    server_struct->d = NULL;            /* libmicrohttpd daemon pointer */
    server_struct->meta_queue = g_async_queue_new();
    server_struct->data_queue = g_async_queue_new();
    server_struct->loop = NULL;

    /* server statistics */
    server_struct->stats = new_stats_t();

    /* default backend (file_backend) */
    server_struct->backend = init_backend_structure(file_store_smeta, file_store_data, file_init_backend, file_build_needed_hash_list, file_get_list_of_files, file_retrieve_data);

    return server_struct;
}


/**
 * Function that gets the data of a specific hash
 * @param server_struct is the main structure for the server.
 * @param hash is the hash (in hex format) of which we want the data.
 * @returns a json formatted string.
 */
static gchar *get_data_from_a_specific_hash(server_struct_t *server_struct, gchar *hash)
{
    gchar *answer = NULL;
    gchar *message = NULL;
    backend_t *backend = NULL;
    hash_data_t *hash_data = NULL;

    if (server_struct != NULL && server_struct->backend != NULL)
        {
            backend = server_struct->backend;

            if (backend->retrieve_data != NULL)
                {
                    hash_data = backend->retrieve_data(server_struct, hash);
                    answer = convert_hash_data_t_to_string(hash_data);
                    free_hash_data_t(hash_data);

                    if (answer == NULL)
                        {
                            message = g_strdup_printf(_("Error while trying to get data from hash %s"), hash);
                            answer = answer_json_error_string(MHD_HTTP_INTERNAL_SERVER_ERROR, message);
                            free_variable(message);
                        }
                }
            else
                {
                    message = g_strdup(_("This backend's missing a retrieve_data function!"));
                    answer = answer_json_error_string(MHD_HTTP_NOT_IMPLEMENTED, message);
                    free_variable(message);
                }
        }
    else
        {
            message = g_strdup(_("Something's wrong with server's initialisation!"));
            answer = answer_json_error_string(MHD_HTTP_INTERNAL_SERVER_ERROR, message);
            free_variable(message);
        }

    return answer;
}


/**
 * Function that gets the argument corresponding to the key 'key' in the
 * url (from connection)
 * @param connection is the connection in MHD
 * @param key the key to look for into the url
 * @param encoded is a boolean that is TRUE if value is base64 encoded
 * @returns a gchar * string that may be freed when no longer needed
 */
static gchar *get_argument_value_from_key(struct MHD_Connection *connection, gchar *key, gboolean encoded)
{
    const char *value = NULL;
    gchar *value_dup = NULL;
    gsize len = 0;

    if (connection != NULL && key != NULL)
        {
            value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);

            if (value != NULL)
                {
                    if (encoded == TRUE)
                        {
                            value_dup = (gchar *) g_base64_decode(value, &len);
                        }
                    else
                        {
                            value_dup = g_strdup(value);
                        }
                }
        }

    return value_dup;
}


/**
 * Function that gets a boolean argument corresponding to the key 'key' in the
 * url (from connection)
 * @param connection is the connection in MHD
 * @param key the key to look for into the url
 * @returns a gboolean that is TRUE if the Value contains 'True' and FALSE
 *          otherwise.
 */
static gboolean get_boolean_argument_value_from_key(struct MHD_Connection *connection, gchar *key)
{
    gchar *value = NULL;

    value = get_argument_value_from_key(connection, key, FALSE);

    if (g_strcmp0(value, "True") == 0)
        {
            free_variable(value);
            return TRUE;
        }
    else
        {
            free_variable(value);
            return FALSE;
        }
}


/**
 * Function to get a list of saved files.
 * @param server_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param url is the requested url
 * @returns a json formatted string or NULL
 */
static gchar *get_a_list_of_files(server_struct_t *server_struct, struct MHD_Connection *connection)
{
    gchar *answer = NULL;
    gchar *message = NULL;
    backend_t *backend = NULL;
    query_t *query = NULL;


    if (server_struct != NULL && server_struct->backend != NULL)
        {
            backend = server_struct->backend;

            if (backend->get_list_of_files != NULL)
                {
                    query = init_query_t(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, FALSE);

                    query->hostname = get_argument_value_from_key(connection, "hostname", FALSE);
                    query->uid = get_argument_value_from_key(connection, "uid", FALSE);
                    query->gid = get_argument_value_from_key(connection, "gid", FALSE);
                    query->owner = get_argument_value_from_key(connection, "owner", FALSE);
                    query->group = get_argument_value_from_key(connection, "group", FALSE);
                    query->filename = get_argument_value_from_key(connection, "filename", TRUE);
                    query->date = get_argument_value_from_key(connection, "date", TRUE);
                    query->afterdate = get_argument_value_from_key(connection, "afterdate", TRUE);
                    query->beforedate = get_argument_value_from_key(connection, "beforedate", TRUE);
                    query->latest = get_boolean_argument_value_from_key(connection, "latest");

                    print_debug(_("hostname: %s, uid: %s, gid: %s, owner: %s, group: %s, filter: %s && %s && %s && %s && %d\n"), \
                                   query->hostname, query->uid, query->gid, query->owner, query->group,                     \
                                   query->filename, query->date, query->afterdate, query->beforedate, query->latest);

                    if (query->hostname != NULL && query-> uid != NULL && query->gid != NULL && query->owner != NULL && query->group != NULL)
                        {
                            answer = backend->get_list_of_files(server_struct, query);
                        }
                    else
                        {
                            message = g_strdup_printf(_("Malformed request: hostname: %s, uid: %s, gid: %s, owner: %s, group: %s"), \
                                                        query->hostname, query->uid, query->gid, query->owner, query->group);
                            answer = answer_json_error_string(MHD_HTTP_BAD_REQUEST, message);
                            free_variable(message);
                        }

                    free_query_t(query); /** All variables hostname, uid... are freed there ! */
                }
            else
                {
                    message = g_strdup_printf(_("Error: no backend defined to get a list of files from it.\n"));
                    print_error(__FILE__, __LINE__, message);
                    answer = answer_json_error_string(MHD_HTTP_NOT_IMPLEMENTED, message);
                    free_variable(message);
                }
        }

    return answer;
}


/**
 * @param hash_data_list is a GList of hash_data_t structure.
 * @param size is a guint number that represents the sum of all 'read'
 *        fields in the hash_data_list. It will be the 'read' field
 *        of the returned hash_data_t.
 * @returns a newlly allocated hash_data_t structure filled with data
 *          from all data fields of the list and read is the sum of all
 *          read fields of the list.
 */
static hash_data_t *create_one_hash_data_t_from_hash_data_list(GList *hash_data_list, guint size)
{
    guchar *data = NULL;
    hash_data_t *hash_data = NULL;
    guint8 *binary = NULL;
    guint pos = 0;

    data = (guchar *) g_malloc(size);

    pos = 0;
    while (hash_data_list != NULL)
        {
            hash_data = hash_data_list->data;

            memcpy(data + pos, hash_data->data, hash_data->read);
            pos = pos + hash_data->read;

            hash_data_list = g_list_next(hash_data_list);
        }

    /**
     * We need a fake hash here in order to be able to make the json string
     * @todo may be we can manage to have the real hashs from the data
     */
    binary = (guint8 *) g_malloc(HASH_LEN);
    hash_data = new_hash_data_t(data, size, binary);

    return hash_data;
}


/**
 * Gets all data from a list of hash obtained from X-Get-Hash-Array HTTP
 * header.
 * @param server_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @returns a newlly allocated gchar * string that contains the anwser to be
 *          sent back to the client (hopefully a json string containing
 *          a hash (that is a fake one here), data and size of the data.
 */
static gchar *get_data_from_a_list_of_hashs(server_struct_t *server_struct, struct MHD_Connection *connection)
{
    const char *header = NULL;
    gchar *answer = NULL;
    gchar *hash = NULL;
    GList *head = NULL;
    GList *header_hdl = NULL;
    GList *hash_data_list = NULL;
    hash_data_t *header_hd = NULL;
    hash_data_t *hash_data = NULL;
    backend_t *backend = server_struct->backend;
    guint size = 0;
    a_clock_t *a_clock = NULL;


    a_clock = new_clock_t();
    header = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "X-Get-Hash-Array");
    header_hdl = make_hash_data_list_from_string((gchar *)header);
    end_clock(a_clock, "X-Get-Hash-Array retrieved in");

    a_clock = new_clock_t();
    head = header_hdl;
    while (header_hdl != NULL)
        {
            header_hd = header_hdl->data;
            hash = hash_to_string(header_hd->hash);
            hash_data = backend->retrieve_data(server_struct, hash);
            free_variable(hash);
            size = size + hash_data->read;
            hash_data_list = g_list_prepend(hash_data_list, hash_data);
            header_hdl = g_list_next(header_hdl);
        }
    end_clock(a_clock, "Read all files");

    a_clock = new_clock_t();
    g_list_free_full(head, free_hdt_struct);
    hash_data_list = g_list_reverse(hash_data_list);

    head = hash_data_list;

    hash_data = create_one_hash_data_t_from_hash_data_list(hash_data_list, size);

    g_list_free_full(head, free_hdt_struct);
    end_clock(a_clock, "Concatenated all data");

    a_clock = new_clock_t();
    answer = convert_hash_data_t_to_string(hash_data);
    free_hash_data_t(hash_data);
    end_clock(a_clock, "Transformed into a JSON string");

    return answer;
}

/**
 * Answers a json string containing all stats about the usage
 * of this server.
 * @param stats is the stats_t structure containing all stats
 *        to be returned.
 */
static gchar *answer_global_stats(stats_t *stats)
{
    json_t *root = NULL;
    json_t *get = NULL;
    json_t *post = NULL;
    json_t *unk = NULL;
    json_t *req = NULL;
    json_t *nbr = NULL;
    gchar *answer = NULL;

    if (stats != NULL && stats->requests != NULL && stats->requests->get != NULL && stats->requests->post != NULL && stats->requests->unknown != NULL)
        {
            root = json_object();

            get = make_json_from_stats("Total requests", stats->requests->get->nb_request);
            nbr = json_integer(stats->requests->get->stats);
            insert_json_value_into_json_root(get, "/Stats.json", nbr);

            post = make_json_from_stats("Total requests", stats->requests->post->nb_request);
            unk = make_json_from_stats("Total requests", stats->requests->unknown->nb_request);
            req = make_json_from_stats("Total requests", stats->requests->nb_request);
            insert_json_value_into_json_root(req, "GET", get);
            insert_json_value_into_json_root(req, "POST", post);
            insert_json_value_into_json_root(req, "Unknown", unk);
            insert_json_value_into_json_root(root, "Requests", req);

            nbr = json_integer(stats->nb_meta_bytes);
            insert_json_value_into_json_root(root, "metadata", nbr);

            nbr = json_integer(stats->nb_files);
            insert_json_value_into_json_root(root, "files", nbr);

            nbr = json_integer(stats->nb_total_bytes);
            insert_json_value_into_json_root(root, "total size", nbr);

            nbr = json_integer(stats->nb_dedup_bytes);
            insert_json_value_into_json_root(root, "dedup size", nbr);

            answer = json_dumps(root, 0);
        }

    return answer;
}


/**
 * Function to answer to get requests in a json way. This mode should be
 * prefered.
 * @param server_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param url is the requested url
 * @note to translators all json requests MUST NOT be translated because
 *       it is the protocol itself !
 * @returns a newlly allocated gchar * string that contains the anwser to be
 *          sent back to the client.
 */
static gchar *get_json_answer(server_struct_t *server_struct, struct MHD_Connection *connection, const char *url)
{
    gchar *answer = NULL;
    gchar *message = NULL;
    gchar *hash = NULL;
    size_t hlen = 0;

    if (g_strcmp0(url, "/Version.json") == 0)
        {
            answer = convert_version_to_json(PROGRAM_NAME, SERVER_DATE, SERVER_VERSION, SERVER_AUTHORS, SERVER_LICENSE);
        }
    else if (g_str_has_prefix(url, "/Stats.json"))
        {
            /* Answer a json string with stats on server's usage */
            add_one_to_get_url_statsjson(server_struct->stats);
            answer = answer_global_stats(server_struct->stats);
        }
    else if (g_str_has_prefix(url, "/File/List.json"))
        {
            answer = get_a_list_of_files(server_struct, connection);
        }
    else if (g_str_has_prefix(url, "/Data/Hash_Array.json"))
        {
            answer = get_data_from_a_list_of_hashs(server_struct, connection);
        }
    else if (g_str_has_prefix(url, "/Data/"))
        {
            hash = g_strndup((const gchar *) url + 6, HASH_LEN*2);  /* HASH_LEN is expressed when hash is in binary form  */
            hash = g_strcanon(hash, "abcdef0123456789", '\0');      /* replace anything not in hexadecimal format with \0 */

            hlen = strlen(hash);
            if (hlen == HASH_LEN*2)
                {
                    print_debug(_("Trying to get data for hash %s\n"), hash);
                    answer = get_data_from_a_specific_hash(server_struct, hash);
                }
            else
                {
                    message = g_strdup_printf(_("Invalid url: in %s hash has length: %zd instead of %d"), url, hlen, HASH_LEN*2);
                    answer = answer_json_error_string(MHD_HTTP_BAD_REQUEST, message);
                    free_variable(message);
                }

            free_variable(hash);
        }
    else
        { /* Some sort of echo to the invalid request */
            message = g_strdup_printf(_("URL not found: %s"), url);
            answer = answer_json_error_string(MHD_HTTP_NOT_FOUND, message);
            free_variable(message);
        }

    return answer;
}


/**
 * Function to answer to get requests in an unformatted way. Only some urls
 * May be like this. As we prefer to speak in json format in normal operation
 * mode
 * @param server_struct is the main structure for the server.
 * @param url is the requested url
 * @returns a newlly allocated gchar * string that contains the anwser to be
 *          sent back to the client.
 */
static gchar *get_unformatted_answer(server_struct_t *server_struct, const char *url)
{
    gchar *answer = NULL;
    gchar *buf1 = NULL;
    gchar *buf2 = NULL;
    gchar *buf3 = NULL;

    if (g_strcmp0(url, "/Version") == 0)
        {
            buf1 = buffer_program_version(PROGRAM_NAME, SERVER_DATE, SERVER_VERSION, SERVER_AUTHORS, SERVER_LICENSE);
            buf2 = buffer_libraries_versions(PROGRAM_NAME);
            buf3 = buffer_selected_option(server_struct->opt);

            answer = g_strconcat(buf1, buf2, buf3, NULL);

            buf1 = free_variable(buf1);
            buf2 = free_variable(buf2);
            buf3 = free_variable(buf3);
        }
    else
        { /* Some sort of echo to the invalid request */
            answer = g_strdup_printf(_("Error: invalid url: %s\n"), url);
        }

    return answer;
}


/**
 * Creates a response sent to the client via MHD_queue_response
 * @param connection is the MHD_Connection connection
 * @param answer is the gchar * string to be sent.
 * @param content_type is a gchar * string that represents the content
 *        type of the answer. It should be "text/plain; charset=utf-8"
 *        when answering to normal urls (not ending with .json) and
 *        "application/json; charset=utf-8" otherwise
 */
static int create_MHD_response(struct MHD_Connection *connection, gchar *answer, gchar *content_type)
{
    struct MHD_Response *response = NULL;
    int success = MHD_NO;

    response = MHD_create_response_from_buffer(strlen(answer), (void *) answer, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
    success = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return success;
}


/**
 * Function to process get requests received from clients.
 * @param server_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param url is the requested url
 * @param con_cls is a pointer used to know if this is the first call or not
 * @returns an int that is either MHD_NO or MHD_YES upon failure or not.
 */
static int process_get_request(server_struct_t *server_struct, struct MHD_Connection *connection, const char *url, void **con_cls)
{
    static int aptr = 0;
    int success = MHD_NO;
    gchar *answer = NULL;
    gchar *content_type;


    if (&aptr != *con_cls)
        {
            /* do never respond on first call */
            *con_cls = &aptr;

            success = MHD_YES;
        }
    else
        {
            if (get_debug_mode() == TRUE)
                {
                    print_debug(_("Requested get url: %s\n"), url);
                    print_headers(connection);
                }

            if (g_str_has_suffix(url, ".json"))
                { /* A json format answer was requested */
                    answer = get_json_answer(server_struct, connection, url);
                    content_type = CT_JSON;
                }
            else
                { /* An "unformatted" answer was requested */
                    answer = get_unformatted_answer(server_struct, url);
                    content_type = CT_PLAIN;
                }

                /* reset when done */
                *con_cls = NULL;

                /* Do not free answer variable as MHD will do it for us ! */
                success = create_MHD_response(connection, answer, content_type);
        }

    return success;

}


/**
 * Selects hashs that are needed by invoking the backend function if it
 * exists and returns a json array.
 * @param server_struct is the main structure for the server.
 * @param
 * @returns a json_t * array of needed hashs that may be freed when no
 *          longer needed.
 */
static json_t *find_needed_hashs(server_struct_t *server_struct, GList *hash_data_list)
{
    json_t *array = NULL;   /** json_t *array is the array that will receive base64 encoded needed hashs */
    GList *needed = NULL;   /** GList that contains needed hashs as answered by the backend if any       */

    /**
     * Creating a json_t * array with the hashs that are needed. If
     * the selected backend does not have a build_needed_hash_list
     * function we are returning the whole hash_data_list !
     */

    if (server_struct != NULL && server_struct->backend != NULL && server_struct->backend->build_needed_hash_list != NULL)
        {
            needed = server_struct->backend->build_needed_hash_list(server_struct, hash_data_list);
            array = convert_hash_list_to_json(needed);
            g_list_free_full(needed, free_hdt_struct);
        }
    else
        {
            array = convert_hash_list_to_json(hash_data_list);
        }

    return array;
}


/**
 * Answers /Meta.json POST request by storing data and answering to the
 * client.
 * @param server_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param received_data is a gchar * string to the data that was received
 *        by the POST request.
 */
static int answer_meta_json_post_request(server_struct_t *server_struct, struct MHD_Connection *connection, gchar *received_data)
{
    server_meta_data_t *smeta = NULL; /** server_meta_data_t *smeta stores meta data along with hostname of the client */
    gchar *answer = NULL;             /** gchar *answer : Do not free answer variable as MHD will do it for us !       */
    json_t *root = NULL;              /** json_t *root is the root that will contain all meta data json formatted      */
    json_t *array = NULL;             /** json_t *array is the array that will receive base64 encoded hashs            */
    size_t nb_bytes = 0;

    smeta = convert_json_to_smeta_data(received_data);

    if (smeta != NULL && smeta->meta != NULL)
        {   /* The convertion went well and smeta contains the meta data */

            nb_bytes = strlen(received_data);
            add_bytes_to_metadata_bytes(server_struct->stats, nb_bytes);
            add_one_saved_file(server_struct->stats);
            add_file_size_to_total_size(server_struct->stats, smeta->meta->size);
            print_debug(_("Received meta data (%zd bytes) for file %s\n"), nb_bytes, smeta->meta->name);

            if (smeta->data_sent == FALSE)
                {
                    array = find_needed_hashs(server_struct, smeta->meta->hash_data_list);
                }
            else
                {
                    array = json_array();
                }

            root = json_object();
            insert_json_value_into_json_root(root, "hash_list", array);
            answer = json_dumps(root, 0);
            json_decref(root);


            /**
             * Sending smeta data into the queue in order to be treated by
             * the corresponding thread. smeta is freed there and should not
             * be used after this "call" here.
             */
            g_async_queue_push(server_struct->meta_queue, smeta);
        }
    else
        {
            answer = answer_json_error_string(MHD_HTTP_INTERNAL_SERVER_ERROR, _("Error: could not convert json to metadata\n"));
        }

    return create_MHD_response(connection, answer, CT_JSON);
}


/**
 * Answers /Hash_Array.json POST request by answering to the client needed
 * hashs
 * @param server_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param received_data is a gchar * string to the data that was received
 *        by the POST request.
 */
static int answer_hash_array_post_request(server_struct_t *server_struct, struct MHD_Connection *connection, gchar *received_data)
{
    gchar *answer = NULL;         /** gchar *answer : Do not free answer variable as MHD will do it for us !  */
    json_t *root = NULL;          /** json_t *root is the root that will contain all meta data json formatted */
    json_t *array = NULL;         /** json_t *array is the array that will receive base64 encoded hashs       */
    GList *hash_data_list = NULL;
    gchar *content_type = NULL;

    if (server_struct != NULL && connection != NULL && received_data != NULL)
        {

            root = load_json(received_data);
            hash_data_list = extract_glist_from_array(root, "hash_list", TRUE);
            json_decref(root);

            print_debug(_("Received hash array of %zd bytes size\n"), strlen(received_data));

            array = find_needed_hashs(server_struct, hash_data_list);

            root = json_object();
            insert_json_value_into_json_root(root, "hash_list", array);
            answer = json_dumps(root, 0);
            json_decref(root);
            g_list_free_full(hash_data_list, free_hdt_struct);
            content_type = CT_JSON;
        }
    else
        {
            answer = answer_json_error_string(MHD_HTTP_INTERNAL_SERVER_ERROR, _("Error: could not convert json to metadata\n"));
            content_type = CT_PLAIN;
        }

    return create_MHD_response(connection, answer, content_type);
}




/**
 * Prints a debug information about received data for a specific hash.
 * @param hash is the binary representation of the hash obtained with
 *        the read data
 * @param read is the size of the data beeing read and leading to this
 *        hash
 */
static void print_received_data_for_hash(guint8 *hash, gssize read)
{
    gchar *encoded_hash = NULL;
    gchar *string_read = NULL;

    encoded_hash = g_base64_encode(hash, HASH_LEN);
    string_read = g_strdup_printf("%"G_GSSIZE_FORMAT, read);

    print_debug(_("Received data for hash: \"%s\" (%s bytes)\n"), encoded_hash, string_read);

    free_variable(string_read);
    free_variable(encoded_hash);
}


/**
 * Function that process the received data from the POST command and
 * answers to the client.
 * Here we may do something with this data (we may want to store it
 * somewhere).
 *
 * @param server_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param url is the requested url
 * @param received_data is a gchar * string to the data that was received
 *        by the POST request.
 */
static int process_received_data(server_struct_t *server_struct, struct MHD_Connection *connection, const char *url, gchar *received_data)
{
    gchar *answer = NULL;                   /** gchar *answer : Do not free answer variable as MHD will do it for us ! */
    int success = MHD_NO;
    gboolean debug = FALSE;
    hash_data_t *hash_data = NULL;
    json_t *root = NULL;
    GList *hash_data_list = NULL;
    GList *head = NULL;
    a_clock_t *elapsed = NULL;

    if (g_strcmp0(url, "/Meta.json") == 0 && received_data != NULL)
        {
            success = answer_meta_json_post_request(server_struct, connection, received_data);
        }
    else if (g_strcmp0(url, "/Hash_Array.json") == 0 && received_data != NULL)
        {
            success = answer_hash_array_post_request(server_struct, connection, received_data);
            /* Here we will try to answer which hashs are needed and then
             * send thoses hashs back in the answer
             */
        }
    else if (g_strcmp0(url, "/Data.json") == 0 && received_data != NULL)
        {

            hash_data = convert_string_to_hash_data(received_data);
            add_hash_size_to_dedup_bytes(server_struct->stats, hash_data);

            if (get_debug_mode() == TRUE)
                {
                    print_received_data_for_hash(hash_data->hash, hash_data->read);
                }

            /**
             * Sending received_data into the queue in order to be treated by
             * the corresponding thread. hash_data is freed by data_thread
             * and should not be used after this "call" here.
             */
            g_async_queue_push(server_struct->data_queue, hash_data);


            /**
             * creating an answer for the client to say that everything went Ok!
             */
            answer = answer_json_success_string(MHD_HTTP_OK, _("Ok!"));
            success = create_MHD_response(connection, answer, CT_PLAIN);
        }
    else if (g_strcmp0(url, "/Data_Array.json") == 0 && received_data != NULL)
        {
            elapsed = new_clock_t();
            root = load_json(received_data);
            end_clock(elapsed, "load_json");
            hash_data_list = extract_glist_from_array(root, "data_array", FALSE);
            head = hash_data_list;
            json_decref(root);
            debug = get_debug_mode();

            while (hash_data_list != NULL)
                {
                    hash_data = hash_data_list->data;
                    add_hash_size_to_dedup_bytes(server_struct->stats, hash_data);

                    if (debug == TRUE)
                        {
                            /* Only for debbugging ! */
                            print_received_data_for_hash(hash_data->hash, hash_data->read);
                        }

                    /** Sending hash_data into the queue. */
                    g_async_queue_push(server_struct->data_queue, hash_data);
                    hash_data_list = g_list_next(hash_data_list);
                }

            g_list_free(head);

            /**
             * creating an answer for the client to say that everything went Ok!
             */

            answer = answer_json_success_string(MHD_HTTP_OK, _("Ok!"));
            success = create_MHD_response(connection, answer, CT_PLAIN);
        }
    else
        {
            /* The url is unknown to the server and we can not process the request ! */
            print_error(__FILE__, __LINE__, "Error: invalid url: %s\n", url);
            answer = answer_json_error_string(MHD_HTTP_BAD_REQUEST, _("Invalid url!\n"));
            success = create_MHD_response(connection, answer, CT_PLAIN);
        }

    return success;
}


/**
 * @param connection is the connection in MHD
 * @returns in bytes the value (a guint) of the Content-Length: header
 */
static guint64 get_content_length(struct MHD_Connection *connection)
{
    const char *length = NULL;
    guint64 len = DEFAULT_SERVER_BUFFER_SIZE;

    length = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Content-Length");

    if (length != NULL)
        {
            /** @todo test len and return code for sscanf to validate a correct entry */
            if (sscanf(length, "%"G_GUINT64_FORMAT, &len) <= 0)
                {
                    print_error(__FILE__, __LINE__, _("Could not guess Content-Length header value: %s\n"), strerror(errno));
                    len = DEFAULT_SERVER_BUFFER_SIZE;
                }

            if (len > 4294967296)
                {
                    len = DEFAULT_SERVER_BUFFER_SIZE;
                }
        }

    return len;
}


/**
 * Function to process post requests.
 * @param server_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param url is the requested url
 * @param con_cls is a pointer used to know if this is the first call or not
 * @param upload_data is a char * pointer to the data being uploaded at this call
 * @param upload_size is a pointer to an size_t value that says how many data
 *        is to be copied from upload_data string.
 * @returns an int that is either MHD_NO or MHD_YES upon failure or not.
 */
static int process_post_request(server_struct_t *server_struct, struct MHD_Connection *connection, const char *url, void **con_cls, const char *upload_data, size_t *upload_data_size)
{
    int success = MHD_NO;
    upload_t *pp = (upload_t *) *con_cls;
    guint64 len = 0;

    /* print_debug("%ld, %s, %p\n", *upload_data_size, url, pp); */ /* This is for early debug only ! */

    if (pp == NULL)
        {
            /* print_headers(connection); */ /* Used for debugging */
            /* Initialzing the structure at first connection       */
            len = get_content_length(connection);
            pp = (upload_t *) g_malloc(sizeof(upload_t));
            pp->pos = 0;
            pp->buffer = g_malloc(sizeof(gchar) * (len + 1));  /* not using g_malloc0 here because it's 1000 times slower */
            pp->number = 0;
            *con_cls = pp;

            success = MHD_YES;
        }
    else if (*upload_data_size != 0)
        {
            /* Getting data whatever they are */
            memcpy(pp->buffer + pp->pos, upload_data, *upload_data_size);
            pp->pos = pp->pos + *upload_data_size;

            pp->number = pp->number + 1;

            *con_cls = pp;
            *upload_data_size = 0;

            success = MHD_YES;
        }
    else
        {
            /* reset when done */
            *con_cls = NULL;
            pp->buffer[pp->pos] = '\0';

            /* Do something with received_data */
            success = process_received_data(server_struct, connection, url, pp->buffer);

            free_variable(pp->buffer);
            free_variable(pp);
        }

    return success;
}


/**
 * Prints all keys-values pairs contained in an HTTP header.
 * @param cls cls may be NULL here (not used).
 * @param[in] kind is the MHD_ValueKind requested
 * @param[in] key is the key to be printed
 * @param[in] value is the value of the corresponding key
 *
 */
static int print_out_key(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
  fprintf(stdout, "\t%s: %s\n", key, value);

  return MHD_YES;
}


/**
 * Prints everything in the header of a connection
 * @param connection the connection that we want to print all headers.
 */
static void print_headers(struct MHD_Connection *connection)
{
    fprintf(stdout, _("Headers for this connection are:\n"));
    MHD_get_connection_values(connection, MHD_HEADER_KIND, &print_out_key, NULL);
}


/**
 * MHD_AccessHandlerCallback function that manages all connections requests
 * @param cls is the server_struct_t * server_struct main server
 *        structure.
 * @todo . free some memory where needed
 *       . manage errors codes
 */
static int ahc(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    server_struct_t *ahc_server_struct = (server_struct_t *) cls;
    int success = MHD_NO;


    if (g_strcmp0(method, "GET") == 0)
        {
            /* We have a GET method that needs to be processed */
            add_one_get_request(ahc_server_struct->stats);
            success = process_get_request(ahc_server_struct, connection, url, con_cls);
        }
    else if (g_strcmp0(method, "POST") == 0)
        {  /* We have a POST method that needs to be processed */
            add_one_post_request(ahc_server_struct->stats);
            success = process_post_request(ahc_server_struct, connection, url, con_cls, upload_data, upload_data_size);
        }
    else
        { /* not a GET nor a POST -> we do not know what to do ! */
            add_one_unknown_request(ahc_server_struct->stats);
            success = MHD_NO;
        }

    return success;
}


/**
 * Thread whose aim is to store meta-data according to the selected backend
 * @param data : server_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
static gpointer meta_data_thread(gpointer user_data)
{
    server_struct_t *server_struct = user_data;
    server_meta_data_t *smeta = NULL;

    if (server_struct != NULL && server_struct->meta_queue != NULL)
        {

            if (server_struct->backend != NULL && server_struct->backend->store_smeta != NULL)
                {

                    while (TRUE)
                        {
                            smeta = g_async_queue_pop(server_struct->meta_queue);

                            if (smeta != NULL && smeta->meta != NULL)
                                {
                                    print_debug(_("meta_data_thread: received from %s meta for file %s\n"), smeta->hostname, smeta->meta->name);
                                    server_struct->backend->store_smeta(server_struct, smeta);
                                    free_smeta_data_t(smeta);
                                }
                            else
                                {
                                    print_error(__FILE__, __LINE__, _("Error: received a NULL pointer.\n"));
                                }
                        }
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error: no meta data store backend defined, meta-data's thread terminating...\n"));
                }
        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error: unable to launch meta-data thread.\n"));
        }

    return NULL;
}


/**
 * Thread whose aim is to store data according to the selected backend
 * @param data : server_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
static gpointer data_thread(gpointer user_data)
{
    server_struct_t *dt_server_struct = user_data;
    hash_data_t *hash_data = NULL;

    if (dt_server_struct != NULL && dt_server_struct->meta_queue != NULL)
        {

            if (dt_server_struct->backend != NULL && dt_server_struct->backend->store_data != NULL)
                {

                    while (TRUE)
                        {
                            hash_data = g_async_queue_pop(dt_server_struct->data_queue);

                            if (hash_data != NULL)
                                {
                                    dt_server_struct->backend->store_data(dt_server_struct, hash_data);
                                }
                        }
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error: no data store backend defined, data's thread terminating...\n"));
                }

        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error while trying to launch data thread"));
        }

    return NULL;
}


/**
 * Installs signals traps in order to be able to close the program as
 * as cleanly as we can.
 * @param main_struct is the main structure of the program it must not
 *        be NULL;
 */
static void install_server_signal_traps(server_struct_t *server_struct)
{
    guint id_int = 0;
    guint id_term = 0;


    fprintf(stdout, "%s\n", __func__);

    if (server_struct != NULL)
        {
            id_int = g_unix_signal_add(SIGINT, int_signal_handler, server_struct);
            id_term = g_unix_signal_add(SIGTERM, int_signal_handler, server_struct);

            if (id_int <= 0 || id_term <= 0)
                {
                    print_error(__FILE__, __LINE__, _("Unable to add signal handlers\n"));
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
    server_struct_t *server_struct = NULL;  /** main structure for 'server' program.           */


    #if !GLIB_CHECK_VERSION(2, 36, 0)
        g_type_init();  /** g_type_init() is deprecated since glib 2.36 */
    #endif

    ignore_sigpipe(); /** into order to get libmicrohttpd portable */

    init_international_languages();

    server_struct = init_server_main_structure(argc, argv);

    if (server_struct != NULL && server_struct->opt != NULL && server_struct->backend != NULL)
        {
            server_struct->loop = g_main_loop_new(g_main_context_default(), FALSE);

            install_server_signal_traps(server_struct);

            /* Initializing the choosen backend by calling it's function */
            if (server_struct->backend->init_backend != NULL)
                {
                   server_struct->backend->init_backend(server_struct);
                }

            /* Before starting anything else, start the threads */
            server_struct->meta_thread = g_thread_new("meta-data", meta_data_thread, server_struct);
            server_struct->data_thread = g_thread_new("data", data_thread, server_struct);

            /* Starting the libmicrohttpd daemon */
            server_struct->d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG, server_struct->opt->port, NULL, NULL, &ahc, server_struct, MHD_OPTION_CONNECTION_MEMORY_LIMIT, (size_t) 131070 , MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120, MHD_OPTION_END);

            if (server_struct->d == NULL)
                {
                    print_error(__FILE__, __LINE__, _("Error while spawning libmicrohttpd daemon\n"));
                    return 1;
                }

            /**
             * main program stops here (until we exit main loop)
             * somewhere else in the program.
             */
            g_main_loop_run(server_struct->loop);

        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error: initialization failed.\n"));
        }

    return 0;
}

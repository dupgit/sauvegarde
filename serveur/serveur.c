/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    serveur.c
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
 * @file serveur.c
 * This file contains all the stuff for the serveur program of "Sauvegarde"
 * project. The aim of this program is to save every checksum and data and
 * meta data of every 'client' program that is connected to.
 */

#include "serveur.h"

static serveur_struct_t *init_serveur_main_structure(int argc, char **argv);
static gchar *get_unformatted_answer(serveur_struct_t *serveur_struct, const char *url);
static int process_get_request(serveur_struct_t *serveur_struct, struct MHD_Connection *connection, const char *url, void **con_cls);
static int process_post_request(serveur_struct_t *serveur_struct, struct MHD_Connection *connection, const char *url, void **con_cls, const char *upload_data, size_t *upload_data_size);
static int ahc(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls);
static gpointer meta_datas_thread(gpointer user_data);
static gpointer datas_thread(gpointer user_data);

/**
 * Inits main serveur's structure
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns a serveur_struct_t * structure that contains everything that is
 *          needed for 'serveur' program.
 */
static serveur_struct_t *init_serveur_main_structure(int argc, char **argv)
{
    serveur_struct_t *serveur_struct = NULL;  /** main structure for 'serveur' program. */

    serveur_struct = (serveur_struct_t *) g_malloc0(sizeof(serveur_struct_t));

    serveur_struct->opt = do_what_is_needed_from_command_line_options(argc, argv);
    serveur_struct->hashs = new_hash_struct();
    serveur_struct->d = NULL;            /* libmicrohttpd daemon pointer */
    serveur_struct->meta_queue = g_async_queue_new();
    serveur_struct->data_queue = g_async_queue_new();

    /* default backend */
    serveur_struct->backend = init_backend_structure(file_store_smeta, file_store_data, file_init_backend, build_needed_hash_list);

    return serveur_struct;
}


/**
 * Function that gets the data of a specific hash
 * @param serveur_struct is the main structure for the server.
 * @param hash is the hash of which we want the data
 * @returns a json formatted string.
 */
static gchar *get_data_from_a_specific_hash(serveur_struct_t *serveur_struct, gchar *hash)
{
    gchar *answer = NULL;

    answer = g_strdup_printf(_("Not yet implemented: %s\n"), hash);

    return answer;

}


/**
 * Function to get a list of all files saved
 * @param serveur_struct is the main structure for the server.
 * @returns a json formatted string or NULL
 */
static gchar *get_a_list_of_files(serveur_struct_t *serveur_struct)
{
    gchar *answer = NULL;

    answer = g_strdup_printf(_("Not yet implemented.\n"));

    return answer;
}


/**
 * Function to answer to get requests in a json way. This mode should be
 * prefered.
 * @param serveur_struct is the main structure for the server.
 * @param url is the requested url
 * @note to translators all json requests MUST NOT be translated because
 *       it is the protocol itself !
 * @returns a newlly allocated gchar * string that contains the anwser to be
 *          sent back to the client.
 */
static gchar *get_json_answer(serveur_struct_t *serveur_struct, const char *url)
{
    gchar *answer = NULL;
    gchar *hash = NULL;
    size_t hlen = 0;

    if (g_strcmp0(url, "/Version.json") == 0)
        {
            answer = convert_version_to_json(PROGRAM_NAME, SERVEUR_DATE, SERVEUR_VERSION, SERVEUR_AUTHORS, SERVEUR_LICENSE);
        }
    else if (g_strcmp0(url, "/File/List.json") == 0)
        {
            answer = get_a_list_of_files(serveur_struct);
        }
    else if (g_str_has_prefix(url, "/Data/"))
        {
            hash = g_strndup((const gchar *) url + 6, HASH_LEN*2);  /* HASH_LEN is expressed when hash is in binary form */
            hash = g_strcanon(hash, "abcdef0123456789", '\0');

            hlen = strlen(hash);
            if (hlen == HASH_LEN*2)
                {
                    answer = get_data_from_a_specific_hash(serveur_struct, hash);
                }
            else
                {
                    answer = g_strdup_printf("{\"Invalid url (%s) hash length\": %ld instead of %d}", url, hlen, HASH_LEN*2);
                }

            free_variable(hash);
        }
    else
        { /* Some sort of echo to the invalid request */
            answer = g_strdup_printf("{\"Invalid url\": %s}", url);
        }

    return answer;
}


/**
 * Function to answer to get requests in an unformatted way. Only some urls
 * May be like this. As we prefer to speak in json format in normal operation
 * mode
 * @param serveur_struct is the main structure for the server.
 * @param url is the requested url
 * @returns a newlly allocated gchar * string that contains the anwser to be
 *          sent back to the client.
 */
static gchar *get_unformatted_answer(serveur_struct_t *serveur_struct, const char *url)
{
    gchar *answer = NULL;
    gchar *buf1 = NULL;
    gchar *buf2 = NULL;
    gchar *buf3 = NULL;

    if (g_strcmp0(url, "/Version") == 0)
        {
            buf1 = buffer_program_version(PROGRAM_NAME, SERVEUR_DATE, SERVEUR_VERSION, SERVEUR_AUTHORS, SERVEUR_LICENSE);
            buf2 = buffer_libraries_versions(PROGRAM_NAME);
            buf3 = buffer_selected_option(serveur_struct->opt);

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
 * Function to process get requests received from clients.
 * @param serveur_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param url is the requested url
 * @param con_cls is a pointer used to know if this is the first call or not
 * @returns an int that is either MHD_NO or MHD_YES upon failure or not.
 */
static int process_get_request(serveur_struct_t *serveur_struct, struct MHD_Connection *connection, const char *url, void **con_cls)
{
    static int aptr = 0;
    int success = MHD_NO;
    gchar *answer = NULL;
    struct MHD_Response *response = NULL;


    if (&aptr != *con_cls)
        {
            /* do never respond on first call */
            *con_cls = &aptr;

            success = MHD_YES;
        }
    else
        {
            if (g_str_has_suffix(url, ".json"))
                { /* A json format answer was requested */
                    answer = get_json_answer(serveur_struct, url);
                }
            else
                { /* An "unformatted" answer was requested */
                    answer = get_unformatted_answer(serveur_struct, url);
                }

                /* reset when done */
                *con_cls = NULL;

                /* Do not free answer variable as MHD will do it for us ! */
                response = MHD_create_response_from_buffer(strlen(answer), (void *) answer, MHD_RESPMEM_MUST_FREE);
                success = MHD_queue_response(connection, MHD_HTTP_OK, response);

                MHD_destroy_response(response);

        }

    return success;

}


/**
 * Answers /Meta.json POST request by storing datas and answering to the
 * client.
 * @param serveur_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param received_data is a gchar * string to the data that was received
 *        by the POST request.
 */
static int answer_meta_json_post_request(serveur_struct_t *serveur_struct, struct MHD_Connection *connection, gchar *received_data)
{
    struct MHD_Response *response = NULL;
    serveur_meta_data_t *smeta = NULL;
    gchar *answer = NULL;                   /** gchar *answer : Do not free answer variable as MHD will do it for us ! */
    int success = MHD_NO;
    json_t *root = NULL;        /** json_t *root is the root that will contain all meta data json formatted */
    json_t *array = NULL;       /** json_t *array is the array that will receive base64 encoded hashs       */
    gchar *json_str = NULL;     /** gchar *json_str is the string to be returned at the end                 */
    GSList *needed = NULL;

    /* received_data is freed : do not reuse after this ! */
    smeta = convert_json_to_smeta_data(received_data);

    if (smeta != NULL && smeta->meta != NULL)
        {   /* The convertion went well and smeta contains the meta datas */

            print_debug(_("Received meta datas for file %s\n"), smeta->meta->name);

            /**
             * Creating an answer an sending the hashs that are needed. If
             * the selected backend does not have a build_needed_hash_list
             * function we are returning the whole hash_list !
             */
            root = json_object();

            if (serveur_struct != NULL && serveur_struct->backend != NULL && serveur_struct->backend->build_needed_hash_list != NULL)
                {
                    needed = serveur_struct->backend->build_needed_hash_list(serveur_struct, smeta->meta->hash_list);
                    array = convert_hash_list_to_json(needed);
                }
            else
                {
                    array = convert_hash_list_to_json(smeta->meta->hash_list);
                }

            insert_json_value_into_json_root(root, "hash_list", array);
            json_str = json_dumps(root, 0);
            json_decref(root);

            if (needed != NULL)
                {
                    needed = free_list(needed);
                }

            /**
             * Sending smeta datas into the queue in order to be treated by
             * the corresponding thread. smeta is freed there and should not
             * be used after this "call" here.
             */
            g_async_queue_push(serveur_struct->meta_queue, smeta);


            response = MHD_create_response_from_buffer(strlen(json_str), (void *) json_str, MHD_RESPMEM_MUST_FREE);
            success = MHD_queue_response(connection, MHD_HTTP_OK, response);
        }
    else
        {
            answer = g_strdup_printf(_("Error: could not convert json to metadata\n"));
            response = MHD_create_response_from_buffer(strlen(answer), (void *) answer, MHD_RESPMEM_MUST_FREE);
            success = MHD_queue_response(connection, MHD_HTTP_OK, response);
        }

    MHD_destroy_response(response);

    return success;

}


/**
 * Function that process the received data from the POST command and
 * answers to the client.
 * Here we may do something with this data (we may want to store it
 * somewhere).
 *
 * @param serveur_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param url is the requested url
 * @param received_data is a gchar * string to the data that was received
 *        by the POST request.
 */
static int process_received_data(serveur_struct_t *serveur_struct, struct MHD_Connection *connection, const char *url, gchar *received_data)
{
    struct MHD_Response *response = NULL;
    gchar *answer = NULL;                   /** gchar *answer : Do not free answer variable as MHD will do it for us ! */
    int success = MHD_NO;

    hash_data_t *hash_data = NULL;

    if (g_strcmp0(url, "/Meta.json") == 0 && received_data != NULL)
        {
            /* received_data is freed there (do not reuse after this call) */
            success = answer_meta_json_post_request(serveur_struct, connection, received_data);
        }
    else if (g_strcmp0(url, "/Data.json") == 0 && received_data != NULL)
        {
            /**
             * @note do we need to insert datas into an hash tree ?
             *
             * encoded_hash = insert_json_into_hash_tree(serveur_struct->hashs, received_data);
             * print_debug(_("Received data for hash: \"%s\"\n"), encoded_hash);
             * encoded_hash = free_variable(encoded_hash);
             */

            hash_data = convert_json_to_hash_data(received_data);
            received_data = free_variable(received_data);

            print_debug(_("Received data for hash: \"%s\" (%ld bytes)\n"), g_base64_encode(hash_data->hash, HASH_LEN), hash_data->read);

            /**
             * Sending received_data into the queue in order to be treated by
             * the corresponding thread. hash_data is freed by datas_thread
             * and should not be used after this "call" here.
             */
            g_async_queue_push(serveur_struct->data_queue, hash_data);


            /**
             * creating an answer for the client to say that everything went Ok!
             */
            answer = g_strdup_printf(_("Ok!"));
            response = MHD_create_response_from_buffer(strlen(answer), (void *) answer, MHD_RESPMEM_MUST_FREE);
            success = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
        }
    else
        {
            /* The url is unknown to the server and we can not process the request ! */
            received_data = free_variable(received_data);
            print_error(__FILE__, __LINE__, "Error: invalid url: %s\n", url);
            answer = g_strdup_printf(_("Error: invalid url!\n"));
            response = MHD_create_response_from_buffer(strlen(answer), (void *) answer, MHD_RESPMEM_MUST_FREE);
            success = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
        }

    return success;
}


/**
 * Function to process post requests.
 * @param serveur_struct is the main structure for the server.
 * @param connection is the connection in MHD
 * @param url is the requested url
 * @param con_cls is a pointer used to know if this is the first call or not
 * @param upload_data is a char * pointer to the data being uploaded at this call
 * @param upload_size is a pointer to an size_t value that says how many data
 *        is ti be copied from upload_data string.
 * @returns an int that is either MHD_NO or MHD_YES upon failure or not.
 */
static int process_post_request(serveur_struct_t *serveur_struct, struct MHD_Connection *connection, const char *url, void **con_cls, const char *upload_data, size_t *upload_data_size)
{
    int success = MHD_NO;
    gchar *pp = *con_cls;
    gchar *newpp = NULL;
    gchar *received_data = NULL;
    gchar *buf1 = NULL;



    /* print_debug("%ld, %s, %p\n", *upload_data_size, url, pp); */ /* This is for early debug only ! */

    if (pp == NULL)
        {
            /* Initialzing the structure at first connection */
            pp = g_strdup("");
            *con_cls = pp;

            success = MHD_YES;
        }
    else if (*upload_data_size != 0)
        {
            /* Getting datas whatever they are */
            buf1 = g_strndup(upload_data, *upload_data_size);
            newpp = g_strconcat(pp, buf1, NULL);
            buf1 = free_variable(buf1);
            pp = free_variable(pp);

            *con_cls = newpp;

            *upload_data_size = 0;

            success = MHD_YES;
        }
    else
        {
            /* reset when done */
            *con_cls = NULL;

            received_data = g_strdup(pp);
            pp = free_variable(pp);

            /* Do something with received_data */
            success = process_received_data(serveur_struct, connection, url, received_data);
        }

    return success;
}


/**
 * MHD_AccessHandlerCallback function that manages all connections requests
 * @param cls is the serveur_struct_t * serveur_struct main serveur
 *        structure.
 * @todo . free some memory where needed
 *       . manage errors codes
 */
static int ahc(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    serveur_struct_t *serveur_struct = (serveur_struct_t *) cls;
    int success = MHD_NO;


    if (g_strcmp0(method, "GET") == 0)
        {
            /* We have a GET method that needs to be processed */
            success = process_get_request(serveur_struct, connection, url, con_cls);
        }
    else if (g_strcmp0(method, "POST") == 0)
        {  /* We have a POST method that needs to be processed */
            success = process_post_request(serveur_struct, connection, url, con_cls, upload_data, upload_data_size);
        }
    else
        { /* not a GET nor a POST -> we do not know what to do ! */
            success = MHD_NO;
        }

    return success;
}


/**
 * Thread whose aim is to store meta-data according to the selected backend
 * @param data : serveur_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
static gpointer meta_datas_thread(gpointer user_data)
{
    serveur_struct_t *serveur_struct = user_data;
    serveur_meta_data_t *smeta = NULL;

    if (serveur_struct != NULL && serveur_struct->meta_queue != NULL)
        {

            if (serveur_struct->backend != NULL && serveur_struct->backend->store_smeta != NULL)
                {

                    while (TRUE)
                        {
                            smeta = g_async_queue_pop(serveur_struct->meta_queue);

                            if (smeta != NULL)
                                {
                                    print_debug("meta_data_thread: received from %s meta for file %s\n", smeta->hostname, smeta->meta->name);
                                    serveur_struct->backend->store_smeta(serveur_struct, smeta);
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
                    print_error(__FILE__, __LINE__, _("Error: no meta data store backend defined, meta-datas's thread terminating...\n"));
                }
        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error: unable to launch meta-data thread.\n"));
        }

    return NULL;
}


/**
 * Thread whose aim is to store datas according to the selected backend
 * @param data : serveur_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
static gpointer datas_thread(gpointer user_data)
{
    serveur_struct_t *serveur_struct = user_data;
    hash_data_t *hash_data = NULL;

    if (serveur_struct != NULL && serveur_struct->meta_queue != NULL)
        {

            if (serveur_struct->backend != NULL && serveur_struct->backend->store_data != NULL)
                {

                    while (TRUE)
                        {
                            hash_data = g_async_queue_pop(serveur_struct->data_queue);

                            if (hash_data != NULL)
                                {
                                    serveur_struct->backend->store_data(serveur_struct, hash_data);
                                }
                        }
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error: no data store backend defined, datas's thread terminating...\n"));
                }

        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error while trying to launch data thread"));
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
    serveur_struct_t *serveur_struct = NULL;  /** main structure for 'serveur' program.            */
    GThread *data_thread = NULL;              /** Thread that will take care of storing meta datas */
    GThread *meta_thread = NULL;              /** Thread that will take care of storing datas      */


    #if !GLIB_CHECK_VERSION(2, 36, 0)
        g_type_init();  /** g_type_init() is deprecated since glib 2.36 */
    #endif

    ignore_sigpipe(); /** into order to get libmicrohttpd portable */

    init_international_languages();

    serveur_struct = init_serveur_main_structure(argc, argv);


    if (serveur_struct != NULL && serveur_struct->opt != NULL && serveur_struct->backend != NULL)
        {
            /* Initializing the choosen backend by calling it's function */
            if (serveur_struct->backend->init_backend != NULL)
                {
                   serveur_struct->backend->init_backend(serveur_struct);
                }

            /* Before starting anything else, start the threads */
            meta_thread = g_thread_new("meta-datas", meta_datas_thread, serveur_struct);
            data_thread = g_thread_new("datas", datas_thread, serveur_struct);

            /* Starting the libmicrohttpd daemon */
            serveur_struct->d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG, serveur_struct->opt->port, NULL, NULL, &ahc, serveur_struct, MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120, MHD_OPTION_END);

            if (serveur_struct->d == NULL)
                {
                    print_error(__FILE__, __LINE__, _("Error while spawning libmicrohttpd daemon\n"));
                    return 1;
                }

            /* Unless on error we will never join the threads as they
             * contain a while (TRUE) loop !
             */
            g_thread_join(meta_thread);
            g_thread_join(data_thread);
            MHD_stop_daemon(serveur_struct->d);
        }
    else
        {
            print_error(__FILE__, __LINE__, _("Error: initialization failed.\n"));
        }

    return 0;
}

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
#define PAGE_NOT_FOUND "Error not found"

static serveur_struct_t *init_serveur_main_structure(int argc, char **argv);
static gchar *get_unformatted_answer(serveur_struct_t *serveur_struct, const char *url);
static int process_get_request(serveur_struct_t *serveur_struct, struct MHD_Connection *connection, const char *url, void **con_cls);
static int process_post_request(serveur_struct_t *serveur_struct, struct MHD_Connection *connection, const char *url, void **con_cls, const char *upload_data, size_t *upload_data_size);
static int ahc(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls);

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
    serveur_struct->d = NULL;

    return serveur_struct;
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

    if (g_strcmp0(url, "/Version.json") == 0)
        {
            answer = convert_version_to_json(PROGRAM_NAME, SERVEUR_DATE, SERVEUR_VERSION, SERVEUR_AUTHORS, SERVEUR_LICENSE);
        }
    else
        { /* Some sort of echo to the invalid request */
            answer = g_strdup_printf("{\"Invalid url\": %s\n}", url);
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
            answer = g_strdup_printf(_("Invalid url: %s\n"), url);
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
                { /* A "unformatted" answer was requested */
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
    gchar *answer = NULL;
    gchar *buf1 = NULL;
    struct MHD_Response *response = NULL;


    print_debug("%ld, %s, %p\n", *upload_data_size, url, pp);

    if (pp == NULL)
        {
            /* Initialzing the structure at first connection */
            pp = g_strdup("");
            *con_cls = pp;

            success = MHD_YES;
        }
    else if (*upload_data_size != 0)
        {

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
            print_debug("%ld : %s\n", strlen(received_data), received_data);

            pp = free_variable(pp);
            /* Do something with received_data */
            received_data = free_variable(received_data);

            /* Do not free answer variable as MHD will do it for us ! */
            answer = g_strdup_printf("Got it !\n");
            response = MHD_create_response_from_buffer(strlen(answer), (void *) answer, MHD_RESPMEM_MUST_FREE);
            success = MHD_queue_response(connection, MHD_HTTP_OK, response);

            MHD_destroy_response(response);
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
 * Main function
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns always 0
 * @todo do some real loop here.
 */
int main(int argc, char **argv)
{
    serveur_struct_t *serveur_struct = NULL;  /** main structure for 'serveur' program. */

    #if !GLIB_CHECK_VERSION(2, 36, 0)
        g_type_init();  /** g_type_init() is deprecated since glib 2.36 */
    #endif

    ignore_sigpipe(); /** into order to get libmicrohttpd portable */

    init_international_languages();

    serveur_struct = init_serveur_main_structure(argc, argv);


    if (serveur_struct != NULL && serveur_struct->opt != NULL)
        {
            /* Starting the libmicrohttpd daemon */
            serveur_struct->d = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG, serveur_struct->opt->port, NULL, NULL, &ahc, serveur_struct, MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120, MHD_OPTION_END);

            if (serveur_struct->d == NULL)
                {
                    print_error(__FILE__, __LINE__, _("Error while spawning libmicrohttpd daemon\n"));
                    return 1;
                }

            (void) getc(stdin);
            MHD_stop_daemon(serveur_struct->d);

                /*
                    msg_id = get_json_message_id(message);

                    switch (msg_id)
                        {
                            case ENC_META_DATA:

                                / ** message variable is freed in convert_json_to_smeta_data()
                                 *  function : no need to free it again elsewhere !
                                 *  Use of message variable is safe here because it is known
                                 *  not to be NULL
                                 * /

                                print_debug("Message of size %d received : %s\n", strlen(message), message);

                                smeta = convert_json_to_smeta_data(message);
                                smeta = free_smeta_data_t(smeta);

                            break;

                            case ENC_NOT_FOUND:
                            case ENC_END:
                                / ** We should never end the server with a
                                 *  message from a client !
                                 * /
                            break;
                        }
                */
        }

    return 0;
}

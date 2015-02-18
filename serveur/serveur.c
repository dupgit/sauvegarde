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
 * Iterator over key-value pairs where the value
 * maybe made available in increments and/or may
 * not be zero-terminated.  Used for processing
 * POST data.
 *
 * @param cls user-specified closure
 * @param kind type of the value
 * @param key 0-terminated key for the value
 * @param filename name of the uploaded file, NULL if not known
 * @param content_type mime-type of the data, NULL if not known
 * @param transfer_encoding encoding of the data, NULL if not known
 * @param data pointer to size bytes of data at the
 *              specified offset
 * @param off offset of data in the overall value
 * @param size number of bytes in data available
 * @return MHD_YES to continue iterating,
 *         MHD_NO to abort the iteration
 */
static int post_iterator(void *cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size)
{
    /* serveur_struct_t *serveur_struct = (serveur_struct_t *) cls; */


    fprintf(stdout, "size = %ld\noff = %ld\n", size, off);

    if (strcmp ("DONE", key) == 0)
        {
            fprintf(stdout, "Session terminated\n");
            return MHD_YES;
        }

    print_error(__FILE__, __LINE__, _("Unsupported form value '%s'\n"), key);

    return MHD_YES;
}



/**
 * MHD_AccessHandlerCallback function that manages all connections requests
 * @param cls is the serveur_struct_t * serveur_struct main serveur
 *        structure.
 */
static int ahc(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    static int aptr = 0;
    struct MHD_Response *response = NULL;
    int ret = 0;
    gchar *answer = NULL;
    gchar *buf1 = NULL;
    gchar *buf2 = NULL;
    gchar *buf3 = NULL;
    serveur_struct_t *serveur_struct = (serveur_struct_t *) cls;
    struct MHD_PostProcessor *pp = *con_cls;


    if (g_strcmp0(method, "GET") != 0)
        {
            if (g_strcmp0(method, "POST") != 0)
                {
                    /* not a GET nor a POST -> we do not know what to do ! */
                    return MHD_NO;
                }
            else
                {

                    /* POST request processing */
                    if (pp == NULL)
                        {
                            pp = MHD_create_post_processor(connection, 65536, post_iterator, serveur_struct);
                            *con_cls = pp;

                            return MHD_YES;
                        }

                    if (*upload_data_size)
                        {
                            fprintf(stdout, "%ld, %s, %s, %s, '%s'\n", *upload_data_size, url, method, version, upload_data);
                            MHD_post_process(pp, upload_data, *upload_data_size);
                            *upload_data_size = 0;

                            return MHD_YES;
                        }
                    else
                        {
                            /* reset when done */
                            *con_cls = NULL;

                            answer = g_strdup_printf("Got it !\n");

                            MHD_destroy_post_processor(pp);

                            /* Do not free answer variable as MHD will do it for us ! */
                            response = MHD_create_response_from_buffer(strlen(answer), (void *) answer, MHD_RESPMEM_MUST_FREE);
                            ret = MHD_queue_response(connection, MHD_HTTP_OK, response);

                            MHD_destroy_response(response);

                            return ret;
                        }
                }

            return MHD_NO;  /* unexpected method but we should never end here ! */
        }
    else
        {
            /* GET request processing */

            if (&aptr != *con_cls)
                {
                    /* do never respond on first call */
                    *con_cls = &aptr;

                    return MHD_YES;
                }

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
                {
                    answer = g_strdup_printf("%s\n", url);
                }

            /* reset when done */
            *con_cls = NULL;

            /* Do not free answer variable as MHD will do it for us ! */
            response = MHD_create_response_from_buffer(strlen(answer), (void *) answer, MHD_RESPMEM_MUST_FREE);
            ret = MHD_queue_response(connection, MHD_HTTP_OK, response);

            MHD_destroy_response(response);

            return ret;
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
                    print_error(__FILE__, __LINE__, _("Error while trying to spawn libmicrohttpd daemon\n"));
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

                                print_debug(stdout, "Message of size %d received : %s\n", strlen(message), message);

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

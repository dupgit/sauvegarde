/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    communique.c
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
 * @file communique.c
 * This file contains every call to the libcurl library in order to make a
 * wrapper to this library.
 *  * Defined errors are: http://curl.haxx.se/libcurl/c/libcurl-errors.html
 *  * How to use a buffer to get a message is at:
 *    http://curl.haxx.se/libcurl/c/CURLOPT_ERRORBUFFER.html
 */

#include "libsauvegarde.h"

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);


/**
 * Gets the version for the communication library
 * @returns a newly allocated string that contains the version and that
 *          may be freed with free_variable() when no longer needed.
 */
gchar *get_communication_library_version(void)
{
    curl_version_info_data *data = NULL;

    data = curl_version_info(CURLVERSION_NOW);

    return g_strdup_printf(_("\t. LIBCURL version: %s\n"), data->version);
}


/**
 * Makes the connexion string that is used by ZMQ to create a new socket
 * and verifies that port number is between 1025 and 65534 included.
 * @param ip : a gchar * that contains either an ip address or a hostname
 * @param port : a gint that is comprised between 1025 and 65534 included
 * @returns a newly allocated string that may be freed with free_variable()
 *          function.
 */
gchar *make_connexion_string(gchar *ip, gint port)
{
    /**
     * @todo check the ip string to be sure that it correspond to something that
     *       we can join (IP or hostname).
     */
    gchar *conn = NULL;

    if (ip != NULL && port > 1024 && port < 65535)
        {
            /* We must ensure that ip is correct before doing this ! */
            conn = g_strdup_printf("http://%s:%d", ip, port);
        }

    return conn;
}


/**
 * Used by libcurl to retrieve informations
 * @param buffer is the buffer where received data are written by libcurl
 * @param size is the size of an element in buffer
 * @param nmemb is the number of elements in buffer
 * @param[in,out] userp is a user pointer and MUST be a pointer to comm_t *
 *                structure
 * @returns should return the size of the data taken into account.
 *          Everything different from the size passed to this function is
 *          considered as an error by libcurl.
 */
static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    comm_t *comm = (comm_t *) userp;
    gchar *buf1 = NULL;
    gchar *concat = NULL;

    if (comm != NULL)
        {
            if (comm->seq == 0)
                {
                    comm->buffer = g_strndup(buffer, size * nmemb);
                }
            else
                {
                    buf1 = g_strndup(buffer, size * nmemb);
                    concat = g_strdup_printf("%s%s", comm->buffer, buf1);
                    free_variable(buf1);
                    free_variable(comm->buffer);
                    comm->buffer = concat;
                }

            comm->seq = comm->seq + 1;
        }

    return (size * nmemb);
}




/**
 * Used by libcurl to retrieve informations
 * @param buffer is the buffer where received data are written by libcurl
 * @param size is the size of an element in buffer
 * @param nitems is the number of elements in buffer
 * @param[in,out] userp is a user pointer and MUST be a pointer to comm_t *
 *                structure
 * @returns should return the size of the data taken into account.
 *          Everything different from the size passed to this function is
 *          considered as an error by libcurl.
 */
static size_t read_data(char *buffer, size_t size, size_t nitems, void *userp)
{
    comm_t *comm = (comm_t *) userp;
    size_t whole_size = 0;

    if (comm != NULL)
        {
            if (comm->pos >= comm->length)
                {
                    return 0;
                }
            else
                {
                    whole_size = size * nitems;

                    if ((comm->pos + whole_size) > comm->length)
                        {
                            whole_size = comm->length - comm->pos;
                            memcpy(buffer, comm->readbuffer + comm->pos, whole_size);
                            comm->pos = comm->length;
                            return (whole_size);
                        }
                    else
                        {
                            memcpy(buffer, comm->readbuffer + comm->pos, whole_size);
                            comm->pos = comm->pos + whole_size;
                            return whole_size;
                        }
                }
        }

    return 0;
}


/**
 * Uses curl to send a GET command to the http url
 * @param comm a comm_t * structure that must contain an initialized
 *        curl_handle (must not be NULL)
 * @param url a gchar * url where to send the command to. It must NOT
 *        contain the http://ip:port string. And must contain the first '/'
 *        ie to get 'http://127.0.0.1:5468/Version' url must be '/Version'.
 * @returns a CURLcode (http://curl.haxx.se/libcurl/c/libcurl-errors.html)
 *          CURLE_OK upon success, any other error code in any other
 *          situation. When CURLE_OK is returned, the data that the server
 *          sent is in the comm->buffer gchar * string.
 */
gint get_url(comm_t *comm, gchar *url)
{
    gint success = CURLE_FAILED_INIT;
    gchar *real_url = NULL;
    gchar *error_buf = NULL;

    if (comm != NULL && url != NULL && comm->curl_handle != NULL && comm->conn != NULL)
        {
            error_buf = (gchar *) g_malloc(CURL_ERROR_SIZE + 1);
            comm->seq = 0;
            comm->length = 0;
            comm->pos = 0;
            real_url = g_strdup_printf("%s%s", comm->conn, url);

            curl_easy_setopt(comm->curl_handle, CURLOPT_URL, real_url);
            curl_easy_setopt(comm->curl_handle, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(comm->curl_handle, CURLOPT_WRITEDATA, comm);
            curl_easy_setopt(comm->curl_handle, CURLOPT_ERRORBUFFER, error_buf);

            success = curl_easy_perform(comm->curl_handle);

            real_url = free_variable(real_url);

            if (success == CURLE_OK && comm->buffer != NULL)
                {
                    print_debug(_("Answer is: \"%s\"\n"), comm->buffer);
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error while sending GET command and receiving data: %s\n"), error_buf);
                }

            free_variable(error_buf);
        }

    return success;
}


/**
 * Uses curl to send a POST command to the http server url
 * @param comm a comm_t * structure that must contain an initialized
 *        curl_handle (must not be NULL). buffer field of this structure
 *        is sent as data in the POST command.
 * @param url a gchar * url where to send the command to. It must NOT
 *        contain the http://ip:port string. And must contain the first '/'
 *        ie to get 'http://127.0.0.1:5468/Version' url must be '/Version'.
 * @returns a CURLcode (http://curl.haxx.se/libcurl/c/libcurl-errors.html)
 *          CURLE_OK upon success, any other error code in any other
 *          situation. When CURLE_OK is returned, the data that the server
 *          sent is in the comm->buffer gchar * string.
 * @todo manage errors codes
 */
gint post_url(comm_t *comm, gchar *url)
{
    gint success = CURLE_FAILED_INIT;
    gchar *real_url = NULL;
    gchar *error_buf = NULL;
    gchar *len = NULL;
    struct curl_slist *chunk = NULL;

    if (comm != NULL && url != NULL && comm->curl_handle != NULL && comm->conn != NULL && comm->readbuffer != NULL)
        {
            error_buf = (gchar *) g_malloc(CURL_ERROR_SIZE + 1);
            comm->seq = 0;
            comm->pos = 0;
            real_url = g_strdup_printf("%s%s", comm->conn, url);

            comm->length = strlen(comm->readbuffer);

            curl_easy_reset(comm->curl_handle);
            curl_easy_setopt(comm->curl_handle, CURLOPT_POST, 1);
            curl_easy_setopt(comm->curl_handle, CURLOPT_READFUNCTION, read_data);
            curl_easy_setopt(comm->curl_handle, CURLOPT_READDATA, comm);
            curl_easy_setopt(comm->curl_handle, CURLOPT_URL, real_url);
            curl_easy_setopt(comm->curl_handle, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(comm->curl_handle, CURLOPT_WRITEDATA, comm);
            curl_easy_setopt(comm->curl_handle, CURLOPT_ERRORBUFFER, error_buf);
            /* curl_easy_setopt(comm->curl_handle, CURLOPT_VERBOSE, 1L); */
            chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");

            len = g_strdup_printf("Content-Length: %zd", comm->length);
            chunk = curl_slist_append(chunk, len);

            if (g_str_has_suffix(url, ".json"))
                {
                    chunk = curl_slist_append(chunk, "Content-Type: application/json");
                }
            else
                {
                    chunk = curl_slist_append(chunk, "Content-Type: text/plain");
                }

            curl_easy_setopt(comm->curl_handle, CURLOPT_HTTPHEADER, chunk);

            success = curl_easy_perform(comm->curl_handle);
            curl_slist_free_all(chunk);

            if (success != CURLE_OK)
                {
                    print_error(__FILE__, __LINE__, _("Error while sending POST command (to \"%s\"): %s\n"), real_url, error_buf);
                }
            else if (comm->buffer != NULL)
                {
                    print_debug(_("Answer is: \"%s\"\n"), comm->buffer); /** @todo  Not sure that we will need this debug information later */
                }

            free_variable(real_url);
            free_variable(error_buf);
            free_variable(len);
        }

    return success;
}


/**
 * Checks wether the server is alive or not and checks its version
 * @param comm a comm_t * structure that must contain an initialized
 *        curl_handle (must not be NULL).
 * @returns TRUE if the server is alive and has a correct version.
 *          FALSE otherwise
 */
gboolean is_serveur_alive(comm_t *comm)
{
    gint success = CURLE_FAILED_INIT;
    gchar *version = NULL;

    success = get_url(comm, "/Version.json");
    version = get_json_version(comm->buffer);

    free_variable(comm->buffer);

    if (success == CURLE_OK && version !=  NULL)
        {
            if (comm->conn != NULL)
                {
                    print_debug("Server (version %s) is alive at %s.\n", version, comm->conn);
                }
            else
                {
                    print_debug("Server (version %s) is alive.\n", version);
                }

            free_variable(version);
            return TRUE;
        }
    else
        {
            if (comm->conn != NULL)
                {
                    print_debug("Server is not alive (%s).\n", comm->conn);
                }
            else
                {
                    print_debug("Server is not alive.\n");
                }

            free_variable(version);
            return FALSE;
        }
}


/**
 * Creates a new communication comm_t * structure.
 * @param conn a gchar * connection string that should be some url like
 *        string : http://ip:port or http://servername:port
 * @returns a newly allocated comm_t * structure where sender and receiver
 *          are set to NULL.
 */
comm_t *init_comm_struct(gchar *conn)
{
    comm_t *comm = NULL;

    comm = (comm_t *) g_malloc0(sizeof(comm_t));

    comm->curl_handle = curl_easy_init();
    comm->buffer = NULL;
    comm->conn = conn;
    comm->readbuffer = NULL;
    comm->pos = 0;
    comm->length = 0;

    return comm;
}


/**
 * Frees and releases a comm_t * structure
 * @param comm a comm_t * structure to be freed
 */
void free_comm_t(comm_t *comm)
{
    if (comm != NULL)
        {
            curl_easy_cleanup(comm->curl_handle);
            free_variable(comm->buffer);
            free_variable(comm->conn);
            free_variable(comm);
        }
}

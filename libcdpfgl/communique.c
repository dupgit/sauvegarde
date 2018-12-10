/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    communique.c
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
 * @file communique.c
 * This file contains every call to the libcurl library in order to make a
 * wrapper to this library.
 *  * Defined errors are: http://curl.haxx.se/libcurl/c/libcurl-errors.html
 *  * How to use a buffer to get a message is at:
 *    http://curl.haxx.se/libcurl/c/CURLOPT_ERRORBUFFER.html
 */

#include "libcdpfgl.h"

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
static size_t read_data(char *buffer, size_t size, size_t nitems, void *userp);
static gboolean does_url_end_with_json(gchar *url);
static struct curl_slist *append_content_type_to_header(struct curl_slist *chunk, gchar *url);

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
 * @param srv_conf is a structure that contains ip and port needed to
 *                 make the connection string.
 * @returns a newly allocated string that may be freed with free_variable()
 *          function.
 */
gchar *make_connexion_string(srv_conf_t *srv_conf)
{
    /**
     * @todo check the ip string to be sure that it correspond to something that
     * we can join (IP or hostname) ?.
     */
    gchar *conn = NULL;

    if (srv_conf != NULL && srv_conf->ip != NULL && srv_conf->port > 1024 && srv_conf->port < 65535)
        {
            /* We must ensure that ip is correct before doing this ! */
            conn = g_strdup_printf("http://%s:%d", srv_conf->ip, srv_conf->port);
        }

    return conn;
}

/**
 * Copy len bytes of buffer 'buffer' to a newly allocated buffer
 * @param buffer is a gchar * string to be copied (it may be or not NULL
 *        terminated).
 * @param len is the number of bytes to copy.
 * @returns a newly allocated gchar * string that is buffer's copy and
 *          that may be freed when no longer needed
 */
static gchar *copy_buffer(void *buffer, size_t len)
{
    gchar *destbuffer = NULL;

    destbuffer = (gchar *) g_malloc(len + 1);
    g_assert_nonnull(destbuffer);
    if (buffer != NULL)
        {
            memcpy(destbuffer, buffer, len);
            /* mostly for text only to avoid extra buffer data */
            destbuffer[len] = '\0';
        }
    else
        {
            destbuffer[0] = '\0';
        }


    return destbuffer;
}


/**
 * Concatenates buffer with buf1: puts buf1 at 'pos' position of buffer
 * @param buffer is a gchar * string that may be NULL terminated or not
 * @param pos is the position where to put buf1 (should be length of buffer)
 * @param buf1 is a gchar * string that may be NULL terminated or not
 * @param len is the len of buf1 (at least the number of bytes to concatenate
 * @returns a newly allocated gchar * string that may be (or not) NULL
 *          terminated and that contains buffer followed by buf1.
 */
static gchar *concat_buffer(gchar *buffer, guint64 pos, gchar *buf1, size_t len)
{
    gchar *concat = NULL;

    concat = (gchar *) g_malloc(pos + len + 1);
    g_assert_nonnull(concat);
    if (buffer != NULL && buf1 != NULL)
        {
            memcpy(concat, buffer, pos);
            memcpy(concat + pos, buf1, len);

            /* mostly for text only to avoid extra buffer data */
            concat[pos + len] = '\0';
        }
    else if (buffer != NULL)
        {
            /* buf1 is NULL */
            memcpy(concat, buffer, pos);
            concat[pos] = '\0';
        }
    else if (buf1 != NULL)
        {
            /* buffer is NULL */
            memcpy(concat, buf1, len);
            concat[len] = '\0';
        }
    else
        {
            free_variable(concat);
            concat = NULL;
        }

    return concat;
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
    gchar *concat = NULL;

    if (comm != NULL)
        {
            if (comm->seq == 0)
                {
                    comm->buffer = copy_buffer(buffer, size * nmemb);
                    comm->pos = size * nmemb;
                }
            else
                {
                    concat = concat_buffer(comm->buffer, comm->pos, buffer, size * nmemb);
                    comm->pos = comm->pos + size * nmemb;

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
            else if (buffer != NULL)
                {
                    whole_size = size * nitems;

                    if ((comm->pos + whole_size) > comm->length)
                        {
                            whole_size = comm->length - comm->pos;
                            memcpy(buffer, comm->readbuffer + comm->pos, whole_size);
                            comm->pos = comm->length;
                            return whole_size;
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
 * @param url is the url to be checked (must not be NULL)
 * @returns true if the given url finishes with .json (before parameters)
 * and false otherwise
 */
static gboolean does_url_end_with_json(gchar *url)
{
    gchar **strings = NULL;

    if (url != NULL)
        {
            strings = g_strsplit(url, "?", 2);

            if (g_str_has_suffix(strings[0], ".json"))
                {
                    g_strfreev(strings);
                    return TRUE;
                }
            else
                {
                    g_strfreev(strings);
                    return FALSE;
                }
        }

    return FALSE;
}


/**
 * @param chunk is the list of chunk headers as defined by libcurl
 * @param url is the url to be checked must not be NULL
 * @returns the appended list containing a 'Content-Type' header that
 *          is application/json if the URL ends with .json and
 *          is text/plain otherwise
 */
static struct curl_slist *append_content_type_to_header(struct curl_slist *chunk, gchar *url)
{
    gchar *content_type = NULL;

    if (does_url_end_with_json(url))
        {
            content_type = g_strconcat("Content-Type: ", CT_JSON, NULL);
            chunk = curl_slist_append(chunk, content_type);
        }
    else
        {
            content_type = g_strconcat("Content-Type: ", CT_PLAIN, NULL);
            chunk = curl_slist_append(chunk, content_type);
        }

    free_variable(content_type);

    return chunk;
}


/**
 * Uses curl to send a GET command to the http url
 * @param comm a comm_t * structure that must contain an initialized
 *        curl_handle (must not be NULL)
 * @param url a gchar * url where to send the command to. It must NOT
 *        contain the http://ip:port string. And must contain the first '/'
 *        ie to get 'http://127.0.0.1:5468/Version' url must be '/Version'.
 * @param header may be a gchar * string containing an HTTP header that
 *        we want to pass into the HTTP GET request. If NULL no header
 *        will be added.
 * @returns a CURLcode (http://curl.haxx.se/libcurl/c/libcurl-errors.html)
 *          CURLE_OK upon success, any other error code in any other
 *          situation. When CURLE_OK is returned, the data that the server
 *          sent is in the comm->buffer gchar * string.
 */
gint get_url(comm_t *comm, gchar *url, gchar *header)
{
    gint success = CURLE_FAILED_INIT;
    gchar *real_url = NULL;
    gchar *error_buf = NULL;
    struct curl_slist *chunk = NULL;

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

            /* Setting header options */
            chunk = append_content_type_to_header(chunk, url);
            if (header != NULL)
                {
                    chunk = curl_slist_append(chunk, header);
                }
            curl_easy_setopt(comm->curl_handle, CURLOPT_HTTPHEADER, chunk);

            /* Performing the HTTP GET request */
            success = curl_easy_perform(comm->curl_handle);
            curl_slist_free_all(chunk);

            if (success == CURLE_OK && comm->buffer != NULL)
                {
                    print_debug(_("Answer length: %d\n"), strlen(comm->buffer));
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error while sending GET command and receiving data: %s\n"), error_buf);
                }

            free_variable(real_url);
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
    gchar *uncomp = NULL;
    gchar *readbuffer_orig = NULL;   /** A pointer to keep readbuffer one while sending compressed data */
    struct curl_slist *chunk = NULL;
    compress_t *cmpbuf = NULL;

    if (comm != NULL && url != NULL && comm->curl_handle != NULL && comm->conn != NULL && comm->readbuffer != NULL)
        {

            error_buf = (gchar *) g_malloc(CURL_ERROR_SIZE + 1);
            comm->seq = 0;
            comm->pos = 0;
            real_url = g_strdup_printf("%s%s", comm->conn, url);

            /* readbuffer here should be plain base64 encoded text */
            comm->uncomp_len = strlen(comm->readbuffer);

            if (comm->cmptype != COMPRESS_NONE_TYPE)
                {
                    /* Compress here */
                    cmpbuf = compress_buffer(comm->readbuffer, comm->cmptype);
                    comm->length = cmpbuf->len;
                    readbuffer_orig = comm->readbuffer;
                    comm->readbuffer =  cmpbuf->text;
                }
            else
                {
                    comm->length = strlen(comm->readbuffer);
                }

            curl_easy_reset(comm->curl_handle);
            curl_easy_setopt(comm->curl_handle, CURLOPT_POST, 1);
            curl_easy_setopt(comm->curl_handle, CURLOPT_READFUNCTION, read_data);
            curl_easy_setopt(comm->curl_handle, CURLOPT_READDATA, comm);
            curl_easy_setopt(comm->curl_handle, CURLOPT_URL, real_url);
            curl_easy_setopt(comm->curl_handle, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(comm->curl_handle, CURLOPT_WRITEDATA, comm);
            curl_easy_setopt(comm->curl_handle, CURLOPT_ERRORBUFFER, error_buf);
            /* curl_easy_setopt(comm->curl_handle, CURLOPT_VERBOSE, 1L); */

            /* Setting header options */
            if (comm->cmptype == COMPRESS_ZLIB_TYPE)
                {
                    curl_easy_setopt(comm->curl_handle, CURLOPT_POSTFIELDSIZE, comm->length);
                    chunk = curl_slist_append(chunk, "Content-Encoding: gzip");
                    uncomp = g_strdup_printf("%s: %zd", X_UNCOMPRESSED_CONTENT_LENGTH, comm->uncomp_len);
                    chunk = curl_slist_append(chunk, uncomp);
                }

            chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
            len = g_strdup_printf("Content-Length: %zd", comm->length);
            chunk = curl_slist_append(chunk, len);

            chunk = append_content_type_to_header(chunk, url);
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
            free_compress_t(cmpbuf);

            /* This function is called by the one that manages the local database
             * directly with the values that are in the database and a strlen() is
             * done at it's begining so the data in the database MUST be stlen()
             * compatible. That is to say that they MUST be stored untransformed.
             * So we get them as they were when entering this function.
             */
            if (comm->cmptype != COMPRESS_NONE_TYPE)
                {
                    comm->readbuffer = readbuffer_orig;
                    comm->length = comm->uncomp_len;
                }

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
gboolean is_server_alive(comm_t *comm)
{
    gint success = CURLE_FAILED_INIT;
    gchar *version = NULL;

    if (comm != NULL)
        {
            success = get_url(comm, "/Version.json", NULL);
            version = get_json_version(comm->buffer);

            free_variable(comm->buffer);

            if (success == CURLE_OK && version !=  NULL)
                {
                    if (comm->conn != NULL)
                        {
                            print_debug(_("Server (version %s) is alive at %s.\n"), version, comm->conn);
                        }
                    else
                        {
                            print_debug(_("Server (version %s) is alive.\n"), version);
                        }

                    free_variable(version);
                    return TRUE;
                }
            else
                {
                    if (comm->conn != NULL)
                        {
                            print_debug(_("Server is not alive (%s).\n"), comm->conn);
                        }
                    else
                        {
                            print_debug(_("Server is not alive.\n"));
                        }

                    free_variable(version);
                    return FALSE;
                }
        }
    return FALSE;
}


/**
 * Creates a new communication comm_t * structure.
 * @param conn a gchar * connection string that should be some url like
 *        string : http://ip:port or http://servername:port
 * @param cmptype is the compression type (according to compress.h)
 *        to be applied when communicating
 * @returns a newly allocated comm_t * structure where sender and receiver
 *          are set to NULL.
 */
comm_t *init_comm_struct(gchar *conn, gshort cmptype)
{
    comm_t *comm = NULL;

    comm = (comm_t *) g_malloc0(sizeof(comm_t));
    g_assert_nonnull(comm);

    comm->curl_handle = curl_easy_init();
    comm->buffer = NULL;
    comm->conn = g_strdup(conn);
    comm->readbuffer = NULL;
    comm->seq = 0;
    comm->pos = 0;
    comm->length = 0;
    comm->uncomp_len = 0;
    comm->cmptype = cmptype;

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
            free_variable(comm->readbuffer);
            free_variable(comm->conn);
            free_variable(comm);
        }
}


/**
 * Creates X-Get-Hash-Array HTTP header with the hash list
 * @param hash_extract is an hash_extract_t structure pointer that
 *        contains a pointer to the begining of the list and a gchar *
 *        hash_string that is a comma separated hash list string. Both
 *        values are [in,out] parameters for this function.
 * @param max is a gint that represents the maximum number of hashs to
 *        convert.
 * @returns the header
 */
gchar *create_x_get_hash_array_http_header(hash_extract_t *hash_extract, guint max)
{
    gchar *header = NULL;

    if (hash_extract != NULL)
        {
            header = g_strconcat(X_GET_HASH_ARRAY, ": ", convert_max_hashs_from_hash_list_to_gchar(hash_extract, max), NULL);
        }
    else
        {
            header = g_strconcat(X_GET_HASH_ARRAY, ": ", NULL);
        }

    return header;
}

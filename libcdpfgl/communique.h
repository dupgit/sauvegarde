/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    communique.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2016 Olivier Delhomme
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
 * @file communique.h
 * This file contains all definitions and functions calls to the libcurl
 * library
 */

#ifndef _COMMUNIQUE_H_
#define _COMMUNIQUE_H_


/**
 * @def X_GET_HASH_ARRAY
 * Defines header name string that will be inserted into the get request
 * in order to get a concatened array of hashs.
 */
#define X_GET_HASH_ARRAY ("X-Get-Hash-Array: ")


/**
 * @def CT_JSON
 * Defines the Content-Type HTTP header for JSON requests / answers
 *
 * @def CT_PLAIN
 * Defines the Content-Type HTTP header for plain text requests / answers
 */
#define CT_JSON ("application/json; charset=utf-8")
#define CT_PLAIN ("text/plain; charset=utf-8")


/**
 * @struct comm_t
 * @brief Structure that will contain everything needed to the
 *        communication layer.
 *
 * This structure contains everything needed for the communication layer.
 * Buffers in this structure is ok because it is not possible to use
 * one curl_handle into different threads.
 */
typedef struct
{
    CURL *curl_handle; /**< Curl easy handle for a connection                */
    gchar *buffer;     /**< Buffer to pass things from the callback function */
    gchar *readbuffer; /**< Buffer to be read                                */
    gchar *conn;       /**< Connexion string that should be http://ip:port   */
    guint seq;         /**< sequence number when receiving multiples parts   */
    guint64 pos;       /**< Position in readbuffer                           */
    size_t length;     /**< length of buffer                                 */
} comm_t;


/**
 * gets the version for the communication library (ZMQ for now)
 * @returns a newly allocated string that contains the version and that
 *          may be freed with free_variable() when no longer needed.
 */
extern gchar *get_communication_library_version(void);


/**
 * Makes the connexion string that is used by ZMQ to create a new socket
 * and verifies that port number is between 1025 and 65534 included.
 * @param ip : a gchar * that contains either an ip address or a hostname
 * @param port : a gint that is comprised between 1025 and 65534 included
 * @returns a newly allocated string that may be freed with free_variable()
 *          function.
 */
extern gchar *make_connexion_string(gchar *ip, gint port);


/**
 * Creates a new communication comm_t * structure.
 * @param conn a gchar * connection string that should be some url like
 *        string : http://ip:port or http://servername:port
 * @returns a newly allocated comm_t * structure where sender and receiver
 *          are set to NULL.
 */
extern comm_t *init_comm_struct(gchar *conn);


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
extern gint get_url(comm_t *comm, gchar *url, gchar *header);


/**
 * Uses curl to send a POST command to the http server url
 * @param comm a comm_t * structure that must contain an initialized
 *        curl_handle (must not be NULL). buffer field of this structure
 *        is send as data in the POST command.
 * @param url a gchar * url where to send the command to. It must NOT
 *        contain the http://ip:port string. And must contain the first '/'
 *        ie to get 'http://127.0.0.1:5468/Version' url must be '/Version'.
 * @returns a CURLcode (http://curl.haxx.se/libcurl/c/libcurl-errors.html)
 *          CURLE_OK upon success, any other error code in any other
 *          situation. When CURLE_OK is returned, the data that the server
 *          sent is in the comm->buffer gchar * string.
 */
extern gint post_url(comm_t *comm, gchar *url);


/**
 * Checks wether the server is alive or not and checks its version
 * @param comm a comm_t * structure that must contain an initialized
 *        curl_handle (must not be NULL).
 * @returns TRUE if the server is alive and has a correct version.
 *          FALSE otherwise
 */
extern gboolean is_server_alive(comm_t *comm);


/**
 * Frees and releases a comm_t *structure
 * @param comm a comm_t * structure to be freed
 */
extern void free_comm_t(comm_t *comm);


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
extern gchar *create_x_get_hash_array_http_header(hash_extract_t *hash_extract, guint max);

#endif /* #ifndef _COMMUNIQUE_H_ */

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    communique.h
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
 * @file communique.h
 * This file contains all definitions and functions calls to the zmq
 * library
 */

#ifndef _COMMUNIQUE_H_
#define _COMMUNIQUE_H_


/**
 * @def MAX_MESSAGE_SIZE
 * Defines the maximum message size that we will be able to send or receive
 * default is 131072
 */
#define MAX_MESSAGE_SIZE (131072)

/**
 * @struct comm_t
 * @brief Structure that will contain everything needed to the
 *        communication layer.
 */
typedef struct
{
    CURL *curl_handle; /**< Curl easy handle for a connection                */
    gchar *buffer;     /**< Buffer to pass things from the callback function */
    gchar *conn;       /**< Connexion string that should be http://ip:port   */
    gint seq;          /**< sequence number when receiving multiples parts   */
} comm_t;



/**
 * gets the version for the communication library (ZMQ for now)
 * @returns a newly allocated string that contains the version and that
 *          may be freed with free_variable() when no longer needed.
 */
extern gchar *get_communication_library_version(void);


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
 * @param url a gchar * url where to send the command to (must not be NULL)
 * @returns a CURLcode (http://curl.haxx.se/libcurl/c/libcurl-errors.html)
 *          CURLE_OK upon success, any other error code in any other
 *          situation. When CURLE_OK is returned, the datas that the server
 *          sent is in the comm->buffer gchar * string.
 */
extern gint get_url(comm_t *comm, gchar *url);


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
 *          situation. When CURLE_OK is returned, the datas that the server
 *          sent is in the comm->buffer gchar * string.
 */
extern gint post_url(comm_t *comm, gchar *url);

/**
 * Checks wether the serveur is alive or not and checks its version
 * @param comm a comm_t * structure that must contain an initialized
 *        curl_handle (must not be NULL).
 * @returns TRUE if the serveur is alive and has a correct version.
 *          FALSE otherwise
 */
extern gboolean is_serveur_alive(comm_t *comm);

/**
 * This function sends the datas that corresponds to the hashs in the json
 * formatted string's answer.
 * @param comm a comm_t * structure that must contain an initialized
 *             curl_handle (must not be NULL). Buffer field of this
 *             structure is sent as data in the POST command.
 * @param hashs is the hash structure that contains the binary tree.
 * @param answer is the answer of the serveur containing a json formatted
 *        hash list.
 * @returns a CURLcode (http://curl.haxx.se/libcurl/c/libcurl-errors.html)
 *          CURLE_OK upon success of whole hash's transmissions, any other
 *          error code in any other situation.
 */
extern gint send_datas_to_server(comm_t *comm, hashs_t *hashs, gchar *answer);

#endif /* #ifndef _COMMUNIQUE_H_ */

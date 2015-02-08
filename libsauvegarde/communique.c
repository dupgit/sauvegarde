/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    communique.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 Olivier Delhomme
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
 * This file contains every call to the zmq library in order to make a
 * wrapper to this library
 */

#include "libsauvegarde.h"

static comm_t *init_comm_struct(void);
static void create_new_sender(comm_t *comm, int socket_type);
static void create_new_receiver(comm_t *comm, int socket_type);
static void connect_socket_somewhere(void *socket, gchar *somewhere);
static void bind_socket_somewhere(void *socket, gchar *somewhere);


/**
 * Gets the version for the communication library
 * @returns a newly allocated string that contains the version and that
 *          may be freed with free_variable() when no longer needed.
 */
gchar *get_communication_library_version(void)
{
    curl_version_info_data *data = NULL;

    data = curl_version_info(CURLVERSION_NOW);

    return g_strdup_printf(_("\t. LIBCURL version : %s\n"), data->version);
}


/**
 * Creates a new communication comm_t * structure.
 * @returns a newly allocated comm_t * structure where sender and receiver
 *          are set to NULL.
 */
static comm_t *init_comm_struct(void)
{
    comm_t *comm = NULL;

    comm = (comm_t *) g_malloc0(sizeof(comm_t));

    comm->sender = NULL;
    comm->receiver = NULL;

    return comm;
}


/**
 * Creates a new sender and set sender field accordingly
 * @param[in,out] comm : an allocated comm_t struct.
 * @param socket_type : type of socket (from libzmq) we want to create.
 */
static void create_new_sender(comm_t *comm, int socket_type)
{
    if (comm != NULL)
        {
            comm->sender = zsock_new(socket_type);
        }
}


/**
 * Creates a new receiver and set receiver field accordingly
 * @param[in,out] comm : an allocated comm_t struct.
 * @param socket_type : type of socket (from libzmq) we want to create.
 */
static void create_new_receiver(comm_t *comm, int socket_type)
{
    if (comm != NULL)
        {
            comm->receiver = zsock_new(socket_type);
        }
}


/**
 * Connects a socket somewhere eg : tcp://localhost:5468
 * @param socket is the socket to be connected to somewhere
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp://localhost:5468" or "tcp://10.1.1.60:3128"...
 */
static void connect_socket_somewhere(void *socket, gchar *somewhere)
{
    if (socket != NULL && somewhere != NULL)
        {
            zsock_connect(socket, somewhere);
        }
}


/**
 * Binds a socket somewhere eg : tcp:// *:5468
 * @param socket is the socket to be bind from somewhere
 * @param somewhere is the string that will define the connection we want
 *        to be bind on.
 */
static void bind_socket_somewhere(void *socket, gchar *somewhere)
{
    if (socket != NULL && somewhere != NULL)
        {
            zsock_bind(socket, somewhere);
        }
}


/**
 * Creates a new socket to send messages and connects it to "somewhere"
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp://localhost:5468" or "tcp://10.1.1.60:3128"...
 * @param socket_type : type of socket (from libzmq) we want to create.
 * @returns  a newly allocated comm_t * structure where context should not
 *           be NULL and sender should not be null but receiver is set to
 *           NULL.
 */
comm_t *create_sender_socket(gchar *somewhere, int socket_type)
{
    comm_t *comm = NULL;

    if (somewhere != NULL)
        {
            comm = init_comm_struct();
            create_new_sender(comm, socket_type);
            connect_socket_somewhere(comm->sender, somewhere);
        }

    return comm;

}


/**
 * Creates a new socket to receive messages and binds it to "somewhere"
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp:// *:5468" for instance.
 * @param socket_type : type of socket (from libzmq) we want to create.
 * @returns  a newly allocated comm_t * structure where context should not
 *           be NULL and receiver should not be null but sender is set to
 *           NULL.
 */
comm_t *create_receiver_socket(gchar *somewhere, int socket_type)
{
    comm_t *comm = NULL;

    if (somewhere != NULL)
        {
            comm = init_comm_struct();
            create_new_receiver(comm, socket_type);
            bind_socket_somewhere(comm->receiver, somewhere);
        }

    return comm;
}


/**
 * transforms the buffer into a gchar * message.
 * @param buffer is the char * buffer that we gets from the zstr_* functions
 * @returns a gchar * string if buffer is not NULL and strlen could be
 *          determined properly.
 */
static gchar *get_message_from_buffer(char *buffer)
{
    gint size = 0;
    gchar *message = NULL;

    if (buffer != NULL)
        {
            size = strlen(buffer);

            if (size > 0)
                {
                    message = g_strdup(buffer);
                }
        }

    return message;

}


/**
 * Sends a message throught sender socket
 * @param comm : the communication structure that handles sockets. sender
 *        field is the one used to send message.
 * @param message is a guchar * message to be sent.
 * @returns size of the message sent. 0 may be returned if comm or message
 *          are NULL.
 */
gint send_message(comm_t *comm, gchar *message)
{
    char *msg = NULL;
    char *buffer = NULL;
    gchar *answer = NULL;
    gint size = 0;

    if  (comm != NULL && message != NULL)
        {
            /* Sending the message */
            msg = g_strdup(message);
            size = zstr_send(comm->sender, msg);

            return size;
        }
    else
        {
            return 0;
        }
}


/**
 * Waits to receive a message.
 * @param comm : the communication structure that handles sockets. receiver
 *        field is the one used to receive message.
 * @returns a newly allocated guchar * message that can be freed when no
 *          longer needed.
 */
gchar *receive_message(comm_t *comm)
{
    size_t size = 0;
    char *buffer = NULL;
    gchar *message = NULL;


    if  (comm != NULL)
        {
            buffer = zstr_recv(comm->receiver);
            message = get_message_from_buffer(buffer);

            buffer = free_variable(buffer);
        }

    return message;
}

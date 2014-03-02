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

static comm_t *create_new_context(void);
static void create_new_push_sender(comm_t *comm);
static void connect_socket_somewhere(void *socket, gchar *somewhere);


/**
 * gets the version for the communication library (ZMQ for now)
 * @returns a newly allocated string that contains the version and that
 *          may be freed with g_free when no longer needed.
 */
gchar *get_communication_library_version(void)
{
    int zmq_major = 0;
    int zmq_minor = 0;
    int zmq_patch = 0;

    zmq_version(&zmq_major, &zmq_minor, &zmq_patch);

    return g_strdup_printf(_("\t. ZMQ version : %d.%d.%d\n"), zmq_major, zmq_minor, zmq_patch);
}


/**
 * Creates a new communication context within ZMQ
 * @returns a newly allocated comm_t * structure where context should not
 *          be NULL. sender and receiver are set to NULL.
 */
static comm_t *create_new_context(void)
{
    comm_t *comm = NULL;

    comm = (comm_t *) g_malloc0(sizeof(comm_t));

    if (comm != NULL)
        {
            comm->context = zmq_ctx_new();
        }

    comm->sender = NULL;
    comm->receiver = NULL;

    return comm;
}


/**
 * Creates a new PUSH sender and set sender field accordingly
 * @param comm : an allocated comm_t struct where context field is expected
 *               to be not NULL.
 */
static void create_new_push_sender(comm_t *comm)
{
    if (comm != NULL && comm->context != NULL)
        {
            comm->sender = zmq_socket(comm->context, ZMQ_PUSH);
        }
}


/**
 * Creates a new PULL receiver and set receiver field accordingly
 * @param comm : an allocated comm_t struct where context field is expected
 *               to be not NULL.
 */
static void create_new_pull_receiver(comm_t *comm)
{
    if (comm != NULL && comm->context != NULL)
        {
            comm->receiver = zmq_socket(comm->context, ZMQ_PULL);
        }
}


/**
 * Connects a socket somewhere eg : tcp://localhost:5558
 * @param socket is the socket to be connected to somewhere
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp://localhost:5468" or "tcp://10.1.1.60:3128"...
 */
static void connect_socket_somewhere(void *socket, gchar *somewhere)
{
    if (socket != NULL && somewhere != NULL)
        {
            zmq_connect(socket, somewhere);
        }
}


/**
 * Binds a socket somewhere eg : tcp:// *:5558
 * @param socket is the socket to be bind from somewhere
 * @param somewhere is the string that will define the connection we want
 *        to be bind on.
 */
static void bind_socket_somewhere(void *socket, gchar *somewhere)
{
    if (socket != NULL && somewhere != NULL)
        {
            zmq_bind(socket, somewhere);
        }
}


/**
 * Creates and connects a new PUSH socket to somewhere
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp://localhost:5468" or "tcp://10.1.1.60:3128"...
 * @returns  a newly allocated comm_t * structure where context should not
 *           be NULL and sender should not be null but receiver is set to
 *           NULL.
 */
comm_t *create_push_socket(gchar *somewhere)
{
    comm_t *comm = NULL;

    if (somewhere != NULL)
        {
            comm = create_new_context();
            create_new_push_sender(comm);
            connect_socket_somewhere(comm->sender, somewhere);
        }

    return comm;

}


/**
 * Creates and binds a new PULL socket to somewhere
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp:// *:5468" for instance.
 * @returns  a newly allocated comm_t * structure where context should not
 *           be NULL and receiver should not be null but sender is set to
 *           NULL.
 */
comm_t *create_pull_socket(gchar *somewhere)
{
    comm_t *comm = NULL;

    if (somewhere != NULL)
        {
            comm = create_new_context();
            create_new_pull_receiver(comm);
            bind_socket_somewhere(comm->receiver, somewhere);
        }

    return comm;
}


/**
 * Sends a message throught sender socket
 * @param comm : the communication structure that handles sockets. sender
 *        field is the one used to send message.
 * @param message is a gchar * message to be sent.
 * @returns size of the message sent. 0 may be returned if comm or message
 *          are NULL.
 */
gint send_message(comm_t *comm, gchar *message)
{
    gint size = 0;

    if  (comm != NULL && message != NULL)
        {
            size = zmq_send(comm->sender, message, strlen(message), 0);
        }

    return size;
}


/**
 * Waits to receive a message.
 * @param comm : the communication structure that handles sockets. receiver
 *        field is the one used to receive message.
 * @returns a newly allocated gchar * message that can be freed when no
 *          longer needed.
 */
gchar *receive_message(comm_t *comm)
{
    gint size = 0;
    char buffer[MAX_MESSAGE_SIZE + 1]; /* +1 to be sure to be able to have a final trailing \0x0 ! */

    gchar *message = NULL;

    if  (comm != NULL)
        {
             size = zmq_recv(comm->receiver, buffer, MAX_MESSAGE_SIZE, 0);

             if (size != -1)
                {
                    message = g_strndup(buffer, size); /* g_strndup adds the trailing \0x0 */
                }
        }

    return message;
}




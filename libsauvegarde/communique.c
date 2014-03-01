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
comm_t *create_new_context(void)
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
void create_new_push_sender(comm_t *comm)
{
    if (comm != NULL && comm->context != NULL)
        {
            comm->sender = zmq_socket(comm->context, ZMQ_PUSH);
        }
}


/**
 * Connects a socket somewhere eg : tcp://localhost:5558
 * @param socket is the socket to be connect to somewhere
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp://localhost:5468" or "tcp://10.1.1.60:3128"...
 */
void connect_socket_somewhere(void *socket, gchar *somewhere)
{
    if (socket != NULL && somewhere != NULL)
        {
            zmq_connect(socket, somewhere);
        }
}



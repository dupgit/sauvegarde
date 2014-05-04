/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    communique.h
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
 * @file communique.h
 * This file contains all definitions and functions calls to the zmq
 * library
 */

#ifndef _COMMUNIQUE_H_
#define _COMMUNIQUE_H_


/**
 * @def MAX_MESSAGE_SIZE
 * Defines the maximum message size that we will be able to send or receive
 * default is 32768
 */
#define MAX_MESSAGE_SIZE (32768)

/**
 * @struct comm_t
 * @brief Structure that will contain everything needed to the
 *        communication layer.
 */
typedef struct
{
   void *context;   /**< context to be used by the functions calling ZMQ */
   void *sender;    /**< "socket" for the sender                         */
   void *receiver;  /**< "socket" for the receiver                       */
} comm_t;



/**
 * gets the version for the communication library (ZMQ for now)
 * @returns a newly allocated string that contains the version and that
 *          may be freed with g_free when no longer needed.
 */
extern gchar *get_communication_library_version(void);


/**
 * Creates and connects a new typesocket socket to somewhere
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp://localhost:5468" or "tcp://10.1.1.60:3128"...
 * @returns  a newly allocated comm_t * structure where context should not
 *           be NULL and sender should not be null but receiver is set to
 *           NULL.
 */
extern comm_t *create_push_socket(gchar *somewhere);


/**
 * Creates and connects a new PULL socket to somewhere
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp://localhost:5468" or "tcp://10.1.1.60:3128"...
 * @returns  a newly allocated comm_t * structure where context should not
 *           be NULL and receiver should not be null but sender is set to
 *           NULL.
 */
extern comm_t *create_pull_socket(gchar *somewhere);


/**
 * Sends a message throught sender socket
 * @param comm : the communication structure that handles sockets. sender
 *        field is the one used to send message.
 * @param message is a gchar * message to be sent.
 * @param size is the size of message buffer to be sent.
 * @returns size of the message sent. 0 may be returned if comm or message
 *          are NULL.
 */
extern gint send_message(comm_t *comm, guchar *message, gint size);


/**
 * Waits the arrival of a message
 * @param comm : the communication structure that handles sockets. receiver
 *        field is the one used to receive message.
 * @returns a newly allocated guchar * message that can be freed when no
 *         longer needed.
 */
extern guchar *receive_message(comm_t *comm);


/**
 * Sends a packed message (into buffer) and frees memory
 * @param comm is the structure that stores sockets
 * @param buffer is the packed buffer to transmit to the wire
 */
gint send_packed_message(comm_t *comm, msgpack_sbuffer *buffer);



#endif /* #ifndef _COMMUNIQUE_H_ */

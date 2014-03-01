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
 * @struct comm_t
 * Structure that will contain everything needed to the communication layer.
 */
typedef struct
{
   void *context;   /** context to be used by the functions calling ZMQ */
   void *sender;    /** "socket" for the sender                         */
   void *receiver;  /** "socket" for the receiver                       */
} comm_t;



/**
 * gets the version for the communication library (ZMQ for now)
 * @returns a newly allocated string that contains the version and that
 *          may be freed with g_free when no longer needed.
 */
extern gchar *get_communication_library_version(void);


/**
 * Creates a new communication context within ZMQ
 * @returns a newly allocated comm_t * structure where context should not
 *          be NULL. sender and receiver are set to NULL.
 */
extern comm_t *create_new_context(void);


/**
 * Creates a new PUSH sender and set sender field accordingly
 * @param comm : an allocated comm_t struct where context field is expected
 *               to be not NULL.
 */
void create_new_push_sender(comm_t *comm);


/**
 * Connects a socket somewhere eg : tcp://localhost:5558
 * @param socket is the socket to be connect to somewhere
 * @param somewhere is the string that will define the connection we want
 *        eg "tcp://localhost:5468" or "tcp://10.1.1.60:3128"...
 */
void connect_socket_somewhere(void *socket, gchar *somewhere);


#endif /* #ifndef _COMMUNIQUE_H_ */

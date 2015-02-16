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
    CURL *curl_handle;  /**< Curl easy handle for a connection                */
    gchar *buffer;      /**< Buffer to pass things from the callback function */
} comm_t;



/**
 * gets the version for the communication library (ZMQ for now)
 * @returns a newly allocated string that contains the version and that
 *          may be freed with free_variable() when no longer needed.
 */
extern gchar *get_communication_library_version(void);


/**
 * Creates a new communication comm_t * structure.
 * @returns a newly allocated comm_t * structure where sender and receiver
 *          are set to NULL.
 */
extern comm_t *init_comm_struct(void);

extern void use_curl(comm_t *comm);

#endif /* #ifndef _COMMUNIQUE_H_ */

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    server.h
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
 * @file server.h
 *
 * This file contains all the definitions of the functions and structures
 * that are used by 'cdpfglserver' Sauvegarde's server.
 * @todo add some stats structure that will keep some values about the
 *       activity of cdglserver server.
 */
#ifndef _SERVER_H_
#define _SERVER_H_

#include "config.h"

#define MHD_PLATFORM_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <glib-unix.h>
#include <sys/inotify.h>
#include <errno.h>
#include <math.h>


#include "libcdpfgl.h"

/**
 * @def SERVER_DATE
 * Defines server's creation date
 *
 * @def SERVER_AUTHORS
 * Defines server's main authors
 *
 * @def SERVER_LICENSE
 * Defines server's license (at least GPL v2)
 *
 * @def SERVER_VERSION
 * Defines server's version (which may be different of Sauvegarde's whole
 * project and programs that composes it).
 *
 * @def PROGRAM_NAME
 * Defines the main program name (cdpfglserver - stands for Continuous Data
 * Protection under Gnu/Linux server)
 */
#define SERVER_AUTHORS ("Olivier DELHOMME <olivier.delhomme@free.fr>")
#define SERVER_DATE _("02 11 2015")
#define SERVER_LICENSE _("GPL v3 or later")
#define SERVER_VERSION _("0.0.7")
#define PROGRAM_NAME ("cdpfglserver")

#include "options.h"
#include "backend.h"

/**
 * @def DEFAULT_SERVER_BUFFER_SIZE
 * Defines default server buffer size used in MHD callbacks to reassemble
 * POST data. Default is 8MB.
 */
#define DEFAULT_SERVER_BUFFER_SIZE (8388608)

/**
 * @struct server_struct_t
 * @brief Structure that contains everything needed by the program.
 *
 * This structure was named server_struct_t to avoid any confusion with
 * main_struct that is the structure for 'client' program.
 */
typedef struct
{
    options_t *opt;           /**< Options of the program from the command line    */
    struct MHD_Daemon *d;     /**< libmicrohttpd daemon structure                  */
    backend_t *backend;
    GAsyncQueue *meta_queue;  /**< An asynchronous queue where smeta data will
                               *   be transmitted as it arrives                    */
    GAsyncQueue *data_queue;  /**< An asynchronous queue where data will be
                               *   transmitted as it arrives                       */
    GThread *data_thread;     /**< Thread that will take care of storing data      */
    GThread *meta_thread;     /**< Thread that will take care of storing meta data */
    GMainLoop* loop;          /**< Main loop in glib                               */
} server_struct_t;


/**
 * @struct upload_t
 * @brief Structure to manage uploads of data in POST command.
 *
 */
typedef struct
{
    gchar *buffer;   /**< buffer that will grab all upload_data from MHD_ahc callback       */
    guint64 pos;     /**< position in the buffer (at the end it is the size of that buffer) */
    guint64 number;  /**< number of upload_data buffers received                            */
} upload_t;



#include "file_backend.h"


#endif /* #ifndef _SERVER_H_ */

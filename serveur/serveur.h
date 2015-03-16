/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    serveur.h
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
 * @file serveur.h
 *
 * This file contains all the definitions of the functions and structures
 * that are used by 'serveur' Sauvegarde's server.
 */
#ifndef _SERVEUR_H_
#define _SERVEUR_H_

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
#include <sys/inotify.h>
#include <errno.h>

#include <libsauvegarde.h>

/**
 * @def SERVEUR_DATE
 * Defines serveur's creation date
 *
 * @def SERVEUR_AUTHORS
 * Defines serveur's main authors
 *
 * @def SERVEUR_LICENSE
 * Defines serveur's license (at least GPL v2)
 *
 * @def SERVEUR_VERSION
 * Defines serveur's version (which may be different of Sauvegarde's whole
 * project and programs that composes it).
 *
 * @def PROGRAM_NAME
 * Defines the main program name (serveur - server in french)
 */
#define SERVEUR_AUTHORS ("Olivier DELHOMME <olivier.delhomme@free.fr>")
#define SERVEUR_DATE N_("27 04 2014")
#define SERVEUR_LICENSE N_("GPL v3 or later")
#define SERVEUR_VERSION N_("0.0.1")
#define PROGRAM_NAME ("serveur")

#include "options.h"
#include "backend.h"

/**
 * @struct serveur_struct_t
 * @brief Structure that contains everything needed by the program.
 *
 * This structure was named serveur_struct_t to avoid any confusion with
 * main_struct that is the structure for 'client' program (composed of
 * monitor, ciseaux and antememoire).
 */
typedef struct
{
    options_t *opt;           /**< Options of the program from the command line */
    hashs_t *hashs;           /**< Binary tree that will contain all hashs and
                               *   may be some related data before writing it to
                               *   disk
                               */
    struct MHD_Daemon *d;     /**< libmicrohttpd daemon structure               */
    backend_t *backend;
    GAsyncQueue *meta_queue;  /**< An asynchronous queue where smeta data will
                               *   be transmitted as it arrives                 */
    GAsyncQueue *data_queue;  /**< An asynchronous queue where data will be
                               *   transmitted as it arrives                    */
} serveur_struct_t;


#include "file_backend.h"


#endif /* #ifndef _SERVEUR_H_ */

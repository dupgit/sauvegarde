/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    serveur.h
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
 * @file serveur.h
 *
 * This file contains all the definitions of the functions and structures
 * that are used by 'serveur' Sauvegarde's server.
 */
#ifndef _SERVEUR_H_
#define _SERVEUR_H_

#include "config.h"

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <sys/inotify.h>
#include <errno.h>

#include <libsauvegarde.h>

#include "options.h"


/**
 * @def SERVEUR_DATE
 * Defines serveur's creation date
 *
 * @def SERVEUR_AUTHORS
 * Defines monitor's main authors
 *
 * @def SERVEUR_LICENSE
 * Defines monitor's license (at least GPL v2)

 * @def PROGRAM_NAME
 * Defines the main program name (serveur - server in french)
 */
#define SERVEUR_AUTHORS ("Olivier DELHOMME <olivier.delhomme@free.fr>")
#define SERVEUR_DATE N_("27 04 2014")
#define SERVEUR_LICENSE N_("GPL v3 or later")
#define PROGRAM_NAME ("serveur")

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
    options_t *opt;           /**< Options of the program from the command line                                              */
} serveur_struct_t;




#endif /* #ifndef _SERVEUR_H_ */

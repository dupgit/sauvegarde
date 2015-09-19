/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    monitor.h
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
 * @file monitor.h
 *
 *  This file contains all the definitions for the monitor program.
 */
#ifndef _MONITOR_H_
#define _MONITOR_H_

/* Configuration from ./configure script */
#include "config.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <errno.h>

#include <signal.h>
#include <string.h>
#include <poll.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <sys/fanotify.h>

#include <libsauvegarde.h>

#include "options.h"


/**
 * @def MONITOR_DATE
 * Defines monitor's creation date
 *
 * @def MONITOR_AUTHORS
 * Defines monitor's main authors
 *
 * @def MONITOR_LICENSE
 * Defines monitor's license (at least GPL v2)
 *
 * @def MONITOR_VERSION
 * Defines monitor's version (which may be different of Sauvegarde's whole
 * project and programs that composes it).

 * @def PROGRAM_NAME
 * Defines the main program name for this part (monitor + ciseaux +
 * antememoire).
 */
#define MONITOR_AUTHORS ("Olivier DELHOMME <olivier.delhomme@free.fr>")
#define MONITOR_DATE _("06 09 2015")
#define MONITOR_LICENSE _("GPL v3 or later")
#define MONITOR_VERSION _("0.0.4")
#define PROGRAM_NAME ("client")


/**
 * @def CLIENT_BLOCK_SIZE
 * default block size in bytes
 */
#define CLIENT_BLOCK_SIZE (16384)

/**
 * @def CLIENT_MIN_BUFFER
 *
 * defines the minimum of bytes to have into the buffer before sending
 * datas to the serveur.
 */
#define CLIENT_MIN_BUFFER (1048576)


/**
 * @struct main_struct_t
 * @brief Structure that contains everything needed by the program.
 */
typedef struct
{
    options_t *opt;           /**< Options of the program from the command line                                                */
    const gchar *hostname;    /**< Name of the current machine                                                                 */
    db_t *database;           /**< Database structure that stores everything that is related to the database                   */
    comm_t *comm;             /**< This is used to communicate with the 'serveur' program (which is the server)                */
    gint signal_fd;           /**< signal handler   */
    gint fanotify_fd;         /**< fanotify handler */
} main_struct_t;


/**
 * This function gets meta data and data from a file and sends them
 * to the serveur in order to save the file located in the directory
 * 'directory' and represented by 'fileinfo' variable.
 * @param main_struct : main structure of the program
 * @param directory is the directory we are iterating over
 * @param fileinfo is a glib structure that contains all meta data and
 *        more for a file.
 */
extern void save_one_file(main_struct_t *main_struct, gchar *directory, GFileInfo *fileinfo);


#include "m_fanotify.h"

#endif /* #IFNDEF _MONITOR_H_ */

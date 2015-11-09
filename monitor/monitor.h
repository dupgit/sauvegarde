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
#define MONITOR_DATE _("02 11 2015")
#define MONITOR_LICENSE _("GPL v3 or later")
#define MONITOR_VERSION _("0.0.6")
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
 * datas to the server.
 */
#define CLIENT_MIN_BUFFER (1048576)


/**
 * @def CLIENT_SMALL_FILE_SIZE
 *
 * defines the size under which a file is considered as small (ie that
 * may be totaly in memory).
 * 134217728 == 128 MB.
 * 134217 == 134 KB (only for tests since there is still no command line
 * or configuration file option to set this).
 */
#define CLIENT_SMALL_FILE_SIZE (134217728)


/**
 * @struct file_event_t
 * @brief stores all the necessary things to manage an event on a file.
 */
typedef struct
{
    gchar *directory;
    GFileInfo *fileinfo;
} file_event_t;


/**
 * @struct main_struct_t
 * @brief Structure that contains everything needed by the program.
 */
typedef struct
{
    options_t *opt;                 /**< Options of the program from the command line                                                     */
    const gchar *hostname;          /**< Name of the current machine                                                                      */
    db_t *database;                 /**< Database structure that stores everything that is related to the database                        */
    comm_t *comm;                   /**< This is used to communicate with the 'server' program (which is the server)                      */
    gint signal_fd;                 /**< signal handler                                                                                   */
    gint fanotify_fd;               /**< fanotify handler                                                                                 */
    GThread *save_one_file;         /**< thread that is used to save one file at a time (directory carving and live backup runs together) */
    GThread *carve_all_directories; /**< thread used to carve all directories and let fanotify executing itself                           */
    GAsyncQueue *save_queue;        /**< Queue where is sent all file_event_t structures upon event or while directory carving.           */
    GAsyncQueue *dir_queue;         /**< A queue to collect directories when carving to avoid thread collision                            */
    GSList *regex_exclude_list;     /**< List of regular expressions used to exclude directories or files.                                */
} main_struct_t;


/**
 * This function gets meta data and data from a file and sends them
 * to the server in order to save the file located in the directory
 * 'directory' and represented by 'fileinfo' variable.
 * @param main_struct : main structure of the program
 * @param directory is the directory we are iterating over
 * @param fileinfo is a glib structure that contains all meta data and
 *        more for a file.
 */
extern void save_one_file(main_struct_t *main_struct, gchar *directory, GFileInfo *fileinfo);


/**
 * @returns a newly alloacted file_event_t * structure that must be freed
 * when no longer needed
 * @param
 */
extern file_event_t *new_file_event_t(gchar *directory, GFileInfo *fileinfo);

#include "m_fanotify.h"

#endif /* #IFNDEF _MONITOR_H_ */

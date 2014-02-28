/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    monitor.h
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
#include <sys/inotify.h>
#include <errno.h>
#include <libsauvegarde.h>

#include "options.h"
#include "path.h"

/**
 * @def MONITOR_DATE
 * defines monitor's creation date
 *
 * @def MONITOR_AUTHORS
 * defines monitor's main authors
 *
 * @def MONITOR_LICENSE
 * defines monitor's license (at least GPL v2)
 *
 * @def MONITOR_TIME
 * defines monitor default rate limit in minutes
 */
#define MONITOR_AUTHORS ("Olivier Delhomme")
#define MONITOR_DATE N_("15 02 2014")
#define MONITOR_LICENSE N_("GPL v3 or later")
#define MONITOR_TIME (5)


/**
 * @struct main_struct_t
 * Structure that will contain everything needed by the program
 */
typedef struct
{
    options_t *opt;        /** options of the program from the command line            */
    GTree *path_tree;      /** Balanced Binary Trees to store path_t * paths monitored */
    const gchar *hostname; /** the name of the current machine                         */
    int fd;                /** file descriptor to be used bu inotify                   */
} main_struct_t;


/**
 * @struct thread_t
 * Structure to be passed to a thread that will traverse the directory list
 */
typedef struct
{
    main_struct_t *main_struct;
    GSList *dir_list;
} thread_data_t;



#endif /* #IFNDEF _MONITOR_H_ */
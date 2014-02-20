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

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "config.h"
#include "options.h"
#include "path.h"

/**
 * @def MONITOR_DATE
 * defines heraia's creation date
 *
 * @def MONITOR_AUTHORS
 * defines heraia's main authors
 *
 * @def MONITOR_LICENSE
 * defines heraia's license (at least GPL v2)
 */
#define MONITOR_AUTHORS "Olivier Delhomme"
#define MONITOR_DATE "15 02 2014"
#define MONITOR_LICENSE ("GPL v3 or later")


/**
 * @struct main_struct_t
 * Structure that will contain everything needed by the program
 */
typedef struct
{
    options_t *opt;        /** options of the program from the command line            */
    GTree *path_tree;      /** Balanced Binary Trees to store path_t * paths monitored */
    const gchar *hostname; /** the name of the current machine                         */
} main_struct_t;


#endif /* #IFNDEF _MONITOR_H_ */

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    restore.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 - 2016 Olivier Delhomme
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
 * @file restore.h
 *
 * This file contains all the definitions of the functions and structures
 * that are used by 'cdpfglrestore' program.
 */
#ifndef _RESTORE_H_
#define _RESTORE_H_

/* Configuration from ./configure script */
#include "config.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gi18n-lib.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "libcdpfgl.h"

#include "options.h"

/**
 * @struct res_struct_t
 * @brief This structure is used to keep all parameters for cdpfglrestore's
 *        program.
 */
typedef struct
{
    options_t *opt;  /**< Program's options                                             */
    comm_t *comm;    /**< Communication structure to operate with cdpfglserver's server */
    gchar *hostname; /**< Host name where the program is executing itself              */
} res_struct_t;


/**
 * @def RESTORE_DATE
 * Defines restore's creation date
 *
 * @def RESTORE_AUTHORS
 * Defines restore's main authors
 *
 * @def RESTORE_LICENSE
 * Defines restore's license (at least GPL v3)
 *
 * @def RESTORE_VERSION
 * Defines restore's version (which may be different of Sauvegarde's whole
 * project and programs that composes it).

 * @def PROGRAM_NAME
 * Defines the main program name for this part.
 */
#define RESTORE_AUTHORS ("Olivier DELHOMME <olivier.delhomme@free.fr>")
#define RESTORE_DATE _("11 29 2016")
#define RESTORE_LICENSE _("GPL v3 or later")
#define RESTORE_VERSION _("0.0.10")
#define PROGRAM_NAME ("cdpfglrestore")



#endif /* #ifndef _RESTORE_OPTIONS_H_ */

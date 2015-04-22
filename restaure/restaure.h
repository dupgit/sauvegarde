/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    restaure.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 Olivier Delhomme
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
 * @file restaure.h
 *
 * This file contains all the definitions of the functions and structures
 * that are used by 'restaure' program.
 */
#ifndef _RESTAURE_H_
#define _RESTAURE_H_

/* Configuration from ./configure script */
#include "config.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <libsauvegarde.h>

#include "options.h"

typedef struct
{
    options_t *opt;  /**< program's options */
} res_struct_t;


/**
 * @def RESTAURE_DATE
 * Defines restaure's creation date
 *
 * @def RESTAURE_AUTHORS
 * Defines restaure's main authors
 *
 * @def RESTAURE_LICENSE
 * Defines restaure's license (at least GPL v2)
 *
 * @def RESTAURE_VERSION
 * Defines restaure's version (which may be different of Sauvegarde's whole
 * project and programs that composes it).

 * @def PROGRAM_NAME
 * Defines the main program name for this part (restaure + ciseaux +
 * antememoire).
 */
#define RESTAURE_AUTHORS ("Olivier DELHOMME <olivier.delhomme@free.fr>")
#define RESTAURE_DATE N_("22 04 2015")
#define RESTAURE_LICENSE N_("GPL v3 or later")
#define RESTAURE_VERSION N_("0.0.1")
#define PROGRAM_NAME ("restaure")



#endif /* #ifndef _RESTAURE_OPTIONS_H_ */

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    ciseaux.h
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
 * @file ciseaux.h
 *
 *  This file contains all the definitions for the ciseaux program.
 */
#ifndef _CISEAUX_H_
#define _CISEAUX_H_

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "config.h"
#include "options.h"

/**
 * @def CISEAUX_DATE
 * defines ciseaux's creation date
 *
 * @def CISEAUX_AUTHORS
 * defines ciseaux's main authors
 *
 * @def CISEAUX_LICENSE
 * defines ciseaux's license (at least GPL v2)
 */
#define CISEAUX_AUTHORS ("Olivier Delhomme")
#define CISEAUX_DATE ("24 02 2014")
#define CISEAUX_LICENSE ("GPL v3 or later")

/**
 * @def CISEAUX_BLOCK_SIZE
 * default block size in bytes
 *
 * @def CISEAUX_MAX_THREADS
 * default maximum number of threads in a thread pool
 */
#define CISEAUX_BLOCK_SIZE (32768)
#define CISEAUX_MAX_THREADS (16)


/**
 * @struct main_struct_t
 * Structure that will contain everything needed by the program
 */
typedef struct
{
    options_t *opt;   /** options of the program from the command line                  */
    GThreadPool *tp;  /** Thread pool that will be used to calculate the hashs of files */
} main_struct_t;




#endif /* #ifndef _CISEAUX_H_ */

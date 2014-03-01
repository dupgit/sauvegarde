/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    configuration.h
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
 * @file configuration.h
 *
 * This file contains all the definitions for the tools that deals with
 * the configuration file of "Sauvegarde" programs.
 */
#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

/**
 * @note on key naming scheme :
 *  GN == Group Name that will group KN_* keys used for a same purpose or
 *        a same program. It should begin with an uppercase letter.
 *  KN == Key Name that will stor a value or a list of values. It should
 *        be lowercase only.
 *
 * Thoses key names and group names should not be translated.
 */
 /**
  * @def GN_MONITOR
  * Defines the group name for all preferences related to "monitor" program
  *
  * @def GN_CISEAUX
  * Defines the group name for all preferences related to "ciseaux" program
  */
#define GN_MONITOR ("Monitor")
#define GN_CISEAUX ("Ciseaux")


/** Below you'll find some definitions for the ciseaux program */
/**
 * @def KN_BLOCK_SIZE
 * Defines the key name for the blocksize option. Expected value is of
 * type gint64 but may only be positive.
 *
 * @def KN_MAX_THREADS
 * Defines the key name for the max-threads option. Expected value is of
 * type gint64 but value may only be positive. A typical value should be
 * between 2 and 32 (default is 16).
 */
#define KN_BLOCK_SIZE ("blocksize")
#define KN_MAX_THREADS ("max-threads")


/** Below you'll find some definitions for the monitor program */
/**
 * @def KN_DIR_LIST
 * Defines a list of directories that we want to watch.
 */
#define KN_DIR_LIST ("directory-list")


/**
 * Gets the probable filename for the configuration file of sauvegarde
 * project. This is needed when one wants to install the project in an
 * uncommon location such as a homedir for instance.
 * @param progname is the name of the program we want to search for in the
 *        user's path
 * @returns a gchar * which contain the filename of the configuration file
 *          relative to progname or NULL if something went wrong.
 */
extern gchar *get_probable_etc_path(gchar *progname);


#endif /* #ifndef _CONFIGURATION_H_ */

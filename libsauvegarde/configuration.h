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
  *
  * @def GN_ANTEMEMOIRE
  * Defines the group name for all preferences related to "antememoire" program
  */
#define GN_MONITOR ("Monitor")
#define GN_CISEAUX ("Ciseaux")
#define GN_ANTEMEMOIRE ("AnteMemoire")


/** Below you'll find some definitions for the ciseaux program */
/**
 * @def KN_BLOCK_SIZE
 * Defines the key name for the blocksize option. Expected value is of
 * type gint64 but may only be positive.
 */
#define KN_BLOCK_SIZE ("blocksize")


/** Below you'll find some definitions for the monitor program */
/**
 * @def KN_DIR_LIST
 * Defines a list of directories that we want to watch.
 */
#define KN_DIR_LIST ("directory-list")


/** Below you'll find some definitions for the antememoire program */
/**
 * @def KN_CACHE_DIR
 * Defines a directory where we will put some cache files and stuff
 * temporary needed to do the job. The program needs write access to this
 * directory.
 */
#define KN_CACHE_DIR ("cache-directory")

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

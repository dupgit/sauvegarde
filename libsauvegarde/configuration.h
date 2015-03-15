/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    configuration.h
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
 * @file configuration.h
 *
 * This file contains all the definitions for the tools that deals with
 * the configuration file of "Sauvegarde" programs.
 */
#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

/**
 * @note on key naming scheme :
 *
 *  GN == Group Name that will group KN_* keys used for a same purpose or
 *        a same program. It should begin with an uppercase letter.
 *  KN == Key Name that will stor a value or a list of values. It should
 *        be lowercase only.
 *
 * To translators : thoses key names and group names must not be translated.
 */

 /**
  * @def GN_MONITOR
  * Defines the group name for all preferences related to "monitor"
  * program.
  *
  * @def GN_CISEAUX
  * Defines the group name for all preferences related to "ciseaux"
  * program.
  *
  * @def GN_ANTEMEMOIRE
  * Defines the group name for all preferences related to "antememoire"
  * program.
  *
  * @def GN_SERVEUR
  * Defines the group name for all preferences related to "serveur"
  * program.
  *
  */
#define GN_MONITOR ("Monitor")
#define GN_CISEAUX ("Ciseaux")
#define GN_ANTEMEMOIRE ("AnteMemoire")
#define GN_SERVEUR ("Serveur")
#define GN_ALL ("All")


/** Below you'll find some definitions for all the programs */
/**
 * @def KN_DEBUG_MODE
 * Defines the key name for debug mode that may be used by any program
 * in configuration files.
 */
#define KN_DEBUG_MODE ("debug-mode")


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
 * @def KN_DB_NAME
 * Defines the name of the database to be used for the local cache
 */
#define KN_DB_NAME ("cache-db-name")


/**
 * @def KN_SERVEUR_IP
 * Defines server's IP address for the client.
 */
#define KN_SERVEUR_IP ("serveur-ip")


/** Below you'll find some definitions for the serveur program */
/**
 * @def KN_SERVEUR_PORT
 * Defines the port number on which serveur program will listen for
 * connexions
 */
#define KN_SERVEUR_PORT ("serveur-port")


/**
 * Gets the probable filename for the configuration file of sauvegarde
 * project. This is needed when one wants to install the project in an
 * uncommon location such as a homedir for instance.
 * @param progname is the name of the program we want to search for in the
 *        user's path
 * @param default configuration file name that should be a const string.
 * @returns a gchar * which contain the filename of the configuration file
 *          relative to progname or NULL if something went wrong.
 */
extern gchar *get_probable_etc_path(gchar *progname, const gchar *configfile);


/**
 * This functions converts a gchar ** array to a GSList of gchar *.
 * The function appends to the list first_list (if it exists - it may be
 * NULL) each entry of the array so elements are in the same order in the
 * array and in the list.
 * @param array is a gchar * array.
 * @param first_list is a list that may allready contain some elements and
 *        to which we will add all the elements of 'array' array.
 * @returns a newly allocated GSList that may be freed when no longer
 *          needed or NULL if array is NULL.
 */
extern GSList *convert_gchar_array_to_GSList(gchar **array, GSList *first_list);


/**
 * Reads a string from keyname key in the group grouname from keyfile file
 * and displays errormsg in case of an error
 * @param keyfile : the opened keyfile to read from
 * @param filename : the filename of the keyfile file
 * @param groupname : the groupname where to look for the key
 * @param keyname : the key to read the string from
 * @param errormsg : the error message to be displayed in case of an error
 * @returns the string read at the keyname in the groupname of keyfile
 *          file.
 */
extern gchar *read_string_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg);


/**
 * Reads a gint64 from keyname key in the group grouname from keyfile file
 * and displays errormsg in case of an error
 * @param keyfile : the opened keyfile to read from
 * @param filename : the filename of the keyfile file
 * @param groupname : the groupname where to look for the key
 * @param keyname : the key to read the gint64 from
 * @param errormsg : the error message to be displayed in case of an error
 * @returns the gint64 read at the keyname in the groupname of keyfile
 *          file or 0;
 */
extern gint64 read_int64_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg);


/**
 * Reads an integer from keyname key in the group grouname from keyfile file
 * and displays errormsg in case of an error
 * @param keyfile : the opened keyfile to read from
 * @param filename : the filename of the keyfile file
 * @param groupname : the groupname where to look for the key
 * @param keyname : the key to read the gint from
 * @param errormsg : the error message to be displayed in case of an error
 * @returns the gint read at the keyname in the groupname of keyfile
 *          file or 0;
 */
extern gint read_int_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg);


/**
 * Reads an integer from keyname key in the group grouname from keyfile file
 * and displays errormsg in case of an error
 * @param keyfile : the opened keyfile to read from
 * @param filename : the filename of the keyfile file
 * @param groupname : the groupname where to look for the key
 * @param keyname : the key to read the gboolean from
 * @param errormsg : the error message to be displayed in case of an error
 * @returns the boolean read at the keyname in the groupname of keyfile
 *          file or FALSE;
 */
extern gboolean read_boolean_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg);


/**
 * Reads a list of gchar * from keyname key in the group grouname from
 * keyfile file and displays errormsg in case of an error
 * @param keyfile : the opened keyfile to read from
 * @param filename : the filename of the keyfile file
 * @param groupname : the groupname where to look for the key
 * @param keyname : the key to read the list of gchar * from
 * @param errormsg : the error message to be displayed in case of an error
 * @returns the list of gchar * read at the keyname in the groupname of
 *          keyfile file or NULL;
 */
extern GSList *read_list_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg);

#endif /* #ifndef _CONFIGURATION_H_ */

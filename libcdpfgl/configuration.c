/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    configuration.c
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
 * @file configuration.c
 * This file contains the functions to deal with the configuration file
 * of the "Sauvegarde" programs.
 */

#include "libsauvegarde.h"

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
gchar *get_probable_etc_path(gchar *progname, const gchar *configfile)
{
    gchar *abs_path = NULL;
    gchar *path = NULL;
    const gchar * const *system_dirs = NULL;
    gint i = 0;
    gboolean ok = FALSE;

    if (progname != NULL && configfile != NULL)
        {
            /* the first location of the program in the path */
            abs_path = g_find_program_in_path(progname);
            if (abs_path != NULL)
                {
                    path =  g_build_path(G_DIR_SEPARATOR_S, g_path_get_dirname(abs_path), "..", "etc", "sauvegarde", configfile, NULL);
                    free_variable(abs_path);

                    if (file_exists(path) == FALSE)
                        {
                            free_variable(path);
                            system_dirs = g_get_system_config_dirs();
                            i = 0;
                            while (system_dirs[i] != NULL && ok == FALSE)
                                {
                                    path =  g_build_path(G_DIR_SEPARATOR_S, system_dirs[i], "sauvegarde", configfile, NULL);

                                    if (file_exists(path) == FALSE)
                                        {
                                            free_variable(path);
                                        }
                                    else
                                        {
                                            ok = TRUE;
                                        }
                                    i++;
                                }
                        }
                }
        }

    return path;
}


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
gchar *read_string_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg)
{
    gchar *a_string = NULL;   /** the string to be read */
    GError *error = NULL;     /** Glib error handling   */

     if (g_key_file_has_key(keyfile, groupname, keyname, &error) == TRUE)
        {

            a_string = g_key_file_get_string(keyfile, groupname, keyname, &error);

            if (error != NULL)
                {
                    print_error(__FILE__, __LINE__,  "%s %s: %s", errormsg, filename, error->message);
                    error = free_error(error);
                }
        }
    else if (error != NULL)
        {
            print_error(__FILE__, __LINE__, _("Error while looking for %s key in configuration file: %s\n"), keyname, error->message);
            error = free_error(error);
        }

    return a_string;
}


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
gint64 read_int64_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg)
{
    gint64 num = 0;        /** Number to be read     */
    GError *error = NULL;  /** Glib error handling   */

     if (g_key_file_has_key(keyfile, groupname, keyname, &error) == TRUE)
        {
            num =  g_key_file_get_int64(keyfile, groupname, keyname, &error);

            if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, "%s %s: %s", errormsg, filename, error->message);
                    error = free_error(error);
                }
        }
    else if (error != NULL)
        {
            print_error(__FILE__, __LINE__, _("Error while looking for %s key in configuration file: %s\n"), keyname, error->message);
            error = free_error(error);
        }

    return num;
}


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
gint read_int_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg)
{
    gint num = 0;          /** Number to be read     */
    GError *error = NULL;  /** Glib error handling   */

    if (g_key_file_has_key(keyfile, groupname, keyname, &error) == TRUE)
        {
            num =  g_key_file_get_integer(keyfile, groupname, keyname, &error);

            if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, "%s %s: %s", errormsg, filename, error->message);
                    error = free_error(error);
                }
        }
    else if (error != NULL)
        {
            print_error(__FILE__, __LINE__,  _("Error while looking for %s key in configuration file: %s\n"), keyname, error->message);
            error = free_error(error);
        }

    return num;
}


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
gboolean read_boolean_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg)
{
    gboolean bool = FALSE; /** Boolean to be read    */
    GError *error = NULL;  /** Glib error handling   */

    if (g_key_file_has_key(keyfile, groupname, keyname, &error) == TRUE)
        {
            bool =  g_key_file_get_boolean(keyfile, groupname, keyname, &error);

            if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, "%s %s: %s", errormsg, filename, error->message);
                    error = free_error(error);
                }
        }
    else if (error != NULL)
        {
            print_error(__FILE__, __LINE__,  _("Error while looking for %s key in configuration file: %s\n"), keyname, error->message);
            error = free_error(error);
        }

    return bool;
}


/**
 * This functions converts a gchar ** array of directories to a GSList of
 * gchar * paths.
 * The function appends to the list first_list (if it exists - it may be
 * NULL) each entry of the array so elements are in the same order in the
 * array and in the list.
 * @param array is a gchar * array.
 * @param first_list is a list that may already contain some elements and
 *        to which we will add all the elements of 'array' array.
 * @returns a newly allocated GSList that may be freed when no longer
 *          needed or NULL if array is NULL.
 */
GSList *convert_gchar_array_to_GSList(gchar **array, GSList *first_list)
{
    gchar *a_string = NULL;    /** gchar * that is read in the array      */
    GSList *list = first_list; /** The list to be returned (may be NULL)  */
    gint i = 0;
    gint num = 0;              /** Number of elements in the array if any */

    if (array != NULL)
        {
            num = g_strv_length(array);

            for (i = 0; i < num; i++)
                {
                    a_string = normalize_directory(array[i]);
                    list = g_slist_append(list, a_string);
                }
        }

    return list;
}


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
GSList *read_list_from_file(GKeyFile *keyfile, gchar *filename, gchar *groupname, gchar *keyname, gchar *errormsg)
{
    GSList *a_list = NULL;         /** list to be returned                                */
    GError *error = NULL;          /** Glib error handling                                */
    gchar **dirname_array = NULL;  /** array of dirnames read into the configuration file */


    if (g_key_file_has_key(keyfile, groupname, keyname, &error) == TRUE)
        {
            dirname_array = g_key_file_get_string_list(keyfile, groupname, keyname, NULL, &error);

            if (dirname_array != NULL)
                {
                    a_list = convert_gchar_array_to_GSList(dirname_array, a_list);
                    /* The array is no longer needed (everything has been copied with g_strdup) */
                    g_strfreev(dirname_array);
                }
            else if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, _("%s %s: %s\n"), errormsg, filename, error->message);
                    error = free_error(error);
                }
        }
    else if (error != NULL)
        {
            print_error(__FILE__, __LINE__, _("Error while looking for %s key in configuration file: %s\n"), keyname, error->message);
            error = free_error(error);
        }

    return a_list;
}


/**
 * Reads debug mode in keyfile
 * @param keyfile is the GKeyFile structure that is used by glib to read
 *        groups and keys from.
 * @param filename : the filename of the configuration file to read from
 */
void read_debug_mode_from_file(GKeyFile *keyfile, gchar *filename)
{
    gboolean debug = FALSE;

    if (keyfile != NULL && filename != NULL && g_key_file_has_group(keyfile, GN_ALL) == TRUE)
        {
             /* Reading in section [All] the debug mode */
            debug = read_boolean_from_file(keyfile, filename, GN_ALL, KN_DEBUG_MODE, _("Could not load debug mode configuration from file."));

            set_debug_mode(debug);
        }
}





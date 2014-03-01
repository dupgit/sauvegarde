/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    configuration.c
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
 * @returns a gchar * which contain the filename of the configuration file
 *          relative to progname or NULL if something went wrong.
 */
gchar *get_probable_etc_path(gchar *progname)
{
    gchar *abs_path = NULL;
    gchar *path = NULL;

    if (progname != NULL)
        {
            /* the first location of the program in the path */
            abs_path = g_find_program_in_path(progname);
            if (abs_path != NULL)
                {
                    path =  g_build_path(G_DIR_SEPARATOR_S, g_path_get_dirname(abs_path), "..", "etc", "sauvegarde", "sauvegarde.conf", NULL);
                    g_free(abs_path);
                }
        }

    return path;
}

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    path.c
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
 * @file path.c
 *
 *  This file contains all the functions to manage path_t * structure
 */

#include "monitor.h"


/**
 * Allocate a new structure path_t containing a path to monitor and a rate
 * limit for notifications.
 * @param path : the path to be monitored
 * @param rate : the rate in minutes under which a new notification will not
 *        occur.
 * @returns a newly allocated path_t structure that may be freed when no
 *          longer needed (do not forget to free 'path' in it).
 */
path_t *new_path_t(gchar *path, gint64 rate)
{
    path_t *a_path = NULL;

    a_path = (path_t *) g_malloc0(sizeof(path_t));

    a_path->path = g_strdup(path);
    a_path->key = g_quark_from_string(path);
    a_path->rate = rate;
    a_path->wd = -1;

    return a_path;
}

/**
 * Free the memory for path_t * structure
 * @param a path_t * pointer to be freed from memory
 */
void free_path_t(path_t *a_path)
{
    if (a_path != NULL)
        {
            g_free(a_path->path);
            g_free(a_path);
        }
}


/**
 * Comparison function for path_t structure
 * @param a : path_t * to be compared to b
 * @param b : path_t * to be compared to a
 * @returns negative value if a < b; zero if a = b; positive value if a > b.
 */
gint compare_path(gconstpointer a, gconstpointer b)
{
    path_t *path_a = (path_t *) a;
    path_t *path_b = (path_t *) b;
    gint result = -2;

    if (path_a != NULL && path_b != NULL)
        {
            result = g_strcmp0(path_a->path, path_b->path); /* g_strcmp0 handles NULL pointers correctly */
        }
    else if (path_a == NULL && path_b == NULL)
        {
            result = 0;
        }
    else if (path_a == NULL)
        {
            result = 1;
        }
    else  /* path_b == NULL) */
        {
            result = -1;
        }

    return result;
}


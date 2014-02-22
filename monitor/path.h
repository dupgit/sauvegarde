/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    path.h
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
 * @file path.h
 *
 *  This file contains all the definitions for the path_t * structure
 * management
 */
#ifndef _SAVE_PATH_H_
#define _SAVE_PATH_H_

/**
 * @struct path_t
 * Structure that contains a path to be monitored and specific options such$
 * as rate limit in seconds.
 */
typedef struct
{
    gchar *path;     /** path to be monitored                                                                           */
    GQuark key;      /** GQuark associated to the path string                                                           */
    gint64 rate;     /** rate limit under which a second notification should not be notified for this path (in minutes) */
    int wd;          /** watch descriptor to monitor the path 'path'                                                */
} path_t;


/**
 * Comparison function for path_t structure
 * @param a : path_t * to be compared to b
 * @param b : path_t * to be compared to a
 * @returns negative value if a < b; zero if a = b; positive value if a > b.
 */
extern gint compare_path(gconstpointer a, gconstpointer b);


/**
 * Free the memory for path_t * structure
 * @param a path_t * pointer to be freed from memory
 */
extern void free_path_t(path_t *a_path);


/**
 * Allocate a new structure path_t containing a path to monitor and a rate
 * limit for notifications.
 * @param path : the path to be monitored
 * @param rate : the rate in minutes under which a new notification will not
 *        occur.
 * @returns a newly allocated path_t structure that may be freed when no
 *          longer needed (do not forget to free 'path' in it).
 */
extern path_t *new_path_t(gchar *path, gint64 rate);


#endif /* #ifndef _SAVE_PATH_H_ */

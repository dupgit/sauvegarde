/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    database.h
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
 * @file database.h
 *
 * This file contains all the definitions of the functions and structures
 * that are used to wrap database calls of Sauvegarde's project.
 */
#ifndef _DATABASE_H_
#define _DATABASE_H_


/**
 * @def DATABASE_NAME
 * Defines the database used.
 */
#define DATABASE_NAME ("SQLITE")


/**
 * @struct db_t
 * @brief Structure to store everything that is needed for the database.
 */
typedef struct
{
    sqlite3 *db;  /**< database connexion  */
} db_t;


/**
 * @struct file_row_t
 * @brief Structure that contains some fields of files's table row
 */
typedef struct
{
    guint nb_row;          /**< number of row    */
    GSList *file_id_list;  /**< list of file_ids */
} file_row_t;


/**
 * @returns a string containing the version of the database used.
 */
extern gchar *db_version(void);


/**
 * Returns a database
 * @param database_name is the filename of the file that contains the
 *        database
 * @result returns a db_t * filled with the database connexion or NULL
 *         in case of an error.
 */
extern db_t *open_database(gchar *database_name);


/**
 * Says whether a file is in allready in the cache or not
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param meta is the file's metadata that we want to know if it's already
 *        in the cache.
 * @returns a boolean that says TRUE if the file is already in the cache
 *          and FALSE if not.
 */
extern gboolean is_file_in_cache(db_t *database, meta_data_t *meta);

/**
 * Insert file into cache. One should have verified that the file
 * does not already exists in the database.
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param meta is the file's metadata that we want to insert into the
 *        cache.
 * @param hashs : a balanced binary tree that stores hashs.
 */
void insert_file_into_cache(db_t *database, meta_data_t *meta, hashs_t *hashs);

#endif /* #ifndef _DATABASE_H_ */

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    database.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2017 Olivier Delhomme
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
 * @def SQLITE_TYPE_TABLE
 * Defines object type as a table.
 */
#define SQLITE_TYPE_TABLE (0)


/**
 * @def SQLITE_TYPE_INDEX
 * Defines object type as an index.
 */
#define SQLITE_TYPE_INDEX (1)


/**
 * @def DATABASE_SCHEMA_VERSION
 * Defines the schema version that this program is
 * waiting for.
 */
#define DATABASE_SCHEMA_VERSION (1)


/**
 * @struct stmt_t
 * @brief structure to hold all statements needed for the programs.
 */
 typedef struct
 {
    sqlite3_stmt *save_meta_stmt;
    sqlite3_stmt *save_buffer_stmt;
    sqlite3_stmt *get_file_id_stmt;
 } stmt_t;


/**
 * @struct db_t
 * @brief Structure to store everything that is needed for the database.
 */
typedef struct
{
    sqlite3 *db;  /**< database connexion  */
    stmt_t *stmts;
    gint64 version;
    gchar *version_filename;
} db_t;


/**
 * @struct file_row_t
 * @brief Structure that contains some fields of a files's table row
 */
typedef struct
{
    guint nb_row;        /**< number of row */
} file_row_t;


/**
 * @struct transmited_t
 * @brief Structure used to pass things over sqlite3_exec callback procedure
 *        to transmit buffers saved in the database.
 */
typedef struct
{
    db_t *database;
    comm_t *comm;
} transmited_t;


/**
 * @struct list_t
 * @brief Structure used over sqlite_exec callback procedure to make a list
 *        of returned elements of a query.
 */
typedef struct
{
    GList *list;
} list_t;


/**
 * @returns a string containing the version of library's database used.
 */
extern gchar *db_version(void);


/**
 * Returns a database connexion or NULL.
 * @param dirname is the name of the directory where the database is
 *        located.
 * @param filename is the filename of the file that contains the
 *        database
 * @result returns a db_t * filled with the database connexion or NULL
 *         in case of an error.
 */
extern db_t *open_database(gchar *dirname, gchar *filename);


/**
 * Says whether a file is in already in the cache or not
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param meta is the file's metadata that we want to know if it's already
 *        in the cache.
 * @returns a boolean that says TRUE if the file is already in the cache
 *          and FALSE if not.
 */
extern gboolean is_file_in_cache(db_t *database, meta_data_t *meta);


/**
 * Insert meta data into cache (db). One should have verified that the
 * file does not already exists in the database.
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param meta is the file's metadata that we want to insert into the
 *        cache.
 * @param only_meta : a gboolean that when set to TRUE only meta_data will
 *        be saved and hashs data will not !
 */
extern void db_save_meta_data(db_t *database, meta_data_t *meta, gboolean only_meta);


/**
 * This function says if the table 'buffers' is empty or not, that is
 * to say whether we have to transmit unsaved data or not.
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @returns TRUE if table 'buffers' is not empty and FALSE otherwise
 */
extern gboolean db_is_there_buffers_to_transmit(db_t *database);


/**
 * Saves buffers that could not be sent to server
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param url is the url where buffer should have been POSTed
 * @param buffer is the buffer containing data that should have been
 *        POSTed to server but couldn't.
 */
extern void db_save_buffer(db_t *database, gchar *url, gchar *buffer);


/**
 * This function transferts the 'buffers' that are stored in the database
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param comm a comm_t * structure that must contain an initialized
 *        curl_handle (must not be NULL).
 * @returns TRUE if table 'buffers' is not empty and FALSE otherwise
 */
extern gboolean db_transmit_buffers(db_t *database, comm_t *comm);


/**
 * Frees and closes the database connection.
 * @param database is a db_t * structure with an already openned connection
 *        that we want to close.
 */
extern void close_database(db_t *database);


#endif /* #ifndef _DATABASE_H_ */

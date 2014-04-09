/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    database.c
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
 * @file database.c
 * This file contains the functions to wrap database calls in all the
 * programs of "Sauvegarde" project.
 */

#include "libsauvegarde.h"

static int table_callback(void *num, int nbCol, char **data, char **nomCol);
static void verify_if_tables_exists(db_t *database);

/**
 * @returns a string containing the version of the database used.
 */
gchar *db_version(void)
{
    return (gchar *) sqlite3_libversion();
}


/**
 * Prints an error message to stderr and exit (db errors are considered as
 * fatal for now.
 * @param db : file connexion to the database.
 * @param format : the format of the message (as in printf)
 * @param ... : va_list of variable that are to be printed into format.
 */
static void print_db_error(sqlite3 *db, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    sqlite3_close(db);
    exit(EXIT_FAILURE);
}


/**
 * Executes the SQL command onto the database without any callback
 * @param database : the db_t * structure that contains the database connexion
 * @param sql_cmd : a gchar * SQL command to be executed onto the database
 * @param format_message : a gchar * format message to be used in case of an error
 */
static void exec_sql_cmd(db_t *database, gchar *sql_cmd, gchar *format_message)
{
    char *error_message = NULL;
    int result = 0;

    result = sqlite3_exec(database->db, sql_cmd, NULL, 0, &error_message);

    if (result != SQLITE_OK)
        {
            print_db_error(database->db, format_message, error_message);
        }
}


/**
 * Counts the number of row that we have by incrementing i.
 * @param num is an integer that will count the number of rows in the result.
 * @param nb_col gives the number of columns in this row.
 * @param data contains the data of each column.
 * @param name_col contains the name of each column.
 * @returns always 0.
 */
static int table_callback(void *num, int nb_col, char **data, char **name_col)
{
    int *i = (int *) num;

    *i = *i + 1;

    return 0;
}


/**
 * Verifies if the tables are created whithin the database and creates
 * them if there is no tables at all.
 * @param database : the structure to manage database's connexion.
 */
static void verify_if_tables_exists(db_t *database)
{
    char *error_message = NULL;
    int result = 0;
    int *i = NULL;               /** Used to count the number of row */

    i = (int *) g_malloc0(sizeof(int));
    *i = 0;

    /* Trying to get all the tables that are in the database */
    result = sqlite3_exec(database->db, "SELECT * FROM sqlite_master WHERE type='table';", table_callback, i, &error_message);

    if (*i == 0)  /* No row (0) means that there is no table */
        {
            print_debug(stdout, N_("Creating tables into the database\n"));

            /* The database does not contain any tables. So we have to create them.         */
            /* Creation of checksum table that contains checksums and their associated data */
            exec_sql_cmd(database, "CREATE TABLE data (checksum BLOB PRIMARY KEY, size INTEGER, data BLOB);", N_("Error while creating database table 'data': %s\n"));

            /* Creation of buffers table that contains checksums and their associated data */
            exec_sql_cmd(database, "CREATE TABLE buffers (file_id INTEGER PRIMARY KEY, buf_order INTEGER, checksum BLOB);", N_("Error while creating database table 'buffers': %s\n"));

            /* Creation of files table that contains everything about a file */
            exec_sql_cmd(database, "CREATE TABLE files (file_id  INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER, file_user TEXT, file_group TEXT, uid INTEGER, gid INTEGER, atime INTEGER, ctime INTEGER, mtime INTEGER, mode INTEGER, name TEXT);", N_("Error while creating database table 'files': %s\n"));
        }
}


/**
 * Returns a database connexion or NULL.
 * @param database_name is the filename of the file that contains the
 *        database
 * @result returns a db_t * filled with the database connexion or NULL
 *         in case of an error.
 */
db_t *open_database(gchar *database_name)
{
    db_t *database = NULL;
    sqlite3 *db = NULL;
    int result = 0;

    database = (db_t *) g_malloc0(sizeof(db_t));

    result = sqlite3_open(database_name, &db);

    if (result != SQLITE_OK)
        {
            print_db_error(db, _("Error while trying to open %s database: %s\n"), database_name, sqlite3_errmsg(db));
            return NULL;
        }
    else
        {
            database->db = db;
            verify_if_tables_exists(database);
            return database;
        }
}





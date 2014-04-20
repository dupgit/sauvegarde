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

static void print_db_error(sqlite3 *db, const char *format, ...);
static void exec_sql_cmd(db_t *database, gchar *sql_cmd, gchar *format_message);
static int table_callback(void *num, int nbCol, char **data, char **nomCol);
static void verify_if_tables_exists(db_t *database);
static file_row_t *new_file_row_t(void);
static void free_file_row_t(file_row_t *row);
static int get_file_callback(void *a_row, int nb_col, char **data, char **name_col);
static file_row_t *get_file_id(db_t *database, meta_data_t *meta);


/**
 * @returns a string containing the version of the database used.
 */
gchar *db_version(void)
{
    return (gchar *) sqlite3_libversion();
}


/**
 * Prints an error message to stderr and exit (db errors are considered as
 * fatal for now).
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
    int *i = NULL;               /**< Used to count the number of row */

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
 * Says whether a file is in allready in the cache or not
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param meta is the file's metadata that we want to know if it's already
 *        in the cache.
 * @returns a boolean that says TRUE if the file is already in the cache
 *          and FALSE if not.
 */
gboolean is_file_in_cache(db_t *database, meta_data_t *meta)
{
    file_row_t *row = NULL;

    if (meta != NULL && database != NULL)
        {

            row = get_file_id(database, meta);

            if (row != NULL && row->nb_row == 0) /* No row has been returned. It means that the file isn't in the cache */
                {
                    free_file_row_t(row);
                    return FALSE;
                }
            else
                {
                    free_file_row_t(row);
                    return TRUE;
                }

        }
    else
        {
            return FALSE;
        }
}


/**
 * Gets file_ids from returned rows.
 * @param a_row is a file_row_t * structure
 * @param nb_col gives the number of columns in this row.
 * @param data contains the data of each column.
 * @param name_col contains the name of each column.
 * @returns always 0.
 */
static int get_file_callback(void *a_row, int nb_col, char **data, char **name_col)
{
    file_row_t *row = (file_row_t *) a_row;

    row->nb_row = row->nb_row + 1;
    row->file_id_list = g_slist_append(row->file_id_list, g_strdup(data[0]));

    return 0;
}


/**
 * Returns the file_id for the specified file.
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param meta is the file's metadata that we want to insert into the
 *        cache.
 */
static file_row_t *get_file_id(db_t *database, meta_data_t *meta)
{
    file_row_t *row = NULL;
    char *error_message = NULL;
    gchar *sql_command = NULL;
    int db_result = 0;

    row = new_file_row_t();

    sql_command = g_strdup_printf("SELECT file_id from files WHERE name='%s' AND type=%d AND uid=%d AND gid=%d AND atime=%ld AND ctime=%ld AND mtime=%ld AND mode=%d;", meta->name, meta->file_type, meta->uid, meta->gid, meta->atime, meta->ctime, meta->mtime, meta->mode);

    db_result = sqlite3_exec(database->db, sql_command, get_file_callback, row, &error_message);

    free_variable(sql_command);

    if (db_result == SQLITE_OK)
        {
           return row;
        }
    else
        {
            print_db_error(database->db, N_("Error while searching into the table 'files': %s\n"), error_message);
            return NULL;
        }
}


/**
 * Creates and inits a new file_row_t * structure.
 * @returns an empty file_row_t * structure.
 */
static file_row_t *new_file_row_t(void)
{
    file_row_t *row = NULL;

    row = (file_row_t *) g_malloc0(sizeof(file_row_t));

    row->nb_row = 0;
    row->file_id_list = NULL;

    return row;
}


/**
 * Frees everything whithin the file_row_t structure
 * @param row is the variable to be freed totaly
 */
static void free_file_row_t(file_row_t *row)
{
    if (row != NULL)
        {
            g_slist_free(row->file_id_list);
            free_variable(row);
        }
}


/**
 * Insert file into cache. One should have verified that the file
 * does not already exists in the database.
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param meta is the file's metadata that we want to insert into the
 *        cache.
 */
void insert_file_into_cache(db_t *database, meta_data_t *meta)
{
    gchar *sql_command = NULL;     /**< Command to be executed          */


    if (meta != NULL && database != NULL)
        {

            sql_command = g_strdup_printf("INSERT INTO files (type, file_user, file_group, uid, gid, atime, ctime, mtime, mode, name) VALUES (%d, '%s', '%s', %d, %d, %ld, %ld, %ld, %d, '%s');", meta->file_type, meta->owner, meta->group, meta->uid, meta->gid, meta->atime, meta->ctime, meta->mtime, meta->mode, meta->name);

            exec_sql_cmd(database, sql_command,  N_("Error while inserting into the table 'files': %s\n"));

            free_variable(sql_command);
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





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
 * Counts the number of row that we have by incrementing i.
 * @param i is an integer that will count the number of rows in the result.
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
    gchar *sql_cmd = NULL;
    int *i = NULL;               /** Used to count the number of row */

    i = (int *) g_malloc0(sizeof(int));
    *i = 0;

    sql_cmd = g_strdup("SELECT * FROM sqlite_master WHERE type='table';");
    result = sqlite3_exec(database->db, sql_cmd, table_callback, i, &error_message);
    free_variable(sql_cmd);

    if (*i == 0)  /* No row (0) means that their is no table */
        {
            if (ENABLE_DEBUG == TRUE)
                {
                    fprintf(stdout, _("Creating tables into the database\n"));
                }
            /* The database does not contain any tables. So we have to create them.         */
            /* Creation of checksum table that contains checksums and their associated data */
            sql_cmd = g_strdup("CREATE TABLE data (checksum BLOB, size INTEGER, data BLOB);");
            result = sqlite3_exec(database->db, sql_cmd, NULL, 0, &error_message);
            free_variable(sql_cmd);
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
            fprintf(stderr, _("Error while trying to open %s database: %s\n"), database_name, sqlite3_errmsg(db));
            sqlite3_close(db);
            exit(EXIT_FAILURE);
            return NULL;
        }
    else
        {
            database->db = db;
            verify_if_tables_exists(database);
            return database;
        }
}





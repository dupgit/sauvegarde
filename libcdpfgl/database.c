/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    database.c
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
 * @file database.c
 * This file contains the functions to wrap database calls in all the
 * programs of "Sauvegarde" project.
 */

#include "libcdpfgl.h"

static void print_db_error(sqlite3 *db, const char *format, ...);
static void print_on_db_error(sqlite3 *db, int result, const gchar *infos);
static void exec_sql_cmd(db_t *database, gchar *sql_cmd, gchar *format_message);
static int make_list_first_column_callback(void *userp, int nb_col, char **data, char **name_col);
static gint does_db_object_exists(db_t *database, gchar *name, gint type);
static gint does_table_exists(db_t *database, gchar *tablename);
static gint does_index_exists(db_t *database, gchar *indexname);
static int count_lines_callback(void *num, int nbCol, char **data, char **nomCol);
static void check_and_create_object(db_t *database, gchar *name, gint type, gchar *sql_creation_cmd, gchar *err_msg);
static void check_and_create_table(db_t *database, gchar *tablename, gchar *sql_creation_cmd, gchar *err_msg);
static void check_and_create_index(db_t *database, gchar *indexname, gchar *sql_creation_cmd, gchar *err_msg);
static void verify_if_tables_exists(db_t *database);
static file_row_t *get_file_id(db_t *database, meta_data_t *meta);
static file_row_t *new_file_row_t(void);
static void free_file_row_t(file_row_t *row);
static void bind_guint64_value(sqlite3 *db, sqlite3_stmt *stmt, const gchar *name, guint64 value);
static void bind_guint_value(sqlite3 *db, sqlite3_stmt *stmt, const gchar *name, guint value);
static void bind_text_value(sqlite3 *db, sqlite3_stmt *stmt, const gchar *name, gchar *value);
static void bind_values_to_save_meta_data(sqlite3 *db, sqlite3_stmt *stmt, meta_data_t *meta, gboolean only_meta, guint64 cache_time);
static void bind_values_to_save_buffer(sqlite3 *db, sqlite3_stmt *stmt, gchar *url, gchar *buffer);
static void bind_values_to_get_file_id(sqlite3 *db, sqlite3_stmt *stmt, meta_data_t *meta);
static sqlite3_stmt *create_save_meta_stmt(sqlite3 *db);
static sqlite3_stmt *create_save_buffer_stmt(sqlite3 *db);
static sqlite3_stmt *create_get_file_id_stmt(sqlite3 *db);
static stmt_t *new_stmts(sqlite3 *db);
static void free_stmts(stmt_t *stmts);
static list_t *new_list_t(void);
static void free_list_t(list_t *container);


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
}


/**
 * Prints out an error message if an sqlite function just made one.
 * @param db is the concerned sqlite database
 * @param result is the result of the sqlite function
 * @param infos is a gchar * containing some context to help understanding
 *        the error.
 * @note sqlite3_errstr needs at least sqlite 3.7.15
 */
static void print_on_db_error(sqlite3 *db, int result, const gchar *infos)
{
    const char *message = NULL;
    int errcode = 0;

    if (result == SQLITE_ERROR)
        {
            errcode = sqlite3_extended_errcode(db);
            message = sqlite3_errstr(errcode);
            print_db_error(db, _("sqlite error (%d - %d) on %s: %s\n"), result, errcode, infos, message);
        }
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
    const char *message = NULL;
    int result = 0;

    result = sqlite3_exec(database->db, sql_cmd, NULL, 0, &error_message);

    if (result != SQLITE_OK)
        {
            result = sqlite3_extended_errcode(database->db);
            /** @note sqlite3_errstr needs at least sqlite 3.7.15 */
            message = sqlite3_errstr(result);
            print_db_error(database->db, format_message, result, message);
        }
}


/**
 * Does a commit on the database
 * @param database : the db_t * structure that contains the database connexion
 */
static void sql_commit(db_t *database)
{
    exec_sql_cmd(database, "COMMIT;",  _("(%d) Error commiting to the database: %s\n"));
}


/**
 * Does a commit on the database
 * @param database : the db_t * structure that contains the database connexion
 */
static void sql_begin(db_t *database)
{
    exec_sql_cmd(database, "BEGIN;",  _("(%d) Error opening the transaction: %s\n"));
}


/**
 * Makes a list of first column elements returned by a query
 * @param userp is a pointer to a list_t structure that must not be NULL
 * @param nb_col gives the number of columns in this row.
 * @param data contains the data of each column.
 * @param name_col contains the name of each column.
 * @returns always 0.
 */
static int make_list_first_column_callback(void *userp, int nb_col, char **data, char **name_col)
{
    list_t *container = (list_t *)  userp;

    if (container !=NULL && data != NULL)
        {
            container->list = g_list_prepend(container->list, g_strdup(data[0]));
        }

    return 0;
}


/**
 * Tells if a specific table or index exists or not in the selected
 * and opened database.
 * @param database is the database structure
 * @param name is a gchar * string that is the name of
 *        the table or index to look for.
 * @param type is an integer that represents the type of the
 *         object to look for.
 * @returns 0 if the table exists, 1 if it doesn't and -1 on
 *          if an error occurs.
 */
static gint does_db_object_exists(db_t *database, gchar *name, gint type)
{
    char *error_message = NULL;
    int result = 0;
    list_t *container = NULL;
    GList *list = NULL;
    gboolean exists = FALSE;
    gchar *cmd = NULL;

    container = new_list_t();

    if (type == SQLITE_TYPE_TABLE)
        {
            cmd = g_strdup("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;");
        }
    else if (type == SQLITE_TYPE_INDEX)
        {
            cmd = g_strdup("SELECT name FROM sqlite_master WHERE type='index' ORDER BY name;");
        }
    else
        {
            free_list_t(container);
            return -1;
        }

    /* Trying to get all the table names that are in the database */
    result = sqlite3_exec(database->db, cmd, make_list_first_column_callback, container, &error_message);
    free_variable(cmd);


    if (result == SQLITE_OK && container != NULL)
        {
            list = container->list;

            while (list != NULL && exists == FALSE)
                {
                    if (g_strcmp0(list->data, name) == 0)
                        {
                            exists = TRUE;
                        }
                    list = list->next;
                }

            free_list_t(container);

            if (exists == TRUE)
                {
                    return 0;
                }
            else
                {
                    return 1;
                }
        }
    else
        {
            print_on_db_error(database->db, result, "sqlite_master table");
            return -1;
        }
}


/**
 * Tells if a specific table exists or not in the selected
 * and opened database.
 * @param database is the database structure
 * @param tablename is a gchar * string that is the name of
 *        the table to look for.
 * @returns 0 if the table exists, 1 if it doesn't and -1 on
 *          if an error occurs.
 */
static gint does_table_exists(db_t *database, gchar *tablename)
{
    return does_db_object_exists(database, tablename, SQLITE_TYPE_TABLE);
}


/**
 * Tells if a specific endex exists or not in the selected
 * and opened database.
 * @param database is the database structure
 * @param indexname is a gchar * string that is the name of
 *        the index to look for.
 * @returns 0 if the index exists, 1 if it doesn't and -1 on
 *          if an error occurs.
 */
static gint does_index_exists(db_t *database, gchar *indexname)
{
    return does_db_object_exists(database, indexname, SQLITE_TYPE_INDEX);
}


/**
 * Counts the number of row that we have by incrementing i.
 * @param num is an integer that will count the number of rows in the result.
 * @param nb_col gives the number of columns in this row.
 * @param data contains the data of each column.
 * @param name_col contains the name of each column.
 * @returns always 0.
 */
static int count_lines_callback(void *num, int nb_col, char **data, char **name_col)
{
    int *i = (int *) num;

    *i = *i + 1;

    return 0;
}



/**
 * Checks if a table or an index exists and creates it if not.
 * @param database : the structure to manage database's connexion.
 * @param name is the name of the table or index to look for and
 *        may be create.
 * @param is a gint that should be one of SQLITE_TYPE_INDEX or
 *        SQLITE_TYPE_TABLE
 * @param sql_creation_cmd is the SQL command to create the table or
 *        the index if needed.
 * @param err_msg is the error message to be displayed in case of an
 *        error when trying to create the table or index.
 */
static void check_and_create_object(db_t *database, gchar *name, gint type, gchar *sql_creation_cmd, gchar *err_msg)
{
    int result = 0;

    if (type == SQLITE_TYPE_TABLE)
        {
            result = does_table_exists(database, name);
        }
    else if (type == SQLITE_TYPE_INDEX)
        {
            result = does_index_exists(database, name);
        }
    else
        {
            result = -1;
        }

    if (result == 1)
        {
            /* table or index does not exists and we have to create it */
            exec_sql_cmd(database, sql_creation_cmd , err_msg);
        }
}


/**
 * Checks if a table exists and creates it if not.
 * @param database : the structure to manage database's connexion.
 * @param tablename is the name of the table to look for and
 *        may be create.
 * @param sql_creation_cmd is the SQL command to create the table
 *        if needed.
 * @param err_msg is the error message to be displayed in case of an
 *        error when trying to create the table.
 */
static void check_and_create_table(db_t *database, gchar *tablename, gchar *sql_creation_cmd, gchar *err_msg)
{
    check_and_create_object(database, tablename, SQLITE_TYPE_TABLE, sql_creation_cmd, err_msg);
}


/**
 * Checks if an index exists and creates it if not.
 * @param database : the structure to manage database's connexion.
 * @param indexname is the name of the index to look for and
 *        may be create.
 * @param sql_creation_cmd is the SQL command to create the index
 *        if needed.
 * @param err_msg is the error message to be displayed in case of an
 *        error when trying to create the index.
 */
static void check_and_create_index(db_t *database, gchar *indexname, gchar *sql_creation_cmd, gchar *err_msg)
{
    check_and_create_object(database, indexname, SQLITE_TYPE_INDEX, sql_creation_cmd, err_msg);
}


/**
 * Verifies if the tables are created whithin the database and creates
 * them if there is no tables at all.
 * @param database : the structure to manage database's connexion.
 */
static void verify_if_tables_exists(db_t *database)
{
    print_debug(_("Checking tables and index in the database:\n"));

    /* Creation of buffers table that contains checksums and their associated data */
    print_debug(_("\ttable buffers\n"));
    check_and_create_table(database, "buffers",  "CREATE TABLE buffers (buffer_id INTEGER PRIMARY KEY AUTOINCREMENT, url TEXT, data TEXT);", _("(%d) Error while creating database table 'buffers': %s\n"));

    /* Creation of transmited table that may contain id of transmited buffers if any + creation of its indexes */
    print_debug(_("\ttable transmited\n"));
    check_and_create_table(database, "transmited", "CREATE TABLE transmited (buffer_id INTEGER PRIMARY KEY);", _("(%d) Error while creating database table 'transmited': %s\n"));

    print_debug(_("\tindex transmited_buffer_id\n"));
    check_and_create_index(database, "transmited_buffer_id", "CREATE INDEX main.transmited_buffer_id ON transmited (buffer_id ASC)", _("(%d) Error while creating index 'transmited_buffer_id': %s\n"));

    /* Creation of files table that contains everything about a file */
    print_debug(_("\ttable files\n"));
    check_and_create_table(database, "files", "CREATE TABLE files (file_id  INTEGER PRIMARY KEY AUTOINCREMENT, cache_time INTEGER, type INTEGER, inode INTEGER, file_user TEXT, file_group TEXT, uid INTEGER, gid INTEGER, atime INTEGER, ctime INTEGER, mtime INTEGER, mode INTEGER, size INTEGER, name TEXT, transmitted BOOL, link TEXT);", _("(%d) Error while creating database table 'files': %s\n"));

    print_debug(_("\tindex files_inodes\n"));
    check_and_create_index(database, "files_inodes", "CREATE INDEX main.files_inodes ON files (inode ASC)", _("(%d) Error while creating index 'files_inodes': %s\n"));
}


/**
 * Says whether a file is in already in the cache or not
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

            if (row != NULL)
                {
                    if (row->nb_row == 0) /* No row has been returned. It means that the file isn't in the cache */
                        {
                            free_file_row_t(row);
                            return FALSE;
                        }
                    else
                        { /* At least one row has been returned */
                            free_file_row_t(row);
                            return TRUE;
                        }
                }
            else
                {
                    return FALSE;
                }
        }
    else
        {
            return FALSE;
        }
}


/**
 * Returns the file_id for the specified file.
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param meta is the file's metadata that we want to insert into the
 *        cache.
 * @returns a file_row_t structure filed with values returned by the
 *          database.
 */
static file_row_t *get_file_id(db_t *database, meta_data_t *meta)
{
    file_row_t *row = NULL;
    sqlite3_stmt *stmt = NULL;
    gint result = 0;

    row = new_file_row_t();
    stmt = database->stmts->get_file_id_stmt;

    bind_values_to_get_file_id(database->db, stmt, meta);
    result = sqlite3_step(stmt);

    while (result == SQLITE_ROW)
        {
            row->nb_row = row->nb_row + 1;
            result = sqlite3_step(stmt);
        }

    print_on_db_error(database->db, result, "get_file_id");

    sqlite3_reset(stmt);

    return row;
}


/**
 * Creates and inits a new file_row_t * structure.
 * @returns an empty file_row_t * structure.
 */
static file_row_t *new_file_row_t(void)
{
    file_row_t *row = NULL;

    row = (file_row_t *) g_malloc(sizeof(file_row_t));

    row->nb_row = 0;

    return row;
}


/**
 * Frees everything whithin the file_row_t structure
 * @param row is the variable to be freed totaly
 */
static void free_file_row_t(file_row_t *row)
{
    free_variable(row);
}


/**
 * Insert file into cache. One should have verified that the file
 * does not already exists in the database.
 * @todo Use statements to avoid bugs when dealing with strings from user
 *       space and thus avoid sql injection a bit. See
 *       https://sqlite.org/c3ref/prepare.html
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param meta is the file's metadata that we want to insert into the
 *        cache.
 * @param only_meta : a gboolean that when set to TRUE only meta_data will
 *        be saved and hashs data will not ! FALSE means that something
 *        went wrong with server and that all data will be cached localy.
 */
void db_save_meta_data(db_t *database, meta_data_t *meta, gboolean only_meta)
{
    guint64 cache_time = 0;
    gint result = 0;
    sqlite3_stmt *stmt = NULL;

    if (meta != NULL && database != NULL && database->stmts != NULL && database->stmts->save_meta_stmt != NULL)
        {
            cache_time = g_get_real_time();

            /* beginning a transaction */
            sql_begin(database);

            /* Inserting the file into the files table */
            stmt = database->stmts->save_meta_stmt;
            bind_values_to_save_meta_data(database->db, stmt, meta, only_meta, cache_time);
            result = sqlite3_step(stmt);
            print_on_db_error(database->db, result, "sqlite3_step");

            /* ending the transaction here */
            sql_commit(database);
            sqlite3_reset(stmt);
        }
}


/**
 * Saves buffers that could not be sent to server
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param url is the url where buffer should have been POSTed
 * @param buffer is the buffer containing data that should have been
 *        POSTed to server but couldn't.
 */
void db_save_buffer(db_t *database, gchar *url, gchar *buffer)
{
    sqlite3_stmt *stmt = NULL;
    gint result = 0;

    if (database != NULL && url != NULL && buffer != NULL)
        {
            sql_begin(database);

            stmt = database->stmts->save_buffer_stmt;
            bind_values_to_save_buffer(database->db, stmt, url, buffer);
            result = sqlite3_step(stmt);
            print_on_db_error(database->db, result, "db_save_buffer");

            sql_commit(database);
            sqlite3_reset(stmt);
        }
}


/**
 * This function says if the table 'buffers' is empty or not, that is
 * to say whether we have to transmit unsaved data or not.
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @returns TRUE if table 'buffers' is not empty and FALSE otherwise
 */
gboolean db_is_there_buffers_to_transmit(db_t *database)
{
    char *error_message = NULL;
    int result = 0;
    int *i = NULL;               /** int *i is used to count the number of row */

    i = (int *) g_malloc0(sizeof(int));
    *i = 0;

    result = sqlite3_exec(database->db, "SELECT * FROM buffers WHERE buffers.buffer_id NOT IN (SELECT transmited.buffer_id FROM transmited INNER JOIN buffers ON transmited.buffer_id = buffers.buffer_id);", count_lines_callback, i, &error_message);

    if (result == SQLITE_OK && *i == 0)
        {
            /* Table exists but there is no buffers to transmit to server */
            return FALSE;
        }
    else if (result == SQLITE_OK && *i > 0)
        {
            /* Table exists and there is buffers to transmit to server    */
            return TRUE;
        }
    else
        {
            /* result is not SQLITE_OK : something went wrong */
            return FALSE;
        }
}


/**
 * Transmits each row found in the database
 * @param userp is a pointer to a transmited_t * structure that must contain
 *        a comm_t * pointer and a db_t * pointer.
 * @param nb_col gives the number of columns in this row.
 * @param data contains the data of each column.
 * @param name_col contains the name of each column.
 * @returns always 0.
 */
static int transmit_callback(void *userp, int nb_col, char **data, char **name_col)
{
    transmited_t *trans = (transmited_t *) userp;
    gchar *sql_command = NULL;
    gint success = 0;

    if (trans != NULL && data != NULL && trans->comm != NULL && trans->database != NULL)
        {

            trans->comm->readbuffer = data[2];          /** data[2] is the data column in buffers table of the database */
            success = post_url(trans->comm, data[1]);   /** data[1] is the url column in buffers table of the database  */

            if (success == CURLE_OK)
                {
                    sql_begin(trans->database);

                    sql_command = g_strdup_printf("INSERT INTO transmited (buffer_id) VALUES ('%s');", data[0]);
                    exec_sql_cmd(trans->database, sql_command,  _("(%d) Error while inserting into the table 'transmited': %s\n"));
                    free_variable(sql_command);

                    sql_commit(trans->database);
                }
            /** @todo use the result of post to be able to manage errors */
        }

    return 0;
}


/**
 * Allocates a new structure to be passed sqlite3_exec callback in
 * db_transmit_buffers function.
 * @param database is the pointer to the db_t * structure.
 * @param comm_t is the pointer to the comm_t * structure.
 * @returns a newly allocated transmited_t * structure that may be freed
 *          when no longer needed.
 */
static transmited_t *new_transmited_t(db_t *database, comm_t *comm)
{
    transmited_t *trans = NULL;

    trans = (transmited_t *) g_malloc0(sizeof(transmited_t));

    trans->database = database;
    trans->comm = comm;

    return trans;
}


/**
 * Deletes all transmited buffers from the buffers table in database
 * based on transmited table.
 * @param userp is a pointer to a db_t * structure
 * @param nb_col gives the number of columns in this row.
 * @param data contains the data of each column.
 * @param name_col contains the name of each column.
 * @returns always 0.
 */
static int delete_transmited_callback(void *userp, int nb_col, char **data, char **name_col)
{
    db_t *database = (db_t *) userp;
    gchar *sql_command = NULL;

    if (database != NULL && data != NULL)
        {
            sql_begin(database);

            sql_command = g_strdup_printf("DELETE FROM buffers WHERE buffer_id='%s';", data[0]);
            exec_sql_cmd(database, sql_command,  _("(%d) Error while deleting from table 'buffers': %s\n"));
            free_variable(sql_command);

            sql_commit(database);
        }

    return 0;
}


/**
 * Deletes transmited buffers from table 'buffers' of the database
 * @param database is the pointer to the db_t * structure.
 * @returns the result of the sqlite3_exec() function
 */
static int delete_transmited_buffers(db_t *database)
{
    char *error_message = NULL;
    int result = 0;

    /* This should select every buffer_id that where transmited and not deleted (that are still present in buffers table) */
    result = sqlite3_exec(database->db, "SELECT transmited.buffer_id FROM transmited INNER JOIN buffers ON transmited.buffer_id = buffers.buffer_id;", delete_transmited_callback, database, &error_message);

    return result;
}


/**
 * This function transferts the 'buffers' that are stored in the database
 * @param database is the structure that contains everything that is
 *        related to the database (it's connexion for instance).
 * @param comm a comm_t * structure that must contain an initialized
 *        curl_handle (must not be NULL).
 * @returns TRUE if table 'buffers' is not empty and FALSE otherwise
 */
gboolean db_transmit_buffers(db_t *database, comm_t *comm)
{
    char *error_message = NULL;
    int result = 0;
    transmited_t *trans = NULL;

    trans = new_transmited_t(database, comm);

    /* This should select only the rows in buffers that are not in transmited based on the primary key buffer_id */
    result = sqlite3_exec(database->db, "SELECT * FROM buffers WHERE buffers.buffer_id NOT IN (SELECT transmited.buffer_id FROM transmited INNER JOIN buffers ON transmited.buffer_id = buffers.buffer_id);", transmit_callback, trans, &error_message);

    g_free(trans);

    delete_transmited_buffers(database);
    /** @todo Catch the return value of this function and do something with it */

    if (result == SQLITE_OK)
        {
            /* Table exists and buffers has been transmited to the server */
            return TRUE;
        }
    else
        {
            /* result is not SQLITE_OK : something went wrong but buffers may have been transmited to the server */
            return FALSE;
        }
}


/**
 * Binds a guint64 value into the prepared statement.
 * @note sqlite does not store unsigned 64 bits int but it does not matter
 *       as it stores the 64 bits. When we'll read the value we will read
 *       it to an unsigned 64 bits int recovering the exact value we saved.
 * @param db is the database concerned by stmt statement. It is only used
 *        here to print an error if any.
 * @param stmt is the prepared statement in which we want to bind the guint64
 *        'value' in 'name' parameter
 * @param name represents the name of the parameter in the prepared statement
 *        which we want to fill in with 'value'
 * @param value is a guint64 integer to be filled in 'name' parameter in the
 *        prepared statement.
 */
static void bind_guint64_value(sqlite3 *db, sqlite3_stmt *stmt, const gchar *name, guint64 value)
{
    int index = 0;
    int result = 0;

    if (stmt != NULL && name != NULL)
        {
            index = sqlite3_bind_parameter_index(stmt, name);
            result = sqlite3_bind_int64(stmt, index, value);
            print_on_db_error(db, result, name);
        }
}


/**
 * Binds a guint value into the prepared statement.
 * @note sqlite does not store unsigned int but it does not matter has
 *       it stores the 32 bits. When we'll read the value we will read it
 *       to an unsigned 32 bits int recovering the exact value we saved.
 * @param db is the database concerned by stmt statement. It is only used
 *        here to print an error if any.
 * @param stmt is the prepared statement in which we want to bind the guint
 *        'value' in 'name' parameter
 * @param name represents the name of the parameter in the prepared statement
 *        which we want to fill in with 'value'
 * @param value is a guint integer to be filled in 'name' parameter in the
 *        prepared statement.
 */
static void bind_guint_value(sqlite3 *db, sqlite3_stmt *stmt, const gchar *name, guint value)
{
    int index = 0;
    int result = 0;

    if (stmt != NULL && name != NULL)
        {
            index = sqlite3_bind_parameter_index(stmt, name);
            result = sqlite3_bind_int(stmt, index, value);
            print_on_db_error(db, result, name);
        }
}


/**
 * Binds a gchar *value into the prepared statement.
 * @param db is the database concerned by stmt statement. It is only used
 *        here to print an error if any.
 * @param stmt is the prepared statement in which we want to bind the string
 *        'value' in 'name' parameter
 * @param name represents the name of the parameter in the prepared statement
 *        which we want to fill in with 'value'
 * @param value is a gchar * string to be filled in 'name' parameter in the
 *        prepared statement.
 */
static void bind_text_value(sqlite3 *db, sqlite3_stmt *stmt, const gchar *name, gchar *value)
{
    int index = 0;
    int result = 0;

    if (stmt != NULL && name != NULL)
        {
            index = sqlite3_bind_parameter_index(stmt, name);
            result = sqlite3_bind_text(stmt, index, value, -1, NULL);
            print_on_db_error(db, result, name);
        }
}


/**
 * Binds values to the prepared statement
 * @param db is the concerned database where to save meta data
 * @param stmt is the prepared statement where we want to bind values
 * @param meta is the meta_data_t structure containing all meta data
 *        for one file. We want to bind those values into the prepared
 *        statement
 * @param only_meta : a gboolean that when set to TRUE only meta_data will
 *        be saved and hashs data will not ! FALSE means that something
 *        went wrong with server and that all data will be cached localy.
 * @param cache_time is the time (calculatde from epoch) when the cache
 *        has been created.
 */
static void bind_values_to_save_meta_data(sqlite3 *db, sqlite3_stmt *stmt, meta_data_t *meta, gboolean only_meta, guint64 cache_time)
{
    bind_guint64_value(db, stmt, ":cache_time", cache_time);
    bind_guint_value(db, stmt, ":type", meta->file_type);
    bind_guint64_value(db, stmt, ":inode", meta->inode);
    bind_text_value(db, stmt, ":file_user", meta->owner);
    bind_text_value(db, stmt, ":file_group", meta->group);
    bind_guint_value(db, stmt, ":uid", meta->uid);
    bind_guint_value(db, stmt, ":gid", meta->gid);
    bind_guint64_value(db, stmt, ":atime", meta->atime);
    bind_guint64_value(db, stmt, ":ctime", meta->ctime);
    bind_guint64_value(db, stmt, ":mtime", meta->mtime);
    bind_guint_value(db, stmt, ":mode", meta->mode);
    bind_guint64_value(db, stmt, ":size", meta->size);
    bind_text_value(db, stmt, ":name", meta->name);
    bind_guint_value(db, stmt, ":transmited", (guint) only_meta);
    bind_text_value(db, stmt, ":link", meta->link);
}


/**
 * Creates the statements that will be used to save file's meta data to
 * the database
 * @param db is an sqlite * pointer to an opened database.
 * @returns the newly created statement.
 */
static sqlite3_stmt *create_save_meta_stmt(sqlite3 *db)
{
    sqlite3_stmt *stmt = NULL;
    int result = 0;

    result = sqlite3_prepare_v2(db, "INSERT INTO files (cache_time, type, inode, file_user, file_group, uid, gid, atime, ctime, mtime, mode, size, name, transmitted, link) VALUES (:cache_time, :type, :inode, :file_user, :file_group, :uid, :gid, :atime, :ctime, :mtime, :mode, :size, :name, :transmited, :link);", -1, &stmt, NULL);
    print_on_db_error(db, result, "create_save_meta_stmt");

    return stmt;
}


/**
 * Binds values to the prepared statement
 * @param db is the concerned database where to save meta data
 * @param stmt is the prepared statement where we want to bind values
 */
static void bind_values_to_save_buffer(sqlite3 *db, sqlite3_stmt *stmt, gchar *url, gchar *buffer)
{

    bind_text_value(db, stmt, ":url", url);
    bind_text_value(db, stmt, ":data", buffer);
}



/**
 * Creates the statements that will be used to save buffer data and url
 * that should have been sent to the server
 * @param db is an sqlite * pointer to an opened database.
 * @returns the newly created statement.
 */
static sqlite3_stmt *create_save_buffer_stmt(sqlite3 *db)
{
    sqlite3_stmt *stmt = NULL;
    int result = 0;

    result = sqlite3_prepare_v2(db, "INSERT INTO buffers (url, data) VALUES (:url, :data);", -1, &stmt, NULL);
    print_on_db_error(db, result, "create_save_buffer_stmt");

    return stmt;
}


/**
 * Binds values to the prepared statement
 * @param db is the concerned database where to save meta data
 * @param stmt is the prepared statement where we want to bind values
 * @param meta is the meta_data_t structure containing all meta data
 *        for one file. We want to bind some of those values into the
 *        prepared statement
 */
static void bind_values_to_get_file_id(sqlite3 *db, sqlite3_stmt *stmt, meta_data_t *meta)
{
    bind_guint64_value(db, stmt, ":inode", meta->inode);
    bind_text_value(db, stmt, ":name", meta->name);
    bind_guint_value(db, stmt, ":file_type", meta->file_type);
    bind_guint_value(db, stmt, ":uid", meta->uid);
    bind_guint_value(db, stmt, ":gid", meta->gid);
    bind_guint64_value(db, stmt, ":ctime", meta->ctime);
    bind_guint64_value(db, stmt, ":mtime", meta->mtime);
    bind_guint_value(db, stmt, ":mode", meta->mode);
    bind_guint64_value(db, stmt, ":size", meta->size);
}


/**
 * Creates the statements that will be used to retrieve a file_id if it
 * exist with the given parameters.
 * @param db is an sqlite * pointer to an opened database.
 * @returns the newly created statement.
 */
static sqlite3_stmt *create_get_file_id_stmt(sqlite3 *db)
{
    sqlite3_stmt *stmt = NULL;
    int result = 0;

    result = sqlite3_prepare_v2(db, "SELECT file_id from files WHERE inode=:inode AND name=:name AND type=:file_type AND uid=:uid AND gid=:gid AND ctime=:ctime AND mtime=:mtime AND mode=:mode AND size=:size;", -1, &stmt, NULL);
    print_on_db_error(db, result, "create_save_buffer_stmt");

    return stmt;
}


/**
 * Creates a new stmt_t * strcuture
 * @param db is an sqlite * pointer to an opened database.
 * @returns a new unfilled stmt_t * structure which can be freed when
 *          no longer needed by calling free_stmts()
 */
static stmt_t *new_stmts(sqlite3 *db)
{
    stmt_t *stmts = NULL;


    stmts = (stmt_t *) g_malloc0(sizeof(stmt_t));

    stmts->save_meta_stmt = create_save_meta_stmt(db);
    stmts->save_buffer_stmt = create_save_buffer_stmt(db);
    stmts->get_file_id_stmt = create_get_file_id_stmt(db);

    return stmts;
}


/**
 * Frees a malloc'ed stmt_t * structure
 * @param stmts stmts is the stmt_t * structure to be freed
 */
static void free_stmts(stmt_t *stmts)
{

    if (stmts != NULL)
        {
            sqlite3_finalize(stmts->save_meta_stmt);
            sqlite3_finalize(stmts->save_buffer_stmt);
            sqlite3_finalize(stmts->get_file_id_stmt);
            g_free(stmts);
        }
}

/**
 * Allocates a new list_t * structure
 * @return a newly allocated list_t * structure that may be
 *         freed by free_list_t function when no longer needed
 */
static list_t *new_list_t(void)
{
    list_t *container = NULL;

    container = (list_t *) g_malloc0(sizeof(list_t));

    container->list = NULL;

    return container;
}


/**
 * Frees memory
 * @param container is a list_t structure to be freed
 */
static void free_list_t(list_t *container)
{
    if (container != NULL)
        {
            g_list_free_full(container->list, free_gchar_variable);
            free_variable(container);
        }
}


/**
 * Frees and closes the database connection.
 * @param database is a db_t * structure with an already openned connection
 *        that we want to close.
 */
void close_database(db_t *database)
{
    if (database != NULL)
        {
            print_debug(_("\tClosing database.\n"));
            free_stmts(database->stmts);
            sqlite3_close(database->db);
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
            print_db_error(db, _("(%d) Error while trying to open %s database: %s\n"), result, database_name, sqlite3_errmsg(db));
            free_variable(database);
            sqlite3_close(db);

            return NULL;
        }
    else
        {
            database->db = db;
            database->stmts = new_stmts(db);
            sqlite3_extended_result_codes(db, 1);
            verify_if_tables_exists(database);

            return database;
        }
}





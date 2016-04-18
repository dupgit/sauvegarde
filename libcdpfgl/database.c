/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    database.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2016 Olivier Delhomme
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
static int table_callback(void *num, int nbCol, char **data, char **nomCol);
static void verify_if_tables_exists(db_t *database);
static file_row_t *new_file_row_t(void);
static void free_file_row_t(file_row_t *row);
static int get_file_callback(void *a_row, int nb_col, char **data, char **name_col);
static file_row_t *get_file_id(db_t *database, meta_data_t *meta);
static void bind_guint64_value(sqlite3 *db, sqlite3_stmt *stmt, const gchar *name, guint64 value);
static void bind_guint_value(sqlite3 *db, sqlite3_stmt *stmt, const gchar *name, guint value);
static void bind_text_value(sqlite3 *db, sqlite3_stmt *stmt, const gchar *name, gchar *value);
static void bind_values_to_save_meta_data(sqlite3 *db, sqlite3_stmt *stmt, meta_data_t *meta, gboolean only_meta, guint64 cache_time);
static sqlite3_stmt *create_save_meta_stmt(sqlite3 *db);
static stmt_t *new_stmts(sqlite3 *db);
static void free_stmts(stmt_t *stmts);

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
    exec_sql_cmd(database, "BEGIN;",  _("(%d) Error openning the transaction: %s\n"));
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
    int *i = NULL;               /** int *i is used to count the number of row */

    i = (int *) g_malloc0(sizeof(int));
    *i = 0;

    /* Trying to get all the tables that are in the database */
    result = sqlite3_exec(database->db, "SELECT * FROM sqlite_master WHERE type='table';", table_callback, i, &error_message);

    if (result == SQLITE_OK && *i == 0)  /* No row (0) means that there is no table */
        {
            print_debug(_("Creating tables into the database\n"));

            /* The database does not contain any tables. So we have to create them.         */
            /* Creation of buffers table that contains checksums and their associated data */
            exec_sql_cmd(database, "CREATE TABLE buffers (buffer_id INTEGER PRIMARY KEY AUTOINCREMENT, url TEXT, data TEXT);", _("(%d) Error while creating database table 'buffers': %s\n"));

            /* Creation of transmited table that may contain id of transmited buffers if any + creation of its indexes */
            exec_sql_cmd(database, "CREATE TABLE transmited (buffer_id INTEGER PRIMARY KEY);", _("(%d) Error while creating database table 'transmited': %s\n"));

            exec_sql_cmd(database, "CREATE INDEX main.transmited_buffer_id ON transmited (buffer_id ASC)", _("(%d) Error while creating index 'transmited_buffer_id': %s\n"));

            /* Creation of files table that contains everything about a file */
            exec_sql_cmd(database, "CREATE TABLE files (file_id  INTEGER PRIMARY KEY AUTOINCREMENT, cache_time INTEGER, type INTEGER, inode INTEGER, file_user TEXT, file_group TEXT, uid INTEGER, gid INTEGER, atime INTEGER, ctime INTEGER, mtime INTEGER, mode INTEGER, size INTEGER, name TEXT, transmitted BOOL, link TEXT);", _("(%d) Error while creating database table 'files': %s\n"));

            exec_sql_cmd(database, "CREATE INDEX main.files_inodes ON files (inode ASC)", _("(%d) Error while creating index 'files_inodes': %s\n"));
        }

    free_variable(i);
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
    row->id_list = g_slist_append(row->id_list, g_strdup(data[0]));

    return 0;
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
    char *error_message = NULL;
    gchar *sql_command = NULL;
    int db_result = 0;

    row = new_file_row_t();

    sql_command = g_strdup_printf("SELECT file_id from files WHERE inode=%" G_GUINT64_FORMAT " AND name='%s' AND type=%d AND uid=%d AND gid=%d AND ctime=%" G_GUINT64_FORMAT " AND mtime=%" G_GUINT64_FORMAT " AND mode=%d AND size=%" G_GUINT64_FORMAT ";", meta->inode, meta->name, meta->file_type, meta->uid, meta->gid, meta->ctime, meta->mtime, meta->mode, meta->size);

    db_result = sqlite3_exec(database->db, sql_command, get_file_callback, row, &error_message);

    free_variable(sql_command);
    /* exec_sql_cmd(database, "COMMIT;",  _("(%d) Error commiting to the database: %s\n")); */

    if (db_result == SQLITE_OK)
        {
           return row;
        }
    else
        {
            print_db_error(database->db, _("(%d) Error while searching into the table 'files': %s\n"), db_result, error_message);
            return NULL; /* to avoid a compilation warning as we exited with failure in print_db_error */
        }
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
    row->id_list = NULL;

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
            g_slist_free_full(row->id_list, free_gchar_variable);
            free_variable(row);
        }
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
    int result = 0;
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
    gchar *sql_command = NULL;     /** gchar *sql_command is the command to be executed */

    if (database != NULL && url != NULL && buffer != NULL)
        {
            sql_begin(database);

            sql_command = g_strdup_printf("INSERT INTO buffers (url, data) VALUES ('%s', '%s');", url, buffer);
            exec_sql_cmd(database, sql_command,  _("(%d) Error while inserting into the table 'buffers': %s\n"));
            free_variable(sql_command);

            sql_commit(database);
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

    result = sqlite3_exec(database->db, "SELECT * FROM buffers WHERE buffers.buffer_id NOT IN (SELECT transmited.buffer_id FROM transmited INNER JOIN buffers ON transmited.buffer_id = buffers.buffer_id);", table_callback, i, &error_message);

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
    print_on_db_error(db, result, "sqlite3_prepare_v2");

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
            g_free(stmts);
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





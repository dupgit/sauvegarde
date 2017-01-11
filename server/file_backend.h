/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    file_backend.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 - 2017 Olivier Delhomme
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
 * @file server/file_backend.h
 *
 * This file contains all definition for the functions for the file backend
 * system.
 */

#ifndef _SERVER_FILE_BACKEND_H_
#define _SERVER_FILE_BACKEND_H_


/**
 * @def FILE_BACKEND_BUFFER_SIZE
 * Defines the buffer size used to read files in the file_backend backend.
 * Should be at least as big as CLIENT_BLOCK_SIZE
 */
#define FILE_BACKEND_BUFFER_SIZE (65536)


/**
 * @def FILE_BACKEND_LEVEL
 * Defines default level for directory creation
 */
#define FILE_BACKEND_LEVEL (2)

/**
 * @struct file_backend_t
 * @brief Structure that contains everything needed by file backend.
 *
 * This structure contains the prefix for the path where data are located.
 * and a level that indicates the number of level of directories
 * indirections. Default value is 2 but 3 or 4 may be used.
 * A level of 2 with a block size of 16384 bytes should be sufficient
 * to store up to 512 Gbytes of deduplicated data. A level of 3 should be
 * ok up to 256 tera bytes of deduplicated data. A level of 4 should be ok
 * for up to 65536 tera bytes !
 */
typedef struct
{
    gchar *prefix; /**< Prefix for the path where data are located */
    guint level;   /**< level of directories defaults to 3 */
} file_backend_t;



/**
 * @struct buffer_t
 * @brief used to know where we are in the buffer when extracting lines of
 *        it.
 */
typedef struct
{
    GFileInputStream *stream;   /**< Stream from which we want to read things */
    gssize pos;                 /**< Position into the buffer                 */
    gssize size;                /**< number of bytes read into the buffer     */
    gchar *buf;                 /**< buffer read                              */
} buffer_t;



/**
 * Stores meta data into a flat file. A file is created for each host that
 * sends meta data. This code is not thread safe (it means that this is
 * not safe to call it from different threads unless some mechanism
 * garantees that a write will never occur in the same file at the same
 * time.
 * @param server_struct is the server's main structure where all
 *        informations needed by the program are stored.
 * @param smeta the server's structure for file meta data. It contains the
 *        hostname that sent it. This structure MUST be freed by this
 *        function.
 */
extern void file_store_smeta(server_struct_t *server_struct, server_meta_data_t *smeta);


/**
 * Inits the backend : takes care of the directories we want to write to.
 * user_data of the backend structure is a gchar * that represents the
 * prefix path where to store data.
 * @param server_struct is the server's main structure where all
 *        informations needed by the program are stored.
 */
extern void file_init_backend(server_struct_t *server_struct);


/**
 * Stores data into a flat file. The file is named by its hash in hex
 * representation (one should easily check that the sha256sum of such a
 * file gives its name !).
 * @param server_struct is the server's main structure where all
 *        informations needed by the program are stored.
 * @param hash_data is a hash_data_t * structure that contains the hash and
 *        the corresponding data in a binary form and a 'read' field that
 *        contains the number of bytes in 'data' field.
 */
extern void file_store_data(server_struct_t *server_struct, hash_data_t *hash_data);


/**
 * Builds a list of hashs that server's server needs.
 * @param server_struct is the server's main structure where all
 *        informations needed by the program are stored.
 * @param hash_list is the list of hashs that we have to check for.
 */
extern GList *file_build_needed_hash_list(server_struct_t *server_struct, GList *hash_list);


/**
 * Gets the list of all saved files
 * @param server_struct is the structure that contains all data for the
 *        server.
 * @param query is the structure that contains everything about the
 *        requested query.
 * @returns a JSON string containing all filenames requested
 */
extern gchar *file_get_list_of_files(server_struct_t *server_struct, query_t *query);


/**
 * Retrieves data from a flat file. The file is named by its hash in hex
 * representation (one should easily check that the sha256sum of such a
 * file gives its name !).
 * @param server_struct is the server's main structure where all
 *        informations needed by the program are stored.
 * @param hex_hash is a gchar * hash in hexadecimal format as retrieved
 *        from the url.
 */
extern hash_data_t *file_retrieve_data(server_struct_t *server_struct, gchar *hex_hash);

#endif /* #ifndef _SERVER_FILE_BACKEND_H_ */

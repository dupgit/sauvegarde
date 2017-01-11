/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    backend.h
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
 * @file server/backend.h
 *
 * This file contains all the definitions for the backend management of
 * cdpfglserver's server.
 */
#ifndef _SERVER_BACKEND_H_
#define _SERVER_BACKEND_H_

/**
 * Function templates definition to be used by backend_t structure.
 * void * pointers are ment to be server_struct_t * pointers.
 * @todo have a function that will say if a hash and its associated data
 *       have already been stored.
 * @todo have a function that will be called when reading configuration
 *       file to access some specific backend configurations.
 * @todo try to imagine a solution to append backend specific new options
 *       to the command line.
 */
typedef void (* store_smeta_func) (void *, server_meta_data_t *);   /**< Stores a server_meta_data_t structure according to the backend                            */
typedef void (* store_data_func) (void *, hash_data_t *);            /**< Stores a hash_data_t structure according to the backend                                    */
typedef GList * (* build_needed_hash_list_func) (void *, GList *);   /**< A function that will check if a hash is already known and build a list
                                                                      *   of needed hashs that the client may send                                                   */
typedef void (* init_backend_func) (void *);                         /**< A function that will initialize the backend if needed                                      */
typedef gchar * (* get_list_of_files_func) (void *, query_t *);      /**< A function that returns a JSON formatted string of saved files corresponding to the query  */
typedef hash_data_t * (* retrieve_data_func) (void *, gchar *);      /**< A function that returns the buffer associated to a specific hash                           */


/**
 * @struct backend_t
 * @brief This structure contains pointers to the selected backend functions.
 */
typedef struct
{
    store_smeta_func store_smeta;
    store_data_func store_data;
    build_needed_hash_list_func build_needed_hash_list;
    init_backend_func init_backend;
    get_list_of_files_func get_list_of_files;
    retrieve_data_func retrieve_data;
    void *user_data;                                     /**< user_data should be used by backends to store their own internal structure */
} backend_t;



/**
 * Inits the backend with the correct functions
 * @param store_smeta a function to store server_meta_data_t structure
 * @param store_data a function to store data
 * @param init_backend a function to init the backend
 * @param build_needed_hash_list a function that must build a GSList * needed hash list
 * @param get_list_of_files gets the list of saved files
 * @param retrieve_data retrieves data from a specified hash.
 * @returns a newly created backend_t structure initialized to nothing !
 */
extern backend_t *init_backend_structure(void *store_smeta, void *store_data, void *init_backend, void *build_needed_hash_list, void *get_list_of_files, void * retrieve_data);





#endif /* #ifndef _SERVER_BACKEND_H_ */

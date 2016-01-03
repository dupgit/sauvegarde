/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    backend.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 - 2016 Olivier Delhomme
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
 * @file server/backend.c
 *
 * This file contains all the functions for the backend management of
 * cdpfglserver's server.
 */

#include "server.h"


/**
 * Inits the backend with the correct functions
 * @todo write some backends !
 * @param store_smeta a function to store server_meta_data_t structure
 * @param store_data a function to store data
 * @param init_backend a function to init the backend
 * @param build_needed_hash_list a function that must build a GSList * needed hash list
 * @param get_list_of_files gets the list of saved files
 * @param retrieve_data retrieves data from a specified hash.
 * @returns a newly created backend_t structure initialized to nothing !
 */
backend_t *init_backend_structure(void *store_smeta, void *store_data, void *init_backend, void *build_needed_hash_list, void *get_list_of_files, void * retrieve_data)
{
    backend_t *backend = NULL;

    backend = (backend_t *) g_malloc0(sizeof(backend_t));

    backend->user_data = NULL;
    backend->store_smeta = store_smeta;
    backend->store_data = store_data;
    backend->init_backend = init_backend;
    backend->build_needed_hash_list = build_needed_hash_list;
    backend->get_list_of_files = get_list_of_files;
    backend->retrieve_data = retrieve_data;

    return backend;
}



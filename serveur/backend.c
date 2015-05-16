/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    backend.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 Olivier Delhomme
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
 * @file serveur/backend.c
 *
 * This file contains all the functions for the backend management of
 * serveur's server.
 */

#include "serveur.h"


/**
 * Inits the backend with the correct functions
 * @todo write some backends !
 * @param store_smeta a function to store serveur meta data structure
 * @param store_data a function to store datas
 * @param init_backend a function to init the backend
 * @param build_needed_hash_list a function that must build a GSList * needed hash list
 * @param get_list_of_files gets the list of saved files
 * @returns a newly created backend_t structure initialized to nothing !
 */
backend_t *init_backend_structure(void *store_smeta, void *store_data, void *init_backend, void *build_needed_hash_list, void *get_list_of_files)
{
    backend_t *backend = NULL;

    backend = (backend_t *) g_malloc0(sizeof(backend_t));

    backend->user_data = NULL;
    backend->store_smeta = store_smeta;
    backend->store_data = store_data;
    backend->init_backend = init_backend;
    backend->build_needed_hash_list = build_needed_hash_list;
    backend->get_list_of_files = get_list_of_files;

    return backend;
}


/**
 * Creates a query_t * structure filled with the corresponding datas
 * @param hostname hostname (where to look for)
 * @param uid uid for the file(s)
 * @param gid gid for the file(s)
 * @param owner owner for the file(s) hopefully corresponding to uid
 * @param group group for the file(s) hopefully corresponding to gid
 * @returns a newly allocated query_t * structure filled  with the
 *          corresponding datas that may be freed when no longer needed.
 */
query_t *init_query_structure(gchar *hostname, gchar *uid, gchar *gid, gchar *owner, gchar *group)
{
    query_t *query = NULL;

    query = (query_t *) g_malloc0(sizeof(query_t));

    query->hostname = hostname;
    query->uid = uid;
    query->gid = gid;
    query->owner = owner;
    query->group = group;

    return query;
}




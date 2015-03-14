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
 * @todo write some backend !
 * @param serveur_struct is the main serveur's structure that may contain
 *        informations needed to connect the right backend (when one will
 *        have a choice to make!)
 */
backend_t * init_backend_structure(serveur_struct_t *serveur_struct)
{
    backend_t *backend = NULL;

    backend = (backend_t *) g_malloc0(sizeof(backend_t));

    backend->store_smeta = NULL;

    return backend;
}

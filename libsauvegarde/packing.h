/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    packing.h
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
 * @file packing.h
 *
 * This file contains all the definitions of the functions and structures
 * to pack and unpack messages for all the programs of "Sauvegarde"
 * project.
 */
#ifndef _PACKING_H_
#define _PACKING_H_

/**
 * @def JANSSON_SUCCESS
 * Defines jansson success answer
 */
#define JANSSON_SUCCESS (0)


/**
 * This function should return a JSON string with all informations from
 * the meta_data_t structure.
 * @param meta is the structure that contains all meta data for a file or
 *        a directory.
 * @returns a JSON formated string
 */
gchar *convert_meta_data_to_json(meta_data_t *meta);


/**
 * This function should return a newly allocated meta_data_t * structure
 * with all informations included from the json string.
 * @param json_str is a gchar * contianing the JSON formated string.
 * @returns a newly_allocated meta_data_t * structure that can be freed
 *          with free_meta_data_t() function when no longer needed. This
 *          function can return NULL if json_str is NULL itself.
 */
meta_data_t *convert_json_to_meta_data(gchar *json_str);

#endif /* #ifndef _PACKING_H_ */

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    options.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2018 Olivier Delhomme
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
 * @file options.h
 * This file contains hedars of the functions thats deals with the common
 * options values of the "Sauvegarde" programs.
 */

#include "libcdpfgl.h"

#ifndef _LIBCDPFGL_OPTIONS_H_
#define _LIBCDPFGL_OPTIONS_H_


/**
 * @struct srv_conf_t
 * @brief Serveur common configuration parameters
 */
typedef struct
{
    gchar *ip;              /**< A string representing the IP address where server is located (may be a hotsname)             */
    gint port;              /**< Port number on which to send things to cdpfglserver's server (on which it must listen)       */
} srv_conf_t;

/**
 * @returns a newlly allocated srv_conf_t structure that can be freed
 *          with free_srv_conf_t() function
 */
extern srv_conf_t *new_srv_conf_t(void);


/**
 * Frees a srv_conf_t structture variable
 * @param srv_conf is a srv_conf_t * structure to be freed.
 */
extern void free_srv_conf_t(srv_conf_t *srv_conf);

/**
 * Reads keys in keyfile if groupname is in that keyfile and fills
 * options_t *opt structure accordingly.
 * @param[in,out] opt : options_t * structure to store options read from the
 *                configuration file "filename".
 * @param keyfile is the GKeyFile structure that is used by glib to read
 *        groups and keys from.
 * @param filename : the filename of the configuration file to read from
 */
extern srv_conf_t *read_from_group_server(GKeyFile *keyfile, gchar *filename);


/**
 * Drops configfile if it was already used and return filename
 * that will be the new configfile
 * @param configfile configuration filename to be freed if needed
 * @param filename the new configfilename
 * @returns a copy of filename string that may be freed with g_free()
 *          when no longer needed.
 */
extern gchar *manage_opt_configfile(gchar *configfile, gchar *filename);


/**
 * Drops srv_conf if it was already used and returns a new srv_conf_t
 * structure read from 'filename configuration file.
 * @param srv_conf to be dropped if used
 * @param keyfile keyfile file where to read keys configuration values
 * @returns a newly allocated srv conf_t * structure that may be freed
 *          with free_srv_conf_t() when no longer needed.
 */
extern srv_conf_t *manage_opt_srv_conf(srv_conf_t *srv_conf, GKeyFile *keyfile, gchar *filename);

#endif /* #ifndef _LIBCDPFGL_OPTIONS_H_ */

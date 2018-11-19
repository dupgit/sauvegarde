/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    options.c
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
 * @file options.c
 * This file contains the functions to deal with the common options values
 * of the "Sauvegarde" programs.
 */

#include "libcdpfgl.h"

/**
 *
 */
srv_conf_t *new_srv_conf_t(void)
{
    srv_conf_t *srv_conf = NULL;

    srv_conf = (srv_conf_t *) g_malloc0(sizeof(srv_conf));

    return srv_conf;
}


/**
 *
 *
 */
void free_srv_conf_t(srv_conf_t *srv_conf)
{
    if (srv_conf != NULL)
        {
            g_free(srv_conf);
        }
}


/**
 * Reads keys in keyfile if groupname is in that keyfile and fills
 * options_t *opt structure accordingly.
 * @param keyfile is the GKeyFile structure that is used by glib to read
 *        groups and keys from.
 * @param filename : the filename of the configuration file to read from
 * @returns a newlly allocated srv_conf_t * structure that can be freed
 * with free_srv_conf_t() when no longer needed.
 */
srv_conf_t *read_from_group_server(GKeyFile *keyfile, gchar *filename)
{
    gint port = 0;
    srv_conf_t *srv_conf = NULL;

    srv_conf = new_srv_conf_t();

    if (srv_conf != NULL && keyfile != NULL && filename != NULL && g_key_file_has_group(keyfile, GN_SERVER) == TRUE)
        {
            /* Reading the port number if any */
            port = read_int_from_file(keyfile, filename, GN_SERVER, KN_SERVER_PORT, _("Could not load server port number from file."), SERVER_PORT);

            if (port > 1024 && port < 65535)
                {
                    srv_conf->port = port;
                }
            /* @todo: put a warning in case of a wrong port number */

            /* Reading IP address of server's host if any */
            srv_conf->ip = read_string_from_file(keyfile, filename, GN_SERVER, KN_SERVER_IP, _("Could not load cache database name"));
        }

    return srv_conf;
}


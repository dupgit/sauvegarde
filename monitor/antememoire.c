/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    antememoire.c
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
 * @file antememoire.c
 * Here on should find everything that will deal with the cache.
 */

#include "monitor.h"


/**
 * This function is a thread that is waiting to receive messages from
 * the checksum function and whose aim is to store somewhere the data
 * of a buffer that a been checksumed.
 * @param data : main_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
gpointer store_buffer_data(gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    meta_data_t *meta = NULL;
    db_t *database = NULL;
    msgpack_sbuffer *buffer = NULL;

    if (main_struct != NULL)
        {
            database = main_struct->database;

            do
                {
                    meta = g_async_queue_pop(main_struct->store_queue);

                    if (meta->name != NULL)   /* if name is null than it should not be processed */
                        {
                            buffer = pack_meta_data_t(meta);
                            if (buffer != NULL)
                                {
                                    send_packed_message(main_struct->comm, buffer);
                                }

                            print_debug(stdout, "Inserting into database file %s\n", meta->name);
                            insert_file_into_cache(database, meta, main_struct->hashs);
                        }
                }
            while (meta->name != NULL);   /* A null name means that we have to quit */
        }

    return NULL;
}

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
 * @returns a newly allocated meta_data_t * empty structure. We use 65534
 * as default uid and gid to avoid using 0 which is dedicated to a
 * priviledged user.
 */
 meta_data_t *new_meta_data_t(void)
 {
    meta_data_t *meta = NULL;

    meta = (meta_data_t *) g_malloc0(sizeof(meta_data_t));

    if (meta != NULL)
        {
            meta->file_type = 0;
            meta->mode = 0;
            meta->atime = 0;
            meta->ctime = 0;
            meta->mtime = 0;
            meta->owner = NULL;
            meta->group = NULL;
            meta->uid = 65534;  /* nfsnobody on my system ie unpriviledged user */
            meta->gid = 65534;  /* nfsnobody on my system ie unpriviledged user */
            meta->name = NULL;
            meta->hash_list = NULL;
        }

    return meta;
 }

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
    GSList *head = NULL;
    gint i = 0;
    guint8 *a_hash = NULL;
    gchar *string = NULL;
    data_t *a_data = NULL;

    if (main_struct != NULL)
        {
            do
                {

                    meta = g_async_queue_pop(main_struct->store_queue);

                    fprintf(stdout, "%s :\n", meta->name);
                    head = meta->hash_list;

                    while (head != NULL)
                        {
                            a_hash = head->data;
                            a_data = g_tree_lookup(main_struct->hashs->tree_hash, a_hash);

                            string = hash_to_string(a_hash);
                            fprintf(stdout, "%ld, %s\n", a_data->read, string);
                            free_variable(string);

                            head = g_slist_next(head);
                        }

                }
            while (meta->name != NULL);
        }

    return NULL;
}

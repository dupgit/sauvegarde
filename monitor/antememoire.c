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
 * This function has to split the messages received in order to store
 * everything properly in the database. It waits to get new messages
 * from the store queue (checksums).
 * @param main_struct is the main structure. It is used here to get the
 *        store queue
 * @param to_store : the message received to be splited and stored into
 *        a database.
 */
static gchar *split_messages(main_struct_t *main_struct, gchar *to_store)
{
    int filetype = 0;
    char *owner = NULL;
    char *dates = NULL;
    char *mode = NULL;
    char *filename = NULL;
    long int read = 0;
    char *a_hash = NULL;
    char *checksum = NULL;
    long int i = 0;

    owner = (char *) g_malloc0(64);
    dates = (char *) g_malloc0(64);
    mode = (char *) g_malloc0(64);
    filename = (char *) g_malloc0(1024);
    a_hash = (char *) g_malloc0(HASH_LEN+1);

    if (to_store != NULL)
        {
            sscanf(to_store, "%d\n%s\n%s\n%s\n%s", &filetype, owner, dates, mode, filename);

            fprintf(stdout, "%d - %s - %s - %s - %s\n", filetype, owner, dates, mode, filename);

            if (filetype == G_FILE_TYPE_REGULAR)
                {
                    do
                        {
                            checksum = free_variable(checksum);
                            checksum = g_async_queue_pop(main_struct->store_queue);
                            if (checksum != NULL)
                                {
                                    sscanf(checksum, "-> %ld\n%ld\n%s", &i, &read, a_hash);
                                    fprintf(stdout, "%s\n", a_hash);
                                }
                        }
                    while (checksum[0] == '-');
                }
            else if (filetype == G_FILE_TYPE_DIRECTORY)
                {

                }
        }

    free_variable(owner);
    free_variable(dates);
    free_variable(mode);
    free_variable(filename);
    free_variable(a_hash);

    return checksum;
}


/**
 * @returns a newly allocated meta_data_t * empty structure. We use 65534
 * as default uid and gid to avoid using 0 which is dedicated to a
 * priviledged user.
 */
 meta_data_t *new_meta_data_t()
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
            hash_list = NULL;
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

    if (main_struct != NULL)
        {
            do
                {

                    meta = g_async_queue_pop(main_struct->store_queue);

                }
            while (1);
        }

    return NULL;
}

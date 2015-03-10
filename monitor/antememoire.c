/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    antememoire.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2015 Olivier Delhomme
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
 * Here on should find everything that will deal with the cache. All writes
 * to the cache should only occur in store_buffer_data()'s thread.
 */

#include "monitor.h"

/**
 * This function sends the datas that corresponds to the hashs in the json
 * formatted string's answer.
 */
static void send_datas_to_server(comm_t *comm, hashs_t *hashs, gchar *answer)
{
    data_t *a_data = NULL;
    json_t *root = NULL;
    json_error_t error;                /** json_error_t *error will handle json errors  */
    GSList *hash_list = NULL;
    GSList *head = NULL;
    gint success = CURLE_FAILED_INIT;

    if (hashs != NULL && answer != NULL)
        {

            root = json_loads(answer, 0, &error);

            if (root != NULL)
                {

                    hash_list = extract_gslist_from_array(root, "hash_list");
                    head = hash_list;

                    while (hash_list != NULL)
                        {
                            a_data = g_tree_lookup(hashs->tree_hash, hash_list->data);

                            if (a_data != NULL)
                                {
                                    comm->buffer = convert_data_to_json(a_data, hash_list->data);
                                    success = post_url(comm, "/Data.json");

                                    if (success == CURLE_OK)
                                        {
                                            a_data->buffer = free_variable(a_data->buffer);
                                            a_data->read = 0;
                                            a_data->into_cache = FALSE;
                                        }
                                }
                            else if (a_data == NULL)
                                {
                                    print_error(__FILE__, __LINE__, "Error, some data may be missing : unable to find datas for hash\n");
                                }
                            free_variable(hash_list->data);
                            hash_list = g_slist_next(hash_list);
                        }

                    g_slist_free(head);
                }

            json_decref(root);
        }
}





/**
 * This function saves meta_data_t structure into the cache or to the
 * serveur's server (it then inserts hostname into the message that is
 * sent).
 * @param meta : the meta_data_t * structure to be saved.
 * @param main_struct : main structure of the program (contains pointers
 *        to the communication socket, the cache database connexion and the
 *        balanced binary tree that contains hashs.
 */
static void send_meta_data_to_serveur_or_store_into_cache(meta_data_t *meta, main_struct_t *main_struct)
{
    gchar *json_str = NULL;
    gint success = CURLE_FAILED_INIT;

    if (main_struct != NULL && meta != NULL && main_struct->hostname != NULL)
        {
            json_str = convert_meta_data_to_json(meta, main_struct->hostname);

            print_debug(_("json string of %ld bytes is beeing sent\n"), strlen(json_str));

            /* sends message here */
            main_struct->comm->buffer = json_str;
            success = post_url(main_struct->comm, "/Meta.json");

            json_str = free_variable(json_str);

            if (success == CURLE_OK)
                {   /**
                     * Message has been sent and an answer has been received correctly.
                     * This answer is a list of hashs that the server needs. So we want
                     * to send the datas that corresponds with the hashs. Answer is in
                     * comm->buffer
                     */
                    send_datas_to_server(main_struct->comm, main_struct->hashs, main_struct->comm->buffer);

                }
            else
                {   /* Something went wrong when sending the datas and thus we have to store them localy. */

                    /** @note insert_file_into_cache is fast but does not garantee that the data is on the disk ! */
                    print_debug("Inserting into database cache file %s\n", meta->name);
                    insert_file_into_cache(main_struct->database, meta, main_struct->hashs);
                }
        }
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
    capsule_t *capsule = NULL;

    if (main_struct != NULL)
        {

            if (main_struct->comm != NULL)
                {
                    get_url(main_struct->comm, "/Version");

                    do
                        {
                            capsule = g_async_queue_pop(main_struct->store_queue);
                            print_debug(_("A capsule (%d) has been received in store's thread\n"), capsule->command);

                            switch (capsule->command)
                                {
                                    case ENC_META_DATA:
                                        send_meta_data_to_serveur_or_store_into_cache((meta_data_t *) capsule->data, main_struct);
                                        /* capsule = free_variable(capsule); */
                                    break;

                                    case ENC_END:
                                    break;

                                    default :
                                        print_error(__FILE__, __LINE__, _("Error: default case !\n"));
                                    break;
                                }

                        }
                    while (capsule->command != ENC_END);

                    /* capsule = free_variable(capsule); */
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error while initializing libcurl.\n"));
                }
        }

    return NULL;
}

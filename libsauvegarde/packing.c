/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    packing.c
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
 * @file packing.c
 * This file contains the functions to pack and unpack messages for all the
 * programs of "Sauvegarde" project.
 */

#include "libsauvegarde.h"


/**
 * Packs into a buffer the meta_data_t structure
 * @param meta is the meta data to be serialized into buffer
 * @returns a serialized buffer (msgpack_sbuffer *)
 */
msgpack_sbuffer *pack_meta_data_t(meta_data_t *meta)
{
    /* Some of these lines are from the msgpack quick start guide */

    msgpack_sbuffer *buffer = NULL;  /** Buffer in which the datas will be packed */
    msgpack_packer *pk = NULL;       /** Packer structure used to pack datas      */
    gint array_size = 0;             /** Number of "objects" to pack into buffer  */
    size_t string_size = 0;          /** Used to store string sizes               */
    GSList *head = NULL;             /** Used to iterate over the hash list       */
    gchar *encoded_hash = NULL;      /** base64 encoded hash                      */

    head = meta->hash_list;

    array_size = 10 + g_slist_length(head);  /* from 'file_type' to 'name' + all hashs */

    /* creates buffer and serializer instance. */
    buffer = msgpack_sbuffer_new();
    pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);

    /* size of the array (number of objects to serialize) */
    msgpack_pack_array(pk, array_size);

    /* Serializing all objects of meta_data_t structure */
    msgpack_pack_uint8(pk, meta->file_type);
    msgpack_pack_uint32(pk, meta->mode);
    msgpack_pack_uint64(pk, meta->atime);
    msgpack_pack_uint64(pk, meta->ctime);
    msgpack_pack_uint64(pk, meta->mtime);

    string_size = strlen(meta->owner);
    msgpack_pack_raw(pk, string_size);
    msgpack_pack_raw_body(pk, meta->owner, string_size);

    string_size = strlen(meta->group);
    msgpack_pack_raw(pk, string_size);
    msgpack_pack_raw_body(pk, meta->group, string_size);

    msgpack_pack_uint32(pk, meta->uid);
    msgpack_pack_uint32(pk, meta->gid);

    string_size = strlen(meta->name);
    msgpack_pack_raw(pk, string_size);
    msgpack_pack_raw_body(pk, meta->name, string_size);

    while (head != NULL)
        {
            /* Encoding hashs into a base64 string should be easier to transmit over the wire */
            /* Memory will have to be freed once the message has been sent */
            encoded_hash = g_base64_encode((guchar*) head->data, HASH_LEN);

            string_size = strlen(encoded_hash);
            msgpack_pack_raw(pk, string_size);
            msgpack_pack_raw_body(pk, encoded_hash, string_size);

            head = g_slist_next(head);
        }

    return buffer;
}






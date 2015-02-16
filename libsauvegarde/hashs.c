/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    hashs.c
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
 * @file hashs.c
 * This file contains the functions to deal with hashs in all the programs
 * of "Sauvegarde" project.
 */

#include "libsauvegarde.h"

/**
 * Allocate a new hashs_t structure with a GTree
 * @returns a newly allocated hashs_t structure with a GTree initialized
 *          with compare_two_hashs function to sort the hashs.
 */
hashs_t *new_hash_struct(void)
{
    hashs_t *hashs = NULL;

    hashs = (hashs_t *) g_malloc0(sizeof(hashs_t));

    hashs->tree_hash = g_tree_new(compare_two_hashs);
    hashs->total_bytes = 0;
    hashs->in_bytes = 0;

    return hashs;
}


/**
 * Comparison function used with the GTree structure to sort hashs
 * properly.
 * @param a is a hash in a binary form (a guint8 *)
 * @param b is a hash in a binary form to be compared with a. Comparison is
 *        done comparing byte 1 of a an b, if there equal compares byte 2
 *        and so on. Worst case is when the two hashs are equals.
 * @returns a negative value if a < b, zero if a = b and a positive value
 * if a > b.
 */
gint compare_two_hashs(gconstpointer a, gconstpointer b)
{
    guint8 *hash_a = (guint8 *) a;
    guint8 *hash_b = (guint8 *) b;
    guint first = 0;
    guint second = 0;
    guint i = 0;

    if (a != NULL)
        {
            if (b != NULL) /* a and b are not NULL -> we can compare them */
                {
                    while (first == second && i < HASH_LEN)  /* we compare bytes from the hashs (byte to byte) */
                        {
                            first = (guint) hash_a[i];
                            second = (guint) hash_b[i];
                            i = i + 1;
                        }

                    if (i == HASH_LEN && first == second) /* a is equal to b */
                        {
                            return 0;
                        }
                    if (first < second) /* a is first */
                        {
                            return -1;
                        }
                    else /* b is first */
                        {
                            return 1;
                        }
                }
            else  /* a is not NULL but b is NULL (a is first) */
                {
                    return -1;
                }
        }
    else
        {
            if (b != NULL)  /* a is NULL and b is not NULL (b is first) */
                {
                    return 1;
                }
            else  /* a and b are NULL (they are equal) */
                {
                    return 0;
                }
        }
}


/**
 * A function to insert a binary hash into the GTree structure
 * @param hashs : the hash structure that contains the binary tree in which
 *        we want to insert the second parameter
 * @param a_hash is a hash in a binary form
 * @param buffer is the data whom checksum is a_hashs
 * @param read is the number of bytes in guchar *buffer
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 */
void insert_into_tree(hashs_t *hashs, guint8 *a_hash, guchar *buffer, gssize read, meta_data_t *meta)
{
    guint8 *a_hash_dup = NULL;  /** A hashs to be duplicated  */
    guchar *buffer_dup = NULL;  /** A duplicated buffer       */
    data_t *a_data = NULL;      /** Struture to store buffers */


    if (hashs != NULL && a_hash != NULL)
        {
            a_hash_dup = g_memdup(a_hash, HASH_LEN);
            buffer_dup = g_memdup(buffer, read);

            meta->hash_list = g_slist_prepend(meta->hash_list, a_hash_dup);

            hashs->total_bytes = hashs->total_bytes + read;

            if (g_tree_lookup(hashs->tree_hash, a_hash_dup) == NULL)
                {
                    hashs->in_bytes = hashs->in_bytes + read;
                    a_data = new_data_t_structure(buffer_dup, read, FALSE);
                    g_tree_insert(hashs->tree_hash, a_hash_dup, a_data); /* the checksum itself is the key to get buffer's data */
                }
        }
}


/**
 * Transforms a binary hashs into a printable string (gchar *)
 * @param a_hash is a hash in a binary form that we want to transform into
 *        a string.
 * @returns a string that conatins the hash in an hexadecimal form.
 */
gchar *hash_to_string(guint8 *a_hash)
{
    gchar *string = NULL;
    gchar *octet = NULL;
    guint i = 0;

    if (a_hash != NULL)
        {
            string = (gchar *) g_malloc0(HASH_LEN*2 + 1); /* two char per bytes */
            octet = (gchar *) g_malloc0(3);

            for(i = 0; i < HASH_LEN; i++)
                {
                    octet = g_strdup_printf("%02x", a_hash[i]);
                    memmove(string + i*2, octet, 2);
                }
        }

    free_variable(octet);
    return string;
}


/**
 * Creates a new data_t * structure populated with the buffer and its size.
 * @param buffer : the data to be stored
 * @param read : the size of that buffer
 * @param into_cache : says wether it is already into the cache (TRUE) or
 *        not (FALSE)
 * @returns a newly allocated data_t * structure that can be freed when no
 *         longer needed.
 */
data_t *new_data_t_structure(guchar *buffer, gssize read, gboolean into_cache)
{
    data_t *a_data;

    a_data = (data_t *) g_malloc0(sizeof(data_t));

    a_data->buffer = buffer;
    a_data->read = read;
    a_data->into_cache = into_cache;

    return a_data;
}


/**
 * Frees data buffer and returns NULL.
 * @param a_data : the stucture that contains buffer data and its size to
 * be freed
 * @returns always NULL.
 */
gpointer free_data_t_structure(data_t *a_data)
{

    if (a_data != NULL)
        {
            free_variable(a_data->buffer);
            free_variable(a_data);
        }

    return NULL;
}


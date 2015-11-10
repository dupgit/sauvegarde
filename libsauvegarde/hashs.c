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
 * Comparison function used to compare two hashs (binary form) mainly
 * used to sort hashs properly.
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
 * Transforms a binary hashs into a printable string (gchar *)
 * @param a_hash is a hash in a binary form that we want to transform into
 *        a string.
 * @returns a string that conatins the hash in an hexadecimal form.
 * @todo manage memory concerns here !
 */
gchar *hash_to_string(guint8 *a_hash)
{
    gchar *string = NULL;
    gchar *octet = NULL;
    guint i = 0;

    if (a_hash != NULL)
        {
            string = (gchar *) g_malloc0(HASH_LEN*2 + 1); /* two char per bytes */
            /* octet = (gchar *) g_malloc(3); */

            for(i = 0; i < HASH_LEN; i++)
                {
                    octet = g_strdup_printf("%02x", a_hash[i]);
                    memmove(string + i*2, octet, 2);
                    free_variable(octet);
                }
        }

    /* free_variable(octet); */
    return string;
}


/**
 * @param value is the gchar to be evaluated (should be 0 to 9 or a to f)
 * @returns the guint value of a gchar character.
 * @note this "stupid" function as a CCN of 17 ! Avoid complexification
 *       of this function if you want to go for a CCN below 10.
 */
static guint int_value(gchar value)
{
    switch (value)
        {
            case '0' :
                return 0;
                break;
            case '1' :
                return 1;
                break;
            case '2' :
                return 2;
                break;
            case '3' :
                return 3;
                break;
            case '4' :
                return 4;
                break;
            case '5' :
                return 5;
                break;
            case '6' :
                return 6;
                break;
            case '7' :
                return 7;
                break;
            case '8' :
                return 8;
                break;
            case '9' :
                return 9;
                break;
            case 'a' :
                return 10;
                break;
            case 'b' :
                return 11;
                break;
            case 'c' :
                return 12;
                break;
            case 'd' :
                return 13;
                break;
            case 'e' :
                return 14;
                break;
            case 'f' :
                return 15;
                break;
            default :
                return 0;  /* This default case should never happen */
                break;
        }
}


/**
 * Transforms a binary hashs into a printable string (gchar *)
 * @param str_hash a string (gchar *) that conatins the hash in an
 *        hexadecimal form.
 * @returns a hash in a binary form (guint8 *).
 */
guint8 *string_to_hash(gchar *str_hash)
{
    guint8 *string = NULL;
    guint8 octet = 0;
    guint i = 0;

    if (str_hash != NULL)
        {
            string = (guint8 *) g_malloc0(HASH_LEN + 1); /* two char per bytes */

            for(i = 0; i < HASH_LEN * 2; i = i + 2)
                {
                    octet = int_value(str_hash[i])*16 + int_value(str_hash[i+1]);
                    memmove(string + i/2, &octet, 1);
                }
        }

    return string;
}


/**
 * Frees hash_data_t *buffer and returns NULL.
 * @param hash_data : the stucture that contains buffer data, hash data
 *        and its size to be freed.
 * @returns always NULL.
 */
gpointer free_hash_data_t_structure(hash_data_t *hash_data)
{

    if (hash_data != NULL)
        {
            free_variable(hash_data->data);
            free_variable(hash_data->hash);
            free_variable(hash_data);
        }

    return NULL;
}


/**
 * handler for g_slist_free_full
 * @param data must be a hash_data_t * structure.
 */
void free_hdt_struct(gpointer data)
{
    free_hash_data_t_structure(data);
}


/**
 * Inits and returns a newly hash_data_t structure.
 * @returns a newly hash_data_t structure.
 */
hash_data_t *new_hash_data_t(guchar *data, gssize read, guint8 *hash)
{
    hash_data_t *hash_data = NULL;

    hash_data = (hash_data_t *) g_malloc(sizeof(hash_data_t));

    hash_data->hash = hash;
    hash_data->data = data;
    hash_data->read = read;

    return hash_data;
}


/**
 * Converts the hash list to a list of comma separated hashs in one gchar *
 * string. Hashs are base64 encoded
 * @param hash_list a GList of hash_data_t * elements
 * @returns a list of comma separated hashs in one gchar * string.
 */
gchar *convert_hash_data_list_to_gchar(GList *hash_list)
{
    GList *head = hash_list;
    gchar *base64 = NULL;
    gchar *list = NULL;
    gchar *old_list = NULL;
    hash_data_t *hash_data = NULL;

    while (head != NULL)
        {
            hash_data = head->data;
            base64 = g_base64_encode(hash_data->hash, HASH_LEN);

            if (old_list == NULL)
                {
                    list = g_strdup_printf("\"%s\"", base64);
                    old_list = list;
                }
            else
                {
                    list = g_strdup_printf("%s, \"%s\"", old_list, base64);
                    free_variable(old_list);
                    old_list = list;
                }

            free_variable(base64);

            head = g_list_next(head);
        }

    list = old_list;

    return list;
}


/**
 * Makes a path from a binary hash : 0E/39/AF for level 3 with hash (in hex)
 * begining by 0E39AF.
 * @param path is a gchar * prefix for the path (ie /var/tmp/sauvegarde for
 *        instance).
 * @param hash is a guint8 pointer to the binary representation of a hash.
 * @param level The level we want the path to have. It is an unsigned int
 *        and must be less than HASH_LEN. a level of N gives 2^N
 *        directories. We should add a level when more than 512 files are
 *        in each last subdirectories.
 * @returns a string as a gchar * made of the path and the hex
 *          representation of hash on 'level' levels. With the example above
 *          it will return /var/tmp/sauvegarde/0E/39/AF
 */
gchar *make_path_from_hash(gchar *path, guint8 *hash, guint level)
{
    gchar *octet = NULL;
    gchar *old_path = NULL;
    gchar *new_path = NULL;
    guint i = 0;

    if (path != NULL && hash != NULL && level < HASH_LEN)
        {

            old_path = g_strdup(path);

            for(i = 0; i < level; i++)
                {
                    octet = g_strdup_printf("%02x", hash[i]);
                    new_path = g_build_filename(old_path, octet, NULL);

                    free_variable(old_path);
                    free_variable(octet);

                    old_path = new_path;
                }
        }

    return old_path;
}


/**
 * makes a GSList of hash_data_t * element where 'hash' field is base64
 * decoded hashs from a string containning base64 * encoded hashs that
 * must be separated by comas.
 * @param the string containing base64 encoded hashs such as : *
 *        "cCoCVkt/AABf04jn2+rfDmqJaln6P2A9uKolBjEFJV4=", "0G8MaPZ/AADNyaPW7ZP2s0BI4hAdZZIE2xO1EwdOzhE="
 *        for instance.
 * @returns a GSList of hash_data_t * where each elements contains a
 *          base64 decoded hash (binary form).
 */
GList *make_hash_data_list_from_string(gchar *hash_string)
{
    uint i = 0;
    gchar **hashs = NULL;
    gchar *a_hash = NULL;
    hash_data_t *hash_data = NULL;
    GList *hash_list = NULL;
    gsize len = 0;

    if (hash_string != NULL)
        {
            /* hash list generation */
            hashs = g_strsplit(hash_string, ",", -1);

            while (hashs[i] != NULL)
                {
                    a_hash = g_strndup(g_strchug(hashs[i] + 1), strlen(g_strchug(hashs[i])) - 2);

                    /* we have to base64 decode it to insert it into the hash_data_t * structure
                     * and then into the meta_data one.
                     */
                    hash_data = new_hash_data_t(NULL, 0, g_base64_decode(a_hash, &len));
                    hash_list = g_list_prepend(hash_list, hash_data);
                    free_variable(a_hash);
                    i = i + 1;
                }

            g_strfreev(hashs);

            hash_list = g_list_reverse(hash_list);
        }

    return hash_list;
}


/**
 * Tells wheter a hash (picking it in a hash_data_t structure is in the
 * needed list of hash_data_t structures.
 * @param hash_data contains the hash that we are looking for into the
 *        needed list.
 * @param needed is a GList of hash_data_t structures that may already
 *        contain one with the same hash than the one in hash_data
 * @returns TRUE if the hash is found, FALSE otherwise
 */
gboolean hash_data_is_in_list(hash_data_t *hash_data, GList *needed)
{
    gboolean found = FALSE;
    hash_data_t *needed_hash_data = NULL;

    if (hash_data != NULL)
        {
            while (needed != NULL && found == FALSE)
                {
                    needed_hash_data = needed->data;
                    if (compare_two_hashs(hash_data->hash, needed_hash_data->hash) == 0)
                        {
                            found = TRUE;
                        }
                    else
                        {
                            needed = g_list_next(needed);
                        }
                }
        }

    return found;
}

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    hashs.c
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
            if (b != NULL)
                {
                    while (first == second && i < HASH_LEN)
                        {
                            first = (guint) hash_a[i];
                            second = (guint) hash_b[i];
                            i = i + 1;
                        }

                    if (i == HASH_LEN)
                        {
                            return 0;
                        }
                    if (first < second)
                        {
                            return -1;
                        }
                    else
                        {
                            return 1;
                        }
                }
            else
                {
                    return -1;
                }
        }
    else
        {
            if (b != NULL)
                {
                    return 1;
                }
            else
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
 */
void insert_into_tree(hashs_t *hashs, guint8 *a_hash)
{
    guint8 *a_hash_dup = NULL;  /** A hashs to be duplicated */


    if (hashs != NULL && a_hash != NULL)
        {
            a_hash_dup = g_memdup(a_hash, HASH_LEN);
            g_tree_insert(hashs->tree_hash, a_hash_dup, NULL);
        }
}

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
 * Comparison function used with the GTree structure to sort hashs
 * properly.
 * @returns a negative value if a < b, zero if a = b and a positive value
 * if a > b.
 */
gint compare_two_hashs(gconstpointer a, gconstpointer b)
{
    gchar *hash_a = (gchar *) a;
    gchar *hash_b = (gchar *) b;
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

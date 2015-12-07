/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    clock.c
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
 * @file clock.c
 * This file contains the functions to measure time.
 */

#include "libcdpfgl.h"

/**
 * Creates a new clock_t structure filled accordingly
 * @returns a clock_t * structure with begin field set.
 */
a_clock_t *new_clock_t(void)
{
    a_clock_t * my_clock = NULL;

    my_clock = (a_clock_t *) g_malloc0(sizeof(a_clock_t));

    my_clock->end = NULL;
    my_clock->begin = g_date_time_new_now_local();

    return my_clock;
}


/**
 * Ends the clock and prints the elapsed time and then frees everything
 * @param my_clock is a clock_t * structure with begin already filled
 * @param message is a message that we want to include into the displayed
 *        result in order to know what was measured.
 */
void end_clock(a_clock_t *my_clock, gchar *message)
{
    GTimeSpan difference = 0;

    if (my_clock != NULL && my_clock->begin != NULL)
        {
            my_clock->end = g_date_time_new_now_local();
            difference = g_date_time_difference(my_clock->end, my_clock->begin);

            g_date_time_unref(my_clock->begin);
            g_date_time_unref(my_clock->end);
            free_variable(my_clock);

            print_debug(_("Elapsed time (%s): %d\n"), message, difference);
        }
}




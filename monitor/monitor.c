/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    monitor.c
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
 * @file monitor.c
 *
 * This file is the main file for the monitor program. This monitor
 * program has to monitor file changes onto filesystems. It should notice
 * when a file is created, deleted or changed
 */

#include "monitor.h"

static void print_libraries_versions(void);


/**
 * Prints version of the libraries we are using.
 */
static void print_libraries_versions(void)
{

    fprintf(stdout, "GLIB version : %d.%d.%d\n", glib_major_version, glib_minor_version, glib_micro_version);

}


/**
 * main function
 */
int main(int argc, char **argv)
{

    g_type_init();

    print_libraries_versions();

    return 0;
}

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    options.h
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
 * @file options.h
 *
 *  This file contains all the definitions for the options of the command
 *  line.
 */
#ifndef _OPTIONS_H_
#define _OPTIONS_H_

/**
 * @struct options_t
 * Structure Options gives a way to store program options passed from the
 * command line.
 */
typedef struct
{
    gboolean version;     /** TRUE if we have to display program's version                                      */
    GSList *dirname_list; /** Directory names that were left in the command line                                */
    gint64 blocksize;     /** block size in bytes                                                               */
    gchar *configfile;    /** filename for the configuration file specified on the command line                 */
    gboolean noprint;     /** FALSE by default, TRUE if we want to quiet the program while calculating checsums */
    gchar *dircache;      /** Directory where we will cache files and temporary stuff to do the job             */
} options_t;


/**
 * This function parses command line options.
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 */
extern options_t *manage_command_line_options(int argc, char **argv);


/**
 * Frees the options structure if necessary
 * @param opt : the malloc'ed options_t structure
 */
extern void free_options_t_structure(options_t *opt);


/**
 * Decides what to do upon command lines options passed to the program
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option (in manage_command_line_options function).
 */
extern options_t *do_what_is_needed_from_command_line_options(int argc, char **argv);


#endif /* #IFNDEF _OPTIONS_H_ */

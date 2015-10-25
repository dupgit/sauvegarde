/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    options.h
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
 * @file serveur/options.h
 *
 * This file contains all the definitions for the options of the command
 * line for 'cdpfglserver' program. It is very similar to monitor's options.h
 * file.
 *
 */
#ifndef _SERVEUR_OPTIONS_H_
#define _SERVEUR_OPTIONS_H_

/**
 * @struct options_t
 * @brief Selected options for 'cdpfglserver' program
 *
 * Structure Options gives a way to store program options passed from the
 * command line or read from a configuration file for 'cdpfglserver' program.
 */
typedef struct
{
    gboolean version;   /**< TRUE if we have to display program's version                             */
    gchar *configfile;  /**< filename for the configuration file specified on the command line        */
    gint port;          /**< port number on which the cdpfglserver program will listen for connexions */
} options_t;



/**
 * Frees the options structure if necessary
 * @param opt : the malloc'ed options_t structure
 */
extern void free_options_t_structure(options_t *opt);

/**
 * creates a buffer containing every selected options ...
 * @param opt the options_t * structure that contains all selected options
 *        from the command line and that will be used by the program.
 * @returns options as selected when invoking the program with -v option
 * into a newly allocated buffer that may be freed when no longer needed
 */
extern gchar *buffer_selected_option(options_t *opt);


/**
 * This function parses command line options. It sets the options in this
 * order. It means that the value used for an option is the one set in the
 * lastest step.
 * 0) default values are set into the options_t * structure
 * 1) reads the default configuration file if any.
 * 2) reads the configuration file mentionned on the command line.
 * 3) sets the command line options (except for the list of directories,
 *    all other values are replaced by thoses in the command line)
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option
 */
extern options_t *manage_command_line_options(int argc, char **argv);


/**
 * Decides what to do upon command lines options passed to the program
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option (in manage_command_line_options function).
 */
extern options_t *do_what_is_needed_from_command_line_options(int argc, char **argv);


#endif /* #IFNDEF _SERVEUR_OPTIONS_H_ */

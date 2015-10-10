/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    options.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 Olivier Delhomme
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
 * @file restaure/options.h
 *
 * This file contains all the definitions for the options of the command
 * line for 'restaure' program.
 *
 */
#ifndef _RESTAURE_OPTIONS_H_
#define _RESTAURE_OPTIONS_H_


/**
 * @struct options_t
 * @brief Selected options for 'serveur' program
 *
 * Structure Options gives a way to store program options passed from the
 * command line or read from a configuration file for 'serveur' program.
 *
 * list, restore, date and where are pure command line options that will
 * not (unless requested by some user) be in the configuration file.
 */
typedef struct
{
    gboolean version;   /**< TRUE if we have to display program's version                                           */
    gchar *list;        /**< Should contain a filename to be searched into saved files filename's list              */
    gchar *restore;     /**< Must contain a filename or a directory name to be restored (latest version by default  */
    gchar *date;        /**< Should contain a date in the correct format to filter only files at that specific date */
    gchar *configfile;  /**< Filename for the configuration file specified on the command line                      */
    gchar *ip;          /**< A string representing the IP address where serveur is located (may be a hotsname)      */
    gint port;          /**< Port number on which to send things to serveur's server (on which it must listen)      */
    gchar *where;       /**< where is a string that should contain a directory where to restore a file / dirtectory */
} options_t;


/**
 * Decides what to do upon command lines options passed to the program
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns options_t structure malloc'ed and filled upon choosen command
 *          line's option (in manage_command_line_options function).
 */
extern options_t *do_what_is_needed_from_command_line_options(int argc, char **argv);


/**
 * Frees the option structure
 * @param opt is the structure to be freed
 */
extern void free_options_t_structure(options_t *opt);

#endif /* #ifndef _RESTAURE_OPTIONS_H_ */

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    libsauvegarde.h
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
 * @file libsauvegarde.h
 *
 *  This file contains all the definitions for the common tools of
 * "Sauvegarde" collection programs.
 */
#ifndef _LIBSAUVEGARDE_H_
#define _LIBSAUVEGARDE_H_

/* Configuration from ./configure script */
#include "config.h"

#define MHD_PLATFORM_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <zmq.h>
#include <czmq.h>
#include <sqlite3.h>
#include <jansson.h>
#include <curl/curl.h>

#include "configuration.h"
#include "communique.h"
#include "files.h"
#include "hashs.h"
#include "database.h"
#include "packing.h"


/**
 * Returns a newly allocated buffer that contains all informations about
 * the version of the libraries we are using.
 * @param name : name of the program of which we want to print the version.
 */
extern gchar *buffer_libraries_versions(gchar *name);

/**
 * Prints version of the libraries we are using.
 * @param name : name of the program of which we want to print the version.
 */
extern void print_libraries_versions(gchar *name);

/**
 * returns a newly allocated buffer that contains all informations about
 * program's version, authors and license.
 * @param name : name of the program of which we want to print the version.
 * @param date : publication date of this version
 * @param version : version of the program.
 * @param authors : authors that contributed to this program
 * @param license : license in use for this program and its sources
 */
extern gchar *buffer_program_version(gchar *name, gchar *date, gchar *version, gchar *authors, gchar *license);

/**
 * Prints the version of the program.
 * All parameters are of (gchar *) type.
 * @param name : name of the program of which we want to print the version.
 * @param date : publication date of this version
 * @param version : version of the program.
 * @param authors : authors that contributed to this program
 * @param license : license in use for this program and its sources
 */
extern void print_program_version(gchar *name, gchar *date, gchar *version, gchar *authors, gchar *license);


/**
 *  Inits internationalization domain for sauvegarde project
 */
extern void init_international_languages(void);


/**
 * Sets options parameters
 * @param context is the context for options it must have been created
 *        previously and not NULL.
 * @param entries are the entries for the options.
 * @param help is a boolean to choose if we want GOption to display
 *        an automaticaly formatted help.
 * @param bugreport is the message we want to display related to bug
 *        reports. It is displayed at the end of the options help message.
 * @param summary is a gchar* string that will be displayed before the
 *        description of the options. It is supposed to be a summary of
 *        what the program does.
 */
extern void set_option_context_options(GOptionContext *context, GOptionEntry entries[], gboolean help, gchar *bugreport, gchar *summary);


/**
 * Frees a pointer if it is not NULL and returns NULL
 * @param to_free is the pointer to be freed (must have been malloc with
 *         g_malloc* functions).
 * @returns NULL
 */
extern gpointer free_variable(gpointer to_free);


/**
 * Unrefs an object if it is not NULL and returns NULL
 * @param object_to_unref is the pointer to be unref'ed.
 * @returns NULL
 */
extern gpointer free_object(gpointer object_to_unref);


/**
 * Frees an error if it exists and return NULL
 * @param error : the error to be freed
 * @returns NULL
 */
extern gpointer free_error(gpointer error);


/**
 * Prints a message if the debug flag is set
 * @param stream : a FILE * file to print into (stdout, stderr, ...)
 * @param format : the format of the message (as in printf)
 * @param ... : va_list of variable that are to be printed into format.
 */
extern void print_debug(FILE *stream, const char *format, ...);


#if !GLIB_CHECK_VERSION(2, 31, 0)
/**
 * defines a wrapper to the g_thread_create function used in glib before
 * 2.31
 */
extern GThread *g_thread_new(const gchar *unused, GThreadFunc func, gpointer data);
#endif


/**
 * Tries to create a directory
 * @param directory is the gchar * string that contains a directory name
 *        to be created (does nothing if it exists).
 */
extern void create_directory(gchar *directory);


/**
 * A signal handler for SIGPIPE (needed by libmicrohttpd in order to be
 * portable.
 */
extern void ignore_sigpipe(void);


#endif /* #ifndef _LIBSAUVEGARDE_H_ */

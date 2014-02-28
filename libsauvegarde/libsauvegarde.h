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

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n-lib.h>

#include "configuration.h"

/**
 * Prints version of the libraries we are using.
 */
extern void print_libraries_versions(void);


/**
 * Prints the version of the program.
 * @param date : publication date of this version
 * @param authors : authors that contributed to this program
 * @param license : license in use for this program and its sources
 */
extern void print_program_version(gchar *date, gchar *authors, gchar *license);

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

#endif /* #ifndef _LIBSAUVEGARDE_H_ */
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    libsauvegarde.c
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
 * @file libsauvegarde.c
 * This library contains all tools that may be used in at least two of
 * the "Sauvegarde" programs.
 */

#include "libsauvegarde.h"


/**
 * Prints version of the libraries we are using.
 */
void print_libraries_versions(void)
{
    gchar *comm_version = NULL;

    fprintf(stdout, _("%s was compiled with the following libraries:\n"), PACKAGE_NAME);
    fprintf(stdout, _("\t. GLIB version : %d.%d.%d\n"), glib_major_version, glib_minor_version, glib_micro_version);

    comm_version = get_communication_library_version();
    if (comm_version != NULL)
        {
            fprintf(stdout, "%s", comm_version);
            free_variable(comm_version);
        }
    fprintf(stdout, _("\t. %s version : %s\n"), DATABASE_NAME, db_version());
    fprintf(stdout, _("\t. JANSSON version : %d.%d.%d\n"), JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION, JANSSON_MICRO_VERSION);
}


/**
 * Prints the version of the program.
 * @param date : publication date of this version
 * @param authors : authors that contributed to this program
 * @param license : license in use for this program and its sources
 */
void print_program_version(gchar *date, gchar *authors, gchar *license)
{

    fprintf(stdout, _("%s version : %s (%s)\n"), PACKAGE_NAME, PACKAGE_VERSION, date);
    fprintf(stdout, _("Author(s) : %s\n"), authors);
    fprintf(stdout, _("License : %s\n"), license);
    fprintf(stdout, "\n");
}


/**
 *  Inits internationalization domain for sauvegarde project
 */
void init_international_languages(void)
{
    gchar *result = NULL;
    gchar *codeset = NULL;
    gchar *text_domain = NULL;

    setlocale(LC_ALL, "");
    result = bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
    codeset = bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    text_domain = textdomain(GETTEXT_PACKAGE);

    print_debug(stdout, _("Debug mode is activated.\n"));
    print_debug(stdout, _("Gettext package : %s\n"), GETTEXT_PACKAGE);
    print_debug(stdout, _("Bindtextdomain : %s\n"), result);
    print_debug(stdout, _("Code set : %s\n"), codeset);
    print_debug(stdout, _("Text domain : %s\n"), text_domain);
}


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
void set_option_context_options(GOptionContext *context, GOptionEntry entries[], gboolean help, gchar *bugreport, gchar *summary)
{
    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
    g_option_context_set_help_enabled(context, help);
    g_option_context_set_description(context, bugreport);
    g_option_context_set_summary(context, summary);
}


/**
 * Frees a pointer if it is not NULL and returns NULL
 * @param to_free is the pointer to be freed (must have been malloc with
 *         g_malloc* functions).
 * @returns NULL
 */
gpointer free_variable(gpointer to_free)
{
    if (to_free != NULL)
        {
            g_free(to_free);
            to_free = NULL;
        }

    return NULL;
}


/**
 * Unrefs an object if it is not NULL and returns NULL
 * @param object_to_unref is the pointer to be unref'ed.
 * @returns NULL
 */
gpointer free_object(gpointer object_to_unref)
{
    if (object_to_unref != NULL)
        {
            g_object_unref(object_to_unref);
            object_to_unref = NULL;
        }

    return NULL;
}


/**
 * Frees an error if it exists and return NULL
 * @param error : the error to be freed
 * @returns NULL
 */
gpointer free_error(gpointer error)
{
    if (error != NULL)
        {
            g_error_free(error);
            error = NULL;
        }

    return NULL;
}


/**
 * Prints a message if the debug flag is set
 * @param stream : a FILE * file to print into (stdout, stderr, ...)
 * @param format : the format of the message (as in printf)
 * @param ... : va_list of variable that are to be printed into format.
 */
void print_debug(FILE *stream, const char *format, ...)
{
    va_list ap;

    if (ENABLE_DEBUG == TRUE)
        {
            va_start(ap, format);
            vfprintf(stream, format, ap);
            va_end(ap);
        }
}

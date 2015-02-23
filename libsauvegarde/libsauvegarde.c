/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    libsauvegarde.c
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
 * @file libsauvegarde.c
 * This library contains all tools that may be used in at least two of
 * the "Sauvegarde" programs.
 */

#include "libsauvegarde.h"

static void catcher(int sig);
static void print_buffer(gchar *buffer);


/**
 * Prints the buffer to stdout and free its memory !
 * @param buffer : the buffer to be printed and then freed
 */
static void print_buffer(gchar *buffer)
{
    if (buffer != NULL)
        {
            fprintf(stdout, "%s", buffer);
            buffer = free_variable(buffer);
        }
}


/**
 * @returns a newlly allocated gchar containing MHD version number with
 *          the following format : major.minor.maint-build. It may me freed
 *          when no longer needed.
 */
static gchar *make_MHD_version(void)
{
    gint build = 0;
    gint maint = 0;
    gint minor = 0;
    gint major = 0;
    gchar *version = NULL;

    build = ((MHD_VERSION >> 4) & 15) * 10 + (MHD_VERSION & 15);
    maint = ((MHD_VERSION >> 12) & 15) * 10 + ((MHD_VERSION >> 8) & 15);
    minor = ((MHD_VERSION >> 20) & 15) * 10 + ((MHD_VERSION >> 16) & 15);
    major = ((MHD_VERSION >> 28) & 15) * 10 + ((MHD_VERSION >> 24) & 15);

    version = g_strdup_printf("%d.%d.%d-%d", major, minor, maint, build);

    return version;
}


/**
 * Returns a newly allocated buffer that contains all informations about
 * the version of the libraries we are using.
 * @param name : name of the program of which we want to print the version.
 */
gchar *buffer_libraries_versions(gchar *name)
{
    gchar *buffer = NULL;
    gchar *buf1 = NULL;
    gchar *comm_version = NULL;

    if (name != NULL)
        {
            buffer = g_strdup_printf(_("%s was compiled with the following libraries:\n\t. GLIB version : %d.%d.%d\n"), name, glib_major_version, glib_minor_version, glib_micro_version);

            if (g_strcmp0(name, "serveur") == 0)
                {
                    comm_version = make_MHD_version();
                    buf1 = g_strdup_printf("%s\t. LIBMHD : %s\n", buffer, comm_version);
                    buffer = free_variable(buffer);
                    comm_version = free_variable(comm_version);
                }
            else
                {
                    comm_version = get_communication_library_version();

                    if (comm_version != NULL)
                        {
                            buf1 = g_strdup_printf("%s%s", buffer, comm_version);
                            comm_version = free_variable(comm_version);
                            buffer = free_variable(buffer);
                        }
                }

            if (buf1 == NULL && buffer != NULL)
                {
                    buf1 = g_strdup(buffer);
                    buffer = free_variable(buffer);
                }

            buffer = g_strdup_printf(_("%s\t. %s version : %s\n\t. JANSSON version : %d.%d.%d\n"), buf1, DATABASE_NAME, db_version(), JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION, JANSSON_MICRO_VERSION);
            buf1 = free_variable(buf1);
        }

    return buffer;
}


/**
 * Prints version of the libraries we are using.
 * @param name : name of the program of which we want to print the version.
 */
void print_libraries_versions(gchar *name)
{
    gchar *buffer = NULL;

    buffer = buffer_libraries_versions(name);
    print_buffer(buffer);
}


/**
 * Returns a newly allocated buffer that contains all informations about
 * program's version, authors and license.
 * @param name : name of the program of which we want to print the version.
 * @param date : publication date of this version
 * @param version : version of the program.
 * @param authors : authors that contributed to this program
 * @param license : license in use for this program and its sources
 */
gchar *buffer_program_version(gchar *name, gchar *date, gchar *version, gchar *authors, gchar *license)
{
    gchar *buffer = NULL;

        if (name != NULL && date != NULL && version != NULL && authors != NULL && license != NULL)
        {
            buffer = g_strdup_printf(_("%s version : %s (%s)\nAuthor(s) : %s\nLicense : %s\n\n"), name, version, date, authors, license);
        }

    return buffer;
}


/**
 * Prints the version of the program.
 * All parameters are of (gchar *) type.
 * @param name : name of the program of which we want to print the version.
 * @param date : publication date of this version
 * @param version : version of the program.
 * @param authors : authors that contributed to this program
 * @param license : license in use for this program and its sources
 */
void print_program_version(gchar *name, gchar *date, gchar *version, gchar *authors, gchar *license)
{
    gchar *buffer = NULL;

    buffer = buffer_program_version(name, date, version, authors, license);
    print_buffer(buffer);
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

    print_debug(_("Debug mode is activated.\n"));
    print_debug(_("Gettext package : %s\n"), GETTEXT_PACKAGE);

    if (result != NULL)
        {
            print_debug(_("Bindtextdomain : %s\n"), result);
        }

    if (codeset != NULL)
        {
            print_debug(_("Code set : %s\n"), codeset);
        }

    if (text_domain != NULL)
        {
            print_debug(_("Text domain : %s\n"), text_domain);
        }
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
    if (context != NULL && bugreport != NULL && summary != NULL)
        {
            g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
            g_option_context_set_help_enabled(context, help);
            g_option_context_set_description(context, bugreport);
            g_option_context_set_summary(context, summary);
        }
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
 * @param format : the format of the message (as in printf)
 * @param ... : va_list of variable that are to be printed into format.
 */
void print_debug(const char *format, ...)
{
    va_list ap;

    if (ENABLE_DEBUG == TRUE)
        {
            va_start(ap, format);
            vfprintf(stdout, format, ap);
            va_end(ap);
        }
}


/**
 * Prints an error message
 * @param char *filename
 * @param int lineno
 * @param format : the format of the message (as in printf)
 * @param ... : va_list of variable that are to be printed into format.
 */
void print_error(char *filename, int lineno, const char *format, ...)
{
    va_list ap;

    if (ENABLE_DEBUG == TRUE)
        {
            fprintf(stderr, "[%s, %d] ", filename, lineno);
            va_start(ap, format);
            vfprintf(stderr, format, ap);
            va_end(ap);
        }
}


#if !GLIB_CHECK_VERSION(2, 31, 0)
/**
 * defines a wrapper to the g_thread_create function used in glib before
 * 2.31
 */
GThread *g_thread_new(const gchar *unused, GThreadFunc func, gpointer data)
{
    GThread *thread = g_thread_create(func, data, TRUE, NULL);

    if (thread == NULL)
        {
            g_error(_("g_thread_create failed !"));
        }

    return thread;
}
#endif


/**
 * Tries to create a directory
 * @param directory is the gchar * string that contains a directory name
 *        to be created (does nothing if it exists).
 */
void create_directory(gchar *directory)
{
    GFile *dir = NULL;
    GError *error = NULL;

    if (directory != NULL)
        {
            dir = g_file_new_for_path(directory);
            g_file_make_directory_with_parents(dir, NULL, &error);

            if (error != NULL)
                {
                    print_error(__FILE__, __LINE__, ("Failed to create directory %s : %s\n"), directory, error->message);
                }

            dir = free_object(dir);
        }
}


/**
 * A signal catcher that does nothing for SIGPIPE (needed by libmicrohttpd
 * in order to be portable.
 */
static void catcher(int sig)
{
}


/**
 * A signal handler for SIGPIPE (needed by libmicrohttpd in order to be
 * portable.
 */
void ignore_sigpipe(void)
{
    struct sigaction oldsig;
    struct sigaction sig;

    sig.sa_handler = &catcher;
    sigemptyset (&sig.sa_mask);

    #ifdef SA_INTERRUPT
        sig.sa_flags = SA_INTERRUPT;  /* SunOS */
    #else
        sig.sa_flags = SA_RESTART;
    #endif

    if (0 != sigaction (SIGPIPE, &sig, &oldsig))
        {
            fprintf (stderr, "Failed to install SIGPIPE handler: %s\n", strerror (errno));
        }
}

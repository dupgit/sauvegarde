/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    libcdpfgl.c
 *
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2017 Olivier Delhomme
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
 * @file libcdpfgl.c
 * This library contains all tools that may be used in cdpfgl's programs.
 */

#include "libcdpfgl.h"

static void catcher(int sig);
static void print_buffer(gchar *buffer);


/**
 * Initializing debug_mode by default to the compilation choice.
 */
gboolean debug_mode = ENABLE_DEBUG;


/**
 * Sets debug_mode on or off if mode is TRUE or FALSE.
 * @param mode a boolean to say whether we want to have debug enabled
 *       (TRUE) or not (FALSE).
 */
void set_debug_mode(gboolean mode)
{
    if (debug_mode == TRUE && mode == FALSE)
        {
            print_debug(_("Debug mode is disabled.\n"));
        }
    else if (debug_mode == FALSE && mode == TRUE)
        {
            print_debug(_("Debug mode is activated.\n"));
        }

    debug_mode = mode;
}


/**
 * Sets the debug mode from command line read option
 * @param debug is a gint read from the command line and should be 0 or 1
 *        but is initialized to something different in order to be able to
 *        detect if the option has been invoked or not.
 */
void set_debug_mode_upon_cmdl(gint debug)
{
    if (debug == 0)
        {
            set_debug_mode(FALSE);
        }
    else if (debug == 1)
        {
            set_debug_mode(TRUE);
        }
}


/**
 * @returns the debug mode (TRUE if activated and FALSE if not).
 */
gboolean get_debug_mode(void)
{
    return debug_mode;
}


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
gchar *make_MHD_version(void)
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
            buffer = g_strdup_printf(_("%s was compiled with the following libraries:\n\t. GLIB version: %d.%d.%d\n"), name, glib_major_version, glib_minor_version, glib_micro_version);

            if (g_strcmp0(name, "cdpfglserver") == 0)
                {
                    comm_version = make_MHD_version();
                    buf1 = g_strdup_printf("%s\t. LIBMHD: %s\n", buffer, comm_version);
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

            buffer = g_strdup_printf(_("%s\t. %s version: %s\n\t. JANSSON version: %d.%d.%d\n\t. ZLIB version: %s"), buf1, DATABASE_NAME, db_version(), JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION, JANSSON_MICRO_VERSION, ZLIB_VERSION);
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
 * Prints if string is not NULL then prints it to stdout right in the
 * 'description' printf format.
 * @param description is a fprintf format string that must contain a %s
 *        in order to include the string 'string'
 * @param string is the string to be printed.
 */
void print_string_option(gchar *description, gchar *string)
{
    if (string != NULL)
    {
        fprintf(stdout, description, string);
    }
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
            buffer = g_strdup_printf(_("%s version: %s-%s (%s)\nAuthor(s): %s\nLicense: %s\n\n"), name, version, REVISION, date, authors, license);
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
 *  Inits internationalization domain for cdpfgl project
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
    print_debug(_("Gettext package: %s\n"), GETTEXT_PACKAGE);

    if (result != NULL)
        {
            print_debug(_("Bindtextdomain: %s\n"), result);
        }

    if (codeset != NULL)
        {
            print_debug(_("Code set: %s\n"), codeset);
        }

    if (text_domain != NULL)
        {
            print_debug(_("Text domain: %s\n"), text_domain);
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
 * Parses command line options
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @param entries is an array describing options and where to put their values
 */
void parse_command_line(int argc, char **argv, GOptionEntry entries[], gchar *summary)
{
    GError *error = NULL;
    GOptionContext *context;
    gchar *bugreport = NULL;  /** Bug Report message                               */

    bugreport = g_strconcat(_("Please report bugs to: "), PACKAGE_BUGREPORT, NULL);
    context = g_option_context_new("");

    set_option_context_options(context, entries, TRUE, bugreport, summary);

    if (!g_option_context_parse(context, &argc, &argv, &error))
        {
            g_print(_("Option parsing failed: %s\n"), error->message);
            exit(EXIT_FAILURE);
        }

    g_option_context_free(context);
    free_variable(bugreport);
}


/**
 * @cmdline is a string from the command line (if any)
 * @option_str is the string already in the options_t * structure is it
 *             freed when cmdline is returned because option_str is the
 *             one we want to feed with the returned value.
 * @returns cmdline if it exists and frees option_stror return option_str
 *          if cmdline does not exist.
 */
gchar *set_option_str(gchar *cmdline, gchar *option_str)
{
    if (cmdline != NULL)
        {
            free_variable(option_str);
            return g_strdup(cmdline);
        }
    else
        {
            return option_str;
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
 * Wrapper for the g_slist_free_full function in order to free lists
 * of gchar *
 * @param data is the pointer to a gchar * string to be freed
 */
void free_gchar_variable(gpointer data)
{
    free_variable(data);
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
 * Frees all elements of a gchar * GSList
 * @param list the list to be freed
 * @returns NULL
 */
gpointer free_list(GSList *list)
{
    g_slist_free_full(list, free_gchar_variable);

    return NULL;
}


/**
 * Prints buffer using an hexadecimal form
 * @param buffer is the buffer to be printed
 * @param is the number of bytes of buffer to print.
 */
void print_hex(gchar *buffer, size_t len)
{
    size_t i = 0;
    guchar c = '\0';

    for (i = 0; i < len ; i++)
        {
            c = (guchar) buffer[i];
            fprintf(stdout, "%0x ", c);
        }
    fprintf(stdout, "\n");
}


/**
 * Prints a message if the debug flag is set
 * @param format : the format of the message (as in printf)
 * @param ... : va_list of variable that are to be printed into format.
 */
void print_debug(const char *format, ...)
{
    va_list ap;

    if (debug_mode == TRUE)
        {
            va_start(ap, format);
            vfprintf(stdout, format, ap);
            va_end(ap);
        }
}


/**
 * Prints an error message to stderr
 * @param char *filename should be __FILE__
 * @param int lineno should be __LINE__
 * @param format : the format of the message (as in printf)
 * @param ... : va_list of variable that are to be printed into format.
 */
void print_error(char *filename, int lineno, const char *format, ...)
{
    va_list ap;


    fprintf(stderr, "[%s, %d] ", filename, lineno);
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

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
 * Tries to create a directory. Does not report an error if the directory
 * already exists.
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
                    if (error->code != G_IO_ERROR_EXISTS)
                        {
                            print_error(__FILE__, __LINE__, ("Failed to create directory %s: %s\n"), directory, error->message);
                        }
                    error = free_error(error);
                }

            dir = free_object(dir);
        }
}


/**
 * A signal catcher that does nothing for SIGPIPE (needed by libmicrohttpd
 * in order to be portable).
 */
static void catcher(int sig)
{
}


/**
 * A signal handler for SIGPIPE (needed by libmicrohttpd in order to be
 * portable).
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

    if (sigaction(SIGPIPE, &sig, &oldsig) != 0)
        {
            print_error(__FILE__, __LINE__, _("Failed to install SIGPIPE handler: %s\n"), strerror(errno));
        }
}


/**
 * Waits a number of micro seconds until the number of element in the
 * queue is less than the specified number.
 * @param queue : the queue to be tested
 * @param nbelem : maximum number of element before waiting
 * @param usecs : number of micro seconds to wait
 */
void wait_for_queue_to_flush(GAsyncQueue *queue, guint nbelem, useconds_t usecs)
{
    if (queue != NULL && usecs < 1000000)
        {
            /**
             * Checks if the queue length is less than 8 before
             * continuing because we want to be sure that what we
             * provide is processed...
             */
            while (g_async_queue_length(queue) > nbelem)
                {
                    usleep(usecs);
                }
        }
}


/**
 * @param string is a gchar * string containing another string
 * @param decodeit is a boolean. When set to TRUE it will base64 decode
 *        the string, if FALSE it will return the string as is.
 * @returns a newly allocated substring from caracter 3 to the very last
 *          one of the string parameter.
 */
gchar *get_substring_from_string(gchar *string, gboolean decodeit)
{
    gchar *new_string = NULL;
    gchar *base64 = NULL;
    gsize len = 0;

    if (string != NULL)
        {
            /* we have a leading space before " and a trailing space after " so begins at + 2 and length is - 3 less */
            base64 = g_strndup(string + 2, strlen(string) - 3);

            if (decodeit == TRUE)
                {
                    new_string = (gchar *) g_base64_decode(base64, &len);
                    free_variable(base64);
                }
            else
                {
                    new_string = base64;
                }

        }

    return new_string;
}


/**
 * Reads an uint from a string.
 * @param string a gchar * string containing a number that should be
 *        32 bits at most.
 * @returns a uint from the gchar * string that may contain such a number.
 */
uint get_uint_from_string(gchar *string)
{
    uint guess = 0;

    if (string != NULL)
        {
            sscanf(string, "%d", &guess);
        }

    return guess;
}

/**
 * @param string a gchar * string containing a number coded at most in 64
 *        bits.
 * @returns a guint64 from the gchar * string that may contain such a
 *          number.
 */
guint64 get_guint64_from_string(gchar *string)
{
    guint64 guess_64 = 0;

    if (string != NULL)
        {
            sscanf(string, "%" G_GUINT64_FORMAT "", &guess_64);
        }

    return guess_64;
}


/**
 * Gets digit values at a given place into a gchar YYYY-MM-DD HH:MM:SS
 * formatted string
 * @param date the string to be parsed
 * @param i the offset where to start
 * @param size the size to be parsed
 * @returns a gint that is supposed to be the value read into 'date' at
 *          'i' position ('size' long).
 */
gint get_digit_value(gchar *date, guint i, guint size)
{
    gchar *value = NULL;
    gint digit_value = 0;
    guint j = 0;
    gint ret = 0;

    value = (gchar *) g_malloc0(size + 1);

    while (isdigit(date[i]) && j < size)
        {
            value[j] = date[i];
            i++;
            j++;
        }

    ret = sscanf(value, "%d", &digit_value);

    if (ret != 1)
        {
            digit_value = 0;
        }

    free_variable(value);

    return digit_value;
}


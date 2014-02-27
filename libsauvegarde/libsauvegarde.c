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
    fprintf(stdout, _("%s was compiled with the following libraries:\n"), PACKAGE_NAME);
    fprintf(stdout, _("\t. GLIB version : %d.%d.%d\n"), glib_major_version, glib_minor_version, glib_micro_version);

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

    if (ENABLE_DEBUG == TRUE)
        {
            fprintf(stdout, "Gettext package : %s\n", GETTEXT_PACKAGE);
            fprintf(stdout, "Bindtextdomain : %s\n", result);
            fprintf(stdout, "Code set : %s\n", codeset);
            fprintf(stdout, "Text domain : %s\n", text_domain);
        }
}

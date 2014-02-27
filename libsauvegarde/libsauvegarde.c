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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    compress.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2017 Olivier Delhomme
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
 * @file compress.c
 * This file is here to manage compression libraries (at least zlib)
 */

#include "libcdpfgl.h"


/**
 * inits compress_t structure with default values */
 */
compress_t *init_compress_t()
{
    compress_t *comp = NULL;

    comp->text = NULL;
    comp->len = 0;
    comp->comp = FALSE;  /* by default nothing is no compressed ! */

    return comp;
}

/**
 * Compress buffer and returns a compressed text
 * @param buffer is the plain buffer text to be compressed
 *        this buffer must be \0 terminated.
 * @returns a compress_t structure containing a compressed text
 *          buffer.
 */
compress_t *compress_buffer(gchar *buffer)
{
    compress_t *comp = NULL;

    comp = init_compress_t();

}

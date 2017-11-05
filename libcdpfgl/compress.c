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


static compress_t *zlib_compress_buffer(compress_t *comp, gchar *buffer);


/**
 * Inits compress_t structure with default values
 */
compress_t *init_compress_t(void)
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
 * @param type is the compression type to use.
 * @returns a compress_t structure containing a compressed text
 *          buffer.
 */
compress_t *compress_buffer(gchar *buffer, gint type)
{
    compress_t *comp = NULL;

    comp = init_compress_t();

    if (type == COMPRESS_ZLIB_TYPE)
        {
            comp = zlib_compress_buffer(comp, buffer);
        }

    return comp;
}

/**
 * Compress buffer using zlib.
 * @param comp is the compress_t structure that may contain
 *        the compressed data
 * @param buffer is the plain text buffer to be compressed
 * @returns a compress_t structure with compressed data in it
 *          or NULL in case that something went wrong.
 */
static compress_t *zlib_compress_buffer(compress_t *comp, gchar *buffer)
{
    z_stream *stream = NULL;
    int ret = Z_OK;

    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = Z_NULL;

    /* Default zlib compression level is set to best (9) */
    ret = deflateInit(stream, 9);
    if (ret != Z_OK)
        {
            return NULL;
        }

    return comp;
}


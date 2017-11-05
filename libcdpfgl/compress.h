/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    compress.h
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
 * @file compress.h
 * This file contains all headers and public functions to manage
 * compression libraries such as zlib.
 */

#ifndef _COMPRESS_H_
#define _COMPRESS_H_

/**
 * @struct compress_t
 * @brief This structure contains pointers to the selected backend functions.
 */
typedef struct
{
    gchar *text;    /* Text (may be plain or compressed) */
    guint64 len;    /* Length of text string above       */
    gboolean comp;  /* True if text is compressed        */
} compress_t;


/**
 * inits compress_t structure with default values */
 */
extern compress_t *init_compress_t();

/**
 * Compress buffer and returns a compressed text
 * @param buffer is the plain buffer text to be compressed
 *        this buffer must be \0 terminated.
 * @returns a compress_t structure containing a compressed text
 *          buffer.
 */
extern compress_t *compress_buffer(gchar *buffer);

#endif  /* _COMPRESS_H_ */



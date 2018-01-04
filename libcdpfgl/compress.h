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
 * @def COMPRESS_ZLIB_TYPE
 * Defines that ZLIB is to be used to compress data
 */
#define COMPRESS_ZLIB_TYPE (0)


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
 * Inits compress_t structure with default values.
 */
extern compress_t *init_compress_t(void);

/**
 * Frees memory of the compress_t structure passed as
 * an argument.
 * @param comp is the compress_t structure to be freed.
 */
extern void free_compress_t(compress_t *comp);

/**
 * Compress buffer and returns a compressed text
 * @param buffer is the plain buffer text to be compressed
 *        this buffer must be \0 terminated.
 * @param type is the compression type to use.
 * @returns a compress_t structure containing a compressed text
 *          buffer.
 */
extern compress_t *compress_buffer(gchar *buffer, gint type);

#endif  /* _COMPRESS_H_ */



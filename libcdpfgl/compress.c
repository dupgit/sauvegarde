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


static void zlib_print_error(char *filename, int lineno, int ret);
static compress_t *zlib_compress_buffer(compress_t *comp, gchar *buffer);
static compress_t *zlib_uncompress_buffer(compress_t *comp, guint64 len);


/**
 * Inits compress_t structure with default values.
 */
compress_t *init_compress_t(void)
{
    compress_t *comp = NULL;

    comp = (compress_t *) g_malloc0(sizeof(compress_t));

    if (comp != NULL)
        {
            comp->text = NULL;
            comp->len = 0;
            comp->comp = FALSE;  /* by default nothing is compressed ! */
        }

    return comp;
}


/**
 * Frees memory of the compress_t structure passed as
 * an argument.
 * @param comp is the compress_t structure to be freed.
 */
void free_compress_t(compress_t *comp)
{
    if (comp != NULL)
        {
            if (comp->text != NULL)
                {
                    g_free(comp->text);
                }
            g_free(comp);
        }
}


/**
 * Compress buffer and returns a compressed text
 * @param buffer is the plain buffer text to be compressed
 *        this buffer must be \0 terminated.
 * @param type is the compression type to use (COMPRESS_ZLIB_TYPE).
 * @returns a compress_t structure containing a compressed text
 *          buffer.
 */
compress_t *compress_buffer(gchar *buffer, gint type)
{
    compress_t *comp = NULL;

    comp = init_compress_t();

    if (type == COMPRESS_ZLIB_TYPE && comp != NULL)
        {
            comp = zlib_compress_buffer(comp, buffer);
        }

    return comp;
}


/**
 * Error reporting for Zlib to stderr
 * @param char *filename should be __FILE__
 * @param int lineno should be __LINE__
 * @param ret is an int that represents a zlib error code
 */
static void zlib_print_error(char *filename, int lineno, int ret)
{
    switch (ret)
        {
            case Z_ERRNO:
                if (ferror(stdin))
                    print_error(filename, lineno, _("Error reading stdin."));
                if (ferror(stdout))
                    print_error(filename, lineno, _("Error writing stdout."));
                break;
            case Z_STREAM_ERROR:
                print_error(filename, lineno, _("Error: invalid compression level."));
                break;
            case Z_DATA_ERROR:
                print_error(filename, lineno, _("Error: invalid or incomplete deflate data."));
                break;
            case Z_MEM_ERROR:
                print_error(filename, lineno, "Error: out of memory.");
                break;
            case Z_VERSION_ERROR:
                print_error(filename, lineno, "Error: zlib version mismatch!");
        }
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
    int ret = Z_OK;
    glong srclen = 0;
    uLongf destlen = 0;
    Bytef *destbuffer = NULL;

    srclen = strlen(buffer);
    destlen = compressBound((uLong) srclen);
    destbuffer = (Bytef *) g_malloc(sizeof(Bytef)*destlen + 1);

    if (destbuffer != NULL)
        {
            /* Default zlib compression level is set to best (9) */
            ret = compress2(destbuffer, &destlen, (const Bytef *) buffer, (uLong) srclen, 9);

            if (ret != Z_OK)
                {
                    zlib_print_error(__FILE__, __LINE__, ret);
                    free_compress_t(comp);
                    comp = NULL;
                }
            else if (comp != NULL)
                {
                    comp->text = (gchar *) destbuffer;
                    comp->len = destlen;
                    comp->comp = TRUE;
                }
        }

    return comp;
}


/**
 * Uncompress buffer and returns an uncompressed text
 * @param buffer is the compressed buffer to be uncompressed
 * @param cmplen is the len of the above compressed buffer
 * @param textlen is the len of the uncompressed data
 * @param type is the compression type to use (COMPRESS_ZLIB_TYPE).
 * @returns a compress_t structure containing a compressed text
 *          buffer.
 */
compress_t *uncompress_buffer(gchar *buffer, guint64 cmplen, guint64 textlen, gint type)
{
    compress_t *comp = NULL;

    comp = init_compress_t();

    if (type == COMPRESS_ZLIB_TYPE && comp != NULL)
        {
            comp->text = buffer;
            comp->len = cmplen;
            comp->comp = TRUE;
            comp = zlib_uncompress_buffer(comp, textlen);
        }

    return comp;
}


/**
 * Uncompress buffer using zlib
 * @param comp is the compress_t structure that contains the compressed
 *        data and that will contain the uncompressed data.
 * @param len is the len of the uncompressed data
 * @returns a compress_t structure with uncompressed data in it
 *          or NULL in case that something went wrong.
 */
static compress_t *zlib_uncompress_buffer(compress_t *comp, guint64 len)
{
    int ret = Z_OK;
    uLongf destlen = (uLongf) len;
    Bytef *destbuffer = NULL;
    const Bytef *srcbuffer = NULL;
    uLong sourcelen = 0;

    destbuffer = (Bytef *) g_malloc0(destlen + 1);

    if (destbuffer != NULL && comp != NULL)
        {
            sourcelen = comp->len;
            srcbuffer = (const Bytef *) comp->text;

            ret = uncompress(destbuffer, &destlen, srcbuffer, sourcelen);

            /* print_debug("ret : %d - Buffer (size %ld) uncompressed into a buffer (size %ld)\n", ret, sourcelen, destlen); */

            if (ret != Z_OK)
                {
                    zlib_print_error(__FILE__, __LINE__, ret);
                    free_compress_t(comp);
                    comp = NULL;
                }
            else
                {
                    comp->text = (gchar *) destbuffer;
                    comp->text[destlen] = '\0';
                    comp->len = destlen;
                    comp->comp = FALSE;
                }
        }

    return comp;
}


/**
 * Verify if a compress type is allowed
 * @param cmptype is a gshort that should represents the compression type
 * @returns a boolean: True if we know the compression type, False otherwise.
 */
gboolean is_compress_type_allowed(gshort cmptype)
{
    if (cmptype == COMPRESS_NONE_TYPE || cmptype == COMPRESS_ZLIB_TYPE)
        {
            return TRUE;
        }
    else
        {
            return FALSE;
        }
}

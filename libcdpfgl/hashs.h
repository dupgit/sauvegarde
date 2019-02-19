/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    hashs.h
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
 * @file hashs.h
 *
 * This file contains all the definitions of the functions and structures
 * that are used to deal with hashs in all Sauvegarde's project.
 */
#ifndef _HASHS_H_
#define _HASHS_H_

/**
 * @def HASH_LEN
 * Defines the length in byte of hash's binary form
 */
#define HASH_LEN (32)

/**
 * @struct hash_data_t
 * @brief Structure to store a hash and the corresponding data
 */
typedef struct
{
    guint8 *hash;
    guchar *data;
    gssize read;     /* Always the lenght of *data buffer (compressed or not) */
    gshort cmptype;  /* tells wether the data here has been compressed or not and what type of compression it is */
    gssize uncmplen; /* The length of the uncompressed buffer if it has been compressed */
} hash_data_t;


/**
 * @struct hash_extract_t
 * @brief helps to extract hashs strings from a GList hash_list. hash_list
 *        is a pointer to a GList of guint8 * hashs. hash_string should be
 *        a gchar * comma separated list of those hashs (base64 encoded).
 */
typedef struct
{
    GList *hash_list;
    gchar *hash_string;
} hash_extract_t;


/**
 * Comparison function used to compare two hashs (binary form) mainly
 * used to sort hashs properly.
 * @returns a negative value if a < b, zero if a = b and a positive value
 * if a > b.
 */
extern gint compare_two_hashs(gconstpointer a, gconstpointer b);


/**
 * Transforms a binary hashs into a printable string (gchar *)
 * @param a_hash is a hash in a binary form that we want to transform into
 *        a string.
 * @returns a string that contains the hash in an hexadecimal form.
 */
extern gchar *hash_to_string(guint8 *a_hash);


/**
 * Transforms a binary hashs into a printable string (gchar *)
 * @param str_hash a string (gchar *) that conatins the hash in an
 *        hexadecimal form.
 * @returns a hash in a binary form (guint8 *).
 */
extern guint8 *string_to_hash(gchar *str_hash);


/**
 * Frees hash_data_t *buffer and returns NULL.
 * @param hash_data : the stucture that contains buffer data, hash data
 * and its size to be freed.
 * @returns always NULL.
 */
extern void free_hash_data_t(hash_data_t *hash_data);


/**
 * handler for g_slist_free_full
 * @param data must be a hash_data_t * structure.
 */
extern void free_hdt_struct(gpointer data);


/**
 * Inits and returns a newly hash_data_t structure. (and compresses data if cmptype is
 * a compression type such as COMPRESS_ZLIB_TYPE.
 * @returns a newly created hash_data_t structure.
 */
extern hash_data_t *new_hash_data_t(guchar * data, gssize read, guint8 *hash, gshort cmptype);

/**
 * Inits and returns a newly hash_data_t structure filed with the value as stated and
 * does nothing with the data
 * @returns a newly created hash_data_t structure.
 */
extern hash_data_t *new_hash_data_t_as_is(guchar * data, gssize read, guint8 *hash, gshort cmptype, gssize uncmplen);

/**
 * Converts the hash list to a list of comma separated hashs in one gchar *
 * string. Hashs are base64 encoded
 * @param hash_list à GSList of hashs
 * @returns a list of comma separated hashs in one gchar * string.
 */
extern gchar *convert_hash_data_list_to_gchar(GList *hash_list);


/**
 * Makes a path from a binary hash : 0E/39/AF for level 3 with hash (in hex)
 * begining by 0E39AF.
 * @param path is a gchar * prefix for the path (ie /var/tmp/cdpfgl for
 *        instance).
 * @param hash is a guint8 pointer to the binary representation of a hash.
 * @param level The level we want the path to have. It is an unsigned int
 *        and must be less than HASH_LEN. a level of N gives 2^N
 *        directories. We should add a level when more than 512 files are
 *        in each last subdirectories.
 * @returns a string as a gchar * made of the path and the hex
 *          representation of hash on 'level' levels. With the example above
 *          it will return /var/tmp/cdpfgl/0E/39/AF
 */
extern gchar *make_path_from_hash(gchar *path, guint8 *hash, guint level);


/**
 * makes a GSList of hash_data_t * element where 'hash' field is base64
 * decoded hashs from a string containning base64 * encoded hashs that
 * must be separated by commas.
 * @param the string containing base64 encoded hashs such as : *
 *        "cCoCVkt/AABf04jn2+rfDmqJaln6P2A9uKolBjEFJV4=", "0G8MaPZ/AADNyaPW7ZP2s0BI4hAdZZIE2xO1EwdOzhE="
 *        for instance.
 * @returns a GSList of hash_data_t * where each elements contains a
 *          base64 decoded hash (binary form).
 */
extern GList *make_hash_data_list_from_string(gchar *hash_string);


/**
 * Tells wheter a hash (picking it in a hash_data_t structure is in the
 * needed list of hash_data_t structures.
 * @param hash_data contains the hash that we are looking for into the
 *        needed list.
 * @param needed is a GList of hash_data_t structures that may already
 *        contain one with the same hash than the one in hash_data
 * @returns TRUE if the hash is found, FALSE otherwise
 */
extern gboolean hash_data_is_in_list(hash_data_t *hash_data, GList *needed);


/**
 * This function is an helper to be called by g_list_copy_deep. It copies
 * only the hash and read variables of an hash_data_t *hash
 * @param src is the source pointer it must be a hash_data_t * pointer.
 * @param user_data is not used and may be NULL
 * @returns A pointer to the copy
 */
extern gpointer copy_only_hash(gconstpointer src, gpointer user_data);


/**
 * Creates a new hash_extract_t * empty structure that may be freed when
 * no longer neeeded
 * @returns a newlly allocated hash_extract_t * empty structure that may
 *          be freed when no longer neeeded
 */
extern hash_extract_t *new_hash_extract_t(void);


/**
 * Converts at max hashs from hash_extract->hash_list into a gchar *
 * string that are comma separated.
 * @param hash_extract contains one pointer to a GList og guint8 * binary
 *        hashs and one pointer to a gchar * string that contains thoses
 *        hashs, base64 encoded and comma separated.
 * @param max is a gint that represents the maximum number of hashs to
 *        convert.
 * @returns a correctly filled gchar *string
 */
extern gchar *convert_max_hashs_from_hash_list_to_gchar(hash_extract_t *hash_extract, gint max);


/**
 * Calculates a hash for the buffer string buffer with size bytes long
 * @param buffer is a buffer thut may contain \0 bytes
 * @param size is the size that we want to be taken into account to
 *             calculate the hash of the buffer.
 * @returns a newly allocatted guint8 * hash that may be freed when no
 *          longer needed.
 */
extern guint8 *calculate_hash_for_string(guchar *buffer, guint size);

#endif /* #ifndef _HASHS_H_ */

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    hashs.h
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
 * @struct hashs_t
 * @brief Storing hashs and things related in a balanced binary tree.
 *
 * Structure that contains a balanced binary tree to store hashs in a
 * binary form to save space. keys are hashs and value is the data that
 * generated the corresponding checksum.
 */
typedef struct
{
    GTree *tree_hash;    /**< A balanced binary tree to stores hashs and data               */
    guint64 total_bytes; /**< Total number of bytes that were computed to get the checksums */
    guint64 in_bytes;    /**< Number of bytes into the GTree (deduplicated)                 */
} hashs_t;


/**
 * @struct data_t
 * @brief Structure that stores buffer data (buffer) and its size (read).
 *
 * It will be associated with the checksum into the GTree balanced binary
 * tree of the hashs_t structure.
 */
typedef struct
{
    guchar *buffer;      /**< buffer data read from the file              */
    gssize read;         /**< size of that buffer                         */
    gboolean into_cache; /**< TRUE if it has been inserted into the cache */
} data_t;


/**
 * @struct hash_data_t
 * @brief Structure to store a hash and the corresponding datas
 */
typedef struct
{
    guint8 *hash;
    guint8 *data;
    gssize read;
} hash_data_t;

/**
 * Allocate a new hashs_t structure with a GTree
 * @returns a newly allocated hashs_t structure with a GTree initialized
 *          with compare_two_hashs function to sort the hashs.
 */
extern hashs_t *new_hash_struct(void);


/**
 * Comparison function used with the GTree structure to sort hashs
 * properly.
 * @returns a negative value if a < b, zero if a = b and a positive value
 * if a > b.
 */
extern gint compare_two_hashs(gconstpointer a, gconstpointer b);


/**
 * A function to insert a binary hash into the GTree structure
 * @param hashs : the hash structure that contains the binary tree in which
 *        we want to insert the second parameter
 * @param a_hash is a hash in a binary form
 * @param buffer is the data whom checksum is a_hashs
 * @param read is the number of bytes in guchar *buffer
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 */
extern void insert_into_tree(hashs_t *hashs, guint8 *a_hash, guchar *buffer, gssize read, meta_data_t *meta);


/**
 * Transforms a binary hashs into a printable string (gchar *)
 * @param a_hash is a hash in a binary form that we want to transform into
 *        a string.
 * @returns a string that contains the hash in an hexadecimal form.
 */
extern gchar *hash_to_string(guint8 *a_hash);


/**
 * Creates a new data_t * structure populated with the buffer and its size.
 * @param buffer : the data to be stored
 * @param read : the size of that buffer
 * @param into_cache : says wether it is already into the cache (TRUE) or
 *        not (FALSE)
 * @returns a newly allocated data_t * structure that can be freed when no
 *         longer needed.
 */
extern data_t *new_data_t_structure(guchar *buffer, gssize read, gboolean into_cache);


/**
 * Frees data buffer
 * @param a_data : the stucture that contains buffer data and its size to
 * be freed
 */
extern gpointer free_data_t_structure(data_t *a_data);


/**
 * Inits and returns a newly hash_data_t structure.
 * @returns a newly hash_data_t structure.
 */
hash_data_t *new_hash_data_t(guint8 * data, gssize read, guint8 *hash);


/**
 * Converts the hash list to a list of comma separated hashs in one gchar *
 * string. Hashs are base64 encoded
 * @param hash_list à GSList of hashs
 * @returns a list of comma separated hashs in one gchar * string.
 */
extern gchar *convert_hash_list_to_gchar(GSList *hash_list);


/**
 * Makes a path from a binary hash : 0E/39/AF for level 3 with hash (in hex)
 * begining by 0E39AF.
 * @param path is a gchar * prefix for the path (ie /var/tmp/sauvegarde for
 *        instance).
 * @param hash is a guint8 pointer to the binary representation of a hash.
 * @param level The level we want the path to have. It is an unsigned int
 *        and must be less than HASH_LEN. a level of N gives 2^N
 *        directories. We should add a level when more than 512 files are
 *        in each last subdirectories.
 * @returns a string as a gchar * made of the path and the hex
 *          representation of hash on 'level' levels. With the example above
 *          it will return /var/tmp/sauvegarde/0E/39/AF
 */
extern gchar *make_path_from_hash(gchar *path, guint8 *hash, guint level);


#endif /* #ifndef _HASHS_H_ */

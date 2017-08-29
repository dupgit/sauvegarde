/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    stats.h
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
 * @file stats.h
 *
 * This file contains all the definitions of the functions and structures
 * that are used by 'cdpfglserver' Sauvegarde's server for its statistics.
 */
#ifndef _STATS_H_
#define _STATS_H_

#include "config.h"

/**
 * @struct req_get_t
 * @brief structure to keep stats about 'GET' requests
 */
typedef struct
{
    guint64 nb_request;       /** total number of 'GET' requests          */
    guint64 stats;            /** number of GET /Stats.json URL           */
    guint64 version;          /** number of GET /Version.json URL         */
    guint64 verstxt;          /** number of GET /Version URL              */
    guint64 file_list;        /** number of GET /File/List.json URL       */
    guint64 data_hash;        /** number of GET /Data/0xxxx.json URL      */
    guint64 data_hash_array;  /** number of GET /Data/Hash_Array.json URL */
    guint64 unktxt;           /** number of GET to unknown text URL       */
    guint64 unk;              /** number of GET to unknown json URL       */
} req_get_t;


typedef struct
{
    guint64 nb_request; /** total number of 'POST' requests                 */
    guint64 meta;       /** Counts usage of 'POST' for /Meta.json URL       */
    guint64 data;       /** Counts usage of 'POST' for /Data.json URL       */
    guint64 data_array; /** Counts usage of 'POST' for /Data_Array.json URL */
    guint64 hash_array; /** Counts usage of 'POST' for /Hash_Array.json URL */
} req_post_t;


typedef struct
{
    guint64 nb_request; /** total number of 'unknown requests */
} req_unk_t;


/**
 * @struct requests_t
 * @brief structure to keep track of request usages
 */
typedef struct
{
    req_get_t *get;      /** keeps stats about 'GET' requests     */
    req_post_t *post;    /** keeps stats about 'POST' requests    */
    req_unk_t *unknown;  /** keeps stats about 'unknown' requests */
    guint64 nb_request;  /** total number of requests             */
} requests_t;


/**
 * @struct stats_t
 * @brief Structure that will contain some statistics.
 */
typedef struct
{
    requests_t *requests;
    guint64 nb_files;        /**< nb_files is the number of version of files saved                                              */
    guint64 nb_dedup_bytes;  /**< nb_dedup_bytes is the number of bytes saved by the server (the dedup ones)                    */
    guint64 nb_total_bytes;  /**< nb_total_bytes is the number of bytes represented by file sizes of saved files (before dedup) */
    guint64 nb_meta_bytes;   /**< nb_meta_bytes is the number of bytes of all the meta data saved                               */
} stats_t;


/**
 * Creates a new stats_t structure initialized
 * with 0.
 * @returns a newly allocated stats_t structure that
 *          can be freed when no longer needed (free_stats_t)
 */
extern stats_t *new_stats_t(void);


/**
 * Frees an allocated stats_t structure
 * @param stats is an allocated stats_t structure to be freed.
 */
extern void free_stats_t(stats_t *stats);


/**
 * Adds in stats_t structure one 'GET' request.
 * @param stats is a stats_t structure to keep some stats about
 *        server runs.
 */
extern void add_one_get_request(stats_t *stats);


/**
 * Adds in stats_t structure one 'POST' more get request.
 * @param stats is a stats_t structure to keep some stats about
 *        server runs.
 */
extern void add_one_post_request(stats_t *stats);


/**
 * Counts in stats_t structure one 'unknown' more get request.
 * @param stats is a stats_t structure to keep some stats about
 *        server runs.
 */
extern void add_one_unknown_request(stats_t *stats);


/**
 * Adds one to the number of files saved
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
extern void add_one_saved_file(stats_t *stats);


/**
 * Adds file size to stats->nb_total_bytes in order to count the real
 * size of what has been saved
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param size is the file sized being saved to be added
 */
extern void add_file_size_to_total_size(stats_t *stats, guint64 size);


/**
 * Adds the size  of the hash to the total dedup number of bytes
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param hash_data is a hash_data_t structure with read field representing
 *        the size of the hashed buffer.
 */
extern void add_hash_size_to_dedup_bytes(stats_t *stats, hash_data_t *hash_data);


/**
 * Adds one to the number of visits of /Stats.json url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
extern void add_one_to_get_url_stats(stats_t *stats);


/**
 * Adds one to the number of visits of /Version.json or /Version url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param txt is a boolean set to TRUE if the URL is a text one (not ending with
 *            .json
 */
extern void add_one_to_get_url_version(stats_t *stats, gboolean txt);


/**
 * Adds one to the number of visits of /File/List.json url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
extern void add_one_to_get_url_file_list(stats_t *stats);


/**
 * Adds one to the number of visits of /Data/0xxxx.json url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
extern void add_one_to_get_url_data_hash(stats_t *stats);


/**
 * Adds one to the number of visits of /Data/Hash_Array.json url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
extern void add_one_to_get_url_data_hash_array(stats_t *stats);


/**
 * Adds one to the number of visits of unknown URL (if txt is FALSE then the
 * unknown URL ends with .json
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param txt is a boolean set to TRUE if the URL is a text one (not ending with
 *            .json
 */
extern void add_one_to_get_url_unknown(stats_t *stats, gboolean txt);


/**
 * Adds one to the number of visits of /Meta.json
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param length is the total length of the request.
 */
extern void add_length_and_one_to_post_url_meta(stats_t *stats, guint64 length);


#endif /* #ifndef _STATS_H_ */

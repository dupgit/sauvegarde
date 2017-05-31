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
    guint64 nb_request;  /** total number of 'GET' requests */
} req_get_t;


typedef struct
{
    guint64 nb_request; /** total number of 'POST' requests */
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
 * Adds meta data bytes to the stats structure
 * @param stats is a stats_t structure to keep some stats about server runs
 * @param nb_bytes is a size_t number representing the number of bytes to
 *                 add to the stats
 */
extern void add_bytes_to_metadata_bytes(stats_t *stats, size_t nb_bytes);

#endif /* #ifndef _STATS_H_ */

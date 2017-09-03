/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    stats.c
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
 * @file stats.c
 *
 * This file contains all functions and structures that are used by
 * 'cdpfglserver' Sauvegarde's server for its statistics.
 */

#include "server.h"

static req_get_t *new_req_get_t(void);
static void free_req_get_t(req_get_t *req_get);
static req_post_t *new_req_post_t(void);
static void free_req_post_t(req_post_t *req_post);
static req_unk_t *new_req_unk_t(void);
static void free_req_unk_t(req_unk_t *req_unk);
static requests_t *new_requests_t(void);
static void free_requests_t(requests_t *requests);
static void add_bytes_to_metadata_bytes(stats_t *stats, size_t nb_bytes);

/**
 * Creates a new req_get_t structure initialized with zeros
 * @return a newly allocated req_get_t structure that can
 *         be freed with free_req_get_t() function when no
 *         longer needed.
 */
static req_get_t *new_req_get_t(void)
{
    req_get_t *req_get = NULL;

    req_get = (req_get_t *) g_malloc0(sizeof(req_get_t));

    req_get->nb_request = 0;
    req_get->stats = 0;
    req_get->version = 0;
    req_get->verstxt = 0;
    req_get->file_list = 0;
    req_get->data_hash = 0;
    req_get->data_hash_array = 0;
    req_get->unktxt = 0;
    req_get->unk = 0;

    return req_get;
}


/**
 * Frees an allocated req_get_t structure
 * @param req_get is an allocated req_get_t structure to be freed.
 */
static void free_req_get_t(req_get_t *req_get)
{
    if (req_get != NULL)
        {
            g_free(req_get);
        }
}


/**
 * Creates a new req_post_t structure initialized with zeros
 * @return a newly allocated req_post_t structure that can
 *         be freed with free_req_post_t() function when no
 *         longer needed.
 */
static req_post_t *new_req_post_t(void)
{
    req_post_t *req_post = NULL;

    req_post = (req_post_t *) g_malloc0(sizeof(req_post_t));

    req_post->nb_request = 0;
    req_post->meta = 0;
    req_post->data = 0;
    req_post->data_array = 0;
    req_post->hash_array = 0;

    return req_post;
}


/**
 * Frees an allocated req_post_t structure
 * @param req_post is an allocated req_post_t structure to be freed.
 */
static void free_req_post_t(req_post_t *req_post)
{
    if (req_post != NULL)
        {
            g_free(req_post);
        }
}


/**
 * Creates a new req_unk_t structure initialized with zeros
 * @return a newly allocated req_ink_t structure that can
 *         be freed with free_req_unk_t() function when no
 *         longer needed.
 */
static req_unk_t *new_req_unk_t(void)
{
    req_unk_t *req_unk = NULL;

    req_unk = (req_unk_t *) g_malloc0(sizeof(req_unk_t));

    req_unk->nb_request = 0;

    return req_unk;
}


/**
 * Frees an allocated req_get_t structure
 * @param req_unk is an allocated req_unk_t structure to be freed.
 */
static void free_req_unk_t(req_unk_t *req_unk)
{
    if (req_unk != NULL)
        {
            g_free(req_unk);
        }
}


/**
 * Creates a new req_requests_t structure initialized with zeros
 * @return a newly allocated requests_t structure that can
 *         be freed with free_requests_t() function when no
 *         longer needed.
 */
static requests_t *new_requests_t(void)
{
    requests_t *requests = NULL;


    requests = (requests_t *) g_malloc0(sizeof(requests_t));

    requests->get = new_req_get_t();
    requests->post = new_req_post_t();
    requests->unknown = new_req_unk_t();
    requests->nb_request = 0;

    return requests;
}


/**
 * Frees an allocated request_t structure
 * @param requests is an allocated requests_t structure to be freed.
 */
static void free_requests_t(requests_t *requests)
{
    if (requests != NULL)
        {
            free_req_get_t(requests->get);
            free_req_post_t(requests->post);
            free_req_unk_t(requests->unknown);
            g_free(requests);
        }
}


/**
 * Creates a new stats_t structure initialized
 * with 0.
 * @returns a newly allocated stats_t structure that
 *          can be freed when no longer needed (free_stats_t)
 */
stats_t *new_stats_t(void)
{
    stats_t *stats = NULL;

    stats = (stats_t *) g_malloc0(sizeof(stats_t));

    stats->requests = new_requests_t();
    stats->nb_files = 0;
    stats->nb_dedup_bytes = 0;
    stats->nb_total_bytes = 0;
    stats->nb_meta_bytes = 0;

    return stats;
}


/**
 * Frees an allocated stats_t structure
 * @param stats is an allocated stats_t structure to be freed.
 */
void free_stats_t(stats_t *stats)
{
    if (stats != NULL)
        {
            free_requests_t(stats->requests);
            g_free(stats);
        }
}


/**
 * Adds in stats_t structure one 'GET' request.
 * @param stats is a stats_t structure to keep some stats about
 *        server runs.
 */
void add_one_get_request(stats_t *stats)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->get != NULL)
        {
            stats->requests->nb_request += 1;
            stats->requests->get->nb_request +=1;
        }
}


/**
 * Adds in stats_t structure one 'POST' more get request.
 * @param stats is a stats_t structure to keep some stats about
 *        server runs.
 */
void add_one_post_request(stats_t *stats)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->post != NULL)
        {
            stats->requests->nb_request += 1;
            stats->requests->post->nb_request += 1;
        }
}


/**
 * Counts in stats_t structure one 'unknown' more get request.
 * @param stats is a stats_t structure to keep some stats about
 *        server runs.
 */
void add_one_unknown_request(stats_t *stats)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->unknown != NULL)
        {
            stats->requests->nb_request += 1;
            stats->requests->unknown->nb_request += 1;
        }
}


/**
 * Adds one to the number of files saved
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
void add_one_saved_file(stats_t *stats)
{
    if (stats != NULL)
        {
            stats->nb_files += 1;
        }
}


/**
 * Adds meta data bytes to the stats structure
 * @param stats is a stats_t structure to keep some stats about server runs
 * @param nb_bytes is a size_t number representing the number of bytes to
 *        add to the stats
 */
static void add_bytes_to_metadata_bytes(stats_t *stats, size_t nb_bytes)
{
    if (stats != NULL)
        {
            stats->nb_meta_bytes += (guint64) nb_bytes;
        }
}


/**
 * Adds file size to stats->nb_total_bytes in order to count the real
 * size of what has been saved
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param size is the file sized being saved to be added
 */
void add_file_size_to_total_size(stats_t *stats, guint64 size)
{
    if (stats != NULL)
        {
            stats->nb_total_bytes += size;
        }
}


/**
 * Adds the size  of the hash to the total dedup number of bytes
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param hash_data is a hash_data_t structure with read field representing
 *        the size of the hashed buffer.
 */
void add_hash_size_to_dedup_bytes(stats_t *stats, hash_data_t *hash_data)
{
    if (stats != NULL && hash_data != NULL)
        {
            stats->nb_dedup_bytes += hash_data->read;
        }
}


/**
 * Adds one to the number of visits of /Stats.json url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
void add_one_to_get_url_stats(stats_t *stats)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->get != NULL)
        {
            stats->requests->get->stats += 1;
        }
}


/**
 * Adds one to the number of visits of /Version.json or /Version url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param txt is a boolean set to TRUE if the URL is a text one (not ending with
 *            .json
 */
void add_one_to_get_url_version(stats_t *stats, gboolean txt)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->get != NULL)
        {
            if (txt == TRUE)
                {
                    stats->requests->get->verstxt += 1;
                }
            else
                {
                    stats->requests->get->version += 1;
                }
        }
}


/**
 * Adds one to the number of visits of /File/List.json url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
void add_one_to_get_url_file_list(stats_t *stats)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->get != NULL)
        {
            stats->requests->get->file_list += 1;
        }
}


/**
 * Adds one to the number of visits of /Data/0xxxx.json url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
void add_one_to_get_url_data_hash(stats_t *stats)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->get != NULL)
        {
            stats->requests->get->data_hash += 1;
        }
}


/**
 * Adds one to the number of visits of /Data/Hash_Array.json url
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
void add_one_to_get_url_data_hash_array(stats_t *stats)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->get != NULL)
        {
            stats->requests->get->data_hash_array += 1;
        }
}


/**
 * Adds one to the number of visits of unknown URL (if txt is FALSE then the
 * unknown URL ends with .json
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param txt is a boolean set to TRUE if the URL is a text one (not ending with
 *            .json
 */
void add_one_to_get_url_unknown(stats_t *stats, gboolean txt)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->get != NULL)
        {
            if (txt == TRUE)
                {
                    stats->requests->get->unktxt += 1;
                }
            else
                {
                    stats->requests->get->unk += 1;
                }
        }
}


/*** POST requests ***/
/**
 * Adds one to the number of visits of /Meta.json
 * @param stats is a stats_t structure to keep some stats about server's usage.
 * @param length is the total length of the request.
 */
void add_length_and_one_to_post_url_meta(stats_t *stats, guint64 length)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->post != NULL)
        {
            stats->requests->post->meta += 1;
            add_bytes_to_metadata_bytes(stats, length);
        }
}


/**
 * Adds one to the number of visits of /Hash_Array.json
 * @param stats is a stats_t structure to keep some stats about server's usage.
 */
void add_one_to_post_url_hash_array(stats_t *stats)
{
    if (stats != NULL && stats->requests != NULL && stats->requests->post != NULL)
        {
            stats->requests->post->hash_array += 1;
        }
}

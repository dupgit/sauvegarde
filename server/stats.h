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
 * @struct stats_t
 * @brief Structure that will contain some statistics.
 */
typedef struct
{
    guint64 nb_get;          /**< nb_get is the number of GET requests                                                          */
    guint64 nb_post;         /**< nb_post is the number of POST requests                                                        */
    guint64 nb_files;        /**< nb_files is the number of version of files saved                                              */
    guint64 nb_dedup_bytes;  /**< nb_dedup_bytes is the number of bytes saved by the server (the dedup ones)                    */
    guint64 nb_total_bytes;  /**< nb_total_bytes is the number of bytes represented by file sizes of saved files (before dedup) */
    guint64 nb_meta_bytes;   /**< nb_meta_bytes is the number of bytes of all the meta data saved                               */
} stats_t;

#endif /* #ifndef _STATS_H_ */

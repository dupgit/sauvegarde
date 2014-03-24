/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    antememoire.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 Olivier Delhomme
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
 * @file antememoire.h
 *
 *  This file contains all the definitions for the ciseaux program.
 */
#ifndef _ANTEMEMOIRE_H_
#define _ANTEMEMOIRE_H_

/**
 * @struct meta_data_t
 * Structure to store all meta data associated with a file or a directory
 * command line. We want to limit memory consumption and thus we use the
 * guint instead of gchar *.
 * @note Do we need to store the blocksize here ? Does it have any sense ?
 *       Is it necessary to store the size read for each hashed buffer ? If
 *       we do it has to be done into the GTree in insert_into_tree function
 */
typedef struct
{
    guint8 file_type;  /** type of the file : FILE, DIR, SYMLINK...             */
    guint32 mode;      /** UNIX mode of the file : contains rights for the file */
    guint64 atime;     /** access time                                          */
    guint64 ctime;     /** changed time                                         */
    guint64 mtime;     /** modified time                                        */
    gchar *owner;      /** owner for the file ie root, apache, dup...           */
    gchar *group;      /** group for the file ie root, apache, admin...         */
    guint32 uid;       /** uid  (owner)                                         */
    guint32 gid;       /** gid  (group owner)                                   */
    gchar *name;       /** name for the file or the directory                   */
    GSList hash_list;  /** List of hashs of the file                            */
} meta_data_t;


/**
 * @returns a newly allocated meta_data_t * empty structure. We use 65534
 * as default uid and gid to avoid using 0 which is dedicated to a
 * priviledged user.
 */
 meta_data_t *new_meta_data_t();


/**
 * This function is a thread that is waiting to receive messages from
 * the checksum function and whose aim is to store somewhere the data
 * of a buffer that a been checksumed.
 * @param data : main_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
extern gpointer store_buffer_data(gpointer data);

#endif /* #ifndef _ANTEMEMOIRE_H_ */

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    files.h
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
 * @file files.h
 *
 *  This file contains all the definitions for the part that deals with
 *  files of "Sauvegarde" collection programs.
 */
#ifndef _FILES_H_
#define _FILES_H_

/**
 * @struct meta_data_t
 * @brief Stores file's meta datas.
 *
 * Structure to store all meta data associated with a file or a directory
 * command line. We want to limit memory consumption and thus we use the
 * guint instead of gchar *.
 * @note Do we need to store the blocksize here ? Does it have any sense ?
 *       Is it necessary to store the size read for each hashed buffer ? If
 *       we do it has to be done into the GTree in insert_into_tree function
 */
typedef struct
{
    guint8 file_type;  /**< type of the file : FILE, DIR, SYMLINK...             */
    guint32 mode;      /**< UNIX mode of the file : contains rights for the file */
    guint64 atime;     /**< access time                                          */
    guint64 ctime;     /**< changed time                                         */
    guint64 mtime;     /**< modified time                                        */
    gchar *owner;      /**< owner for the file ie root, apache, dup...           */
    gchar *group;      /**< group for the file ie root, apache, admin...         */
    guint32 uid;       /**< uid  (owner)                                         */
    guint32 gid;       /**< gid  (group owner)                                   */
    gchar *name;       /**< name for the file or the directory                   */
    GSList *hash_list; /**< List of hashs of the file                            */
} meta_data_t;


/**
 * Gets the filename of a  GFile
 * @param a_file : the GFile to get the filename from.
 * @returns the name of the GFile if any or "--" gchar * string that may be
 *          freed when no longer needed
 */
extern gchar *get_filename_from_gfile(GFile *a_file);


/**
 * Returns the username of the owner of the a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 * @returns the "user:group uid:gid" of the file or an empty string if an
 *          error occurs
 */
extern gchar *get_username_owner_from_gfile(GFileInfo *fileinfo, meta_data_t *meta);


/**
 * Returns the dates of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 * @returns "access_time changed_time created_time" gchar *string
 */
extern gchar *get_dates_from_gfile(GFileInfo *fileinfo, meta_data_t *meta);


/**
 * Get unix mode of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 * @returns a newly allocated string with file mode in decimal
 *          representation.
 */
extern gchar *get_file_mode_from_gfile(GFileInfo *fileinfo, meta_data_t *meta);


/**
 * @returns a newly allocated meta_data_t * empty structure. We use 65534
 * as default uid and gid to avoid using 0 which is dedicated to a
 * priviledged user.
 */
extern meta_data_t *new_meta_data_t(void);


/**
 * Frees the meta_data_t * structure
 * @param meta is a meta_data_t * structure to be freed
 * @returns always NULL
 */
extern gpointer free_meta_data_t(meta_data_t *meta);

#endif /* #ifndef _FILES_H_ */

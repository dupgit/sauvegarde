/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    files.h
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
 * @file files.h
 *
 *  This file contains all the definitions for the part that deals with
 *  files of "Sauvegarde" collection programs.
 */
#ifndef _FILES_H_
#define _FILES_H_

/**
 * @struct meta_data_t
 * @brief Stores file's meta data.
 *
 * Structure to store all meta data associated with a file or a directory
 * command line. We want to limit memory consumption and thus we use the
 * guint instead of gchar *.
 */
typedef struct
{
    guint64 inode;          /**< file's inode.                                                                    */
    guint8 file_type;       /**< type of the file : FILE, DIR, SYMLINK...                                         */
    guint32 mode;           /**< UNIX mode of the file : contains rights for the file                             */
    guint64 atime;          /**< access time                                                                      */
    guint64 ctime;          /**< changed time                                                                     */
    guint64 mtime;          /**< modified time                                                                    */
    guint64 size;           /**< size of the file                                                                 */
    gchar *owner;           /**< owner for the file ie root, apache, dup...                                       */
    gchar *group;           /**< group for the file ie root, apache, admin...                                     */
    guint32 uid;            /**< uid (owner)                                                                      */
    guint32 gid;            /**< gid (group owner)                                                                */
    gchar *name;            /**< name for the file or the directory                                               */
    gchar *link;            /**< link name where points the LINK if file_type is a link                           */
    GSList *hash_data_list; /**< List of hash_data_t structures of the file (hash and data are in a binary form)  */
    gboolean in_cache;      /**< in_cache is a boolean that may be TRUE if the file is in the local cache (client)*/
    gint64 blocksize;       /**< blocksize is the blocksize to be applied on the file                             */
} meta_data_t;


/**
 * @struct serveur_meta_data_t
 * @brief This structure contains a meta_data_t * field with all
 *        informations about a file's metadata and a hostname field that
 *        says from which machine the file comes from.
 */
typedef struct
{
    gchar *hostname;    /**< machine's hostname from which comes the metadata below */
    gboolean data_sent; /**< Tells wether data has already been sent to us or not   */
    meta_data_t *meta;  /**< metadata of a file of the 'hostname' machine           */
} serveur_meta_data_t;


/**
 * Gets the filename of a  GFile
 * @param a_file : the GFile to get the filename from.
 * @returns the name of the GFile if any or "--" gchar * string that may be
 *          freed when no longer needed
 */
extern gchar *get_filename_from_gfile(GFile *a_file);


/**
 * Returns the inode of the file fileinfo
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *                   (GFile *)
 * @param[out] meta : meta_data_t * structure that contains all meta data
 *                    for the corresponding file (populated here with
 *                    inode number.
 * @returns the inode file.
 */
extern guint64 get_inode_from_gfile(GFileInfo *fileinfo, meta_data_t *meta);


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
 * sets the dates to a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        to set to the corresponding file.
 */
extern void set_dates_to_gfile(GFileInfo *fileinfo, meta_data_t *meta);


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
 * Set unix mode of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 */
extern void set_file_mode_to_gfile(GFileInfo *fileinfo, meta_data_t *meta);



/**
 * Gets the size of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 * @returns a newly allocated string with file size in decimal
 *          representation.
 */
extern gchar *get_file_size_from_gfile(GFileInfo *fileinfo, meta_data_t *meta);


/**
 * Returns the file size from a GFile * file
 * @param file is the GFile from which we want the size.
 * @returns a guint64 that represents the file size or 0.
 */
extern guint64 get_file_size(GFile *file);


/**
 * @returns a newly allocated meta_data_t * empty structure. We use 65534
 * as default uid and gid to avoid using 0 which is dedicated to a
 * priviledged user.
 */
extern meta_data_t *new_meta_data_t(void);


/**
 * @returns a newly allocated serveur_meta_data_t * empty structure.
 */
extern serveur_meta_data_t *new_smeta_data_t(void);


/**
 * Frees the meta_data_t * structure
 * @param meta is a meta_data_t * structure to be freed
 * @param free_link is a boolean that when set to TRUE will free
 * @returns always NULL
 */
extern gpointer free_meta_data_t(meta_data_t *meta, gboolean free_link);


/**
 * Frees the serveur_meta_data_t * structure
 * @param smeta is a meta_data_t * structure to be freed
 * @returns always NULL
 */
extern gpointer free_smeta_data_t(serveur_meta_data_t *smeta);


/**
 * Wrapper for the g_slist_free_full function
 * the pointer to the data to be freed
 * @param the pointer to the data to be freed by free_smeta_data_t call.
 */
extern void gslist_free_smeta(gpointer data);


/**
 * Checks if a filename exists or not.
 * @param filename that we want to check.
 * @returns TRUE if filename exists and FALSE if not.
 */
extern gboolean file_exists(gchar *filename);


/**
 * Comparison function to be used when sorting filenames
 * @param a filename a
 * @param b filename b to be compared with a
 * @returns a negative integer if the a comes before b, 0 if they are
 *          equal, or a positive integer if the a comes after b.
 */
extern gint compare_filenames(gconstpointer a, gconstpointer b);


/**
 * Prints a file ands its meta data to the screen
 * @param smeta is the server meta data of the file to be printed on the
 *        screen
 */
extern void print_smeta_to_screen(serveur_meta_data_t *smeta);


/**
 * Sets file attributes
 * @param file is a GFile pointer and must not be null
 * @param meta is the structure that contains all meta data for the
 *        file that we want to set.
 */
extern void set_file_attributes(GFile *file, meta_data_t *meta);


/**
 * Makes a symbolic link named  with 'file' filename that points to the
 * target 'points_to'
 * @param file is the file to create as a symbolic link
 * @param points_to is the target of the link
 */
extern void make_symbolic_link(GFile *file, gchar *points_to);



/**
 * Replaces ~ (if found at the first place) by the home directory
 * of the user.
 * @param path is a gchar * string that should contain a path
 * @returns always returns a newly allocated gchar * string that contains
 *          the normalized path or the path itself;
 */
extern gchar *normalize_directory(gchar *path);
#endif /* #ifndef _FILES_H_ */

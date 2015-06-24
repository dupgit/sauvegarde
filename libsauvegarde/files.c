/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    files.c
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
 * @file files.c
 * This file contains the functions to deal with files of the "Sauvegarde"
 * collection programs.
 */

#include "libsauvegarde.h"


/**
 * @returns a newly allocated meta_data_t * empty structure. We use 65534
 * as default uid and gid to avoid using 0 which is dedicated to a
 * priviledged user.
 */
meta_data_t *new_meta_data_t(void)
{
    meta_data_t *meta = NULL;

    meta = (meta_data_t *) g_malloc0(sizeof(meta_data_t));

    if (meta != NULL)
        {
            meta->file_type = 0;
            meta->inode = 0;
            meta->mode = 0;
            meta->atime = 0;
            meta->ctime = 0;
            meta->mtime = 0;
            meta->size = 0;
            meta->owner = NULL;
            meta->group = NULL;
            meta->uid = 65534;  /* nfsnobody on my system ie unpriviledged user */
            meta->gid = 65534;  /* nfsnobody on my system ie unpriviledged user */
            meta->name = NULL;
            meta->hash_list = NULL;
        }

    return meta;
}


/**
 * @returns a newly allocated serveur_meta_data_t * empty structure.
 */
serveur_meta_data_t *new_smeta_data_t(void)
{
    serveur_meta_data_t *smeta = NULL;

    smeta = (serveur_meta_data_t *) g_malloc0(sizeof(serveur_meta_data_t));

    if (smeta != NULL)
        {
            smeta->hostname = NULL;
            smeta->meta = NULL;
        }

    return smeta;
}


/**
 * Frees the meta_data_t * structure
 * @param meta is a meta_data_t * structure to be freed
 * @returns always NULL
 */
gpointer free_meta_data_t(meta_data_t *meta)
{
    if (meta != NULL)
        {
            free_variable(meta->owner);
            free_variable(meta->group);
            free_variable(meta->name);
            g_slist_free(meta->hash_list);
            free_variable(meta);
        }

    return NULL;
}


/**
 * Frees the serveur_meta_data_t * structure
 * @param smeta is a meta_data_t * structure to be freed
 * @returns always NULL
 */
gpointer free_smeta_data_t(serveur_meta_data_t *smeta)
{
    if (smeta != NULL)
        {
            smeta->meta = free_meta_data_t(smeta->meta);
            smeta->hostname = free_variable(smeta->hostname);
            smeta = free_variable(smeta);
        }

    return NULL;
}


/**
 * Wrapper for the g_slist_free_full function
 * the pointer to the data to be freed
 * @param the pointer to the data to be freed by free_smeta_data_t call.
 */
void gslist_free_smeta(gpointer data)
{
    free_smeta_data_t((serveur_meta_data_t *)data);
}


/**
 * Gets the filename of a  GFile
 * @param a_file : the GFile to get the filename from.
 * @returns the name of the GFile if any or "--" gchar * string that may be
 *          freed when no longer needed
 */
gchar *get_filename_from_gfile(GFile *a_file)
{
    gchar *filename = NULL;

    if (a_file != NULL)
        {
            filename = g_file_get_parse_name(a_file);
        }
    else
        {
            filename = NULL;
        }

    return filename;
}


/**
 * Returns the inode of the file fileinfo
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *                   (GFile *)
 * @param[out] meta : meta_data_t * structure that contains all meta data
 *                    for the corresponding file (populated here with
 *                    inode number.
 * @returns the inode file.
 */
guint64 get_inode_from_gfile(GFileInfo *fileinfo, meta_data_t *meta)
{
    guint64 inode = 0;


    if (fileinfo != NULL && meta != NULL)
        {
            inode = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_UNIX_INODE);

            meta->inode = inode;
        }

    return inode;
}


/**
 * Returns the username of the owner of the file fileinfo
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *                   (GFile *)
 * @param[out] meta : meta_data_t * structure that contains all meta data
 *                    for the corresponding file (populated here with owner,
 *                    group, uid and gid.
 * @returns the "user:group uid:gid" of the file or an empty string if an
 *          error occurs
 */
gchar *get_username_owner_from_gfile(GFileInfo *fileinfo, meta_data_t *meta)
{
    gchar *owner = NULL;
    gchar *group = NULL;
    gchar *result = NULL;
    gchar *ids = NULL;
    guint32 uid = 0;
    guint32 gid = 0;


    if (fileinfo != NULL && meta != NULL)
        {


            owner = g_file_info_get_attribute_as_string(fileinfo, G_FILE_ATTRIBUTE_OWNER_USER);
            group = g_file_info_get_attribute_as_string(fileinfo, G_FILE_ATTRIBUTE_OWNER_GROUP);

            uid = g_file_info_get_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_UID);
            gid = g_file_info_get_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_GID);

            ids = g_strdup_printf("%d:%d", uid, gid);

            meta->owner = g_strdup(owner);
            meta->group = g_strdup(group);
            meta->uid = uid;
            meta->gid = gid;

            result = g_strconcat(owner, ":", group, " ", ids, NULL);

            free_variable(ids);
            free_variable(group);
            free_variable(owner);
        }
    else
        {
            result = g_strdup("");
        }

    return result;
}


/**
 * Returns the dates of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 * @returns "access_time changed_time modified_time" gchar *string
 */
gchar *get_dates_from_gfile(GFileInfo *fileinfo, meta_data_t *meta)
{
    guint64 atime = 0;
    guint64 ctime = 0;
    guint64 mtime = 0;
    gchar *result = NULL;

    if (fileinfo != NULL)
        {
            atime = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_ACCESS);
            ctime = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_CHANGED);
            mtime = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_MODIFIED);

            meta->atime = atime;
            meta->ctime = ctime;
            meta->mtime = mtime;

            result = g_strdup_printf("%" G_GUINT64_FORMAT " %"  G_GUINT64_FORMAT " %"  G_GUINT64_FORMAT "", atime, ctime, mtime);
        }
    else
        {
            result = g_strdup("");
        }

    return result;
}


/**
 * Get unix mode of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 * @returns a newly allocated string with file mode in decimal
 *          representation.
 */
gchar *get_file_mode_from_gfile(GFileInfo *fileinfo, meta_data_t *meta)
{
    guint32 mode = 0;
    gchar *result = NULL;

    if (fileinfo != NULL)
        {
            mode = g_file_info_get_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_MODE);
            meta->mode = mode;

            result =  g_strdup_printf("%d", mode);
        }
    else
        {
            result = g_strdup("");
        }

    return result;

}


/**
 * Gets the size of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 * @returns a newly allocated string with file size in decimal
 *          representation.
 */
gchar *get_file_size_from_gfile(GFileInfo *fileinfo, meta_data_t *meta)
{
    guint64 size = 0;
    gchar *result = NULL;

    if (fileinfo != NULL)
        {
            size = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_STANDARD_SIZE);
            meta->size = size;

            result = g_strdup_printf("%"  G_GUINT64_FORMAT "", size);
        }
    else
        {
            result = g_strdup("");
        }

    return result;
}


/**
 * Checks if a filename exists or not.
 * @param filename that we want to check.
 * @returns TRUE if filename exists and FALSE if not.
 */
gboolean file_exists(gchar *filename)
{
    GFile *file = NULL;
    gboolean exists = FALSE;

    if (filename != NULL)
        {
            file = g_file_new_for_path(filename);
            exists = g_file_query_exists(file, NULL);
            free_object(file);
        }

    return exists;
}


/**
 * Comparison function to be used when sorting filenames. First filenames
 * are compared and when an equality is found then the modified time is
 * compared (as a second sorting criteria)
 * @param a serveur_meta_data_t * representing a file 'a'
 * @param b serveur_meta_data_t * representing a file 'b' to be compared
 *          with 'a'
 * @returns a negative integer if the a comes before b, 0 if they are
 *          equal, or a positive integer if the a comes after b.
 */
gint compare_filenames(gconstpointer a, gconstpointer b)
{
    gchar *key_a = NULL;
    gchar *key_b = NULL;
    gint value = 0;
    serveur_meta_data_t *sa = (serveur_meta_data_t *) a;
    serveur_meta_data_t *sb = (serveur_meta_data_t *) b;


    key_a = g_utf8_collate_key_for_filename(sa->meta->name, -1);
    key_b = g_utf8_collate_key_for_filename(sb->meta->name, -1);

    value = strcmp(key_a, key_b);

    if (value == 0)
        { /* second sorting criteria : modification time */
            if (sa->meta->mtime < sb->meta->mtime)
                {
                    value = -1;
                }
            else if (sa->meta->mtime > sb->meta->mtime)
                {
                    value = 1;
                }
            else
                {
                    value = 0;
                }
        }

    free_variable(key_a);
    free_variable(key_b);

    return value;
}



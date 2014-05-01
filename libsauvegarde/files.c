/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    files.c
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
            meta->mode = 0;
            meta->atime = 0;
            meta->ctime = 0;
            meta->mtime = 0;
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
 * Returns the username of the owner of the a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
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

    if (fileinfo != NULL)
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

            result = g_strdup_printf("%ld %ld %ld", atime, ctime, mtime);
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


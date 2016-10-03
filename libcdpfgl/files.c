/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    files.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2016 Olivier Delhomme
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

#include "libcdpfgl.h"


/**
 * @returns a newly allocated meta_data_t * empty structure. We use 65534
 * as default uid and gid to avoid using 0 which is dedicated to a
 * priviledged user.
 */
meta_data_t *new_meta_data_t(void)
{
    meta_data_t *meta = NULL;

    meta = (meta_data_t *) g_malloc(sizeof(meta_data_t));

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
            meta->link = NULL;
            meta->hash_data_list = NULL;
            meta->in_cache = FALSE; /* a newly meta data is not in the local cache ! */
            meta->blocksize = 16384; /* Default blocksize */
        }

    return meta;
}


/**
 * @returns a newly allocated server_meta_data_t * empty structure.
 */
server_meta_data_t *new_smeta_data_t(void)
{
    server_meta_data_t *smeta = NULL;

    smeta = (server_meta_data_t *) g_malloc(sizeof(server_meta_data_t));

    if (smeta != NULL)
        {
            smeta->hostname = NULL;
            smeta->data_sent = FALSE;
            smeta->meta = NULL;
        }

    return smeta;
}


/**
 * Frees the meta_data_t * structure
 * @param meta is a meta_data_t * structure to be freed
 * @param free_link is a boolean that when set to TRUE will free
 * @returns always NULL
 */
gpointer free_meta_data_t(meta_data_t *meta, gboolean free_link)
{
    if (meta != NULL)
        {
            free_variable(meta->owner);
            free_variable(meta->group);
            free_variable(meta->name);

            if (free_link == TRUE)
                {
                    /* meta->link should not be freed only in 'client' program */
                    free_variable(meta->link);
                }

            g_list_free_full(meta->hash_data_list, free_hdt_struct);
            free_variable(meta);
        }

    return NULL;
}


/**
 * Wrapper for free_meta_data_t() function to be used
 * with g_list_free_full(). Links are freed by this
 * function.
 * @param data is the pointer to the meta_data_t * structure
 *        to be freed
 */
void free_glist_meta_data_t(gpointer data)
{
    free_meta_data_t((meta_data_t *) data, TRUE);
}


/**
 * Inserts a meta_data_t structure into a GList
 * @param list is the list where we want to insert the meta_data_t structure
 * @param meta is the meta_data_t structure we want to insert
 * @returns a new list with meta_data inserted.
 */
GList *insert_meta_data_t_in_list(GList *list, meta_data_t *meta)
{
    list = g_list_insert_sorted(list, meta, compare_meta_data_t);

    return list;
}


/**
 * Frees the server_meta_data_t * structure
 * @param smeta is a meta_data_t * structure to be freed
 * @returns always NULL
 */
gpointer free_smeta_data_t(server_meta_data_t *smeta)
{
    if (smeta != NULL)
        {
            smeta->meta = free_meta_data_t(smeta->meta, TRUE);
            smeta->hostname = free_variable(smeta->hostname);
            smeta = free_variable(smeta);
        }

    return NULL;
}


/**
 * Wrapper for the g_slist_free_full function
 * the pointer to the data to be freed
 * @param data the pointer to the data to be freed by free_smeta_data_t call.
 */
void gslist_free_smeta(gpointer data)
{
    free_smeta_data_t((server_meta_data_t *)data);
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
    gchar *result = NULL;

    if (fileinfo != NULL && meta != NULL)
        {


            meta->owner = g_file_info_get_attribute_as_string(fileinfo, G_FILE_ATTRIBUTE_OWNER_USER);
            meta->group = g_file_info_get_attribute_as_string(fileinfo, G_FILE_ATTRIBUTE_OWNER_GROUP);

            meta->uid = g_file_info_get_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_UID);
            meta->gid = g_file_info_get_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_GID);

            result = g_strdup_printf("%s:%s %d:%d", meta->owner, meta->group, meta->uid, meta->gid);
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
    gchar *result = NULL;

    if (fileinfo != NULL && meta != NULL)
        {
            meta->atime = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_ACCESS);
            meta->ctime = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_CHANGED);
            meta->mtime = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_MODIFIED);

            result = g_strdup_printf("%" G_GUINT64_FORMAT " %"  G_GUINT64_FORMAT " %"  G_GUINT64_FORMAT "", meta->atime, meta->ctime, meta->mtime);
        }
    else
        {
            result = g_strdup("");
        }

    return result;
}


/**
 * sets the dates to a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        to set to the corresponding file.
 */
void set_dates_to_gfile(GFileInfo *fileinfo, meta_data_t *meta)
{
    if (fileinfo != NULL && meta != NULL)
        {
            g_file_info_set_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_ACCESS, meta->atime);
            g_file_info_set_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_CHANGED, meta->ctime);
            g_file_info_set_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_TIME_MODIFIED, meta->mtime);
        }
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
    gchar *result = NULL;

    if (fileinfo != NULL)
        {
            meta->mode = g_file_info_get_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_MODE);

            result =  g_strdup_printf("%d", meta->mode);
        }
    else
        {
            result = g_strdup("");
        }

    return result;
}


/**
 * Set unix mode of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @param meta : meta_data_t * structure that contains all meta data for
 *        the corresponding file.
 */
void set_file_mode_to_gfile(GFileInfo *fileinfo, meta_data_t *meta)
{
    if (fileinfo != NULL && meta != NULL)
        {
            print_debug(_("Setting mode: %d\n"), meta->mode);
            g_file_info_set_attribute_uint32(fileinfo, G_FILE_ATTRIBUTE_UNIX_MODE, meta->mode);
        }
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
    gchar *result = NULL;

    if (fileinfo != NULL)
        {
            meta->size = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_STANDARD_SIZE);

            result = g_strdup_printf("%"  G_GUINT64_FORMAT "", meta->size);
        }
    else
        {
            result = g_strdup("");
        }

    return result;
}


/**
 * Returns the file size from a GFile * file
 * @param file is the GFile from which we want the size.
 * @returns a guint64 that represents the file size or 0.
 */
guint64 get_file_size(GFile *file)
{
    GError *error = NULL;
    GFileInfo *fileinfo = NULL;
    guint64 size = 0;

    if (file != NULL)
        {
            fileinfo = g_file_query_info(file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
            size = g_file_info_get_attribute_uint64(fileinfo, G_FILE_ATTRIBUTE_STANDARD_SIZE);
            free_object(fileinfo);
        }

    return size;
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
 * Searchs for a filename that doesn't exists yet
 * @param all_versions is true when we want to save all versions of a
 *        single file.
 * @param basename is the basename of the file ie without it directory
 *        location.
 * @param where is the directory location where the filename should be
 *        create.
 * @param newname is the original basename or the original basename
 *        slightly modified
 * @param the_date is a string representing the last modification date
 *        of the file.
 */
gchar *get_unique_filename(gboolean all_versions, gchar *basename, gchar *where, gchar *newname, gchar *the_date)
{
    gchar *filename = NULL;
    guint i = 0;

    i = 0;
    filename = g_build_filename(where, newname, NULL);

    while (file_exists(filename))
        {
            free_variable(filename);
            free_variable(newname);

            if (all_versions == TRUE)
                {
                    newname = g_strdup_printf("%s-%d_%s", the_date, i, basename);
                }
            else
                {
                    newname = g_strdup_printf("%d-%s", i, basename);
                }

            filename = g_build_filename(where, newname, NULL);
            i++;
        }

    return filename;
}


/**
 * Comparison function to be used to sort meta_data_t into
 * a list.
 * @param a meta_data_t * containing meta data of a file 'a'
 * @param b meta_data_t * containing meta data ofa file 'b' to be compared
 *          with 'a'
 * @returns a negative integer if the a comes before b, 0 if they are
 *          equal, or a positive integer if the a comes after b.
 */
gint compare_meta_data_t(gconstpointer a, gconstpointer b)
{
    meta_data_t *meta_a = (meta_data_t *) a;
    meta_data_t *meta_b = (meta_data_t *) b;
    gchar *key_a = NULL;
    gchar *key_b = NULL;
    gint value = 0;


    if (meta_a != NULL && meta_b != NULL)
        {

            key_a = g_utf8_collate_key_for_filename(meta_a->name, -1);
            key_b = g_utf8_collate_key_for_filename(meta_b->name, -1);

            value = strcmp(key_a, key_b);

            if (value == 0)
                { /* second sorting criteria : modification time */
                    if (meta_a->mtime < meta_b->mtime)
                        {
                            value = -1;
                        }
                    else if (meta_a->mtime > meta_b->mtime)
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
        }
    else if (meta_a == NULL && meta_b == NULL)
        {
            value = 0;
        }
    else if (meta_a == NULL)
        {
            value = 1;
        }
    else
        {
            value = -1;
        }

    return value;
}


/**
 * Comparison function to be used when sorting filenames. First filenames
 * are compared and when an equality is found then the modified time is
 * compared (as a second sorting criteria)
 * @param a server_meta_data_t * representing a file 'a'
 * @param b server_meta_data_t * representing a file 'b' to be compared
 *          with 'a'
 * @returns a negative integer if the a comes before b, 0 if they are
 *          equal, or a positive integer if the a comes after b.
 */
gint compare_filenames(gconstpointer a, gconstpointer b)
{
    server_meta_data_t *sa = (server_meta_data_t *) a;
    server_meta_data_t *sb = (server_meta_data_t *) b;

    if (sa != NULL && sb != NULL)
        {
            return  compare_meta_data_t(sa->meta, sb->meta);
        }
    else if (sa == NULL && sb == NULL)
        {
            return 0;
        }
    else if (sa == NULL)
        {
            return 1;
        }
    else
        {
            return -1;
        }
}


/**
 * Gets the date into a string with a specific format
 * @param nanosec is a number that represents nanoseconds since epoch
 * @param escaped is a boolean that may be TRUE if we need a date string
 *        escaped and filesystems compliant.
 */
gchar *transform_date_to_string(guint64 nanosec, gboolean escaped)
{
    GDateTime *la_date = NULL;
    gchar *the_date = NULL;

    la_date = g_date_time_new_from_unix_local(nanosec);

    if (escaped == TRUE)
        {
            /** @note Not including the time zone here. Should we ? */
            the_date = g_date_time_format(la_date, "%F-%H-%M-%S");
        }
    else
        {
            the_date = g_date_time_format(la_date, "%F %T %z");
        }

    g_date_time_unref(la_date);

    return the_date;
}


/**
 * Prints a file and its meta data to the screen
 * @param smeta is the server meta data of the file to be printed on the
 *        screen
 */
void print_smeta_to_screen(server_meta_data_t *smeta)
{
    meta_data_t *meta = NULL;   /**< helper to access smeta->meta structure do not free ! */
    gchar *the_date = NULL;

    if (smeta !=  NULL && smeta->meta != NULL)
        {
            meta = smeta->meta;

            switch (meta->file_type)
                {
                    case 1:
                        fprintf(stdout, "[FILE] ");
                    break;
                    case 2:
                        fprintf(stdout, "[DIR ] ");
                    break;
                    case 3:
                        fprintf(stdout, "[LINK] ");
                    break;
                    default:
                        fprintf(stdout, "[    ] ");
                    break;
                }

            the_date = transform_date_to_string(meta->mtime, FALSE);

            fprintf(stdout, "%s %s\n", the_date, meta->name);

            free_variable(the_date);
        }
}


/**
 * Sets file attributes
 * @param file is a GFile pointer and must not be null
 * @param meta is the structure that contains all meta data for the
 *        file that we want to set.
 */
void set_file_attributes(GFile *file, meta_data_t *meta)
{
    GError *error = NULL;
    GFileInfo *fileinfo = NULL;

    if (file != NULL && meta != NULL)
        {
            fileinfo = g_file_query_info(file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

            if (fileinfo == NULL || error != NULL)
                {
                    print_error(__FILE__, __LINE__, _("Error while getting file information: %s\n"), error->message);
                    error = free_error(error);
                }
            else
                {
                    set_file_mode_to_gfile(fileinfo, meta);
                    set_dates_to_gfile(fileinfo, meta);

                    if (g_file_set_attributes_from_info(file, fileinfo, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error) == FALSE && error != NULL)
                        {
                            print_error(__FILE__, __LINE__, _("Error or warning for file (%s): %s\n"), meta->name, error->message);
                            free_error(error);
                        }

                    free_object(fileinfo);
                }
        }
    else
        {
            /* To translators : do not translate this ! */
            print_error(__FILE__, __LINE__, "set_file_attribute(file = %p, meta = %p)\n", file, meta);
        }
}


/**
 * Makes a symbolic link named  with 'file' filename that points to the
 * target 'points_to'
 * @param file is the file to create as a symbolic link
 * @param points_to is the target of the link
 */
void make_symbolic_link(GFile *file, gchar *points_to)
{
    gchar *filename = NULL;
    GError *error = NULL;

    if (file != NULL && points_to != NULL)
        {
            if (g_file_make_symbolic_link(file, points_to, NULL, &error) == FALSE && error != NULL)
                {
                    filename = g_file_get_path(file);
                    print_error(__FILE__, __LINE__, _("Error: unable to create symbolic link %s to %s: %s.\n"), filename, points_to, error->message);
                    free_variable(filename);
                }
        }
}


/**
 * Replaces ~ (if found at the first place) by the home directory
 * of the user.
 * @param path is a gchar * string that should contain a path
 * @returns always returns a newly allocated gchar * string that contains
 *          the normalized path or the path itself;
 */
gchar *normalize_directory(gchar *path)
{
    gchar *dircache = NULL;

    if (path != NULL && path[0] == '~')
        {
            dircache = g_strconcat(g_get_home_dir(), path+1, NULL);
        }
    else
        {
            dircache = g_strdup(path);
        }

    return dircache;
}


/**
 * @param size is the size of the considered file.
 * @returns the maximum number of hashs that may be asked for into a
 *          single GET HTTP request.
 */
gint calculate_max_number_of_hashs(guint64 size)
{

    if (size < 32768)
        {
            return 64;
        }
    else if (size < 262144)
        {
            return 128;
        }
    else if (size < 1048576)
        {
            return 128;
        }
    else if (size < 8388608)
        {
            return 128;
        }
    else if (size < 67108864)
        {
            return 64;
        }
    else if (size < 134217728)
        {
            return 32;
        }
    else
        {
            return 16;
        }
}

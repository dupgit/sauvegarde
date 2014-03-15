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
 * @returns the "user:group uid:gid" of the file or an empty string if an
 *          error occurs
 */
extern gchar *get_username_owner_from_gfile(GFileInfo *fileinfo);


/**
 * Returns the dates of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @returns "access_time changed_time created_time" gchar *string
 */
extern gchar *get_dates_from_gfile(GFileInfo *fileinfo);


/**
 * Get unix mode of a file
 * @param fileinfo : a GFileInfo pointer obtained from an opened file
 *        (GFile *)
 * @returns a newly allocated string with file mode in decimal
 *          representation.
 */
extern gchar *get_file_mode_from_gfile(GFileInfo *fileinfo);

#endif /* #ifndef _FILES_H_ */

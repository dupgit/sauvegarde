/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    fanotify.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 Olivier Delhomme
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
 * @file fanotify.h
 *
 * In this file we have all definition for fanotify's monitor interface.
 */
#ifndef _FANOTIFY_H_
#define _FANOTIFY_H_

/**
 * Inits and starts fanotify notifications
 * @param opt : a filled options_t * structure that contains all options
 *        by default, read into the file or selected in the command line.
 */
extern gint start_fanotify(options_t *opt);


/**
 * Stops fanotify notifications
 */
extern void stop_fanotify(options_t *opt, int fanotify_fd);

#endif /* #IFNDEF _FANOTIFY_H_ */

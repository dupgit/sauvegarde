/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    m_fanotify.h
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 - 2016 Olivier Delhomme
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
 * @file m_fanotify.h
 *
 * In this file we have all definition for fanotify's monitor interface.
 */
#ifndef _M_FANOTIFY_H_
#define _M_FANOTIFY_H_

#define FANOTIFY_BUFFER_SIZE 49152    /* for 24 bytes events this is 2046 events */

/* Enumerate list of FDs to poll */
enum {
  FD_POLL_SIGNAL = 0,
  FD_POLL_FANOTIFY,
  FD_POLL_MAX
};


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


/**
 * fanotify main loop
 * @todo simplify code (CCN is 12 already !)
 */
extern void fanotify_loop(main_struct_t *main_struct);

#endif /* #IFNDEF _M_FANOTIFY_H_ */

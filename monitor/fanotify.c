/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    fanotify.c
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
 * @file fanotify.c
 *
 * This file does fanotify's monitor interface.
 */

#include "monitor.h"


/**
 * Inits and starts fanotify notifications
 * @param opt : a filled options_t * structure that contains all options
 *        by default, read into the file or selected in the command line.
 */
gint start_fanotify(options_t *opt)
{
    gint fanotify_fd = -1;
    GSList *head = NULL;

    /* Setup fanotify notifications (FAN) mask. All these defined in fanotify.h. */
    static uint64_t event_mask =
      (FAN_ACCESS        |  /* File accessed                                              */
       FAN_MODIFY        |  /* File modified                                              */
       FAN_CLOSE_WRITE   |  /* Writtable file closed                                      */
       FAN_CLOSE_NOWRITE |  /* Unwrittable file closed                                    */
       FAN_OPEN          |  /* File was opened                                            */
       FAN_ONDIR         |  /* We want to be reported of events in the directory          */
       FAN_EVENT_ON_CHILD); /* We want to be reported of events in files of the directory */

    if (opt != NULL)
        {

            /* Create new fanotify device */
            if ((fanotify_fd = fanotify_init(FAN_CLOEXEC, O_RDONLY | O_CLOEXEC | O_LARGEFILE)) < 0)
                {
                    print_error(__FILE__, __LINE__, _("Couldn't setup new fanotify device: %s\n"), strerror(errno));
                }
            else
                {
                    head = opt->dirname_list;

                    while (head != NULL)
                        {
                            if (fanotify_mark(fanotify_fd, FAN_MARK_ADD, event_mask, AT_FDCWD, head->data) < 0)
                                {
                                  print_error(__FILE__, __LINE__, _("Couldn't add monitor in directory %s: %s\n"), head->data , strerror(errno));
                                }
                            else
                                {
                                    print_debug(_("Started monitoring directory %s\n"), head->data);
                                }

                            head = g_slist_next(head);
                        }
                }
        }

    return fanotify_fd;
}


/**
 * Stops fanotify notifications
 */
void stop_fanotify(options_t *opt, int fanotify_fd)
{
    GSList *head = NULL;
    /* Setup fanotify notifications (FAN) mask. All these defined in fanotify.h. */
    static uint64_t event_mask =
      (FAN_ACCESS        |  /* File accessed                                              */
       FAN_MODIFY        |  /* File modified                                              */
       FAN_CLOSE_WRITE   |  /* Writtable file closed                                      */
       FAN_CLOSE_NOWRITE |  /* Unwrittable file closed                                    */
       FAN_OPEN          |  /* File was opened                                            */
       FAN_ONDIR         |  /* We want to be reported of events in the directory          */
       FAN_EVENT_ON_CHILD); /* We want to be reported of events in files of the directory */

    if (opt != NULL)
        {
            head = opt->dirname_list;

            while (head != NULL)
                {
                    fanotify_mark(fanotify_fd, FAN_MARK_REMOVE, event_mask, AT_FDCWD, head->data);
                    head = g_slist_next(head);
                }

        }

  close(fanotify_fd);
}

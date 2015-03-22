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
 * This file does fanotify's monitor interface. This file is heavily based
 * on Aleksander Morgado fanotify-example.c's file (ie mainly copied !)
 */

#include "monitor.h"

/**
 * Stops signal handling
 */
void  stop_signals(int signal_fd)
{
    close(signal_fd);
}


/**
 * Starts signal handling
 */
gint start_signals(void)
{
    gint signal_fd = -1;
    sigset_t sigmask;

      /* We want to handle SIGINT and SIGTERM in the signal_fd, so we block them. */
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);
    sigaddset(&sigmask, SIGTERM);

    if (sigprocmask(SIG_BLOCK, &sigmask, NULL) < 0)
        {
            print_error(__FILE__, __LINE__, _("Couldn't block signals: %s\n"), strerror(errno));
        }

      /* Get new FD to read signals from it */
    if ((signal_fd = signalfd(-1, &sigmask, 0)) < 0)
        {
            print_error(__FILE__, __LINE__, _("Couldn't setup signal FD: %s\n"), strerror(errno));
        }

  return signal_fd;
}


/**
 * Inits and starts fanotify notifications
 * @param opt : a filled options_t * structure that contains all options
 *        by default, read into the file or selected in the command line.
 */
gint start_fanotify(options_t *opt)
{
    gint fanotify_fd = -1;
    GSList *head = NULL;

    /* Setup fanotify notifications (FAN) mask. All these defined in linux/fanotify.h. */
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
 * gets path from file descriptor
 */
static char *get_file_path_from_fd (gint fd, char *buffer, size_t  buffer_size)
{
    ssize_t len;

    if (fd <= 0)
        {
            return NULL;
        }

    sprintf(buffer, "/proc/self/fd/%d", fd);

    if ((len = readlink(buffer, buffer, buffer_size - 1)) < 0)
        {
            return NULL;
        }

    buffer[len] = '\0';

    return buffer;
}


static char *get_program_name_from_pid(int pid, char *buffer, size_t  buffer_size)
{
    int fd = 0;
    ssize_t len = 0;
    char *aux = NULL;

    /* Try to get program name by PID */
    sprintf(buffer, "/proc/%d/cmdline", pid);

    if ((fd = open (buffer, O_RDONLY)) < 0)
        {
            return NULL;
        }

    /* Read file contents into buffer */
    if ((len = read (fd, buffer, buffer_size - 1)) <= 0)
        {
            close (fd);
            return NULL;
        }
    else
        {
            close (fd);

            buffer[len] = '\0';
            aux = strstr(buffer, "^@");

            if (aux)
                {
                    *aux = '\0';
                }

            return buffer;
        }
}


/**
 * An example of processing events
 */
static void event_process(struct fanotify_event_metadata *event)
{
    char path[PATH_MAX];

    print_debug(_("Received event in path: %s"), get_file_path_from_fd(event->fd, path, PATH_MAX) ? path : "unknown");
    print_debug(_(" pid=%d (%s): \n"), event->pid, (get_program_name_from_pid (event->pid, path, PATH_MAX) ? path : "unknown"));


    if (event->mask & FAN_OPEN)
        {
             print_debug(_("\tFAN_OPEN\n"));
        }

    if (event->mask & FAN_ACCESS)
        {
            print_debug(_("\tFAN_ACCESS\n"));
        }

    if (event->mask & FAN_MODIFY)
        {
            print_debug(_("\tFAN_MODIFY\n"));
        }

    if (event->mask & FAN_CLOSE_WRITE)
        {
            print_debug(_("\tFAN_CLOSE_WRITE\n"));
        }

    if (event->mask & FAN_CLOSE_NOWRITE)
        {
            print_debug(_("\tFAN_CLOSE_NOWRITE\n"));
        }

    fflush (stdout);

    close(event->fd);
}


/**
 * Stops fanotify notifications
 */
void stop_fanotify(options_t *opt, int fanotify_fd)
{
    GSList *head = NULL;
    /* Setup fanotify notifications (FAN) mask. All these defined in linux/fanotify.h. */
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


/**
 * fanotify main loop
 */
void fanotify_loop(gint signal_fd, gint fanotify_fd)
{
    struct pollfd fds[FD_POLL_MAX];
    struct signalfd_siginfo fdsi;
    char buffer[FANOTIFY_BUFFER_SIZE];
    ssize_t length = 0;
    struct fanotify_event_metadata *fe_mdata = NULL;

    /* Setup polling */
    fds[FD_POLL_SIGNAL].fd = signal_fd;
    fds[FD_POLL_SIGNAL].events = POLLIN;
    fds[FD_POLL_FANOTIFY].fd = fanotify_fd;
    fds[FD_POLL_FANOTIFY].events = POLLIN;

    while (1)
        {
            /* Block until there is something to be read */
            if (poll(fds, FD_POLL_MAX, -1) < 0)
                {
                    print_error(__FILE__, __LINE__, _("Couldn't poll(): '%s'\n"), strerror(errno));
                }

            /* Signal received? */
            if (fds[FD_POLL_SIGNAL].revents & POLLIN)
                {
                    if (read(fds[FD_POLL_SIGNAL].fd, &fdsi, sizeof(fdsi)) != sizeof(fdsi))
                        {
                          print_error(__FILE__, __LINE__, _("Couldn't read signal, wrong size read\n"));
                        }

                    /* Break loop if we got SIGINT or SIGTERM */
                    if (fdsi.ssi_signo == SIGINT || fdsi.ssi_signo == SIGTERM)
                        {
                          break;
                        }

                    print_error(__FILE__, __LINE__, _("Received unexpected signal\n"));
                }

            /* fanotify event received? */
            if (fds[FD_POLL_FANOTIFY].revents & POLLIN)
                {
                    /* Read from the FD. It will read all events available up to
                     * the given buffer size. */
                    if ((length = read (fds[FD_POLL_FANOTIFY].fd, buffer, FANOTIFY_BUFFER_SIZE)) > 0)
                        {
                            fe_mdata = (struct fanotify_event_metadata *) buffer;

                            while (FAN_EVENT_OK(fe_mdata, length))
                                {
                                    event_process(fe_mdata);

                                    if (fe_mdata->fd > 0)
                                        {
                                            close(fe_mdata->fd);
                                        }

                                    fe_mdata = FAN_EVENT_NEXT(fe_mdata, length);
                                }
                        }
                }
        }

}




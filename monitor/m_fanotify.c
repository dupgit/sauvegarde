/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    m_fanotify.c
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
 * @file m_fanotify.c
 *
 * This file does fanotify's monitor interface. This file is heavily based
 * on Aleksander Morgado fanotify-example.c's file (ie mainly copied !)
 * @todo : do something with the ugly code of this file (testing things for
 *         now - but should not be like that after tests).
 */

#include "monitor.h"

static gchar *get_file_path_from_fd(gint fd);
static char *get_program_name_from_pid(int pid);
static void event_process(struct fanotify_event_metadata *event, GSList *dir_list, GAsyncQueue *queue);


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

    /** Leaving only FAN_CLOSE_WRITE for some tests */
    /* Setup fanotify notifications (FAN) mask. All these defined in linux/fanotify.h. */
    static uint64_t event_mask =
      (FAN_CLOSE_WRITE   |      /* Writtable file closed                                      */
       FAN_ONDIR         |      /* We want to be reported of events in the directory          */
       FAN_EVENT_ON_CHILD);     /* We want to be reported of events in files of the directory */

    unsigned int mark_flags = FAN_MARK_ADD | FAN_MARK_MOUNT;

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
                            if (fanotify_mark(fanotify_fd, mark_flags, event_mask, AT_FDCWD, head->data) < 0)
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
static gchar *get_file_path_from_fd(gint fd)
{
    ssize_t len = 0;
    off_t linklen = 0;
    gchar *path = NULL;
    gchar *proc = NULL;
    struct stat st;

    if (fd <= 0)
        {
            return g_strdup_printf("fd unknown");
        }

    proc = g_strdup_printf("/proc/self/fd/%d", fd);

    if (lstat(proc, &st) == -1)
        {
            print_error(__FILE__, __LINE__, _("lstat is wrong: %s\n"), strerror(errno));
            free_variable(proc);
            return  g_strdup_printf("lstat unknown");
        }
    else
        {
            linklen = st.st_size;
        }

    path = (gchar *) g_malloc0((linklen + 1) * sizeof(gchar));

    if ((len = readlink(proc, path, linklen)) < 0)
        {
            free_variable(proc);
            return g_strdup_printf("readlink unknown");
        }

    path[len] = '\0';

    free_variable(proc);
    return path;
}


static char *get_program_name_from_pid(int pid)
{
    int fd = 0;
    ssize_t len = 0;
    char *aux = NULL;
    gchar *cmd = NULL;
    gchar *buffer = NULL;

    /* Try to get program name by PID */
    cmd = g_strdup_printf("/proc/%d/cmdline", pid);

    if ((fd = open(cmd, O_RDONLY)) < 0)
        {
            return NULL;
        }

    buffer = (gchar *) g_malloc0(PATH_MAX);

    /* Read file contents into buffer */
    if ((len = read(fd, buffer, PATH_MAX - 1)) <= 0)
        {
            close (fd);
            return g_strdup("unknown");
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
 * @param dir_list MUST be a list of gchar * g_utf8_casefold()
 *        transformed.
 */
static void event_process(struct fanotify_event_metadata *event, GSList *dir_list, GAsyncQueue *queue)
{
    gchar *path = NULL;
    gchar *progname = NULL;
    GSList *head = dir_list;
    gboolean found = FALSE;
    gchar *pathutf8 = NULL;
    gchar *dirutf8 = NULL;

    path = get_file_path_from_fd(event->fd);

    pathutf8 = g_utf8_casefold(path, -1);

    while (head != NULL && found == FALSE)
        {
            dirutf8 = head->data;

            if (g_str_has_prefix(pathutf8, dirutf8) == TRUE)
                {
                    found = TRUE;
                }
            else
                {
                    head = g_slist_next(head);
                }
        }

    pathutf8 = free_variable(pathutf8);

    if (found == TRUE)
        {
            progname = get_program_name_from_pid(event->pid);
            print_debug(_("Received event file/directory: %s\n"), path);
            print_debug(_(" matching directory is       : %s\n"), head->data);
            print_debug(_(" pid=%d (%s): \n"), event->pid, progname);

            if (event->mask & FAN_CLOSE_WRITE)
                {
                    print_debug(_("\tFAN_CLOSE_WRITE\n"));
                }

            g_async_queue_push(queue, g_strdup(path));

            fflush (stdout);

            free_variable(progname);
        }

    close(event->fd);
    free_variable(path);
}


/**
 * Stops fanotify notifications
 * @param opt is the options of the program
 * @param fanotify_fd is the file descriptor of the file which is
 *        concerned by the event.
 */
void stop_fanotify(options_t *opt, int fanotify_fd)
{
    GSList *head = NULL;
    /* Setup fanotify notifications (FAN) mask. All these defined in linux/fanotify.h.    */
    static uint64_t event_mask =
      (FAN_CLOSE_WRITE   |  /* Writtable file closed                                      */
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
 * Transforms a list of directory into a list of directory ready for
 * comparison
 * @param dir_list is the list of directories to be transformed. This
 *        list is leaved untouched.
 * @returns a newly allocated list that may be freed when no longer
 *          needed.
 */
static GSList *transform_to_utf8_casefold(GSList *dir_list)
{
    GSList *head = NULL;
    GSList *utf8 = NULL;
    gchar *charutf8 = NULL;

    head = dir_list;

    while (head != NULL)
        {
            charutf8 = g_utf8_casefold(head->data, -1);

            /* Order of utf8 list is not very important */
            utf8 = g_slist_prepend(utf8, charutf8);

            head = g_slist_next(head);
        }

    return utf8;
}


/**
 * fanotify main loop
 * @todo simplify code (CCN is 12 already !)
 */
void fanotify_loop(main_struct_t *main_struct)
{
    struct pollfd fds[FD_POLL_MAX];
    struct signalfd_siginfo fdsi;
    char buffer[FANOTIFY_BUFFER_SIZE];
    ssize_t length = 0;
    struct fanotify_event_metadata *fe_mdata = NULL;
    GSList *dir_list_utf8 = NULL;


    gint signal_fd = 0;
    gint fanotify_fd = 0;

    if (main_struct != NULL)
        {
            signal_fd = main_struct->signal_fd;
            fanotify_fd = main_struct->fanotify_fd;


            /* Setup polling */
            fds[FD_POLL_SIGNAL].fd = signal_fd;
            fds[FD_POLL_SIGNAL].events = POLLIN;
            fds[FD_POLL_FANOTIFY].fd = fanotify_fd;
            fds[FD_POLL_FANOTIFY].events = POLLIN;


            dir_list_utf8 = transform_to_utf8_casefold(main_struct->opt->dirname_list);

            while (1)
                {
                    /* Block until there is something to be read */
                    if (poll(fds, FD_POLL_MAX, -1) < 0)
                        {
                            print_error(__FILE__, __LINE__, _("Couldn't poll(): '%s'\n"), strerror(errno));
                        }

                    /* Signal received ? */
                    if (fds[FD_POLL_SIGNAL].revents & POLLIN)
                        {
                            if (read(fds[FD_POLL_SIGNAL].fd, &fdsi, sizeof(fdsi)) != sizeof(fdsi))
                                {
                                  print_error(__FILE__, __LINE__, _("Couldn't read signal, wrong size read\n"));
                                }

                            /* Break loop if we got SIGINT or SIGTERM */
                            if (fdsi.ssi_signo == SIGINT || fdsi.ssi_signo == SIGTERM)
                                {
                                    stop_fanotify(main_struct->opt, main_struct->fanotify_fd);
                                    free_list(dir_list_utf8);
                                    break;
                                }

                            print_error(__FILE__, __LINE__, _("Received unexpected signal\n"));
                        }


                    /* fanotify event received ? */
                    if (fds[FD_POLL_FANOTIFY].revents & POLLIN)
                        {
                            /* Read from the FD. It will read all events available up to
                             * the given buffer size. */
                            if ((length = read (fds[FD_POLL_FANOTIFY].fd, buffer, FANOTIFY_BUFFER_SIZE)) > 0)
                                {
                                    fe_mdata = (struct fanotify_event_metadata *) buffer;

                                    while (FAN_EVENT_OK(fe_mdata, length))
                                        {
                                            event_process(fe_mdata, dir_list_utf8, main_struct->queue);

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

}




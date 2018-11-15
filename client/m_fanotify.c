/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    m_fanotify.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2015 - 2017 Olivier Delhomme
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

#include "client.h"

static gchar *get_file_path_from_fd(gint fd);
static char *get_program_name_from_pid(int pid);
static void prepare_before_saving(main_struct_t *main_struct, gchar *path);
static GSList *does_event_concerns_monitored_directory(gchar *path, GSList *dir_list);
static gboolean filter_out_if_necessary(GSList *head, struct fanotify_event_metadata *event);
static void event_process(main_struct_t *main_struct, struct fanotify_event_metadata *event, GSList *dir_list);


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
 * @param fd is the file descriptor as seen in /proc filesystem
 * @returns the name of the file pointed to by this file descriptor
 */
static gchar *get_file_path_from_fd(gint fd)
{
    gchar *path = NULL;
    gchar *proc = NULL;
    ssize_t len = 0;

    if (fd <= 0)
        {
            print_error(__FILE__, __LINE__, _("Invalid file descriptor: %d\n"), fd);
            return NULL;
        }

    proc = g_strdup_printf("/proc/self/fd/%d", fd);

    path = (gchar *) g_malloc((PATH_MAX) * sizeof(gchar));

    if ((len = readlink(proc, path, PATH_MAX - 1)) < 0)
        {
            print_error(__FILE__, __LINE__, _("'readlink' error: %s\n"), strerror(errno));
            free_variable(proc);
            free_variable(path);
            return NULL;
        }

    free_variable(proc);
    path[len] = '\0';

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

    buffer = (gchar *) g_malloc(PATH_MAX);

    /* Read file contents into buffer */
    if ((len = read(fd, buffer, PATH_MAX - 1)) <= 0)
        {
            close (fd);
            free_variable(buffer);
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
 * Prepares everything in order to call save_one_file function that does
 * everything to save one file !
 * @param main_struct : main structure of the program
 * @param path is the entire path and name of the considered file.
 */
static void prepare_before_saving(main_struct_t *main_struct, gchar *path)
{
    gchar *directory = NULL;
    GFileInfo *fileinfo = NULL;
    GFile *file = NULL;
    GError *error = NULL;
    file_event_t *file_event = NULL;

    if (main_struct != NULL && path != NULL)
        {
            directory = g_path_get_dirname(path);
            file = g_file_new_for_path(path);
            fileinfo = g_file_query_info(file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

            if (error == NULL && fileinfo != NULL)
                {
                    /* file_event is used and freed in the thread
                     * save_one_file_threaded where the queue save_queue
                     * is used
                     */
                    file_event = new_file_event_t(directory, fileinfo);
                    g_async_queue_push(main_struct->save_queue, file_event);

                    free_object(fileinfo);
                    free_object(file);
                    free_variable(directory);
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Unable to get meta data for file %s: %s\n"), path, error->message);
                    free_error(error);
                }
        }
}


/**
 * Returns the pointer to the concerned directory if found NULL otherwise
 * @param path path where the event occured
 * @param dir_list monitored directory list
 * @returns a pointer to the monitored directory where the event occured
 */
static GSList *does_event_concerns_monitored_directory(gchar *path, GSList *dir_list)
{
    GSList *head = dir_list;
    gchar *pathutf8 = NULL;   /* path where the received event occured */
    gchar *dirutf8 = NULL;
    gboolean found = FALSE;

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
            return head;
        }
    else
        {
            return NULL;
        }
}


/**
 * Filters out and returns TRUE if the event concerns a file that has to
 * be saved FALSE otherwise
 * @param head is the matching monitired directory
 * @param event is the fanotify's structure event
 */
static gboolean filter_out_if_necessary(GSList *head, struct fanotify_event_metadata *event)
{
    gchar *progname = NULL;


    if (head != NULL)
        {
            progname = get_program_name_from_pid(event->pid);

            if (g_strcmp0(PROGRAM_NAME, progname) != 0)
                {
                    /* Save files that does not come from our activity */
                    free_variable(progname);

                    return TRUE;
                }
        }

    return FALSE;
}


/**
 * Processes events
 * @param main_struct is the maion structure
 * @param event is the fanotify's structure event
 * @param dir_list MUST be a list of gchar * g_utf8_casefold()
 *        transformed.
 */
static void event_process(main_struct_t *main_struct, struct fanotify_event_metadata *event, GSList *dir_list)
{
    gchar *path = NULL;
    gboolean to_save = FALSE;
    GSList *head = NULL;

    path = get_file_path_from_fd(event->fd);

    if (path != NULL)
        {
            /* Does the event concern a monitored directory ? */
            head = does_event_concerns_monitored_directory(path, dir_list);

            /* Do we need to save this file ? Is it excluded somehow ? */
            to_save = filter_out_if_necessary(head, event);

            if (to_save == TRUE)
                {
                    print_debug(_("Received event file/directory: %s\n"), path);
                    print_debug(_(" matching directory is: %s\n"), head->data);

                    /* we are only watching this event so it is not necessary to print it !
                     * if (event->mask & FAN_CLOSE_WRITE)
                     *   {
                     *       print_debug(_("\tFAN_CLOSE_WRITE\n"));
                     *   }
                     */

                    /* Saving the file effectively */
                    prepare_before_saving(main_struct, path);

                    fflush(stdout);
                }

            close(event->fd);
            free_variable(path);
        }
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
    char buffer[FANOTIFY_BUFFER_SIZE];
    ssize_t length = 0;
    struct fanotify_event_metadata *fe_mdata = NULL;
    GSList *dir_list_utf8 = NULL;
    gint fanotify_fd = 0;


    if (main_struct != NULL)
        {
            fanotify_fd = main_struct->fanotify_fd;


            /* Setup polling */
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

                    /* fanotify event received ? */
                    if (fds[FD_POLL_FANOTIFY].revents & POLLIN)
                        {
                            /* Read from the FD. It will read all events available up to
                             * the given buffer size. */
                            if ((length = read(fds[FD_POLL_FANOTIFY].fd, buffer, FANOTIFY_BUFFER_SIZE)) > 0)
                                {
                                    fe_mdata = (struct fanotify_event_metadata *) buffer;

                                    while (FAN_EVENT_OK(fe_mdata, length))
                                        {
                                            event_process(main_struct, fe_mdata, dir_list_utf8);

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




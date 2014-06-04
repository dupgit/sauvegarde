/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    serveur.c
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
 * @file serveur.c
 * This file contains all the stuff for the serveur program of "Sauvegarde"
 * project. The aim of this program is to save every checksum and data and
 * meta data of every 'client' program that is connected to.
 */

#include "serveur.h"

static serveur_struct_t *init_serveur_main_structure(int argc, char **argv);


/**
 * Inits main serveur's structure
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns a serveur_struct_t * structure that contains everything that is
 *          needed for 'serveur' program.
 */
static serveur_struct_t *init_serveur_main_structure(int argc, char **argv)
{
    serveur_struct_t *serveur_struct = NULL;  /** main structure for 'serveur' program. */

    serveur_struct = (serveur_struct_t *) g_malloc0(sizeof(serveur_struct_t));

    serveur_struct->opt = do_what_is_needed_from_command_line_options(argc, argv);

    return serveur_struct;
}


/**
 * Main function
 * @param argc : number of arguments given on the command line.
 * @param argv : an array of strings that contains command line arguments.
 * @returns always 0
 */
int main(int argc, char **argv)
{
    serveur_struct_t *serveur_struct = NULL;  /** main structure for 'serveur' program. */
    gchar *somewhere = NULL;
    comm_t *comm = NULL;
    gchar *message = NULL;
    serveur_meta_data_t *smeta = NULL;


    g_type_init();

    init_international_languages();

    serveur_struct = init_serveur_main_structure(argc, argv);

    if (serveur_struct != NULL && serveur_struct->opt != NULL)
        {
            somewhere = g_strdup_printf("tcp://*:%d", serveur_struct->opt->port);
            comm = create_pull_socket(somewhere);

            while (1)
                {
                    message = receive_message(comm);

                    /** message variable is freed in convert_json_to_smeta_data() function : no need to free it again elsewhere ! */
                    smeta = convert_json_to_smeta_data(message);

                    smeta = free_smeta_data_t(smeta);
                }
        }

    return 0;
}

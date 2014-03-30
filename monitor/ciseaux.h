/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    ciseaux.h
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
 * @file ciseaux.h
 *
 *  This file contains all the definitions for the ciseaux program.
 */
#ifndef _CISEAUX_H_
#define _CISEAUX_H_


/**
 * @def CISEAUX_BLOCK_SIZE
 * default block size in bytes
 */
#define CISEAUX_BLOCK_SIZE (16384)


/**
 * This function creates one thread to print things and
 * one other thread to calculate the checksums. This function
 * is a thread itself.
 * It waits until the end of the calc_thread thread (this will change
 * as in the future thoses functions should have an end unless the program
 * itself ends.
 * @param data : main_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
extern gpointer ciseaux(gpointer data);

#endif /* #ifndef _CISEAUX_H_ */

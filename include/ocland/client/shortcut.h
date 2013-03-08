/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2012  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ocland.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef SHORTCUT_H_INCLUDED
#define SHORTCUT_H_INCLUDED

/** @struct shortcut_st
 * Ocland pointers shortcuts. Shortcuts allows to access
 * faster to the right server, avoiding the need to quest
 * all servers looking for the correct one. Each pointer
 * returned by each server will be attached with the socket.
 */
struct shortcut_st
{
    /// Pointer returned by server
    void *ocl_ptr;
    /// Socket implied
    int *socket;
};

/// shortcut_st structure abstraction
typedef struct shortcut_st shortcut;

/** Add a new shortcut.
 * @param ocl_ptr Ocland server pointer.
 * @param socket Server socket.
 * @return Number of shortcuts registered.
 */
unsigned int addShortcut(void* ocl_ptr, int* socket);

/** Removes an existing shortcut.
 * @param ocl_ptr Ocland server pointer.
 * @param socket Server socket.
 * @return Number of shortcuts registered.
 */
unsigned int delShortcut(void* ocl_ptr);

/** Get a socket associated with a ocland pointer.
 * @param ocl_ptr Ocland server pointer.
 * @return socket Server socket, NULL if not pointer found.
 */
int* getShortcut(void* ocl_ptr);

#endif // SHORTCUT_H_INCLUDED

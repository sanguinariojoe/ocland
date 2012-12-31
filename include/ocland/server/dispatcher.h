/*
 *  This file is part of ocland, a free CFD program based on SPH.
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

#include <ocland/server/validator.h>

#ifndef DISPATCHER_H_INCLUDED
#define DISPATCHER_H_INCLUDED

/** In ocland each client is assigned to an independent
 * thread. Using this approach, an error caused by a client
 * will shutdown only the involved client thread.
 * @param socket Pointer to the socket decriptor integer.
 */
void *client_thread(void *socket);

/** Read command received and process it. Some commands
 * requires several data exchanges.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int dispatch(int* clientfd, char* buffer, validator v);

#endif // DISPATCHER_H_INCLUDED

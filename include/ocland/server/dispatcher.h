/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2015  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with ocland.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ocland/server/validator.h>

#ifndef DISPATCHER_H_INCLUDED
#define DISPATCHER_H_INCLUDED

/** Read command received and process it. Some commands
 * requires several data exchanges.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int dispatch(int* clientfd, validator v);

#endif // DISPATCHER_H_INCLUDED

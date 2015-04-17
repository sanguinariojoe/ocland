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

/** @file
 * @brief ICD cl_event implementation
 *
 * cl_event is a typedef definition:
 *
 *     typedef struct _cl_event* cl_event
 *
 * In this file such data structure, and all the associated methods, are
 * declared.
 * @see event.c
 */

#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED


#include <ocl_icd.h>
#include <pthread.h>
#include <ocland/client/platform_id.h>
#include <ocland/client/context.h>
#include <ocland/client/command_queue.h>



#endif // EVENT_H_INCLUDED

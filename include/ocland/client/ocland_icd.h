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

#ifndef OCLAND_ICD_H_INCLUDED
#define OCLAND_ICD_H_INCLUDED

// #include <CL/opencl.h>
#include <ocl_icd.h>
#include <pthread.h>

#include <ocland/client/platform_id.h>
#include <ocland/client/device_id.h>
#include <ocland/client/context.h>
#include <ocland/client/command_queue.h>
#include <ocland/client/mem.h>
#include <ocland/client/sampler.h>
#include <ocland/client/program.h>
#include <ocland/client/kernel.h>

/** ICD event identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_event
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_event ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /** @brief Mutex to protect the reference count to be increased/decreased by
     * several threads at the same time
     */
    pthread_mutex_t rcount_mutex;
    /// Server which has generated it
    int *socket;
    /// Associated command queue
    cl_command_queue command_queue;
    /// Associated context
    cl_context context;
    /// The command associated to the event
    cl_command_type command_type;
};

#pragma GCC visibility push(hidden)

struct _cl_icd_dispatch master_dispatch;

#pragma GCC visibility pop

#endif // OCLAND_ICD_H_INCLUDED

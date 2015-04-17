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

/** @brief ICD event identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_event
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    ptr_wrapper_t ptr_on_peer;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /** @brief Mutex to protect the reference count to be increased/decreased by
     * several threads at the same time
     */
    pthread_mutex_t rcount_mutex;
    /// Server where this object is allocated
    oclandServer server;
    /// Associated command queue
    cl_command_queue command_queue;
    /// Associated context
    cl_context context;
    /// The command associated to the event
    cl_command_type command_type;
    /// The command execution status
    cl_int command_execution_status;
    /// Number of registered callback functions
    cl_uint n_pfn_notify;
    /// The event callback functions
    void (CL_CALLBACK ** pfn_notify)(cl_event, cl_int, void *);
    /// event_command_exec_status for each callback function
    cl_int *command_exec_callback_type;
    /// User data for each callback function
    void **user_data;
};


#endif // EVENT_H_INCLUDED

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

/** @file
 * @brief ICD cl_command_queue implementation
 *
 * cl_command_queue is a typedef definition:
 *
 *     typedef struct _cl_command_queue* cl_command_queue
 *
 * In this file such data structure, and all the associated methods, are
 * declared.
 * @see command_queue.c
 */

#ifndef COMMAND_QUEUE_H_INCLUDED
#define COMMAND_QUEUE_H_INCLUDED

#include <ocl_icd.h>
#include <ocland/client/context.h>

/** ICD command queue identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_command_queue
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_command_queue ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server where this device is allocated
    oclandServer server;
    /// Associated context
    cl_context context;
    /// Associated device
    cl_device_id device;
    /// Command queue properties
    cl_command_queue_properties properties;
};

/** @brief Check for command queue validity
 * @param command_queue Command queue to check
 * @return 1 if the command queue is a known one, 0 otherwise.
 */
int hasCommandQueue(cl_command_queue command_queue);

/** @brief Get a command_queue from the server instance pointer.
 * @param srv_command_queue Server command queue instance
 * @return ICD command queue instance, NULL if \a srv_command_queue cannot be
 * found.
 */
cl_command_queue commandQueueFromServer(cl_command_queue srv_command_queue);

/** clCreateCommandQueue ocland abstraction method.
 */
cl_command_queue createCommandQueue(cl_context                   context,
                                    cl_device_id                 device,
                                    cl_command_queue_properties  properties,
                                    cl_int *                     errcode_ret);

/** clRetainCommandQueue ocland abstraction method.
 */
cl_int retainCommandQueue(cl_command_queue command_queue);

/** clReleaseCommandQueue ocland abstraction method.
 */
cl_int releaseCommandQueue(cl_command_queue command_queue);

/** clGetCommandQueueInfo ocland abstraction method.
 */
cl_int getCommandQueueInfo(cl_command_queue      command_queue,
                           cl_command_queue_info param_name,
                           size_t                param_value_size,
                           void *                param_value,
                           size_t *              param_value_size_ret);

#endif // COMMAND_QUEUE_H_INCLUDED

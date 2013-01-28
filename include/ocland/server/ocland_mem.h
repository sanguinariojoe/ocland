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

#include <CL/cl.h>
#include <CL/cl_ext.h>

#include <ocland/server/ocland_event.h>

#ifndef OCLAND_MEM_H_INCLUDED
#define OCLAND_MEM_H_INCLUDED

/** clEnqueueReadBuffer asynchronous operation. Call this method
 * when blocking_read is CL_TRUE. See clEnqueueReadBuffer OpenCL
 * command documentation for further details on the parameters
 * and returned values.
 * @param clientfd Socket already open with the client.
 * @note Memory transfer will be done in a new thread, and in a
 * new socket.
 */
cl_int oclandEnqueueReadBuffer(int *                clientfd ,
                               cl_command_queue     command_queue ,
                               cl_mem               buffer ,
                               size_t               offset ,
                               size_t               cb ,
                               void *               ptr ,
                               cl_uint              num_events_in_wait_list ,
                               ocland_event *       event_wait_list ,
                               cl_bool              want_event ,
                               ocland_event         event);

/** clEnqueueWriteBuffer asynchronous operation. Call this method
 * when blocking_read is CL_TRUE. See clEnqueueReadBuffer OpenCL
 * command documentation for further details on the parameters
 * and returned values.
 * @param clientfd Socket already open with the client.
 * @note Memory transfer will be done in a new thread, and in a
 * new socket.
 */
cl_int oclandEnqueueWriteBuffer(int *               clientfd ,
                                cl_command_queue     command_queue ,
                                cl_mem               buffer ,
                                size_t               offset ,
                                size_t               cb ,
                                void *               ptr ,
                                cl_uint              num_events_in_wait_list ,
                                ocland_event *       event_wait_list ,
                                cl_bool              want_event ,
                                ocland_event         event);

#ifdef CL_API_SUFFIX__VERSION_1_1
/** clEnqueueReadBufferRect asynchronous operation. Call this method
 * when blocking_read is CL_TRUE. See clEnqueueReadBufferRect OpenCL
 * command documentation for further details on the parameters
 * and returned values.
 * @param clientfd Socket already open with the client.
 * @note Memory transfer will be done in a new thread, and in a
 * new socket.
 */
cl_int oclandEnqueueReadBufferRect(int *                clientfd ,
                                   cl_command_queue     command_queue ,
                                   cl_mem               mem ,
                                   const size_t *       buffer_origin ,
                                   const size_t *       region ,
                                   size_t               buffer_row_pitch ,
                                   size_t               buffer_slice_pitch ,
                                   size_t               host_row_pitch ,
                                   size_t               host_slice_pitch ,
                                   void *               ptr ,
                                   cl_uint              num_events_in_wait_list ,
                                   ocland_event *       event_wait_list ,
                                   cl_bool              want_event ,
                                   ocland_event         event);

/** clEnqueueWriteBufferRect asynchronous operation. Call this method
 * when blocking_write is CL_TRUE. See clEnqueueWriteBufferRect OpenCL
 * command documentation for further details on the parameters
 * and returned values.
 * @param clientfd Socket already open with the client.
 * @note Memory transfer will be done in a new thread, and in a
 * new socket.
 */
cl_int oclandEnqueueWriteBufferRect(int *                clientfd ,
                                    cl_command_queue     command_queue ,
                                    cl_mem               buffer ,
                                    const size_t *       buffer_origin ,
                                    const size_t *       region ,
                                    size_t               buffer_row_pitch ,
                                    size_t               buffer_slice_pitch ,
                                    size_t               host_row_pitch ,
                                    size_t               host_slice_pitch ,
                                    void *               ptr ,
                                    cl_uint              num_events_in_wait_list ,
                                    ocland_event *       event_wait_list ,
                                    cl_bool              want_event ,
                                    ocland_event         event);
#endif // CL_API_SUFFIX__VERSION_1_1

#endif // OCLAND_MEM_H_INCLUDED

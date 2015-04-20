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
    /** @brief Shared identifier between the server and the client.
     *
     * Since events data is usually coming from the server in a download stream,
     * and we are not interested into waiting for the server instance when we
     * are queuing a command, the events will be identified by the pointer in
     * the client.
     */
    ptr_wrapper_t ptr;
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

/** @brief Check for event validity
 * @param event Event to check
 * @return 1 if the event is a known one, 0 otherwise.
 */
int hasEvent(cl_event event);

/** @brief Create a new event.
 *
 * The new event generated have not a remote peer instance, and it is set as a
 * CL_COMMAND_USER type
 */
cl_event createEvent(cl_context     context ,
                     cl_int *       errcode_ret);

/** @brief clCreateUserEvent ocland abstraction method.
 *
 * This method is not generating an event in a similar way to commands enqueues,
 * i.e. it is calling to server in order to generate the event, but it is
 * waiting for the remote event instance.
 * Also this events are not requiring for a download stream (they are ever
 * locally controlled)
 * @see createEvent
 */
cl_event createUserEvent(cl_context     context ,
                         cl_int *       errcode_ret);

/** @brief clSetUserEventStatus ocland abstraction method.
 *
 * Actually this method is used to set the status of all the events generated in
 * ocland.
 */
cl_int setEventStatus(cl_event    event ,
                      cl_int      execution_status);

/** @brief clWaitForEvents ocland abstraction method.
 */
cl_int waitForEvents(cl_uint              num_events,
                     const cl_event *     event_list);

/** @brief clFlush ocland abstraction method.
 */
cl_int flush(cl_command_queue  command_queue);

/** @brief clFinish ocland abstraction method.
 */
cl_int finish(cl_command_queue  command_queue);

/** @brief clRetainEvent ocland abstraction method.
 */
cl_int retainEvent(cl_event  event);

/** @brief clReleaseEvent ocland abstraction method.
 */
cl_int releaseEvent(cl_event  event);

/** @brief clGetEventInfo ocland abstraction method.
 */
cl_int getEventInfo(cl_event          event ,
                    cl_event_info     param_name ,
                    size_t            param_value_size ,
                    void *            param_value ,
                    size_t *          param_value_size_ret);

/** @brief clGetEventProfilingInfo ocland abstraction method.
 */
cl_int getEventProfilingInfo(cl_event             event ,
                             cl_profiling_info    param_name ,
                             size_t               param_value_size ,
                             void *               param_value ,
                             size_t *             param_value_size_ret);

/** @brief clSetEventCallback ocland abstraction method.
 */
cl_int setEventCallback(cl_event     event ,
                        cl_int       command_exec_callback_type ,
                        void (CL_CALLBACK *  pfn_notify)(cl_event, cl_int, void *),
                        void *       user_data);

#endif // EVENT_H_INCLUDED

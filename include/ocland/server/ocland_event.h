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

#include <CL/cl.h>
#include <CL/cl_ext.h>

#ifndef OCLAND_EVENT_H_INCLUDED
#define OCLAND_EVENT_H_INCLUDED

/** @struct _ocland_event
 * In ocland events can be a little bit more complicated
 * because when memory is transfered asynchronously there
 * are not only the events associated to the OpenCL
 * memory transfer but also with the host-server memory
 * transfer along the network. ocland creates an event
 * layer over the OpenCL one that forces to wait it before
 * to wait the corresponding OpenCL event.
 */
struct _ocland_event
{
    /// Internally OpenCL managed event
    cl_event event;
    /** ocland status, if you want to wait
     * for this event, you may look for
     * this variable turns into CL_COMPLETE
     */
    cl_int status;
    /// OpenCL associated to this context.
    cl_context context;
    /** OpenCL command queue associated with
     * this event. Can be NULL if this event
     * exist in all the command queues present
     * into context.
     */
    cl_command_queue command_queue;
};

/** @typedef ocland_event
 * Pointer abstraction of _ocland_event structure.
 */
typedef struct _ocland_event* ocland_event;

/** clWaitForEvents extension in order to wait to network
 * traffic has been completed too.
 * @param num_events Number of events inside event_list.
 * @param event_list List of events to wait.
 * @return CL_SUCCESS if the function was executed
 * successfully. CL_INVALID_VALUE if num_events
 * is zero, CL_INVALID_CONTEXT if events specified
 * in event_list do not belong to the same context, and
 * CL_INVALID_EVENT if event objects specified in
 * event_list are not valid event objects.
 */
cl_int oclandWaitForEvents(cl_uint num_events, const ocland_event *event_list);

#endif // OCLAND_EVENT_H_INCLUDED

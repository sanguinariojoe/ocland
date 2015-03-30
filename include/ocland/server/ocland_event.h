/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2015  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
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
    /** Command type called.
     */
    cl_command_type command_type;
};

/** @typedef ocland_event
 * Pointer abstraction of _ocland_event structure.
 */
typedef struct _ocland_event* ocland_event;

/** @brief clWaitForEvents extension in order to wait to network traffic has
 * been completed too.
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

/** @brief clGetEventInfo extension in order to safely return event info.
 *
 * This funtion is required when the event is not registered by the OpenCL
 * implementation.
 *
 * @param event Specifies the event object being queried.
 * @param param_name A pointer to memory where the appropriate result being
 * queried is returned. If param_value is NULL, it is ignored.
 * @param param_value_size Specifies the size in bytes of memory pointed to by
 * param_value.
 * @param param_value Returns the actual size in bytes of data copied to
 * param_value. If param_value_size_ret is NULL, it is ignored.
 * @param param_value_size_ret Specifies the information to query.
 * @return CL_SUCCESS if the function executed successfully. CL_INVALID_VALUE if
 * param_name is not valid, or if size in bytes specified by param_value_size
 * is < size of return type as described in the table above and param_value is
 * not NULL. CL_INVALID_VALUE if information to query given in param_name cannot
 * be queried for event. CL_INVALID_EVENT if event is not a valid event object.
 * CL_OUT_OF_RESOURCES if there is a failure to allocate resources required by
 * the OpenCL implementation on the device.     CL_OUT_OF_HOST_MEMORY if there
 * is a failure to allocate resources required by the OpenCL implementation on
 * the host.
 */
cl_int oclandGetEventInfo(ocland_event      event ,
                          cl_event_info     param_name ,
                          size_t            param_value_size ,
                          void *            param_value ,
                          size_t *          param_value_size_ret);

#endif // OCLAND_EVENT_H_INCLUDED

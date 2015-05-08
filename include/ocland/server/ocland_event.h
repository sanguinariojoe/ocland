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
#include <ocland/common/dataExchange.h>

#ifndef OCLAND_EVENT_H_INCLUDED
#define OCLAND_EVENT_H_INCLUDED

typedef struct validator_st* validator;

/** @struct _ocland_event
 * @brief Intermediate layer to exchange data with the client.
 *
 * The events are not managed in the same way than the other OpenCL instances.
 * In this case, we are storing a client reference to the object.
 */
struct _ocland_event
{
    /// Internally OpenCL managed event
    cl_event event;
    /// Shared identifier with the remote peer
    ptr_wrapper_t ptr_on_peer;
    /// Context owner.
    cl_context context;
    /// Command queue owner.
    cl_command_queue command_queue;
    /// Command status
    cl_int status;
    /// Command type
    cl_command_type command_type;
    /// Validator regarding the client whose generated the object
    validator v;
};

/** @typedef ocland_event
 * Pointer abstraction of _ocland_event structure.
 */
typedef struct _ocland_event* ocland_event;

/** @brief Check for event validity
 * @param event Event to check
 * @return 1 if the event is a known one, 0 otherwise.
 */
int hasEvent(ocland_event event);

/** @brief Get an event from the client instance pointer.
 * @param peer_event Client event instance
 * @return ocland event instance, NULL if \a peer_event cannot be found.
 */
ocland_event eventFromClient(ptr_wrapper_t peer_event);

/** @brief clCreateUserEvent extension.
 * @param event_on_peer Instance of the event in the remote peer.
 */
ocland_event oclandCreateUserEvent(validator v,
                                   cl_context context,
                                   ptr_wrapper_t event_on_peer,
                                   cl_int *errcode_ret);

/** @brief clSetUserEventStatus extension.
 */
cl_int oclandSetUserEventStatus(ocland_event event,
                                cl_int execution_status);

/** @brief clReleaseEvent extension.
 */
cl_int oclandReleaseEvent(ocland_event event);

/** @brief clWaitForEvents extension in order to wait for events identified by
 * the ocland_event containers.
 * @param num_events Number of events inside event_list.
 * @param event_list List of events to wait.
 * @return CL_SUCCESS if the function was executed successfully.
 * CL_INVALID_VALUE if num_events is zero, CL_INVALID_CONTEXT if events
 * specified in event_list do not belong to the same context, and
 * CL_INVALID_EVENT if event objects specified in event_list are not valid event
 * objects.
 */
cl_int oclandWaitForEvents(cl_uint num_events, const ocland_event *event_list);

#endif // OCLAND_EVENT_H_INCLUDED

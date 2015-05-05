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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ocland/common/usleep.h>
#include <ocland/common/verbose.h>
#include <ocland/server/ocland_event.h>
#include <ocland/server/ocland_version.h>
#include <ocland/server/validator.h>

/** @brief Callback function to be registered by the events.
 * @param e Local OpenCL event instance.
 * @param event_command_exec_status New event status.
 * @param user_data Remote event instance, used as identifier.
 */
void (CL_CALLBACK event_notify)(cl_event e,
  	                            cl_int   event_command_exec_status,
                                void     *user_data)
{
    int socket_flag = 0;
    ocland_event event = (ocland_event)user_data;
    int* sockfd = event->v->callbacks_socket;
    ptr_wrapper_t identifier = event->ptr_on_peer;

    // Call the client
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_CONTEXT, identifier, MSG_MORE);
    socket_flag |= Send_size_t(sockfd, sizeof(cl_int), MSG_MORE);
    socket_flag |= Send(sockfd, (void*)(&event_command_exec_status), sizeof(cl_int), 0);
    if(socket_flag < 0){
        VERBOSE("Communication failure during event_notify.\n");
        // FIXME Communication fail, how to proceed??
        return;
    }
}

/// Number of known events
cl_uint num_global_events = 0;
/// List of known events
ocland_event *global_events = NULL;

int hasEvent(ocland_event event){
    cl_uint i;

    if(!num_global_events)
        return 0;

    for(i = 0; i < num_global_events; i++){
        if(event == global_events[i]){
            return 1;
        }
    }

    return 0;
}

ocland_event eventFromClient(ptr_wrapper_t peer_event)
{
    cl_uint i;

    if(!num_global_events)
        return NULL;

    for(i = 0; i < num_global_events; i++){
        if(equal_ptr_wrappers(global_events[i]->ptr_on_peer, peer_event)){
            return global_events[i];
        }
    }

    return NULL;
}

/** @brief Add a set of events to the global list global_events.
 *
 * This method is not checking if the objects are already present in the list,
 * however, accidentally adding the same object several times will only imply
 * performance penalties.
 * @param num_events Number of objects to append.
 * @param events Objects to append.
 * @return CL_SUCCESS if the objects are already generated, an error code
 * otherwise.
 */
cl_int addEvents(cl_uint    num_events,
                 ocland_event*  events)
{
    if(!num_events)
        return CL_SUCCESS;

    ocland_event *backup_events = global_events;

    global_events = (ocland_event*)malloc(
        (num_global_events + num_events) * sizeof(ocland_event));
    if(!global_events){
        VERBOSE("Failure allocating memory for %u events!\n",
                num_global_events + num_events);
        free(backup_events); backup_events = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_events){
        memcpy(global_events,
               backup_events,
               num_global_events * sizeof(ocland_event));
        free(backup_events); backup_events = NULL;
    }

    memcpy(&(global_events[num_global_events]),
           events,
           num_events * sizeof(ocland_event));
    num_global_events += num_events;

    return CL_SUCCESS;
}

/** @brief Get the event index in the global list
 * @param event Object to look for
 * @return Index of the object, num_global_events if it is not found.
 */
cl_uint eventIndex(ocland_event event)
{
    cl_uint i;
    for(i = 0; i < num_global_events; i++){
        if(event == global_events[i])
            break;
    }
    return i;
}

/** @brief Remove a event from the global list.
 *
 * For instance when clReleaseEvent() is called.
 * @param event Object to be removed.
 * @return CL_SUCCESS if the object has been already discarded or
 * CL_INVALID_VALUE if the object does not exist.
 */
cl_int discardEvent(ocland_event event)
{
    if(!hasEvent(event)){
        return CL_INVALID_VALUE;
    }
    cl_uint i, index;

    // Remove the event stuff
    free(event);

    // Remove the event from the global list
    index = eventIndex(event);
    for(i = index; i < num_global_events - 1; i++){
        global_events[i] = global_events[i + 1];
    }
    num_global_events--;
    global_events[num_global_events] = NULL;

    return CL_SUCCESS;
}

ocland_event oclandCreateUserEvent(validator v,
                                   cl_context context,
                                   ptr_wrapper_t event_on_peer,
                                   cl_int *errcode_ret)
{
    cl_int flag;
    ocland_event event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    event->status = CL_SUBMITTED;
    event->context = context;
    event->command_queue = NULL;
    event->command_type = CL_COMMAND_USER;
    event->v = v;
    event->event = clCreateUserEvent(context, &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    memcpy(event->ptr_on_peer.object_ptr,
           event_on_peer.object_ptr,
           sizeof(ptr_wrapper_t));
    event->ptr_on_peer.system_arch = event_on_peer.system_arch;
    event->ptr_on_peer.object_type = event_on_peer.object_type;

    addEvents(1, &event);

    return event;
}

cl_int oclandSetUserEventStatus(ocland_event event,
                                cl_int execution_status)
{
    struct _cl_version version = clGetContextVersion(event->context);
    if(     (!event->event)
        ||  (version.major <  1)
        || ((version.major == 1) && (version.minor < 1)))
    {
        // OpenCL < 1.1, so this function does not exist
        return CL_INVALID_EVENT;
    }
    return clSetUserEventStatus(event->event, execution_status);
}


cl_int oclandReleaseEvent(ocland_event event)
{
    return discardEvent(event);
}

cl_int oclandWaitForEvents(cl_uint num_events, const ocland_event *event_list)
{
    unsigned int i;
    cl_int flag = CL_SUCCESS;
    cl_event *cl_event_list = calloc(num_events, sizeof(cl_event));
    if (NULL == cl_event_list) {
        return CL_OUT_OF_HOST_MEMORY;
    }

    for(i = 0; i < num_events; i++){
        if(!hasEvent(event_list[i])){
            return CL_INVALID_EVENT;
        }
        cl_event_list[i] = event_list[i]->event;
    }

    flag = clWaitForEvents(num_events, cl_event_list);

    free(cl_event_list);
    cl_event_list=NULL;
    return flag;
}

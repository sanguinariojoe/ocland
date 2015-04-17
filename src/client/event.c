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
 * @see event.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/client/commands_enum.h>
#include <ocland/common/verbose.h>
#include <ocland/client/event.h>
#include <ocland/common/dataPack.h>
#include <ocland/common/dataExchange.h>

/// Number of known events
cl_uint num_global_events = 0;
/// List of known events
cl_event *global_events = NULL;

/** @brief Mutex to avoid accesing events becoming destroyed.
 *
 * In ocland every enqueued command is generating an event, even if the user
 * is not requiring it. Such events are automatically released when the command
 * is complete (succesfully or with errors).
 *
 * Of course, if the user is requesting the event, it will be retained, and
 * therefore the user is responsible of its destruction.
 */
pthread_mutex_t global_events_mutex;

int hasEvent(cl_event event){
    cl_uint i;

    if(!num_global_events)
        return 0;
    pthread_mutex_lock(&global_events_mutex);

    for(i = 0; i < num_global_events; i++){
        if(event == global_events[i])
            return 1;
    }

    pthread_mutex_unlock(&global_events_mutex);

    return 0;
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
                 cl_event*  events)
{
    if(!num_events)
        return CL_SUCCESS;

    // If there are not already existing events, the mutex is not initialized
    if(!num_global_events)
        pthread_mutex_init(&global_events_mutex, NULL);

    // Avoid some other thread remove some content from the list
    pthread_mutex_lock(&global_events_mutex);

    cl_event *backup_events = global_events;

    global_events = (cl_event*)malloc(
        (num_global_events + num_events) * sizeof(cl_event));
    if(!global_events){
        VERBOSE("Failure allocating memory for %u events!\n",
                num_global_events + num_events);
        free(backup_events); backup_events = NULL;
        pthread_mutex_unlock(&global_events_mutex);
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_events){
        memcpy(global_events,
               backup_events,
               num_global_events * sizeof(cl_event));
        free(backup_events); backup_events = NULL;
    }

    memcpy(&(global_events[num_global_events]),
           events,
           num_events * sizeof(cl_event));
    num_global_events += num_events;

    pthread_mutex_unlock(&global_events_mutex);

    return CL_SUCCESS;
}

/** @brief Get the event index in the global list
 * @param event Object to look for
 * @return Index of the object, num_global_events if it is not found.
 */
cl_uint eventIndex(cl_event event)
{
    cl_uint i;
    for(i = 0; i < num_global_events; i++){
        if(event == global_events[i])
            break;
    }
    return i;
}

cl_event eventFromServer(ptr_wrapper_t srv_event)
{
    cl_uint i;

    if(!num_global_events)
        return NULL;
    pthread_mutex_lock(&global_events_mutex);

    for(i = 0; i < num_global_events; i++){
        if(equal_ptr_wrappers(srv_event, global_events[i]->ptr_on_peer))
            return global_events[i];
    }

    pthread_mutex_unlock(&global_events_mutex);

    return NULL;
}

/** @brief Remove a event from the global list.
 *
 * For instance when clReleaseEvent() is called.
 * @param event Object to be removed.
 * @return CL_SUCCESS if the object has been already discarded or
 * CL_INVALID_VALUE if the object does not exist.
 */
cl_int discardEvent(cl_event event)
{
    if(!hasEvent(event)){
        return CL_INVALID_VALUE;
    }
    cl_uint i, index;

    // Avoid that some other thread try to access this info (which we are
    // destroying)
    pthread_mutex_lock(&global_events_mutex);

    // Remove the event stuff
    free(event);

    assert(num_global_events > 0);
    // Remove the event from the global list
    index = eventIndex(event);
    for(i = index; i < num_global_events - 1; i++){
        global_events[i] = global_events[i + 1];
    }
    num_global_events--;
    global_events[num_global_events] = NULL;

    pthread_mutex_unlock(&global_events_mutex);

    // If there are no more events then we can release the mutex
    if(!num_global_events)
        pthread_mutex_destroy(&global_events_mutex);

    return CL_SUCCESS;
}

cl_event createEvent(cl_context     context ,
                     cl_int *       errcode_ret)
{
    cl_int flag;
    cl_event event=NULL;
    event = (cl_event)malloc(sizeof(struct _cl_event));
    if(!event){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    event->dispatch = context->dispatch;
    set_null_ptr_wrapper(&event->ptr_on_peer);
    event->rcount = 1;
    pthread_mutex_init(&(event->rcount_mutex), NULL);
    event->server = context->server;
    event->command_queue = NULL;
    event->context = context;
    event->command_type = CL_COMMAND_USER;
    event->command_execution_status = CL_QUEUED;
    event->n_pfn_notify = 0;
    event->pfn_notify = NULL;
    event->command_exec_callback_type = NULL;
    event->user_data = NULL;

    flag = addEvents(1, &event);
    if(flag != CL_SUCCESS){
        free(event); event = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    return event;
}

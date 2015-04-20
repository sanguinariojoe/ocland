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
        if(event == global_events[i]){
            pthread_mutex_unlock(&global_events_mutex);
            return 1;
        }
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
    free(event->pfn_notify);
    free(event->command_exec_callback_type);
    free(event->user_data);
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
    {
        memcpy(event->ptr.object_ptr, &event, sizeof(cl_event));
        event->ptr.system_arch = Get_current_arch();
        event->ptr.object_type = PTR_TYPE_EVENT;
    }
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

cl_event createUserEvent(cl_context     context ,
                         cl_int *       errcode_ret)
{
    cl_int flag;
    cl_event event=NULL;

    event = createEvent(context, &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret=flag;
        return NULL;
    }

    int socket_flag = 0;
    unsigned int comm = ocland_clCreateUserEvent;
    int *sockfd = context->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret=CL_OUT_OF_RESOURCES;
        discardEvent(event);
        return NULL;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_CONTEXT,
                                        context->ptr_on_peer, MSG_MORE);
    // For the events we are not doing it like with the other OpenCL instance,
    // but we are working in a download stream based way, i.e. We are using as
    // shared identifier between the peers the pointer in the client.
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT,
                                        event->ptr, 0);
    // Also we are not waiting for an answer at all.
    if(socket_flag){
        if(errcode_ret) *errcode_ret=CL_OUT_OF_RESOURCES;
        discardEvent(event);
        return NULL;
    }
    setEventStatus(event, CL_SUBMITTED);

    if(errcode_ret) *errcode_ret=CL_SUCCESS;
    return event;
}

cl_int setEventStatus(cl_event    event ,
                      cl_int      execution_status)
{
    cl_uint i;
    // If some other method is calling this method is because it already knows
    // that is safe to set the value, so no need to lock the mutex (even if is
    // the user who is calling it)
    event->command_execution_status = execution_status;
    // We must call the associated callbacks
    for(i=0; i<event->n_pfn_notify;i++){
        if((event->command_exec_callback_type[i] == execution_status) ||
           ((event->command_exec_callback_type[i] == CL_COMPLETE) &&
            (execution_status < 0))){
            event->pfn_notify[i](event, execution_status, event->user_data[i]);
        }
    }
    return CL_SUCCESS;
}

cl_int setUserEventStatus(cl_event    event ,
                          cl_int      execution_status)
{
    int socket_flag = 0;
    unsigned int comm = ocland_clSetUserEventStatus;
    int *sockfd = event->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT,
                                        event->ptr, MSG_MORE);
    socket_flag |= Send(sockfd, &execution_status, sizeof(cl_int), 0);
    // We are not waiting for an answer at all.
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    setEventStatus(event, execution_status);

    return CL_SUCCESS;
}

cl_int waitForEvents(cl_uint              num_events,
                     const cl_event *     event_list)
{
    unsigned int i;

    // This method is called by the user with events that are not automatically
    // destroyed, so is his responsibility to make it thread safe
    for(i = 0; i < num_events; i++){
        while(event_list[i]->command_execution_status > CL_COMPLETE){
            usleep(10);
        }
    }
    return CL_SUCCESS;
}

cl_int flush(cl_command_queue  command_queue)
{
    unsigned int i;

    // We must be careful, maybe we are trying to wait for an event while it
    // is automatically destroyed. The best way to avoid problems is just
    // blocking the objects destruction for a while
    if(!num_global_events)
        return CL_SUCCESS;
    pthread_mutex_lock(&global_events_mutex);

    for(i = 0; i < num_global_events; i++){
        // Ignore the events from other command queues
        if(global_events[i]->command_queue != command_queue)
            continue;
        while(global_events[i]->command_execution_status > CL_SUBMITTED){
            usleep(10);
        }
    }

    pthread_mutex_unlock(&global_events_mutex);

    return CL_SUCCESS;
}

cl_int finish(cl_command_queue  command_queue)
{
    unsigned int i;

    // We must be careful, maybe we are trying to wait for an event while it
    // is automatically destroyed. The best way to avoid problems is just
    // blocking the objects destruction for a while
    if(!num_global_events)
        return CL_SUCCESS;
    pthread_mutex_lock(&global_events_mutex);

    for(i = 0; i < num_global_events; i++){
        // Ignore the events from other command queues
        if(global_events[i]->command_queue != command_queue)
            continue;
        while(global_events[i]->command_execution_status > CL_COMPLETE){
            usleep(10);
        }
    }

    pthread_mutex_unlock(&global_events_mutex);

    return CL_SUCCESS;
}

cl_int retainEvent(cl_event  event)
{
    pthread_mutex_lock(&(event->rcount_mutex));
    event->rcount++;
    pthread_mutex_unlock(&(event->rcount_mutex));
    return CL_SUCCESS;
}

cl_int releaseEvent(cl_event  event)
{
    pthread_mutex_lock(&(event->rcount_mutex));
    event->rcount--;
    pthread_mutex_unlock(&(event->rcount_mutex));
    if(event->rcount){
        return CL_SUCCESS;
    }

    cl_int flag = CL_OUT_OF_RESOURCES;
    // Call the server to clear the instance
    int socket_flag = 0;
    unsigned int comm = ocland_clReleaseEvent;
    int *sockfd = event->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event->ptr, 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }

    // Free the memory
    flag = discardEvent(event);

    return CL_SUCCESS;
}

cl_int getEventInfo(cl_event          event ,
                    cl_event_info     param_name ,
                    size_t            param_value_size ,
                    void *            param_value ,
                    size_t *          param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetEventInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = event->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event->ptr, MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_event_info), MSG_MORE);
    socket_flag |= Send(sockfd, &param_value_size, sizeof(size_t), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        socket_flag |= Recv(sockfd, param_value, size_ret, MSG_WAITALL);
        if(socket_flag){
            return CL_OUT_OF_RESOURCES;
        }
    }
    return CL_SUCCESS;
}

cl_int getEventProfilingInfo(cl_event             event ,
                                   cl_profiling_info    param_name ,
                                   size_t               param_value_size ,
                                   void *               param_value ,
                                   size_t *             param_value_size_ret)
{
    /// @todo Events profiling
    return CL_PROFILING_INFO_NOT_AVAILABLE;
}

cl_int setEventCallback(cl_event     event,
                        cl_int       command_exec_callback_type,
                        void (CL_CALLBACK *  pfn_notify)(cl_event, cl_int, void *),
                        void *       user_data)
{
    // Backup the old list of callbacks
    void (CL_CALLBACK ** pfn_notify_back)(cl_event, cl_int, void *);
    cl_int *event_command_exec_status_back;
    void **user_data_back;
    pfn_notify_back = event->pfn_notify;
    event_command_exec_status_back = event->command_exec_callback_type;
    user_data_back = event->user_data;

    // Allocate memory for the new ones
    event->pfn_notify =
        (void (CL_CALLBACK **)(cl_event, cl_int, void *))malloc(
            (event->n_pfn_notify + 1) * sizeof(
            void (CL_CALLBACK *)(cl_event, cl_int, void *)));
    if(!event->pfn_notify){
        return CL_OUT_OF_HOST_MEMORY;
    }
    event->command_exec_callback_type = (cl_int*)malloc(
        (event->n_pfn_notify + 1) * sizeof(cl_int));
    if(!event->command_exec_callback_type){
        return CL_OUT_OF_HOST_MEMORY;
    }
    event->user_data = (void**)malloc(
        (event->n_pfn_notify + 1) * sizeof(void*));
    if(!event->user_data){
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(event->n_pfn_notify){
        // Restore the backup
        memcpy(event->pfn_notify, pfn_notify_back,
               (event->n_pfn_notify + 1) * sizeof(
               void (CL_CALLBACK *)(cl_event, cl_int, void *)));
        memcpy(event->command_exec_callback_type, event_command_exec_status_back,
               (event->n_pfn_notify + 1) * sizeof(cl_int));
        memcpy(event->user_data, user_data_back,
               (event->n_pfn_notify + 1) * sizeof(void*));
        // Clear the old memory
        free(pfn_notify_back);
        free(event_command_exec_status_back);
        free(user_data_back);
    }

    // Add the new callback
    event->pfn_notify[event->n_pfn_notify] = pfn_notify;
    event->command_exec_callback_type[event->n_pfn_notify] = command_exec_callback_type;
    event->user_data[event->n_pfn_notify] = user_data;
    event->n_pfn_notify++;

    return CL_SUCCESS;
}

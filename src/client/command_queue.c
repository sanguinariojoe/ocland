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
 * @brief ICD cl_command_queue implementation
 * @see command_queue.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/client/commands_enum.h>
#include <ocland/common/verbose.h>
#include <ocland/client/command_queue.h>
#include <ocland/common/dataExchange.h>

/// Number of known contexts
cl_uint num_global_command_queues = 0;
/// List of known contexts
cl_command_queue *global_command_queues = NULL;

int hasCommandQueue(cl_command_queue command_queue){
    cl_uint i;
    for(i = 0; i < num_global_command_queues; i++){
        if(command_queue == global_command_queues[i])
            return 1;
    }
    return 0;
}

/** @brief Add a set of command queues to the global list global_command_queues.
 *
 * The command queues will be added to the platform list as well.
 *
 * This method is not checking if the command queues are already present in the
 * list, however, accidentally adding the same command queue several times will
 * only imply performance penalties.
 * @param num_command_queues Number of command queues to append.
 * @param command_queues Contexts to append.
 * @return CL_SUCCESS if the context are already generated, an error code
 * otherwise.
 */
cl_int addCommandQueues(cl_uint            num_command_queues,
                        cl_command_queue*  command_queues)
{
    if(!num_command_queues)
        return CL_SUCCESS;

    // Add the command_queues to the global list
    cl_command_queue *backup_command_queues = global_command_queues;

    global_command_queues = (cl_command_queue*)malloc(
        (num_global_command_queues + num_command_queues) * sizeof(cl_command_queue));
    if(!global_command_queues){
        VERBOSE("Failure allocating memory for %u command_queues!\n",
                num_global_command_queues + num_command_queues);
        free(backup_command_queues); backup_command_queues = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_command_queues){
        memcpy(global_command_queues,
               backup_command_queues,
               num_global_command_queues * sizeof(cl_command_queue));
        free(backup_command_queues); backup_command_queues = NULL;
    }

    memcpy(&(global_command_queues[num_global_command_queues]),
           command_queues,
           num_command_queues * sizeof(cl_command_queue));
    num_global_command_queues += num_command_queues;

    return CL_SUCCESS;
}

/** @brief Get the command queue index in the global list
 * @param command_queue Command queue to look for
 * @return Index of the context, num_global_command_queues if it is not found.
 */
cl_uint commandQueueIndex(cl_command_queue command_queue)
{
    cl_uint i;
    for(i = 0; i < num_global_command_queues; i++){
        if(command_queue == global_command_queues[i])
            break;
    }
    return i;
}

cl_command_queue commandQueueFromServer(cl_command_queue srv_command_queue)
{
    cl_uint i;
    for(i = 0; i < num_global_command_queues; i++){
        if(srv_command_queue == global_command_queues[i]->ptr)
            return global_command_queues[i];
    }
    return NULL;
}

/** @brief Remove a command queue from the global list.
 *
 * For instance when clReleaseCommandQueue() is called.
 * @param command_queue Command queue to be removed.
 * @return CL_SUCCESS if the command queue has been already discarded or
 * CL_INVALID_VALUE if the command queue does not exist.
 */
cl_int discardCommandQueue(cl_command_queue command_queue)
{
    if(!hasCommandQueue(command_queue)){
        return CL_INVALID_VALUE;
    }
    cl_uint i, index;

    // Remove the command_queue stuff
    index = commandQueueIndex(command_queue);
    free(global_command_queues[index]);

    assert(num_global_command_queues > 0);
    // Remove the command_queue from the global list
    for(i = index; i < num_global_command_queues - 1; i++){
        global_command_queues[i] = global_command_queues[i + 1];
    }
    num_global_command_queues--;
    global_command_queues[num_global_command_queues] = NULL;


    return CL_SUCCESS;
}

cl_command_queue createCommandQueue(cl_context                     context,
                                    cl_device_id                   device,
                                    cl_command_queue_properties    properties,
                                    cl_int *                       errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateCommandQueue;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = context->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }

    // Try to build up the object (we'll probably need to pass it as identifier
    // to the server in a future release)
    cl_command_queue command_queue=NULL, command_queue_srv=NULL;
    command_queue = (cl_command_queue)malloc(sizeof(struct _cl_command_queue));
    if(!command_queue){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    command_queue->dispatch = context->dispatch;
    command_queue->rcount = 1;
    command_queue->server = context->server;
    command_queue->context = context;
    command_queue->device = device;
    command_queue->properties = properties;

    // Call the server to generate the remote object
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    socket_flag |= Send(sockfd, &(device->ptr), sizeof(cl_device_id), MSG_MORE);
    socket_flag |= Send(sockfd, &properties, sizeof(cl_command_queue_properties), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(command_queue); command_queue = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        free(command_queue); command_queue = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &command_queue_srv, sizeof(cl_command_queue), MSG_WAITALL);
    if(socket_flag){
        free(command_queue); command_queue = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    command_queue->ptr = command_queue_srv;
    // Add it to the global list
    flag = addCommandQueues(1, &command_queue);
    if(flag != CL_SUCCESS){
        free(command_queue); command_queue = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return command_queue;
}

cl_int retainCommandQueue(cl_command_queue command_queue)
{
    command_queue->rcount++;
    return CL_SUCCESS;
}

cl_int releaseCommandQueue(cl_command_queue command_queue)
{
    command_queue->rcount--;
    if(command_queue->rcount){
        return CL_SUCCESS;
    }

    // Call the server to clear the instance
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clReleaseCommandQueue;
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }

    // Free the memory
    flag = discardCommandQueue(command_queue);

    return flag;
}

cl_int getCommandQueueInfo(cl_command_queue      command_queue,
                           cl_command_queue_info param_name,
                           size_t                param_value_size,
                           void *                param_value,
                           size_t *              param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetCommandQueueInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_command_queue_info), MSG_MORE);
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


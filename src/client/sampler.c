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

/** @file
 * @brief ICD cl_sampler implementation
 * @see sampler.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/client/commands_enum.h>
#include <ocland/common/verbose.h>
#include <ocland/client/sampler.h>
#include <ocland/common/dataPack.h>
#include <ocland/common/dataExchange.h>

/// Number of known samplers
cl_uint num_global_samplers = 0;
/// List of known samplers
cl_sampler *global_samplers = NULL;

int hasSampler(cl_sampler sampler){
    cl_uint i;
    for(i = 0; i < num_global_samplers; i++){
        if(sampler == global_samplers[i])
            return 1;
    }
    return 0;
}

/** @brief Add a set of samplers to the global list global_samplers.
 *
 * This method is not checking if the objects are already present in the list,
 * however, accidentally adding the same object several times will only imply
 * performance penalties.
 * @param num_samplers Number of objects to append.
 * @param samplers Objects to append.
 * @return CL_SUCCESS if the objects are already generated, an error code
 * otherwise.
 */
cl_int addSamplers(cl_uint      num_samplers,
                   cl_sampler*  samplers)
{
    if(!num_samplers)
        return CL_SUCCESS;

    // Add the samplers to the global list
    cl_sampler *backup_samplers = global_samplers;

    global_samplers = (cl_sampler*)malloc(
        (num_global_samplers + num_samplers) * sizeof(cl_sampler));
    if(!global_samplers){
        VERBOSE("Failure allocating memory for %u samplers!\n",
                num_global_samplers + num_samplers);
        free(backup_samplers); backup_samplers = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_samplers){
        memcpy(global_samplers,
               backup_samplers,
               num_global_samplers * sizeof(cl_sampler));
        free(backup_samplers); backup_samplers = NULL;
    }

    memcpy(&(global_samplers[num_global_samplers]),
           samplers,
           num_samplers * sizeof(cl_sampler));
    num_global_samplers += num_samplers;

    return CL_SUCCESS;
}

/** @brief Get the sampler index in the global list
 * @param sampler Object to look for
 * @return Index of the object, num_global_samplers if it is not found.
 */
cl_uint samplerIndex(cl_sampler sampler)
{
    cl_uint i;
    for(i = 0; i < num_global_samplers; i++){
        if(sampler == global_samplers[i])
            break;
    }
    return i;
}

cl_sampler samplerFromServer(cl_sampler srv_sampler)
{
    cl_uint i;
    for(i = 0; i < num_global_samplers; i++){
        if(srv_sampler == global_samplers[i]->ptr)
            return global_samplers[i];
    }
    return NULL;
}

/** @brief Remove a sampler from the global list.
 *
 * For instance when clReleaseSampler() is called.
 * @param sampler Object to be removed.
 * @return CL_SUCCESS if the object has been already discarded or
 * CL_INVALID_VALUE if the object does not exist.
 */
cl_int discardSampler(cl_sampler sampler)
{
    if(!hasSampler(sampler)){
        return CL_INVALID_VALUE;
    }
    cl_uint i, index;

    // Remove the sampler stuff
    free(sampler);

    assert(num_global_samplers > 0);
    // Remove the sampler from the global list
    index = samplerIndex(sampler);
    for(i = index; i < num_global_samplers - 1; i++){
        global_samplers[i] = global_samplers[i + 1];
    }
    num_global_samplers--;
    global_samplers[num_global_samplers] = NULL;

    return CL_SUCCESS;
}

cl_sampler createSampler(cl_context           context ,
                         cl_bool              normalized_coords ,
                         cl_addressing_mode   addressing_mode ,
                         cl_filter_mode       filter_mode ,
                         cl_int *             errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateSampler;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = context->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    // Try to build up a new instance (we'll need to pass it as identifier
    // to the server)
    cl_sampler sampler=NULL, sampler_srv=NULL;
    sampler = (cl_sampler)malloc(sizeof(struct _cl_sampler));
    if(!sampler){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    sampler->dispatch = context->dispatch;
    sampler->ptr = NULL;
    sampler->rcount = 1;
    sampler->server = context->server;
    sampler->context = context;
    sampler->normalized_coords = normalized_coords;
    sampler->addressing_mode = addressing_mode;
    sampler->filter_mode = filter_mode;

    // Call the server to generate the object
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr_on_peer), sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &normalized_coords, sizeof(cl_bool), MSG_MORE);
    socket_flag |= Send(sockfd, &addressing_mode, sizeof(cl_addressing_mode), MSG_MORE);
    socket_flag |= Send(sockfd, &filter_mode, sizeof(cl_filter_mode), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(sampler); sampler = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        free(sampler); sampler = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &sampler_srv, sizeof(cl_sampler), MSG_WAITALL);
    if(socket_flag){
        free(sampler); sampler = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    sampler->ptr = sampler_srv;

    // Add the object to the global list
    flag = addSamplers(1, &sampler);
    if(flag != CL_SUCCESS){
        free(sampler); sampler = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return sampler;
}

cl_int retainSampler(cl_sampler sampler)
{
    sampler->rcount++;
    return CL_SUCCESS;
}

cl_int releaseSampler(cl_sampler sampler)
{
    sampler->rcount--;
    if(sampler->rcount){
        return CL_SUCCESS;
    }

    // Call the server to clear the instance
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clReleaseSampler;
    int *sockfd = sampler->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(sampler->ptr), sizeof(cl_sampler), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }

    // Free the memory
    flag = discardSampler(sampler);

    return CL_SUCCESS;
}

cl_int getSamplerInfo(cl_sampler          sampler ,
                      cl_sampler_info     param_name ,
                      size_t              param_value_size ,
                      void *              param_value ,
                      size_t *            param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetSamplerInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = sampler->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(sampler->ptr), sizeof(cl_sampler), MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_sampler_info), MSG_MORE);
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


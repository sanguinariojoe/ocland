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
 * @brief ICD cl_mem implementation
 * @see mem.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/client/commands_enum.h>
#include <ocland/common/verbose.h>
#include <ocland/client/mem.h>
#include <ocland/common/dataPack.h>
#include <ocland/common/dataExchange.h>

/// Number of known memory objects
cl_uint num_global_mems = 0;
/// List of known memory objects
cl_mem *global_mems = NULL;

int hasMem(cl_mem mem){
    cl_uint i;
    for(i = 0; i < num_global_mems; i++){
        if(mem == global_mems[i])
            return 1;
    }
    return 0;
}

/** @brief Add a set of memory objects to the global list global_mems.
 *
 * This method is not checking if the objects are already present in the list,
 * however, accidentally adding the same object several times will only imply
 * performance penalties.
 * @param num_mems Number of objects to append.
 * @param mems Objects to append.
 * @return CL_SUCCESS if the objects are already generated, an error code
 * otherwise.
 */
cl_int addMems(cl_uint  num_mems,
               cl_mem*  mems)
{
    if(!num_mems)
        return CL_SUCCESS;

    // Add the mems to the global list
    cl_mem *backup_mems = global_mems;

    global_mems = (cl_mem*)malloc(
        (num_global_mems + num_mems) * sizeof(cl_mem));
    if(!global_mems){
        VERBOSE("Failure allocating memory for %u mems!\n",
                num_global_mems + num_mems);
        free(backup_mems); backup_mems = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_mems){
        memcpy(global_mems,
               backup_mems,
               num_global_mems * sizeof(cl_mem));
        free(backup_mems); backup_mems = NULL;
    }

    memcpy(&(global_mems[num_global_mems]),
           mems,
           num_mems * sizeof(cl_mem));
    num_global_mems += num_mems;

    return CL_SUCCESS;
}

/** @brief Get the memory object index in the global list
 * @param mem Object to look for
 * @return Index of the object, num_global_mems if it is not found.
 */
cl_uint memIndex(cl_mem mem)
{
    cl_uint i;
    for(i = 0; i < num_global_mems; i++){
        if(mem == global_mems[i])
            break;
    }
    return i;
}

cl_mem memFromServer(cl_mem srv_mem)
{
    cl_uint i;
    for(i = 0; i < num_global_mems; i++){
        if(srv_mem == global_mems[i]->ptr)
            return global_mems[i];
    }
    return NULL;
}

/** @brief Remove a memory object from the global list.
 *
 * For instance when clReleaseMemObject() is called.
 * @param mem Object to be removed.
 * @return CL_SUCCESS if the object has been already discarded or
 * CL_INVALID_VALUE if the object does not exist.
 */
cl_int discardMem(cl_mem mem)
{
    if(!hasMem(mem)){
        return CL_INVALID_VALUE;
    }
    cl_uint i, index;

    // Remove the mem stuff
    if(mem->maps){
        for(i = 0; i < mem->map_count; i++){
            free(mem->maps[i]);
        }
        free(mem->maps);
    }
    if(mem->image_format) free(mem->image_format);
    if(mem->image_desc) free(mem->image_desc);
    if(mem->pfn_notify) free(mem->pfn_notify);
    if(mem->user_data) free(mem->user_data);
    free(mem);

    assert(num_global_mems > 0);
    // Remove the mem from the global list
    index = memIndex(mem);
    for(i = index; i < num_global_mems - 1; i++){
        global_mems[i] = global_mems[i + 1];
    }
    num_global_mems--;
    global_mems[num_global_mems] = NULL;

    return CL_SUCCESS;
}

cl_mem createBuffer(cl_context    context ,
                    cl_mem_flags  flags ,
                    size_t        size ,
                    void *        host_ptr ,
                    cl_int *      errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateBuffer;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = context->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    // Try to build up a new instance (we'll need to pass it as identifier
    // to the server)
    cl_mem mem=NULL, mem_srv=NULL;
    mem = (cl_mem)malloc(sizeof(struct _cl_mem));
    if(!mem){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    mem->dispatch = context->dispatch;
    mem->ptr = NULL;
    mem->rcount = 1;
    mem->server = context->server;
    mem->context = context;
    mem->type = CL_MEM_OBJECT_BUFFER;
    // Buffer data
    mem->flags = flags;
    mem->size = size;
    mem->host_ptr = NULL;
    if(flags & CL_MEM_ALLOC_HOST_PTR){
        mem->host_ptr = malloc(size);
        if(!mem->host_ptr){
            free(mem);
            if(errcode_ret) *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
            VERBOSE_OUT(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            return NULL;
        }
    }
    else if((flags & CL_MEM_USE_HOST_PTR) ||
            (flags & CL_MEM_COPY_HOST_PTR)){
        mem->host_ptr = host_ptr;
    }
    mem->map_count = 0;
    mem->maps = NULL;
    // Image data
    mem->image_format = NULL;
    mem->image_desc = NULL;
    mem->element_size = 0;
    // SubBuffer data
    mem->mem_associated = NULL;
    mem->offset = 0;
    // Destruction callback functions
    mem->num_pfn_notify = 0;
    mem->pfn_notify = NULL;
    mem->user_data = NULL;

    // Call the server to generate the object
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr)
        hasPtr = CL_TRUE;
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    socket_flag |= Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    socket_flag |= Send(sockfd, &size, sizeof(size_t), MSG_MORE);
    if(flags & CL_MEM_COPY_HOST_PTR){
        // Send the data compressed
        dataPack in, out;
        in.size = size;
        in.data = host_ptr;
        out = pack(in);
        socket_flag |= Send(sockfd, &hasPtr, sizeof(cl_bool), MSG_MORE);
        socket_flag |= Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        socket_flag |= Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    else{
        socket_flag |= Send(sockfd, &hasPtr, sizeof(cl_bool), 0);
    }
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        if(flags & CL_MEM_ALLOC_HOST_PTR){
            free(mem->host_ptr);
        }
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        if(flags & CL_MEM_ALLOC_HOST_PTR){
            free(mem->host_ptr);
        }
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &mem_srv, sizeof(cl_mem), MSG_WAITALL);
    if(socket_flag){
        if(flags & CL_MEM_ALLOC_HOST_PTR){
            free(mem->host_ptr);
        }
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    mem->ptr = mem_srv;

    // Add the object to the global list
    flag = addMems(1, &mem);
    if(flag != CL_SUCCESS){
        if(flags & CL_MEM_ALLOC_HOST_PTR){
            free(mem->host_ptr);
        }
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return mem;
}

cl_mem createSubBuffer(cl_mem                    buffer ,
                       cl_mem_flags              flags ,
                       cl_buffer_create_type     buffer_create_type ,
                       const void *              buffer_create_info ,
                       cl_int *                  errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateSubBuffer;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    size_t buffer_create_info_size = 0;
    if(buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION)
        buffer_create_info_size = sizeof(cl_buffer_region);
    else{
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return NULL;
    }
    int *sockfd = buffer->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    // Try to build up a new instance
    cl_mem mem=NULL, mem_srv=NULL;
    mem = (cl_mem)malloc(sizeof(struct _cl_mem));
    if(!mem){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    mem->dispatch = buffer->dispatch;
    mem->rcount = 1;
    mem->server = buffer->server;
    mem->context = buffer->context;
    mem->type = CL_MEM_OBJECT_BUFFER;
    // Buffer data
    if(!(flags & CL_MEM_READ_WRITE) &&
       !(flags & CL_MEM_READ_ONLY) &&
       !(flags & CL_MEM_WRITE_ONLY)){
        if(buffer->flags & CL_MEM_READ_WRITE)
            flags |= CL_MEM_READ_WRITE;
        else if(buffer->flags & CL_MEM_READ_ONLY)
            flags |= CL_MEM_READ_ONLY;
        else if(buffer->flags & CL_MEM_WRITE_ONLY)
            flags |= CL_MEM_WRITE_ONLY;
    }
    if(buffer->flags & CL_MEM_USE_HOST_PTR)
        flags |= CL_MEM_USE_HOST_PTR;
    if(buffer->flags & CL_MEM_ALLOC_HOST_PTR)
        flags |= CL_MEM_ALLOC_HOST_PTR;
    if(buffer->flags & CL_MEM_COPY_HOST_PTR)
        flags |= CL_MEM_COPY_HOST_PTR;
    if(!(flags & CL_MEM_HOST_WRITE_ONLY) &&
       !(flags & CL_MEM_HOST_READ_ONLY) &&
       !(flags & CL_MEM_WRITE_ONLY)){
        if(buffer->flags & CL_MEM_HOST_WRITE_ONLY)
            flags |= CL_MEM_HOST_WRITE_ONLY;
        else if(buffer->flags & CL_MEM_HOST_READ_ONLY)
            flags |= CL_MEM_HOST_READ_ONLY;
        else if(buffer->flags & CL_MEM_HOST_NO_ACCESS)
            flags |= CL_MEM_HOST_NO_ACCESS;
    }
    mem->flags = flags;
    mem->size = ((cl_buffer_region*)buffer_create_info)->size;
    mem->host_ptr = NULL;
    mem->map_count = 0;
    mem->maps = NULL;
    // Image data
    mem->image_format = NULL;
    mem->image_desc = NULL;
    mem->element_size = 0;
    // SubBuffer data
    mem->mem_associated = buffer;
    mem->offset = ((cl_buffer_region*)buffer_create_info)->origin;
    if((flags & CL_MEM_ALLOC_HOST_PTR) ||
       (flags & CL_MEM_USE_HOST_PTR) ||
       (flags & CL_MEM_COPY_HOST_PTR)){
        mem->host_ptr = (char*)(buffer->host_ptr) + mem->offset;
    }
    // Destruction callback functions
    mem->num_pfn_notify = 0;
    mem->pfn_notify = NULL;
    mem->user_data = NULL;

    // Call the server to generate the object
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(buffer->ptr), sizeof(cl_mem), MSG_MORE);
    socket_flag |= Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    socket_flag |= Send(sockfd, &buffer_create_type, sizeof(cl_buffer_create_type), MSG_MORE);
    socket_flag |= Send(sockfd, &buffer_create_info_size, sizeof(size_t), MSG_MORE);
    socket_flag |= Send(sockfd, buffer_create_info, buffer_create_info_size, 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &mem_srv, sizeof(cl_mem), MSG_WAITALL);
    if(socket_flag){
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    mem->ptr = mem_srv;

    // Add the object to the global list
    flag = addMems(1, &mem);
    if(flag != CL_SUCCESS){
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return mem;
}

cl_mem createImage(cl_context              context,
                   cl_mem_flags            flags,
                   const cl_image_format * image_format,
                   const cl_image_desc *   image_desc,
                   size_t                  element_size,
                   void *                  host_ptr,
                   cl_int *                errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateImage;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = context->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    // Compute the total image size
    size_t image_size = image_desc->image_depth *
                        image_desc->image_height *
                        image_desc->image_width *
                        element_size;

    // Try to build up a new instance
    cl_mem mem=NULL, mem_srv=NULL;
    mem = (cl_mem)malloc(sizeof(struct _cl_mem));
    if(!mem){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    mem->dispatch = context->dispatch;
    mem->rcount = 1;
    mem->server = context->server;
    mem->context = context;
    mem->type = image_desc->image_type;
    // Buffer data
    mem->flags = flags;
    mem->size = image_size;
    mem->host_ptr = NULL;
    if(flags & CL_MEM_ALLOC_HOST_PTR){
        mem->host_ptr = malloc(image_size);
        if(!mem->host_ptr){
            free(mem);
            if(errcode_ret) *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
            VERBOSE_OUT(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            return NULL;
        }
    }
    else if((flags & CL_MEM_USE_HOST_PTR) ||
            (flags & CL_MEM_COPY_HOST_PTR)){
        mem->host_ptr = host_ptr;
    }
    mem->map_count = 0;
    mem->maps = NULL;
    // Image data
    mem->image_format = (cl_image_format*)malloc(sizeof(cl_image_format));
    mem->image_desc = (cl_image_desc*)malloc(sizeof(cl_image_desc));
    if((!mem->image_format) || (!mem->image_desc)){
        free(mem->image_format);
        free(mem->image_desc);
        free(mem);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    memcpy(mem->image_format, image_format, sizeof(cl_image_format));
    memcpy(mem->image_desc, image_desc, sizeof(cl_image_desc));
    mem->element_size = element_size;
    // SubBuffer data
    mem->mem_associated = NULL;
    mem->offset = 0;
    // Destruction callback functions
    mem->num_pfn_notify = 0;
    mem->pfn_notify = NULL;
    mem->user_data = NULL;

    // Move from host references to the remote ones
    cl_image_desc descriptor;
    memcpy(&descriptor, image_desc, sizeof(cl_image_desc));
    if(descriptor.buffer)
        descriptor.buffer = descriptor.buffer->ptr;

    // Call the server to generate the object
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr)
        hasPtr = CL_TRUE;
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    socket_flag |= Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    socket_flag |= Send(sockfd, &image_size, sizeof(size_t), MSG_MORE);
    socket_flag |= Send(sockfd, image_format, sizeof(cl_image_format), MSG_MORE);
    socket_flag |= Send(sockfd, image_desc, sizeof(cl_image_desc), MSG_MORE);
    socket_flag |= Send(sockfd, &element_size, sizeof(size_t), MSG_MORE);
    if(flags & CL_MEM_COPY_HOST_PTR){
        // Send the data compressed
        dataPack in, out;
        in.size = image_size;
        in.data = host_ptr;
        out = pack(in);
        socket_flag |= Send(sockfd, &hasPtr, sizeof(cl_bool), MSG_MORE);
        socket_flag |= Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        socket_flag |= Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    else{
        socket_flag |= Send(sockfd, &hasPtr, sizeof(cl_bool), 0);
    }
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        if(flags & CL_MEM_ALLOC_HOST_PTR){
            free(mem->host_ptr);
        }
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        if(flags & CL_MEM_ALLOC_HOST_PTR){
            free(mem->host_ptr);
        }
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &mem_srv, sizeof(cl_mem), MSG_WAITALL);
    if(socket_flag){
        if(flags & CL_MEM_ALLOC_HOST_PTR){
            free(mem->host_ptr);
        }
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    mem->ptr = mem_srv;

    // Add the object to the global list
    flag = addMems(1, &mem);
    if(flag != CL_SUCCESS){
        if(flags & CL_MEM_ALLOC_HOST_PTR){
            free(mem->host_ptr);
        }
        free(mem); mem = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return mem;
}

cl_int retainMemObject(cl_mem mem)
{
    mem->rcount++;
    return CL_SUCCESS;
}

cl_int releaseMemObject(cl_mem mem)
{
    mem->rcount--;
    if(mem->rcount){
        return CL_SUCCESS;
    }

    // Call the callback functions
    cl_uint i;
    for(i = 0; i < mem->num_pfn_notify; i++){
        mem->pfn_notify[i](mem, mem->user_data);
    }

    // Call the server to clear the instance
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clReleaseMemObject;
    int *sockfd = mem->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(mem->ptr), sizeof(cl_mem), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }

    // Free the memory
    flag = discardMem(mem);

    return CL_SUCCESS;
}

cl_int getSupportedImageFormats(cl_context           context,
                                cl_mem_flags         flags,
                                cl_mem_object_type   image_type ,
                                cl_uint              num_entries ,
                                cl_image_format *    image_formats ,
                                cl_uint *            num_image_formats)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    cl_uint n=0;
    unsigned int comm = ocland_clGetSupportedImageFormats;
    int *sockfd = context->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    socket_flag |= Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    socket_flag |= Send(sockfd, &image_type, sizeof(cl_mem_object_type), MSG_MORE);
    socket_flag |= Send(sockfd, &num_entries, sizeof(cl_uint), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv(sockfd, &n, sizeof(cl_uint), MSG_WAITALL);
    if(num_image_formats) *num_image_formats = n;
    if(image_formats){
        if(num_entries < n)
            n = num_entries;
        socket_flag |= Recv(sockfd, image_formats, n * sizeof(cl_image_format), MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int getMemObjectInfo(cl_mem            mem ,
                        cl_mem_info       param_name ,
                        size_t            param_value_size ,
                        void *            param_value ,
                        size_t *          param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetMemObjectInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = mem->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(mem->ptr), sizeof(cl_mem), MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_mem_info), MSG_MORE);
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

cl_int getImageInfo(cl_mem            image ,
                    cl_image_info     param_name ,
                    size_t            param_value_size ,
                    void *            param_value ,
                    size_t *          param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetImageInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = image->server->socket;
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(image->ptr), sizeof(cl_mem), MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_image_info), MSG_MORE);
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

cl_int setMemObjectDestructorCallback(cl_mem  memobj,
  	                                  void (CL_CALLBACK  *pfn_notify)(cl_mem memobj,
  	                                                                  void *user_data),
  	                                  void *user_data)
{
    // Backup the already existing callbacks
    void (CL_CALLBACK **pfn_backup)(cl_mem, void*) = memobj->pfn_notify;
    void **user_data_backup = memobj->user_data;

    // Allocate memory for all the callbacks
    memobj->pfn_notify = (void (CL_CALLBACK **)(cl_mem, void*))malloc(
        (memobj->num_pfn_notify + 1) * sizeof(
            void (CL_CALLBACK *)(cl_mem, void*)));
    memobj->user_data = (void **)malloc(
        (memobj->num_pfn_notify + 1) * sizeof(void*));
    if(!memobj->pfn_notify || !memobj->user_data){
        free(memobj->pfn_notify);
        free(memobj->user_data);
        return CL_OUT_OF_HOST_MEMORY;
    }

    // Set the new callback as the first one
    memobj->pfn_notify[0] = pfn_notify;
    memobj->user_data[0] = user_data;

    // Restore the backup
    memcpy(&(memobj->pfn_notify[1]),
           pfn_backup,
           memobj->num_pfn_notify * sizeof(void (CL_CALLBACK *)(cl_mem, void*)));
    memcpy(&(memobj->user_data[1]),
           user_data_backup,
           memobj->num_pfn_notify * sizeof(void*));
    memobj->num_pfn_notify++;

    return CL_SUCCESS;
}

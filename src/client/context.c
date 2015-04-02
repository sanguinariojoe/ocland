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
 * @brief ICD cl_context implementation
 * @see context.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/client/commands_enum.h>
#include <ocland/common/verbose.h>
#include <ocland/client/context.h>
#include <ocland/common/dataExchange.h>

/// Number of known contexts
cl_uint num_global_contexts = 0;
/// List of known contexts
cl_context *global_contexts = NULL;

int hasContext(cl_context context){
    cl_uint i;
    for(i = 0; i < num_global_contexts; i++){
        if(context == global_contexts[i])
            return 1;
    }
    return 0;
}

/** @brief Add a set of contexts to the global list global_contexts.
 *
 * The contexts will be added to the platform list as well.
 *
 * This method is not checking if the contexts are already present in the list,
 * however, accidentally adding the same context several times will only imply
 * performance penalties.
 * @param num_contexts Number of contexts to append.
 * @param contexts Contexts to append.
 * @return CL_SUCCESS if the context are already generated, an error code
 * otherwise.
 * @warning It is guessed that all the contexts of the list \a contexts belongs
 * to the same platform.
 */
cl_int addContexts(cl_uint      num_contexts,
                   cl_context*  contexts)
{
    if(!num_contexts)
        return CL_SUCCESS;

    // Add the contexts to the global list
    cl_context *backup_contexts = global_contexts;

    global_contexts = (cl_context*)malloc(
        (num_global_contexts + num_contexts) * sizeof(cl_context));
    if(!global_contexts){
        VERBOSE("Failure allocating memory for %u contexts!\n",
                num_global_contexts + num_contexts);
        free(backup_contexts); backup_contexts = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_contexts){
        memcpy(global_contexts,
               backup_contexts,
               num_global_contexts * sizeof(cl_context));
        free(backup_contexts); backup_contexts = NULL;
    }

    memcpy(&(global_contexts[num_global_contexts]),
           contexts,
           num_contexts * sizeof(cl_context));
    num_global_contexts += num_contexts;

    // Add the contexts to the platform list
    cl_platform_id platform = contexts[0]->platform;
    backup_contexts = platform->contexts;

    platform->contexts = (cl_context*)malloc(
        (platform->num_contexts + num_contexts) * sizeof(cl_context));
    if(!platform->contexts){
        VERBOSE("Failure allocating memory for %u contexts!\n",
                platform->num_contexts + num_contexts);
        free(backup_contexts); backup_contexts = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_contexts){
        memcpy(platform->contexts,
               backup_contexts,
               platform->num_contexts * sizeof(cl_context));
        free(backup_contexts); backup_contexts = NULL;
    }

    memcpy(&(platform->contexts[platform->num_contexts]),
           contexts,
           num_contexts * sizeof(cl_context));
    platform->num_contexts += num_contexts;

    return CL_SUCCESS;
}

/** @brief Get the context index in the global list
 * @param context Context to look for
 * @return Index of the context, num_global_contexts if it is not found.
 */
cl_uint contextIndex(cl_context context)
{
    cl_uint i;
    for(i = 0; i < num_global_contexts; i++){
        if(context == global_contexts[i])
            break;
    }
    return i;
}

cl_context contextFromServer(cl_context srv_context)
{
    cl_uint i;
    for(i = 0; i < num_global_contexts; i++){
        if(srv_context == global_contexts[i]->ptr)
            return global_contexts[i];
    }
    return NULL;
}

/** @brief Get the context index in the platform list
 * @param context Context to look for
 * @return Index of the context, _cl_platform_id::num_contexts if it is not
 * found.
 */
cl_uint contextInPlatformIndex(cl_context context)
{
    cl_uint i;
    cl_platform_id platform = context->platform;
    for(i = 0; i < platform->num_contexts; i++){
        if(context == platform->contexts[i])
            break;
    }
    return i;
}

/** @brief Remove a context from the global list, and from the platform.
 *
 * For instance when clReleaseContext() is called.
 * @param context context to be removed.
 * @return CL_SUCCESS if the context has been already discarded or
 * CL_INVALID_VALUE if the context does not exist.
 */
cl_int discardContext(cl_context context)
{
    if(!hasContext(context)){
        return CL_INVALID_VALUE;
    }
    cl_uint i, context_index, platform_index;

    context_index = contextIndex(context);
    platform_index = contextInPlatformIndex(context);
    cl_platform_id platform = context->platform;

    // Remove the context stuff
    free(global_contexts[context_index]->devices);
    global_contexts[context_index]->devices = NULL;
    free(global_contexts[context_index]->properties);
    global_contexts[context_index]->properties = NULL;
    assert(context == global_contexts[context_index]);
    free(global_contexts[context_index]);
    // context pointer is freed now
    context = NULL;

    assert(num_global_contexts > 0);
    // Remove the context from the global list
    for(i = context_index; i < num_global_contexts - 1; i++){
        global_contexts[i] = global_contexts[i + 1];
    }
    num_global_contexts--;
    global_contexts[num_global_contexts] = NULL;

    // Remove the context from the platform list
    assert(platform->num_contexts > 0);
    for(i = platform_index; i < platform->num_contexts - 1; i++){
        platform->contexts[i] = platform->contexts[i + 1];
    }
    platform->num_contexts--;
    // free(platform->contexts[platform->num_contexts]);  // Already removed
    platform->contexts[platform->num_contexts] = NULL;

    return CL_SUCCESS;
}

/** @brief Function to be called when the callbacks download stream is broken.
 * @param info_size Size of \a info.
 * @param info An error description string.
 * @param user_data The context affected.
 */
void CL_CALLBACK callbacksStreamError(size_t       info_size,
                                      const void*  info,
                                      void*        user_data)
{
    cl_context context = (cl_context)user_data;
    const char* error_str = (const char*)info;

    VERBOSE("Context (%p) callbacks download stream error: %s\n",
            context,
            error_str);
}

/** @brief Function to be called when the callbacks download stream is notifying
 * something.
 *
 * This function is just an intermediate function to the user defined callback.
 * @param info_size Size of \a info.
 * @param info Info provided by the server, composed by the following:
 *     - Identifier
 *     - Size of the error description string
 *     - Error description string
 *     - Size of the private_info binary array.
 *     - Private_info binary array.
 * @param user_data The User provided data.
 */
void CL_CALLBACK callbacksStreamNotify(size_t       info_size,
                                       const void*  info,
                                       void*        user_data)
{
    char *ptr = (char*)info;
    // Extract the context (identifier)
    cl_context context = *((cl_context*)ptr);
    ptr += sizeof(cl_context*);
    // Extract the errinfo
    size_t errinfo_size = *((size_t*)ptr);
    ptr += sizeof(size_t*);
    const char *errinfo = ptr;  // Let the trash after the '\0' closing char
    ptr += errinfo_size;
    // Extract the private info
    size_t cb = *((size_t*)ptr);
    ptr += sizeof(size_t*);
    const void* private_info = ptr;

    // Call the callback function
    context->pfn_notify(errinfo,
                        private_info,
                        cb,
                        context->user_data);
}

static void convertProperties(const cl_context_properties * in_properties,
                              pointer                     * out_properties,
                              cl_uint                     num_properties)
{
    // convert properties to server format
    // pack 32/64 bit pointers all to 64 bit pointers
    cl_uint context_platform_next = 0;
    cl_uint i;
    for(i = 0; i < num_properties; i++) {
        if (context_platform_next) {
            out_properties[i] = ((cl_platform_id)(in_properties[i]))->ptr_on_peer;
            context_platform_next = 0;
        } else {
            if( (!(i%2)) && (in_properties[i] == CL_CONTEXT_PLATFORM)) {
                // next will be platform pointer which is replaced
                // by real platform pointer from server virtual memory
                context_platform_next = 1;
            }
            out_properties[i] = StorePtr((cl_uint *)(in_properties[i]));
        }
    }
}

cl_context createContext(cl_platform_id                platform,
                         const cl_context_properties * properties,
                         cl_uint                       num_properties,
                         cl_uint                       num_devices ,
                         const cl_device_id *          devices,
                         void (CL_CALLBACK *           pfn_notify)(const char *,
                                                                   const void *,
                                                                   size_t,
                                                                   void *),
                         void *                        user_data,
                         cl_int *                      errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateContext;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = platform->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
        return NULL;
    }

    // Try to build up a context instance (we'll need to pass it as identifier
    // to the server)
    cl_context context=NULL, context_srv=NULL;
    context = (cl_context)malloc(sizeof(struct _cl_context));
    if(!context){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    context->devices = (cl_device_id *)malloc(
        num_devices * sizeof(cl_device_id));
    if(!context->devices){
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    context->dispatch = platform->dispatch;
    context->platform = platform;
    context->rcount = 1;
    context->server = platform->server;
    context->num_devices = num_devices;
    memcpy(context->devices, devices, num_devices * sizeof(cl_device_id));
    context->num_properties = num_properties;
    context->properties = (cl_context_properties *)malloc(
        num_properties * sizeof(cl_context_properties));
    if(!context->properties){
        free(context->devices); context->devices = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    memcpy(context->properties,
           properties,
           num_properties * sizeof(cl_context_properties));
    context->pfn_notify = pfn_notify;
    context->user_data = user_data;
    context->task_notify = NULL;
    context->task_error = NULL;

    // Change the local references to remote ones
    // and pack 32/64 bit pointers all to 64 bit pointers
    pointer *props = NULL;
    if(num_properties) {
        props = (pointer*)malloc(
            num_properties * sizeof(pointer));
        if(!props){
            free(context->devices); context->devices = NULL;
            free(context->properties); context->properties = NULL;
            free(context); context = NULL;
            if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
            return NULL;
        }
        convertProperties(properties, props, num_properties);
    }
    pointer *devs = malloc(num_devices * sizeof(pointer));
    if(!devs){
        free(props); props = NULL;
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    for(i = 0; i < num_devices; i++){
        devs[i] = devices[i]->ptr_on_peer;
    }

    // Call the server to generate the context
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &num_properties, sizeof(cl_uint), MSG_MORE);
    if(num_properties){
        socket_flag |= Send(sockfd,
                            props,
                            num_properties * sizeof(pointer),
                            MSG_MORE);
    }
    socket_flag |= Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send(sockfd, devs, num_devices * sizeof(pointer), MSG_MORE);
    // Shared identifier for the callback functions
    cl_context identifier=NULL;
    if(pfn_notify)
        identifier = context;
    socket_flag |= Send(sockfd, &identifier, sizeof(cl_context), 0);
    // And receive the answer
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(devs); devs = NULL;
        free(props); props = NULL;
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        free(devs); devs = NULL;
        free(props); props = NULL;
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &context_srv, sizeof(cl_context), MSG_WAITALL);
    if(socket_flag){
        free(devs); devs = NULL;
        free(props); props = NULL;
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    free(props); props=NULL;
    free(devs); devs=NULL;

    context->ptr = context_srv;

    // Get, or build up, the callbacks download data stream
    download_stream stream = createCallbackStream(platform->server);
    cl_uint stream_rcount = stream->rcount;
    if(!stream){
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    // Register a new callback function to manage errors in the new stream
    task t=NULL;
    t = setDownloadStreamErrorCallback(stream,
                                       &callbacksStreamError,
                                       (void*)context);
    if(!t){
        releaseDownloadStream(stream);
        if(1 == stream_rcount) {
            platform->server->callbacks_stream = NULL;
        }
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    context->task_error = t;

    // Register the required callback function
    if(pfn_notify){
        t = registerTask(stream->tasks,
                         (void*)context,
                         &callbacksStreamNotify,
                         user_data);
        if(!t){
            releaseDownloadStream(stream);
            if(1 == stream_rcount) {
                platform->server->callbacks_stream = NULL;
            }
            free(context->devices); context->devices = NULL;
            free(context); context = NULL;
            if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        context->task_notify = t;
    }

    // Add the context to the global list
    flag = addContexts(1, &context);
    if(flag != CL_SUCCESS){
        releaseDownloadStream(stream);
        if(1 == stream_rcount) {
            platform->server->callbacks_stream = NULL;
        }
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return context;
}

cl_context createContextFromType(cl_platform_id                platform,
                                 const cl_context_properties * properties,
                                 cl_uint                       num_properties,
                                 cl_device_type                device_type,
                                 void (CL_CALLBACK *           pfn_notify)(const char *,
                                                                           const void *,
                                                                           size_t,
                                                                           void *),
                                 void *                        user_data,
                                 cl_int *                      errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateContextFromType;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = platform->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
        return NULL;
    }

    // Get the list of devices from the platform
    cl_uint num_devices=0;
    flag = getDeviceIDs(platform,
                        device_type,
                        0,
                        NULL,
                        &num_devices);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
        return NULL;
    }
    cl_device_id *devices=NULL;
    devices = (cl_device_id *)malloc(num_devices * sizeof(cl_device_id));
    if(!devices){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    // Try to build up a context instance (we'll need to pass it as identifier
    // to the server)
    cl_context context=NULL, context_srv=NULL;
    context = (cl_context)malloc(sizeof(struct _cl_context));
    if(!context){
        free(devices); devices = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    context->dispatch = platform->dispatch;
    context->platform = platform;
    context->rcount = 1;
    context->server = platform->server;
    context->num_devices = num_devices;
    context->devices = devices;
    context->num_properties = num_properties;
    context->properties = (cl_context_properties *)malloc(
        num_properties * sizeof(cl_context_properties));
    if(!context->properties){
        free(context->devices); context->devices = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    memcpy(context->properties,
           properties,
           num_properties * sizeof(cl_context_properties));
    context->pfn_notify = pfn_notify;
    context->user_data = user_data;
    context->task_notify = NULL;
    context->task_error = NULL;

    // Change the local references to remote ones
    // and pack 32/64 bit pointers all to 64 bit pointers
    pointer *props = NULL;
    if(num_properties) {
        props = (pointer*)malloc(
            num_properties * sizeof(pointer));
        if(!props){
            free(context->devices); context->devices = NULL;
            free(context->properties); context->properties = NULL;
            free(context); context = NULL;
            if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
            return NULL;
        }
        convertProperties(properties, props, num_properties);
    }

    // Call the server to generate the context
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &num_properties, sizeof(cl_uint), MSG_MORE);
    if(num_properties){
        socket_flag |= Send(sockfd,
                            props,
                            num_properties * sizeof(cl_context_properties),
                            MSG_MORE);
    }
    socket_flag |= Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send(sockfd, &device_type, sizeof(cl_device_type), MSG_MORE);
    // Shared identifier for the callback functions
    cl_context identifier=NULL;
    if(pfn_notify)
        identifier = context;
    socket_flag |= Send(sockfd, &identifier, sizeof(cl_context), 0);
    // And receive the answer
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(props); props = NULL;
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        free(props); props = NULL;
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &context_srv, sizeof(cl_context), MSG_WAITALL);
    if(socket_flag){
        free(props); props = NULL;
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    free(props); props=NULL;

    context->ptr = context_srv;

    // Get, or build up, the callbacks download data stream
    download_stream stream = createCallbackStream(platform->server);
    if(!stream){
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    // Register a new callback function to manage errors in the new stream
    task t;
    t = setDownloadStreamErrorCallback(stream,
                                       &callbacksStreamError,
                                       (void*)context);
    if(!t){
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    context->task_error = t;

    // Register the required callback function
    if(pfn_notify){
        t = registerTask(stream->tasks,
                         (void*)context,
                         &callbacksStreamNotify,
                         user_data);
        if(!t){
            free(context->devices); context->devices = NULL;
            free(context->properties); context->properties = NULL;
            free(context); context = NULL;
            if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        context->task_notify = t;
    }

    cl_uint stream_rcount = stream->rcount;
    // Add the context to the global list
    flag = addContexts(1, &context);
    if(flag != CL_SUCCESS){
        releaseDownloadStream(stream);
        if(1 == stream_rcount) {
            platform->server->callbacks_stream = NULL;
        }
        free(context->devices); context->devices = NULL;
        free(context->properties); context->properties = NULL;
        free(context); context = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return context;
}

cl_int retainContext(cl_context context)
{
    context->rcount++;
    return CL_SUCCESS;
}

cl_int releaseContext(cl_context context)
{
    assert(context->rcount);
    context->rcount--;
    if(context->rcount){
        return CL_SUCCESS;
    }

    // Call the server to clear the instance
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clReleaseContext;
    int *sockfd = context->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr), sizeof(cl_context), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }

    // Release the tasks from the data stream
    if(context->task_notify){
        flag = unregisterTask(context->server->callbacks_stream->tasks,
                              context->task_notify);
        if(flag != CL_SUCCESS){
            return CL_OUT_OF_HOST_MEMORY;
        }
    }
    flag = unregisterTask(context->server->callbacks_stream->error_tasks,
                          context->task_error);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_uint stream_rcount = context->server->callbacks_stream->rcount;
    // Release the download stream
    flag = releaseDownloadStream(context->server->callbacks_stream);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_HOST_MEMORY;
    }
    if(1 == stream_rcount) {
        context->server->callbacks_stream = NULL;
    }

    // Free the memory
    flag = discardContext(context);
    if (flag != CL_SUCCESS){
        return CL_INVALID_CONTEXT;
    }

    return CL_SUCCESS;
}

cl_int getContextInfo(cl_context         context,
                      cl_context_info    param_name,
                      size_t             param_value_size,
                      void *             param_value,
                      size_t *           param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetContextInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = context->server->socket;
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_context_info), MSG_MORE);
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

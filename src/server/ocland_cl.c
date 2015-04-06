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
#include <signal.h>

#include <CL/cl.h>
#include <CL/cl_ext.h>

#include <ocland/common/sockets.h>
#include <ocland/common/dataExchange.h>
#include <ocland/common/dataPack.h>
#include <ocland/common/verbose.h>
#include <ocland/server/ocland_cl.h>


#ifndef OCLAND_PORT
    #define OCLAND_PORT 51000u
#endif

#ifndef BUFF_SIZE
    #define BUFF_SIZE 1025u
#endif


int ocland_clGetPlatformIDs(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_uint num_entries;
    cl_int flag = CL_SUCCESS;
    cl_uint num_platforms = 0, n = 0;
    cl_platform_id *platforms = NULL;
    pointer *platforms_ptr = NULL;
    cl_uint i;
    // Receive the parameters
    Recv(clientfd, &num_entries, sizeof(cl_uint), MSG_WAITALL);
    // Read the platforms
    if(num_entries){
        platforms = (cl_platform_id*)malloc(
            num_entries * sizeof(cl_platform_id));
        platforms_ptr = (pointer*)malloc(
            num_entries * sizeof(pointer));
        if (!platforms || !platforms_ptr) {
            flag = CL_OUT_OF_HOST_MEMORY;
        }
    }
    if (CL_SUCCESS == flag) {
        flag = clGetPlatformIDs(num_entries, platforms, &num_platforms);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if (CL_SUCCESS != flag) {
        // something went wrong, error code has been sent to client
        if (platforms) free(platforms); platforms = NULL;
        if (platforms_ptr) free(platforms_ptr); platforms_ptr = NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    n = (num_platforms < num_entries) ? num_platforms : num_entries;
    if(!n){
        Send(clientfd, &num_platforms, sizeof(cl_uint), 0);
    }
    else{
        for (i = 0; i < n; i++) {
            platforms_ptr[i] = StorePtr(platforms[i]);
        }
        Send(clientfd, &num_platforms, sizeof(cl_uint), MSG_MORE);
        Send(clientfd, platforms_ptr, n*sizeof(pointer), 0);
    }
    if(platforms) free(platforms); platforms=NULL;
    if(platforms_ptr) free(platforms_ptr); platforms_ptr=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetPlatformInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_platform_id platform;
    pointer platform_ptr;
    cl_platform_info param_name;
    size_t param_value_size = 0;
    size_t param_value_size_ret = 0;
    void *param_value = NULL;
    // Receive the parameters
    Recv(clientfd, &platform_ptr, sizeof(pointer), MSG_WAITALL);
    Recv(clientfd, &param_name, sizeof(cl_platform_info), MSG_WAITALL);
    Recv_size_t(clientfd, &param_value_size);
    platform = RestorePtr(platform_ptr);
    // Read the data from the platform
    flag = isPlatform(v, platform);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size){
        param_value = (void*)malloc(param_value_size);
        if (!param_value) {
            flag = CL_OUT_OF_HOST_MEMORY;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    flag = clGetPlatformInfo(platform,
                             param_name,
                             param_value_size,
                             param_value,
                             &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value); param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetDeviceIDs(int *clientfd, validator v)
{
    VERBOSE_IN();
    cl_platform_id platform;
    pointer platform_ptr;
    cl_device_type device_type;
    cl_uint num_entries;
    cl_int flag;
    cl_device_id *devices = NULL;
    cl_uint num_devices = 0;
    // Receive the parameters
    Recv(clientfd, &platform_ptr, sizeof(pointer), MSG_WAITALL);
    Recv(clientfd, &device_type, sizeof(cl_device_type), MSG_WAITALL);
    Recv(clientfd, &num_entries, sizeof(cl_uint), MSG_WAITALL);
    platform = RestorePtr(platform_ptr);
    // Read the data from the platform
    flag = isPlatform(v, platform);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(num_entries){
        devices = (cl_device_id*)malloc(num_entries*sizeof(cl_device_id));
    }
    flag = clGetDeviceIDs(platform, device_type, num_entries, devices, &num_devices);
    if( devices && (flag == CL_SUCCESS) )
        registerDevices(v, num_devices, devices);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if (CL_SUCCESS != flag) {
        // something went wrong, error code has been sent to client
        if (devices) free(devices); devices=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    if(devices){
        cl_uint i;
        Send(clientfd, &num_devices, sizeof(cl_uint), MSG_MORE);
        if(num_entries < num_devices)
            num_devices = num_entries;
        for(i = 0; i < num_devices; i++) {
            int flags = (i == num_devices - 1) ? 0 : MSG_MORE;
            pointer device_ptr = StorePtr(devices[i]);
            Send(clientfd, &device_ptr, sizeof(pointer), flags);
        }
    }
    else{
        Send(clientfd, &num_devices, sizeof(cl_uint), 0);
    }
    free(devices);devices=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetDeviceInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_device_id device;
    cl_platform_info param_name;
    size_t param_value_size = 0;
    size_t param_value_size_ret=0;
    void *param_value = NULL;
    // Receive the parameters
    pointer device_ptr;
    Recv(clientfd, &device_ptr, sizeof(pointer), MSG_WAITALL);
    Recv(clientfd, &param_name, sizeof(cl_device_info), MSG_WAITALL);
    Recv_size_t(clientfd, &param_value_size);
    device = RestorePtr(device_ptr);
    // Read the data from the device
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size){
        param_value = malloc(param_value_size);
        if (!param_value) {
            flag = CL_OUT_OF_HOST_MEMORY;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    flag = clGetDeviceInfo(device, param_name, param_value_size, param_value, &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateContext(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_uint num_properties = 0;
    cl_context_properties *properties = NULL;
    cl_uint num_devices = 0;
    cl_device_id *devices = NULL;
    pointer *devices_ptrs = NULL;
    cl_context identifier = NULL;
    cl_int flag;
    int socket_flag = 0;
    ocland_context context = NULL;
    // Receive the parameters
    socket_flag |= Recv(clientfd, &num_properties, sizeof(cl_uint), MSG_WAITALL);
    if(num_properties){
        pointer *properties_ptrs = malloc(num_properties * sizeof(pointer));
        properties = malloc(num_properties * sizeof(cl_context_properties));

        if(!properties || !properties_ptrs){
            // Memory troubles!!! Disconnecting the client is a good way to
            // leave this situation without damage
            shutdown(*clientfd, 2);
            *clientfd = -1;
            free(properties); properties = NULL;
            free(properties_ptrs); properties_ptrs = NULL;
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return 1;
        }
        socket_flag |= Recv(clientfd,
                            properties_ptrs,
                            num_properties * sizeof(pointer),
                            MSG_WAITALL);
        for(i = 0; i < num_properties; i++) {
            properties[i] = (cl_context_properties)RestorePtr(properties_ptrs[i]);
        }
        free(properties_ptrs); properties_ptrs = NULL;
    }
    socket_flag |= Recv(clientfd, &num_devices, sizeof(cl_uint), MSG_WAITALL);
    devices = malloc(num_devices * sizeof(cl_device_id));
    devices_ptrs = malloc(num_devices * sizeof(pointer));
    if(!devices || !devices_ptrs){
        // Memory troubles!!! Disconnecting the client is a good way to
        // leave this situation without damage
        shutdown(*clientfd, 2);
        *clientfd = -1;
        free(properties); properties = NULL;
        free(devices); devices = NULL;
        free(devices_ptrs); devices_ptrs = NULL;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }
    pointer identifier_ptr;
    socket_flag |= Recv(clientfd, devices_ptrs, num_devices * sizeof(pointer), MSG_WAITALL);
    socket_flag |= Recv(clientfd, &identifier_ptr, sizeof(pointer), MSG_WAITALL);
    identifier = RestorePtr(identifier_ptr);
    for (i = 0; i < num_devices; i++) {
        devices[i] = RestorePtr(devices_ptrs[i]);
    }
    free(devices_ptrs); devices_ptrs = NULL;
    if(socket_flag < 0){
        // Connectivity problems, just ignore the request (and let the
        // implementation top take care on it)
        free(properties); properties = NULL;
        free(devices); devices = NULL;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }
    // Execute the command (if possible)
    for(i = 0; i < num_properties; i = i + 2){
        if(!properties[i]){
            break;
        }
        if(properties[i] == CL_CONTEXT_PLATFORM){
            flag = isPlatform(v, (cl_platform_id)properties[i+1]);
            if(flag != CL_SUCCESS){
                Send(clientfd, &flag, sizeof(cl_int), 0);
                free(properties);      properties = NULL;
                free(devices);         devices = NULL;
                VERBOSE_OUT(flag);
                return 1;
            }
        }
    }
    for(i = 0; i < num_devices; i++){
        flag = isDevice(v, devices[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(properties); properties=NULL;
            free(devices); devices=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    context = oclandCreateContext(properties,
                                  num_devices,
                                  devices,
                                  identifier,
                                  v->callbacks_socket,
                                  &flag);
    free(properties); properties=NULL;
    free(devices); devices=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct sockaddr_in adr_inet;
    socklen_t len_inet;
    len_inet = sizeof(adr_inet);
    getpeername(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
    printf("%s has built a context\n", inet_ntoa(adr_inet.sin_addr));
    fflush(stdout);
    registerContext(v, context);
    // Answer to the client
    socket_flag |= Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    pointer context_ptr = StorePtr(context);
    socket_flag |= Send(clientfd, &context_ptr, sizeof(pointer), 0);
    if(socket_flag < 0){
        // Ops! The message has not arrived, just release the recently
        // generated context
        oclandReleaseContext(context);
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }

    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateContextFromType(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_uint num_properties = 0;
    cl_context_properties *properties = NULL;
    cl_device_type device_type;
  	cl_context identifier = NULL;
    cl_int flag;
    int socket_flag = 0;
    ocland_context context = NULL;
    // Receive the parameters
    socket_flag |= Recv(clientfd, &num_properties, sizeof(cl_uint), MSG_WAITALL);
    if(num_properties){
        pointer *properties_ptrs = malloc(num_properties * sizeof(pointer));
        properties = malloc(num_properties * sizeof(cl_context_properties));

        if(!properties || !properties_ptrs){
            // Memory troubles!!! Disconnecting the client is a good way to
            // leave this situation without damage
            shutdown(*clientfd, 2);
            *clientfd = -1;
            free(properties); properties = NULL;
            free(properties_ptrs); properties_ptrs = NULL;
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return 1;
        }
        socket_flag |= Recv(clientfd,
                            properties_ptrs,
                            num_properties * sizeof(pointer),
                            MSG_WAITALL);
        for(i = 0; i < num_properties; i++) {
            properties[i] = (cl_context_properties)RestorePtr(properties_ptrs[i]);
        }
        free(properties_ptrs); properties_ptrs = NULL;
    }
    pointer identifier_ptr;
    socket_flag |= Recv(clientfd, &device_type, sizeof(cl_device_type), MSG_WAITALL);
    socket_flag |= Recv(clientfd, &identifier_ptr, sizeof(pointer), MSG_WAITALL);
    identifier = RestorePtr(identifier_ptr);
    if(socket_flag < 0){
        // Connectivity problems, just ignore the request (and let the
        // implementation top take care on it)
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }
    // Execute the command
    for(i = 0; i < num_properties; i = i + 2){
        if(!properties[i]){
            break;
        }
        if(properties[i] == CL_CONTEXT_PLATFORM){
            flag = isPlatform(v, (cl_platform_id)properties[i + 1]);
            if(flag != CL_SUCCESS){
                Send(clientfd, &flag, sizeof(cl_int), 0);
                free(properties); properties=NULL;
                VERBOSE_OUT(flag);
                return 1;
            }
        }
    }
    context = oclandCreateContextFromType(properties,
                                          device_type,
                                          identifier,
                                          v->callbacks_socket,
                                          &flag);
    free(properties); properties=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct sockaddr_in adr_inet;
    socklen_t len_inet;
    len_inet = sizeof(adr_inet);
    getpeername(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
    printf("%s has built a context\n", inet_ntoa(adr_inet.sin_addr));
    fflush(stdout);
    registerContext(v, context);
    // Answer to the client
    pointer context_ptr = StorePtr(context);
    socket_flag |= Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    socket_flag |= Send(clientfd, &context_ptr, sizeof(pointer), 0);
    if(socket_flag < 0){
        // Ops! The message has not arrived, just release the recently
        // generated context
        oclandReleaseContext(context);
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }

    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clRetainContext(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    int socket_flag = 0;
    ocland_context context = NULL;
    // Receive the parameters
    socket_flag |= Recv(clientfd, &context, sizeof(ocland_context),MSG_WAITALL);
    if(socket_flag < 0){
        // Connectivity problems, just ignore the request (and let the
        // implementation top take care on it)
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        socket_flag |= Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = oclandRetainContext(context);
    // Answer to the client
    socket_flag |= Send(clientfd, &flag, sizeof(cl_int), 0);
    if(socket_flag < 0){
        // Ops! The message has not arrived, just revert the retainement (if
        // it was succesfully performed)
        if(flag == CL_SUCCESS)
            oclandReleaseContext(context);
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }

    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clReleaseContext(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    int socket_flag = 0;
    ocland_context context = NULL;
    // Receive the parameters
    socket_flag |= Recv(clientfd, &context, sizeof(ocland_context), MSG_WAITALL);
    if(socket_flag < 0){
        // Connectivity problems, just ignore the request (and let the
        // implementation top take care on it)
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        socket_flag |= Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = oclandReleaseContext(context);
    if(flag == CL_SUCCESS){
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getpeername(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        printf("%s has released a context\n", inet_ntoa(adr_inet.sin_addr));
        // unregister the context
        unregisterContext(v, context);
    }
    // Answer to the client
    socket_flag |= Send(clientfd, &flag, sizeof(cl_int), 0);
    if(socket_flag < 0){
        // Ops! The message has not arrived, how to deal that???
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetContextInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_context context = NULL;
    cl_context_info param_name;
    size_t param_value_size;
    cl_int flag;
    int socket_flag = 0;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    socket_flag |= Recv(clientfd, &context, sizeof(ocland_context), MSG_WAITALL);
    socket_flag |= Recv(clientfd, &param_name, sizeof(cl_context_info), MSG_WAITALL);
    socket_flag |= Recv_size_t(clientfd, &param_value_size);
    if(socket_flag < 0){
        // Connectivity problems, just ignore the request (and let the
        // implementation top take care on it)
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = oclandGetContextInfo(context,
                                param_name,
                                param_value_size,
                                param_value,
                                &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    // Does not care about if the messages are succesfully sent, we cannot
    // react anyway
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateCommandQueue(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_context context;
    cl_device_id device;
    pointer device_ptr;
    cl_command_queue_properties properties;
    cl_int flag;
    cl_command_queue command_queue = NULL;
    // Receive the parameters
    Recv(clientfd, &context, sizeof(ocland_context), MSG_WAITALL);
    Recv(clientfd, &device_ptr, sizeof(pointer), MSG_WAITALL);
    Recv(clientfd, &properties, sizeof(cl_command_queue_properties), MSG_WAITALL);
    device = RestorePtr(device_ptr);
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    command_queue = clCreateCommandQueue(context->context, device, properties, &flag);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct sockaddr_in adr_inet;
    socklen_t len_inet;
    len_inet = sizeof(adr_inet);
    getpeername(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
    printf("%s has built a command queue\n", inet_ntoa(adr_inet.sin_addr));
    registerQueue(v, command_queue);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &command_queue, sizeof(cl_command_queue), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clRetainCommandQueue(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_command_queue command_queue = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    // Execute the command
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clRetainCommandQueue(command_queue);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clReleaseCommandQueue(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_command_queue command_queue = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    // Execute the command
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clReleaseCommandQueue(command_queue);
    if(flag == CL_SUCCESS){
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getpeername(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        printf("%s has released a command queue\n", inet_ntoa(adr_inet.sin_addr));
        unregisterQueue(v,command_queue);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetCommandQueueInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_command_queue command_queue = NULL;
    cl_command_queue_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_command_queue_info),MSG_WAITALL);
    Recv_size_t(clientfd,&param_value_size);
    // Execute the command
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = clGetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateBuffer(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_context context;
    cl_mem_flags flags;
    size_t size;
    cl_bool hasPtr;
    void* host_ptr = NULL;
    cl_int flag;
    cl_mem mem = NULL;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    Recv(clientfd,&flags,sizeof(cl_mem_flags),MSG_WAITALL);
    Recv_size_t(clientfd,&size);
    Recv(clientfd,&hasPtr,sizeof(cl_bool),MSG_WAITALL);
    if(flags & CL_MEM_USE_HOST_PTR) flags -= CL_MEM_USE_HOST_PTR;
    if(flags & CL_MEM_ALLOC_HOST_PTR) flags -= CL_MEM_ALLOC_HOST_PTR;
    if(flags & CL_MEM_COPY_HOST_PTR){
        host_ptr = malloc(size);
        // Receive the data compressed
        dataPack in, out;
        out.size = size;
        out.data = host_ptr;
        Recv_size_t(clientfd, &(in.size));
        in.data = malloc(in.size);
        Recv(clientfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        free(host_ptr); host_ptr=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    mem = clCreateBuffer(context->context, flags, size, host_ptr, &flag);
    free(host_ptr); host_ptr=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerBuffer(v, mem);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &mem, sizeof(cl_mem), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clRetainMemObject(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_mem mem = NULL;
    // Receive the parameters
    Recv(clientfd,&mem,sizeof(cl_mem),MSG_WAITALL);
    // Execute the command
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clRetainMemObject(mem);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clReleaseMemObject(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_mem mem = NULL;
    // Receive the parameters
    Recv(clientfd,&mem,sizeof(cl_mem),MSG_WAITALL);
    // Execute the command
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clReleaseMemObject(mem);
    if(flag == CL_SUCCESS){
        unregisterBuffer(v,mem);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetSupportedImageFormats(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_context context;
    cl_mem_flags flags;
    cl_mem_object_type image_type;
    cl_uint num_entries;
    cl_int flag;
    cl_image_format *image_formats = NULL;
    cl_uint num_image_formats = 0;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    Recv(clientfd,&flags,sizeof(cl_mem_flags),MSG_WAITALL);
    Recv(clientfd,&image_type,sizeof(cl_mem_object_type),MSG_WAITALL);
    Recv(clientfd,&num_entries,sizeof(cl_uint),MSG_WAITALL);
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(num_entries)
        image_formats = (cl_image_format*)malloc(num_entries*sizeof(cl_image_format));
    flag = clGetSupportedImageFormats(context->context, flags, image_type, num_entries, image_formats, &num_image_formats);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(image_formats); image_formats=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(num_entries){
        Send(clientfd, &num_image_formats, sizeof(cl_uint), MSG_MORE);
        if(num_entries < num_image_formats)
            num_image_formats = num_entries;
        Send(clientfd, image_formats, num_entries*sizeof(cl_image_format), 0);
    }
    else{
        Send(clientfd, &num_image_formats, sizeof(cl_uint), 0);
    }
    free(image_formats);image_formats=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetMemObjectInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_mem mem = NULL;
    cl_mem_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    Recv(clientfd,&mem,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_mem_info),MSG_WAITALL);
    Recv_size_t(clientfd,&param_value_size);
    // Execute the command
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = clGetMemObjectInfo(mem, param_name, param_value_size, param_value, &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetImageInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_mem image = NULL;
    cl_image_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    Recv(clientfd,&image,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_image_info),MSG_WAITALL);
    Recv_size_t(clientfd,&param_value_size);
    // Execute the command
    flag = isBuffer(v, image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = clGetImageInfo(image, param_name, param_value_size, param_value, &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateSampler(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_context context;
    cl_bool normalized_coords;
    cl_addressing_mode addressing_mode;
    cl_filter_mode filter_mode;
    cl_int flag;
    cl_sampler sampler = NULL;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    Recv(clientfd,&normalized_coords,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&addressing_mode,sizeof(cl_addressing_mode),MSG_WAITALL);
    Recv(clientfd,&filter_mode,sizeof(cl_filter_mode),MSG_WAITALL);
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    sampler = clCreateSampler(context->context, normalized_coords, addressing_mode, filter_mode, &flag);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerSampler(v, sampler);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &sampler, sizeof(cl_sampler), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clRetainSampler(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_sampler sampler = NULL;
    // Receive the parameters
    Recv(clientfd,&sampler,sizeof(cl_sampler),MSG_WAITALL);
    // Execute the command
    flag = isSampler(v, sampler);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clRetainSampler(sampler);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clReleaseSampler(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_sampler sampler = NULL;
    // Receive the parameters
    Recv(clientfd,&sampler,sizeof(cl_sampler),MSG_WAITALL);
    // Execute the command
    flag = isSampler(v, sampler);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clReleaseSampler(sampler);
    if(flag == CL_SUCCESS){
        unregisterSampler(v,sampler);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetSamplerInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_sampler sampler = NULL;
    cl_sampler_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    Recv(clientfd,&sampler,sizeof(cl_sampler),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_sampler_info),MSG_WAITALL);
    Recv_size_t(clientfd,&param_value_size);
    // Execute the command
    flag = isSampler(v, sampler);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = clGetSamplerInfo(sampler, param_name, param_value_size, param_value, &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateProgramWithSource(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i, j;
    ocland_context context;
    cl_uint count;
    size_t *lengths = NULL;
    char **strings = NULL;
    cl_int flag;
    cl_program program = NULL;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    Recv(clientfd,&count,sizeof(cl_uint),MSG_WAITALL);
    lengths = calloc(count, sizeof(size_t));
    strings = calloc(count, sizeof(char*));
    if( (!lengths) || (!strings) ){
        free(lengths); lengths=NULL;
        free(strings); strings=NULL;
        flag = CL_OUT_OF_HOST_MEMORY;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    Recv_size_t_array(clientfd, lengths, count);
    for(i = 0; i < count; i++){
        strings[i] = malloc(lengths[i]);
        if((lengths[i] > 0)  && !strings[i]){
            for(j = 0; j < i; j++){
                free(strings[j]); strings[j] = NULL;
            }
            free(lengths); lengths=NULL;
            free(strings); strings=NULL;
            flag = CL_OUT_OF_HOST_MEMORY;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
        Recv(clientfd, strings[i], lengths[i], MSG_WAITALL);
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        for(i=0;i<count;i++){
            free(strings[i]); strings[i] = NULL;
        }
        free(lengths); lengths=NULL;
        free(strings); strings=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    program = clCreateProgramWithSource(context->context,
                                        count,
                                        (const char**)strings,
                                        lengths,
                                        &flag);
    for(i=0;i<count;i++){
        free(strings[i]); strings[i] = NULL;
    }
    free(lengths); lengths=NULL;
    free(strings); strings=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerProgram(v, program);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &program, sizeof(cl_program), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateProgramWithBinary(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i, j;
    ocland_context context;
    cl_uint num_devices;
    cl_device_id *device_list=NULL;
    size_t *lengths = NULL;
    unsigned char **binaries = NULL;
    cl_int flag;
    cl_int *binary_status = NULL;
    cl_program program = NULL;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    Recv(clientfd,&num_devices,sizeof(cl_uint),MSG_WAITALL);
    device_list = (cl_device_id*)malloc(num_devices*sizeof(cl_device_id));
    lengths = (size_t*)malloc(num_devices*sizeof(size_t));
    binaries = (unsigned char**)malloc(num_devices*sizeof(unsigned char*));
    binary_status = (cl_int*)malloc(num_devices*sizeof(cl_int));
    if( (!device_list) || (!lengths) || (!binaries) || (!binary_status) ){
        free(device_list); device_list=NULL;
        free(lengths); lengths=NULL;
        free(binaries); binaries=NULL;
        free(binary_status); binary_status=NULL;
        flag = CL_OUT_OF_HOST_MEMORY;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i = 0; i < num_devices; i++){
        pointer device_ptr;
        Recv(clientfd, &device_ptr, sizeof(pointer), MSG_WAITALL);
        device_list[i] = RestorePtr(device_ptr);
    }
    Recv_size_t_array(clientfd, lengths, num_devices);
    for(i=0;i<num_devices;i++){
        binaries[i] = (unsigned char*)malloc(lengths[i]);
        if(!binaries[i]){
            for(j=0;j<i;j++){
                free(binaries[j]); binaries[j] = NULL;
            }
            free(device_list); device_list=NULL;
            free(lengths); lengths=NULL;
            free(binaries); binaries=NULL;
            free(binary_status); binary_status=NULL;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
        Recv(clientfd,binaries[i],lengths[i],MSG_WAITALL);
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        for(i = 0; i < num_devices; i++){
            free(binaries[i]); binaries[i] = NULL;
        }
        free(device_list); device_list=NULL;
        free(lengths); lengths=NULL;
        free(binaries); binaries=NULL;
        free(binary_status); binary_status=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i = 0; i < num_devices; i++){
        flag = isDevice(v, device_list[i]);
        if(flag != CL_SUCCESS){
            for(i=0;i<num_devices;i++){
                free(binaries[i]); binaries[i] = NULL;
            }
            free(device_list); device_list=NULL;
            free(lengths); lengths=NULL;
            free(binaries); binaries=NULL;
            free(binary_status); binary_status=NULL;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    program = clCreateProgramWithBinary(context->context,
                                        num_devices,
                                        device_list,
                                        lengths,
                                        (const unsigned char**)binaries,
                                        binary_status,
                                        &flag);
    for(i=0;i<num_devices;i++){
        free(binaries[i]); binaries[i] = NULL;
    }
    free(device_list); device_list=NULL;
    free(lengths); lengths=NULL;
    free(binaries); binaries=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(binary_status); binary_status=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    registerProgram(v, program);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &program, sizeof(cl_program), 0);
    Send(clientfd, binary_status, num_devices*sizeof(cl_int), 0);
    free(binary_status); binary_status=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clRetainProgram(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_program program = NULL;
    // Receive the parameters
    Recv(clientfd,&program,sizeof(cl_program),MSG_WAITALL);
    // Execute the command
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clRetainProgram(program);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clReleaseProgram(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_program program = NULL;
    // Receive the parameters
    Recv(clientfd,&program,sizeof(cl_program),MSG_WAITALL);
    // Execute the command
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clReleaseProgram(program);
    if(flag == CL_SUCCESS){
        unregisterProgram(v,program);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clBuildProgram(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    unsigned int i;
    cl_program program;
    cl_uint num_devices;
    cl_device_id *device_list=NULL;
    size_t options_size;
    char *options = NULL;
    // Receive the parameters
    Recv(clientfd,&program,sizeof(cl_program),MSG_WAITALL);
    Recv(clientfd,&num_devices,sizeof(cl_uint),MSG_WAITALL);
    device_list = (cl_device_id*)malloc(num_devices*sizeof(cl_device_id));
    for(i = 0; i < num_devices; i++) {
        pointer device_ptr;
        Recv(clientfd, &device_ptr, sizeof(pointer), MSG_WAITALL);
        device_list[i] = RestorePtr(device_ptr);
    }
    Recv_size_t(clientfd, &options_size);
    options = malloc(options_size);
    Recv(clientfd, options, options_size, MSG_WAITALL);
    // Execute the command
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(device_list); device_list=NULL;
        free(options); options=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_devices;i++){
        flag = isDevice(v, device_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(device_list); device_list=NULL;
            free(options); options=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    flag = clBuildProgram(program, num_devices, device_list,
                          options, NULL, NULL);
    free(device_list); device_list=NULL;
    free(options); options=NULL;
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetProgramInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_uint i, num_devices = 0;
    size_t *binary_lengths=NULL;
    unsigned char** binaries=NULL;
    cl_program program = NULL;
    cl_program_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    Recv(clientfd, &program, sizeof(cl_program), MSG_WAITALL);
    Recv(clientfd, &param_name, sizeof(cl_program_info), MSG_WAITALL);
    Recv_size_t(clientfd, &param_value_size);
    // Execute the command
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size){
        param_value = (void*)malloc(param_value_size);
        if(!param_value){
            flag = CL_OUT_OF_RESOURCES;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
        // The flag CL_PROGRAM_BINARIES is requiring to allocate memory for each
        // binary (or setting a NULL pointer if it is not available)
        if(param_name == CL_PROGRAM_BINARIES){
            if(param_value_size % sizeof(unsigned char *)){
                flag = CL_INVALID_VALUE;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                VERBOSE_OUT(flag);
                free(param_value); param_value=NULL;
                return 1;
            }
            binaries = (unsigned char **)param_value;
            num_devices = param_value_size / sizeof(unsigned char *);
            for(i = 0; i < num_devices; i++){
                binaries[i] = NULL;
            }
            binary_lengths = (size_t*)malloc(num_devices * sizeof(size_t));
            if(!binary_lengths){
                flag = CL_OUT_OF_RESOURCES;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                VERBOSE_OUT(flag);
                return 1;
            }

            flag = clGetProgramInfo(program,
                                    CL_PROGRAM_BINARY_SIZES,
                                    num_devices * sizeof(size_t),
                                    binary_lengths,
                                    NULL);
            if(flag != CL_SUCCESS){
                flag = CL_OUT_OF_RESOURCES;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                free(param_value); param_value=NULL;
                VERBOSE_OUT(flag);
                return 1;
            }
            for(i = 0; i < num_devices; i++){
                if(!binary_lengths[i])
                    continue;
                binaries[i] = (unsigned char *)malloc(binary_lengths[i]);
                if(!binaries[i]){
                    flag = CL_OUT_OF_RESOURCES;
                    Send(clientfd, &flag, sizeof(cl_int), 0);
                    free(param_value); param_value=NULL;
                    VERBOSE_OUT(flag);
                    return 1;
                }
            }
        }
    }

    flag = clGetProgramInfo(program,
                            param_name,
                            param_value_size,
                            param_value,
                            &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if((param_name == CL_PROGRAM_BINARIES) && param_value_size){
            for(i = 0; i < num_devices; i++){
                if(!binary_lengths[i])
                    continue;
                free(binaries[i]);
                binaries[i] = NULL;
            }
            free(param_value); param_value=NULL;
        }
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(!param_value){
        Send_size_t(clientfd, param_value_size_ret, 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_name != CL_PROGRAM_BINARIES){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    Send_size_t(clientfd, param_value_size_ret, 0);
    for(i = 0; i < num_devices; i++){
        if(!binary_lengths[i]){
            continue;
        }
        Send(clientfd,
             binaries[i],
             binary_lengths[i],
             0);
        free(binaries[i]); binaries[i]=NULL;
    }
    free(param_value); param_value=NULL;

    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetProgramBuildInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_program program = NULL;
    cl_device_id device = NULL;
    cl_program_build_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    pointer device_ptr;
    // Receive the parameters
    Recv(clientfd,&program,sizeof(cl_program),MSG_WAITALL);
    Recv(clientfd,&device_ptr,sizeof(pointer),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_program_build_info),MSG_WAITALL);
    Recv_size_t(clientfd,&param_value_size);
    device = RestorePtr(device_ptr);
    // Execute the command
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = clGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateKernel(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_program program;
    size_t kernel_name_size;
    char* kernel_name = NULL;
    cl_int flag;
    cl_kernel kernel = NULL;
    // Receive the parameters
    Recv(clientfd,&program,sizeof(cl_program),MSG_WAITALL);
    Recv_size_t(clientfd,&kernel_name_size);
    kernel_name = (char*)malloc(kernel_name_size);
    Recv(clientfd,kernel_name,kernel_name_size,MSG_WAITALL);
    // Execute the command
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(kernel_name); kernel_name=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    kernel = clCreateKernel(program, kernel_name, &flag);
    free(kernel_name); kernel_name=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerKernel(v, kernel);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &kernel, sizeof(cl_kernel), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateKernelsInProgram(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_program program;
    cl_uint num_kernels;
    cl_int flag;
    cl_kernel *kernels = NULL;
    cl_uint num_kernels_ret = 0;
    // Receive the parameters
    Recv(clientfd,&program,sizeof(cl_program),MSG_WAITALL);
    Recv(clientfd,&num_kernels,sizeof(cl_uint),MSG_WAITALL);
    // Execute the command
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(num_kernels)
        kernels = (cl_kernel*)malloc(num_kernels*sizeof(cl_kernel));
    flag = clCreateKernelsInProgram(program, num_kernels, kernels, &num_kernels_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(kernels); kernels=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    cl_uint n = (num_kernels < num_kernels_ret) ? num_kernels : num_kernels_ret;
    for(i=0;i<n;i++){
        registerKernel(v, kernels[i]);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(kernels){
        Send(clientfd, &num_kernels_ret, sizeof(cl_uint), MSG_MORE);
        Send(clientfd, kernels, n*sizeof(cl_kernel), 0);
    }
    else{
        Send(clientfd, &num_kernels_ret, sizeof(cl_uint), 0);
    }
    free(kernels); kernels=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clRetainKernel(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_kernel kernel = NULL;
    // Receive the parameters
    Recv(clientfd,&kernel,sizeof(cl_kernel),MSG_WAITALL);
    // Execute the command
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clRetainKernel(kernel);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clReleaseKernel(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_kernel kernel = NULL;
    // Receive the parameters
    Recv(clientfd,&kernel,sizeof(cl_kernel),MSG_WAITALL);
    // Execute the command
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clReleaseKernel(kernel);
    if(flag == CL_SUCCESS){
        unregisterKernel(v,kernel);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clSetKernelArg(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_kernel kernel;
    cl_uint arg_index;
    size_t arg_size;
    size_t arg_value_size;
    void* arg_value=NULL;
    cl_int flag;
    // Receive the parameters
    Recv(clientfd,&kernel,sizeof(cl_kernel),MSG_WAITALL);
    Recv(clientfd,&arg_index,sizeof(cl_uint),MSG_WAITALL);
    Recv_size_t(clientfd,&arg_size);
    Recv_size_t(clientfd,&arg_value_size);
    if(arg_value_size){
        arg_value = malloc(arg_value_size);
        Recv(clientfd,arg_value,arg_value_size,MSG_WAITALL);
    }
    // Execute the command
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(arg_value); arg_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clSetKernelArg(kernel, arg_index, arg_size, arg_value);
    free(arg_value); arg_value=NULL;
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetKernelInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_kernel kernel = NULL;
    cl_kernel_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    Recv(clientfd,&kernel,sizeof(cl_kernel),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_kernel_info),MSG_WAITALL);
    Recv_size_t(clientfd,&param_value_size);
    // Execute the command
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = clGetKernelInfo(kernel, param_name, param_value_size, param_value, &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetKernelWorkGroupInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_kernel kernel = NULL;
    cl_device_id device;
    cl_kernel_work_group_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    pointer device_ptr;
    // Receive the parameters
    Recv(clientfd,&kernel,sizeof(cl_kernel),MSG_WAITALL);
    Recv(clientfd,&device_ptr,sizeof(pointer),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_kernel_work_group_info),MSG_WAITALL);
    Recv_size_t(clientfd, &param_value_size);
    device = RestorePtr(device_ptr);
    // Execute the command
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = clGetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clWaitForEvents(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_uint num_events;
    ocland_event *event_list = NULL;
    cl_int flag;
    // Receive the parameters
    Recv(clientfd,&num_events,sizeof(cl_uint),MSG_WAITALL);
    event_list = (ocland_event*)malloc(num_events*sizeof(ocland_event));
    Recv(clientfd,event_list,num_events*sizeof(ocland_event),MSG_WAITALL);
    // Execute the command
    for(i=0;i<num_events;i++){
        flag = isEvent(v, event_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_list);event_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    flag = oclandWaitForEvents(num_events, event_list);
    free(event_list);event_list=NULL;
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetEventInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_event event = NULL;
    cl_event_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    Recv(clientfd,&event,sizeof(ocland_event),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_event_info),MSG_WAITALL);
    Recv_size_t(clientfd, &param_value_size);
    // Execute the command
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = malloc(param_value_size);
    flag = oclandGetEventInfo(event,
                              param_name,
                              param_value_size,
                              param_value,
                              &param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clRetainEvent(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    ocland_event event;
    // Receive the parameters
    Recv(clientfd,&event,sizeof(ocland_event),MSG_WAITALL);
    // Execute the command
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clRetainEvent(event->event);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clReleaseEvent(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    ocland_event event;
    // Receive the parameters
    Recv(clientfd,&event,sizeof(ocland_event),MSG_WAITALL);
    // Execute the command
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clReleaseEvent(event->event);
    if(flag == CL_SUCCESS){
        unregisterEvent(v,event);
        free(event);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetEventProfilingInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_event event = NULL;
    cl_profiling_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    Recv(clientfd,&event,sizeof(ocland_event),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_profiling_info),MSG_WAITALL);
    Recv_size_t(clientfd,&param_value_size);
    // Execute the command
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = clGetEventProfilingInfo(event->event,param_name,param_value_size,param_value,&param_value_size_ret);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clFlush(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_command_queue command_queue;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    // Execute the command
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clFlush(command_queue);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clFinish(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i,j;
    cl_int flag;
    cl_command_queue command_queue;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    // Wait for all the ocland events associated to this command queue
    cl_uint num_events = 0;
    ocland_event *event_list = NULL;
    for(i=0;i<v->num_events;i++){
        if(v->events[i]->command_queue == command_queue)
            num_events++;
    }
    if(num_events){
        event_list = (ocland_event*)malloc(num_events*sizeof(ocland_event));
        if(!event_list){
            flag     = CL_OUT_OF_HOST_MEMORY;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
        j = 0;
        for(i=0;i<v->num_events;i++){
            if(v->events[i]->command_queue == command_queue){
                event_list[j] = v->events[i];
                j++;
            }
        }
        flag = oclandWaitForEvents(num_events, event_list);
        free(event_list); event_list=NULL;
        if(flag != CL_SUCCESS){
            flag     = 	CL_INVALID_COMMAND_QUEUE;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Execute the command
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clFinish(command_queue);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueReadBuffer(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem memobj;
    cl_bool blocking_read;
    size_t offset;
    size_t cb;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    void* ptr = NULL;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&memobj,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&blocking_read,sizeof(cl_bool),MSG_WAITALL);
    Recv_size_t(clientfd,&offset);
    Recv_size_t(clientfd,&cb);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = isBuffer(v, memobj);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    ptr   = malloc(cb);
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_READ_BUFFER;
    // ------------------------------------------------------------
    // Blocking read case:
    // We simply call to the read method to get the data and
    // send it to the client.
    // ------------------------------------------------------------
    if(blocking_read == CL_TRUE){
        // We must wait manually for the events manually in order to
        // control the events generated by ocland.
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            free(event_wait_list); event_wait_list=NULL;
        }
        // Execute the command
        flag = clEnqueueReadBuffer(command_queue,memobj,blocking_read,
                                   offset,cb,ptr,
                                   0,NULL,&(event->event));
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(ptr); ptr=NULL;
            free(event); event=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
        // Answer to the client
        Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
        if(want_event){
            Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
            registerEvent(v,event);
            event->status = CL_COMPLETE;
        }
        else{
            free(event); event = NULL;
        }
        dataPack in, out;
        in.size = cb;
        in.data = ptr;
        out = pack(in);
        Send_size_t(clientfd, out.size, MSG_MORE);
        Send(clientfd, out.data, out.size, 0);
        free(out.data);out.data=NULL;
        free(ptr);ptr=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // Another thread will pack the data and send it using another
    // port.
    // ------------------------------------------------------------
    flag = oclandEnqueueReadBuffer(clientfd,command_queue,memobj,
                                   offset,cb,ptr,
                                   num_events_in_wait_list,event_wait_list,
                                   want_event, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // We can't mark the work as done, or destroy the data
    // becuase oclandEnqueueReadBuffer is using it
    if(want_event == CL_TRUE){
        registerEvent(v, event);
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueWriteBuffer(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem memobj;
    cl_bool blocking_write;
    size_t offset;
    size_t cb;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    void* ptr = NULL;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&memobj,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&blocking_write,sizeof(cl_bool),MSG_WAITALL);
    Recv_size_t(clientfd,&offset);
    Recv_size_t(clientfd,&cb);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    ptr = malloc(cb);
    if(blocking_write){
        dataPack in, out;
        out.size = cb;
        out.data = ptr;
        Recv_size_t(clientfd, &(in.size));
        in.data = malloc(in.size);
        Recv(clientfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = isBuffer(v, memobj);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_WRITE_BUFFER;
    // ------------------------------------------------------------
    // Blocking read case:
    // We simply call to the read method to get the data and
    // send it to the client.
    // ------------------------------------------------------------
    if(blocking_write == CL_TRUE){
        // We must wait manually for the events manually in order to
        // control the events generated by ocland.
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            free(event_wait_list); event_wait_list=NULL;
        }
        // Execute the command
        flag = clEnqueueWriteBuffer(command_queue,memobj,blocking_write,
                                   offset,cb,ptr,
                                   0,NULL,&(event->event));
        free(ptr);ptr=NULL;
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event); event=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
        // Answer to the client
        int send_flags = want_event ? MSG_MORE : 0;
        Send(clientfd, &flag, sizeof(cl_int), send_flags);
        if(want_event){
            Send(clientfd, &event, sizeof(ocland_event), 0);
            registerEvent(v,event);
            event->status = CL_COMPLETE;
        }
        else{
            free(event); event = NULL;
        }
        VERBOSE_OUT(flag);
        return 1;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // Another thread will pack the data and send it using another
    // port.
    // ------------------------------------------------------------
    flag = oclandEnqueueWriteBuffer(clientfd,command_queue,memobj,
                                    offset,cb,ptr,
                                    num_events_in_wait_list,event_wait_list,
                                    want_event, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // We can't mark the work as done, or destroy the data
    // becuase oclandEnqueueReadBuffer is using it
    if(want_event == CL_TRUE){
        registerEvent(v, event);
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueCopyBuffer(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem src_buffer;
    cl_mem dst_buffer;
    size_t src_offset;
    size_t dst_offset;
    size_t cb;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&src_buffer,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&dst_buffer,sizeof(cl_mem),MSG_WAITALL);
    Recv_size_t(clientfd,&src_offset);
    Recv_size_t(clientfd,&dst_offset);
    Recv_size_t(clientfd,&cb);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag  = isBuffer(v, src_buffer);
    flag |= isBuffer(v, dst_buffer);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_COPY_BUFFER;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    flag = clEnqueueCopyBuffer(command_queue,src_buffer,dst_buffer,
                               src_offset,dst_offset,cb,
                               0,NULL,&(event->event));
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueCopyImage(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem src_image;
    cl_mem dst_image;
    size_t src_origin[3];
    size_t dst_origin[3];
    size_t region[3];
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&src_image,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&dst_image,sizeof(cl_mem),MSG_WAITALL);
    Recv_size_t_array(clientfd,src_origin,3);
    Recv_size_t_array(clientfd,dst_origin,3);
    Recv_size_t_array(clientfd,region,3);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag  = isBuffer(v, src_image);
    flag |= isBuffer(v, dst_image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_COPY_IMAGE;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    flag = clEnqueueCopyImage(command_queue,src_image,dst_image,
                              src_origin,dst_origin,region,
                              0,NULL,&(event->event));
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueCopyImageToBuffer(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem src_image;
    cl_mem dst_buffer;
    size_t src_origin[3];
    size_t region[3];
    size_t dst_offset;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&src_image,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&dst_buffer,sizeof(cl_mem),MSG_WAITALL);
    Recv_size_t_array(clientfd,src_origin,3);
    Recv_size_t_array(clientfd,region,3);
    Recv_size_t(clientfd,&dst_offset);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag  = isBuffer(v, src_image);
    flag |= isBuffer(v, dst_buffer);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_COPY_IMAGE_TO_BUFFER;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    flag = clEnqueueCopyImageToBuffer(command_queue,src_image,dst_buffer,
                              src_origin,region,dst_offset,
                              0,NULL,&(event->event));
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueCopyBufferToImage(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem src_buffer;
    cl_mem dst_image;
    size_t src_offset;
    size_t dst_origin[3];
    size_t region[3];
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&src_buffer,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&dst_image,sizeof(cl_mem),MSG_WAITALL);
    Recv_size_t(clientfd,&src_offset);
    Recv_size_t_array(clientfd,dst_origin,3);
    Recv_size_t_array(clientfd,region,3);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag  = isBuffer(v, src_buffer);
    flag |= isBuffer(v, dst_image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_COPY_BUFFER_TO_IMAGE;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    flag = clEnqueueCopyBufferToImage(command_queue,
                                      src_buffer,
                                      dst_image,
                                      src_offset,
                                      dst_origin,
                                      region,
                                      0,
                                      NULL,
                                      &(event->event));
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueNDRangeKernel(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_kernel kernel;
    cl_uint work_dim;
    cl_bool has_global_work_offset;
    cl_bool has_local_work_size;
    size_t *global_work_offset = NULL;
    size_t *global_work_size = NULL;
    size_t *local_work_size = NULL;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&kernel,sizeof(cl_kernel),MSG_WAITALL);
    Recv(clientfd,&work_dim,sizeof(cl_uint),MSG_WAITALL);
    Recv(clientfd,&has_global_work_offset,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&has_local_work_size,sizeof(cl_bool),MSG_WAITALL);
    if(has_global_work_offset){
        global_work_offset = (size_t*)malloc(work_dim * sizeof(size_t));
        Recv_size_t_array(clientfd,global_work_offset,work_dim);
    }
    global_work_size = (size_t*)malloc(work_dim * sizeof(size_t));
    Recv_size_t_array(clientfd,global_work_size,work_dim);
    if(has_local_work_size){
        local_work_size = (size_t*)malloc(work_dim * sizeof(size_t));
        Recv_size_t_array(clientfd,local_work_size,work_dim);
    }
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag  = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_NDRANGE_KERNEL;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    flag = clEnqueueNDRangeKernel(command_queue,kernel,work_dim,
                                  global_work_offset,global_work_size,local_work_size,
                                  0,NULL,&(event->event));
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    int send_flags = want_event ? MSG_MORE : 0;
    Send(clientfd, &flag, sizeof(cl_int), send_flags);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueReadImage(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem memobj;
    cl_bool blocking_read;
    size_t origin[3];
    size_t region[3];
    size_t row_pitch;
    size_t slice_pitch;
    size_t element_size;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    void* ptr = NULL;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&memobj,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&blocking_read,sizeof(cl_bool),MSG_WAITALL);
    Recv_size_t_array(clientfd,origin,3);
    Recv_size_t_array(clientfd,region,3);
    Recv_size_t(clientfd,&row_pitch);
    Recv_size_t(clientfd,&slice_pitch);
    Recv_size_t(clientfd,&element_size);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = isBuffer(v, memobj);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        flag = CL_INVALID_COMMAND_QUEUE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    size_t cb = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    ptr   = malloc(cb);
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_READ_IMAGE;
    // ------------------------------------------------------------
    // Blocking read case:
    // We simply call to the read method to get the data and
    // send it to the client.
    // ------------------------------------------------------------
    if(blocking_read == CL_TRUE){
        // We must wait manually for the events manually in order to
        // control the events generated by ocland.
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            free(event_wait_list); event_wait_list=NULL;
        }
        // Execute the command
        flag = clEnqueueReadImage(command_queue,memobj,blocking_read,
                                  origin,region,
                                  row_pitch,slice_pitch,ptr,
                                  0,NULL,&(event->event));
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(ptr); ptr=NULL;
            free(event); event=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
        // Answer to the client
        Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
        if(want_event){
            Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
            registerEvent(v,event);
            event->status = CL_COMPLETE;
        }
        else{
            free(event); event = NULL;
        }
        dataPack in, out;
        in.size = cb;
        in.data = ptr;
        out = pack(in);
        Send_size_t(clientfd, out.size, MSG_MORE);
        Send(clientfd, out.data, out.size, 0);
        free(out.data);out.data=NULL;
        free(ptr);ptr=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // Another thread will pack the data and send it using another
    // port.
    // ------------------------------------------------------------
    flag = oclandEnqueueReadImage(clientfd,command_queue,memobj,
                                   origin,region,
                                   row_pitch,slice_pitch,
                                   element_size,ptr,
                                   num_events_in_wait_list,event_wait_list,
                                   want_event, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // We can't mark the work as done, or destroy the data
    // becuase oclandEnqueueReadBuffer is using it
    if(want_event == CL_TRUE){
        registerEvent(v, event);
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueWriteImage(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem memobj;
    cl_bool blocking_write;
    size_t origin[3];
    size_t region[3];
    size_t row_pitch;
    size_t slice_pitch;
    size_t element_size;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    void* ptr = NULL;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&memobj,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&blocking_write,sizeof(cl_bool),MSG_WAITALL);
    Recv_size_t_array(clientfd,origin,3);
    Recv_size_t_array(clientfd,region,3);
    Recv_size_t(clientfd,&row_pitch);
    Recv_size_t(clientfd,&slice_pitch);
    Recv_size_t(clientfd,&element_size);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    size_t cb = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    ptr = malloc(cb);
    if(blocking_write){
        dataPack in, out;
        out.size = cb;
        out.data = ptr;
        Recv_size_t(clientfd, &(in.size));
        in.data = malloc(in.size);
        Recv(clientfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = isBuffer(v, memobj);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_WRITE_IMAGE;
    // ------------------------------------------------------------
    // Blocking read case:
    // We simply call to the read method to get the data and
    // send it to the client.
    // ------------------------------------------------------------
    if(blocking_write == CL_TRUE){
        // We must wait manually for the events manually in order to
        // control the events generated by ocland.
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            free(event_wait_list); event_wait_list=NULL;
        }
        // Execute the command
        flag = clEnqueueWriteImage(command_queue,memobj,blocking_write,
                                   origin,region,
                                   row_pitch,slice_pitch,ptr,
                                   0,NULL,&(event->event));
        free(ptr);ptr=NULL;
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event); event=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
        // Answer to the client
        Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
        if(want_event){
            Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
            registerEvent(v,event);
            event->status = CL_COMPLETE;
        }
        else{
            free(event); event = NULL;
        }
        VERBOSE_OUT(flag);
        return 1;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // Another thread will pack the data and send it using another
    // port.
    // ------------------------------------------------------------
    flag = oclandEnqueueWriteImage(clientfd,command_queue,memobj,
                                   origin,region,
                                   row_pitch,slice_pitch,
                                   cb,ptr,
                                   num_events_in_wait_list,event_wait_list,
                                   want_event, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // We can't mark the work as done, or destroy the data
    // becuase oclandEnqueueReadBuffer is using it
    if(want_event == CL_TRUE){
        registerEvent(v, event);
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateImage2D(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_context context;
    cl_mem_flags flags;
    cl_image_format image_format;
    size_t image_width;
    size_t image_height;
    size_t image_row_pitch;
    size_t element_size;
    cl_bool hasPtr;
    void* host_ptr = NULL;
    cl_int flag;
    cl_mem image = NULL;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    Recv(clientfd,&flags,sizeof(cl_mem_flags),MSG_WAITALL);
    Recv(clientfd,&image_format,sizeof(cl_image_format),MSG_WAITALL);
    Recv_size_t(clientfd,&image_width);
    Recv_size_t(clientfd,&image_height);
    Recv_size_t(clientfd,&image_row_pitch);
    Recv_size_t(clientfd,&element_size);
    Recv(clientfd,&hasPtr,sizeof(cl_bool),MSG_WAITALL);
    if(flags & CL_MEM_USE_HOST_PTR) flags -= CL_MEM_USE_HOST_PTR;
    if(flags & CL_MEM_ALLOC_HOST_PTR) flags -= CL_MEM_ALLOC_HOST_PTR;
    if(flags & CL_MEM_COPY_HOST_PTR){
        size_t size = image_width*image_height*element_size;
        host_ptr = malloc(size);
        // Receive the data compressed
        dataPack in, out;
        out.size = size;
        out.data = host_ptr;
        Recv_size_t(clientfd, &(in.size));
        in.data = malloc(in.size);
        Recv(clientfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        free(host_ptr); host_ptr=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    image = clCreateImage2D(context->context, flags, &image_format,
                             image_width, image_height,
                             image_row_pitch,
                             host_ptr, &flag);
    free(host_ptr); host_ptr=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerBuffer(v, image);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &image, sizeof(cl_mem), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateImage3D(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_context context;
    cl_mem_flags flags;
    cl_image_format image_format;
    size_t image_width;
    size_t image_height;
    size_t image_depth;
    size_t image_row_pitch;
    size_t image_slice_pitch;
    size_t element_size;
    cl_bool hasPtr;
    void* host_ptr = NULL;
    cl_int flag;
    cl_mem image = NULL;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    Recv(clientfd,&flags,sizeof(cl_mem_flags),MSG_WAITALL);
    Recv(clientfd,&image_format,sizeof(cl_image_format),MSG_WAITALL);
    Recv_size_t(clientfd,&image_width);
    Recv_size_t(clientfd,&image_height);
    Recv_size_t(clientfd,&image_depth);
    Recv_size_t(clientfd,&image_row_pitch);
    Recv_size_t(clientfd,&image_slice_pitch);
    Recv_size_t(clientfd,&element_size);
    Recv(clientfd,&hasPtr,sizeof(cl_bool),MSG_WAITALL);
    if(flags & CL_MEM_USE_HOST_PTR) flags -= CL_MEM_USE_HOST_PTR;
    if(flags & CL_MEM_ALLOC_HOST_PTR) flags -= CL_MEM_ALLOC_HOST_PTR;
    if(flags & CL_MEM_COPY_HOST_PTR){
        size_t size = image_width*image_height*image_depth*element_size;
        host_ptr = malloc(size);
        // Receive the data compressed
        dataPack in, out;
        out.size = size;
        out.data = host_ptr;
        Recv_size_t(clientfd, &(in.size));
        in.data = malloc(in.size);
        Recv(clientfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        free(host_ptr); host_ptr=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    image = clCreateImage3D(context->context, flags, &image_format,
                             image_width, image_height,image_depth,
                             image_row_pitch,image_slice_pitch,
                             host_ptr, &flag);
    free(host_ptr); host_ptr=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerBuffer(v, image);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &image, sizeof(cl_mem), 0);
    VERBOSE_OUT(flag);
    return 1;
}

// ----------------------------------
// OpenCL 1.1
// ----------------------------------
int ocland_clCreateSubBuffer(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_mem mem;
    cl_mem_flags flags;
    cl_buffer_create_type buffer_create_type;
    cl_buffer_region buffer_region = {0};
  	void *buffer_create_info = NULL;
    cl_int flag;
    cl_mem submem = NULL;
    // Receive the parameters
    pointer mem_ptr;
    Recv(clientfd, &mem_ptr, sizeof(pointer), MSG_WAITALL);
    mem = RestorePtr(mem_ptr);
    Recv(clientfd,&flags,sizeof(cl_mem_flags),MSG_WAITALL);
    Recv(clientfd,&buffer_create_type,sizeof(cl_buffer_create_type),MSG_WAITALL);
    if (buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION) {
        buffer_create_info = &buffer_region;
        Recv_size_t(clientfd, &buffer_region.size);
        Recv_size_t(clientfd, &buffer_region.origin);
    }
    // Execute the command
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct _cl_version version = clGetMemObjectVersion(mem);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 1)))
    {
        // OpenCL < 1.1, so this function does not exist
        flag     = CL_INVALID_MEM_OBJECT;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    submem = clCreateSubBuffer(mem, flags, buffer_create_type, buffer_create_info, &flag);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerBuffer(v, submem);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    pointer submem_ptr = StorePtr(submem);
    Send(clientfd, &submem_ptr, sizeof(pointer), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateUserEvent(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_context context;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct _cl_version version = clGetContextVersion(context->context);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 1)))
    {
        // OpenCL < 1.1, so this function does not exist
        flag     = CL_INVALID_CONTEXT;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag     = CL_OUT_OF_HOST_MEMORY;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    event->status        = CL_COMPLETE;
    event->context       = context->context;
    event->command_queue = NULL;
    event->event         = clCreateUserEvent(context->context, &flag);
    event->command_type  = CL_COMMAND_USER;
    if(flag != CL_SUCCESS){
        free(event); event=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerEvent(v, event);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    Send(clientfd, &event, sizeof(ocland_event), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clSetUserEventStatus(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_event event;
    cl_int execution_status;
    cl_int flag;
    // Receive the parameters
    Recv(clientfd,&event,sizeof(ocland_event),MSG_WAITALL);
    Recv(clientfd,&execution_status,sizeof(cl_int),MSG_WAITALL);
    // Execute the command
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct _cl_version version = clGetContextVersion(event->context);
    if(     (!event->event)
        ||  (version.major <  1)
        || ((version.major == 1) && (version.minor < 1)))
    {
        // OpenCL < 1.1, so this function does not exist
        flag     = CL_INVALID_EVENT;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = clSetUserEventStatus(event->event, execution_status);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueReadBufferRect(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem mem;
    cl_bool blocking_read;
    size_t buffer_origin[3];
    size_t host_origin[3] = {0, 0, 0};
    size_t region[3];
    size_t buffer_row_pitch;
    size_t buffer_slice_pitch;
    size_t host_row_pitch;
    size_t host_slice_pitch;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    void* ptr = NULL;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&mem,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&blocking_read,sizeof(cl_bool),MSG_WAITALL);
    Recv_size_t_array(clientfd,buffer_origin,3);
    Recv_size_t_array(clientfd,region,3);
    Recv_size_t(clientfd,&buffer_row_pitch);
    Recv_size_t(clientfd,&buffer_slice_pitch);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        flag = CL_INVALID_COMMAND_QUEUE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    size_t cb = region[0] * region[1] * region[2];
    ptr   = malloc(cb);
    host_row_pitch = region[0];
    host_slice_pitch = region[1] * host_row_pitch;
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_READ_BUFFER_RECT;
    // ------------------------------------------------------------
    // Blocking read case:
    // We simply call to the read method to get the data and
    // send it to the client.
    // ------------------------------------------------------------
    if(blocking_read == CL_TRUE){
        // We must wait manually for the events manually in order to
        // control the events generated by ocland.
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            free(event_wait_list); event_wait_list=NULL;
        }
        // Execute the command
        struct _cl_version version = clGetCommandQueueVersion(command_queue);
        if(     (version.major <  1)
            || ((version.major == 1) && (version.minor < 1))){
            // OpenCL < 1.1, so this function does not exist
            flag = CL_INVALID_COMMAND_QUEUE;
        }
        else{
            flag = clEnqueueReadBufferRect(command_queue, mem, blocking_read,
                                           buffer_origin, host_origin, region,
                                           buffer_row_pitch, buffer_slice_pitch,
                                           host_row_pitch, host_slice_pitch,
                                           ptr, 0, NULL, &(event->event));
        }
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(ptr); ptr=NULL;
            free(event); event=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
        // Answer to the client
        Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
        if(want_event){
            Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
            registerEvent(v,event);
            event->status = CL_COMPLETE;
        }
        else{
            free(event); event = NULL;
        }
        dataPack in, out;
        in.size = cb;
        in.data = ptr;
        out = pack(in);
        Send_size_t(clientfd, out.size, MSG_MORE);
        Send(clientfd, out.data, out.size, 0);
        free(out.data);out.data=NULL;
        free(ptr);ptr=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // Another thread will pack the data and send it using another
    // port.
    // ------------------------------------------------------------
    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 1))){
        // OpenCL < 1.1, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = oclandEnqueueReadBufferRect(clientfd,command_queue,mem,
                                           buffer_origin,region,
                                           buffer_row_pitch,buffer_slice_pitch,
                                           host_row_pitch,host_slice_pitch,
                                           ptr,num_events_in_wait_list,event_wait_list,
                                           want_event, event);
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // We can't mark the work as done, or destroy the data
    // becuase oclandEnqueueReadBuffer is using it
    if(want_event == CL_TRUE){
        registerEvent(v, event);
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueWriteBufferRect(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem mem;
    cl_bool blocking_write;
    size_t buffer_origin[3];
    size_t host_origin[3] = {0, 0, 0};
    size_t region[3];
    size_t buffer_row_pitch;
    size_t buffer_slice_pitch;
    size_t host_row_pitch;
    size_t host_slice_pitch;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    void* ptr = NULL;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&mem,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&blocking_write,sizeof(cl_bool),MSG_WAITALL);
    Recv_size_t_array(clientfd,buffer_origin,3);
    Recv_size_t_array(clientfd,region,3);
    Recv_size_t(clientfd,&buffer_row_pitch);
    Recv_size_t(clientfd,&buffer_slice_pitch);
    Recv_size_t(clientfd,&host_row_pitch);
    Recv_size_t(clientfd,&host_slice_pitch);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    size_t cb = region[2]*host_slice_pitch + region[1]*host_row_pitch + region[0];
    ptr = malloc(cb);
    if(blocking_write){
        dataPack in, out;
        out.size = cb;
        out.data = ptr;
        Recv_size_t(clientfd, &(in.size));
        in.data = malloc(in.size);
        Recv(clientfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_WRITE_BUFFER_RECT;
    // ------------------------------------------------------------
    // Blocking write case:
    // We simply call to the write method.
    // ------------------------------------------------------------
    if(blocking_write == CL_TRUE){
        // We must wait manually for the events manually in order to
        // control the events generated by ocland.
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            free(event_wait_list); event_wait_list=NULL;
        }
        // Execute the command
        struct _cl_version version = clGetCommandQueueVersion(command_queue);
        if(     (version.major <  1)
            || ((version.major == 1) && (version.minor < 1))){
            // OpenCL < 1.1, so this function does not exist
            flag = CL_INVALID_COMMAND_QUEUE;
        }
        else{
            flag = clEnqueueWriteBufferRect(command_queue, mem, blocking_write,
                                            buffer_origin, host_origin, region,
                                            buffer_row_pitch, buffer_slice_pitch,
                                            host_row_pitch, host_slice_pitch,
                                            ptr, 0, NULL, &(event->event));
        }
        free(ptr);ptr=NULL;
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event); event=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
        // Answer to the client
        int send_flags = want_event ? MSG_MORE : 0;
        Send(clientfd, &flag, sizeof(cl_int), send_flags);
        if(want_event){
            Send(clientfd, &event, sizeof(ocland_event), 0);
            registerEvent(v,event);
            event->status = CL_COMPLETE;
        }
        else{
            free(event); event = NULL;
        }
        VERBOSE_OUT(flag);
        return 1;
    }
    // ------------------------------------------------------------
    // Asynchronous write case:
    // Another thread will pack the data and send it using another
    // port.
    // ------------------------------------------------------------
    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 1))){
        // OpenCL < 1.1, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = oclandEnqueueWriteBufferRect(clientfd,command_queue,mem,
                                            buffer_origin,region,
                                            buffer_row_pitch,buffer_slice_pitch,
                                            host_row_pitch,host_slice_pitch,
                                            ptr,num_events_in_wait_list,event_wait_list,
                                            want_event, event);
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(ptr); ptr=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // We can't mark the work as done, or destroy the data
    // becuase oclandEnqueueReadBuffer is using it
    if(want_event == CL_TRUE){
        registerEvent(v, event);
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueCopyBufferRect(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem src_mem;
    cl_mem dst_mem;
    size_t src_origin[3];
    size_t dst_origin[3] = {0, 0, 0};
    size_t region[3];
    size_t src_row_pitch;
    size_t src_slice_pitch;
    size_t dst_row_pitch;
    size_t dst_slice_pitch;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&src_mem,sizeof(cl_mem),MSG_WAITALL);
    Recv(clientfd,&dst_mem,sizeof(cl_mem),MSG_WAITALL);
    Recv_size_t_array(clientfd,src_origin,3);
    Recv_size_t_array(clientfd,dst_origin,3);
    Recv_size_t_array(clientfd,region,3);
    Recv_size_t(clientfd,&src_row_pitch);
    Recv_size_t(clientfd,&src_slice_pitch);
    Recv_size_t(clientfd,&dst_row_pitch);
    Recv_size_t(clientfd,&dst_slice_pitch);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag  = isBuffer(v, src_mem);
    flag |= isBuffer(v, dst_mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_COPY_BUFFER_RECT;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    struct _cl_version version = clGetEventVersion(event->event);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 1))){
        // OpenCL < 1.1, so this function does not exist
        flag = CL_INVALID_EVENT;
    }
    else{
        flag = clEnqueueCopyBufferRect(command_queue, src_mem, dst_mem,
                                        src_origin, dst_origin, region,
                                        src_row_pitch, src_slice_pitch,
                                        dst_row_pitch, dst_slice_pitch,
                                        0, NULL, &(event->event));
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

// ----------------------------------
// OpenCL 1.2
// ----------------------------------
int ocland_clCreateSubDevices(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_uint i;
    cl_device_id device_id;
    pointer device_id_ptr;
    cl_uint num_properties;
    cl_device_partition_property *properties=NULL;
    cl_uint num_entries;
    cl_int flag;
    cl_device_id *devices = NULL;
    cl_uint num_devices = 0;
    // Receive the parameters
    Recv(clientfd, &device_id_ptr, sizeof(pointer), MSG_WAITALL);
    device_id = RestorePtr(device_id_ptr);
    Recv(clientfd,&num_properties,sizeof(cl_uint),MSG_WAITALL);
    if(num_properties){
        pointer * properties_ptrs = malloc(num_properties * sizeof(pointer));
        properties = malloc(num_properties * sizeof(cl_device_partition_property));
        if (!properties || !properties_ptrs) {
            // Memory troubles!!! Disconnecting the client is a good way to
            // leave this situation without damage
            shutdown(*clientfd, 2);
            *clientfd = -1;
            free(properties); properties = NULL;
            free(properties_ptrs); properties_ptrs = NULL;
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return 1;
        }
        Recv(clientfd, properties_ptrs, num_properties * sizeof(pointer), MSG_WAITALL);
        for(i = 0; i < num_properties; i++) {
            properties[i] = (cl_device_partition_property)RestorePtr(properties_ptrs[i]);
        }
    }
    Recv(clientfd,&num_entries,sizeof(cl_uint),MSG_WAITALL);
    // Read the data from the platform
    flag = isDevice(v, device_id);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(properties); properties=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    if(num_entries){
        devices = (cl_device_id*)malloc(num_entries*sizeof(cl_device_id));
    }
    struct _cl_version version = clGetDeviceVersion(device_id);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_DEVICE;
    }
    else{
        flag = clCreateSubDevices(device_id, properties, num_entries,
                                  devices, &num_devices);
    }
    free(properties); properties=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(devices); devices=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(devices){
        Send(clientfd, &num_devices, sizeof(cl_uint), MSG_MORE);
        if(num_entries < num_devices)
            num_devices = num_entries;
        registerDevices(v, num_devices, devices);
        for(i = 0; i < num_devices; i++) {
            int flag = (i == num_devices - 1) ? 0 : MSG_MORE;
            pointer device_ptr = StorePtr(devices[i]);
            Send(clientfd, &device_ptr, sizeof(pointer), flag);
        }
    }
    else{
        Send(clientfd, &num_devices, sizeof(cl_uint), 0);
    }
    free(devices);devices=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clRetainDevice(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_device_id device;
    pointer device_ptr;
    // Receive the parameters
    Recv(clientfd, &device_ptr, sizeof(pointer), MSG_WAITALL);
    device = RestorePtr(device_ptr);
    // Execute the command
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct _cl_version version = clGetDeviceVersion(device);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_DEVICE;
    }
    else{
        flag = clRetainDevice(device);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clReleaseDevice(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_device_id device;
    pointer device_ptr;
    // Receive the parameters
    Recv(clientfd, &device_ptr, sizeof(pointer), MSG_WAITALL);
    device = RestorePtr(device_ptr);
    // Execute the command
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct _cl_version version = clGetDeviceVersion(device);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_DEVICE;
    }
    else{
        flag = clReleaseDevice(device);
    }
    if(flag == CL_SUCCESS){
        unregisterDevices(v, 1, &device);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateImage(int* clientfd, validator v)
{
    VERBOSE_IN();
    ocland_context context;
    cl_mem_flags flags;
    cl_image_format image_format;
    cl_image_desc image_desc;
    size_t element_size;
    cl_bool hasPtr;
    void* host_ptr = NULL;
    cl_int flag;
    cl_mem image = NULL;
    pointer portable_ptr;
    // Receive the parameters
    Recv(clientfd, &portable_ptr, sizeof(pointer), MSG_WAITALL);
    context = RestorePtr(portable_ptr);
    Recv(clientfd,&flags,sizeof(cl_mem_flags),MSG_WAITALL);
    Recv(clientfd,&image_format,sizeof(cl_image_format),MSG_WAITALL);

    // Receive image_desc struct in 32/64 bits portable way
    Recv(clientfd, &(image_desc.image_type), sizeof(cl_mem_object_type), MSG_WAITALL);
    Recv_size_t(clientfd, &image_desc.image_width);
    Recv_size_t(clientfd, &image_desc.image_height);
    Recv_size_t(clientfd, &image_desc.image_depth);
    Recv_size_t(clientfd, &image_desc.image_array_size);
    Recv_size_t(clientfd, &image_desc.image_row_pitch);
    Recv_size_t(clientfd, &image_desc.image_slice_pitch);
    Recv(clientfd, &(image_desc.num_mip_levels), sizeof(cl_uint), MSG_WAITALL);
    Recv(clientfd, &(image_desc.num_samples), sizeof(cl_uint), MSG_WAITALL);
    Recv(clientfd, &portable_ptr, sizeof(pointer), MSG_WAITALL);
    image_desc.buffer = RestorePtr(portable_ptr);

    Recv_size_t(clientfd,&element_size);
    Recv(clientfd,&hasPtr,sizeof(cl_bool),MSG_WAITALL);
    if(flags & CL_MEM_USE_HOST_PTR) flags -= CL_MEM_USE_HOST_PTR;
    if(flags & CL_MEM_ALLOC_HOST_PTR) flags -= CL_MEM_ALLOC_HOST_PTR;
    if(flags & CL_MEM_COPY_HOST_PTR){
        size_t size = image_desc.image_width*image_desc.image_height*image_desc.image_depth*element_size;
        host_ptr = malloc(size);
        // Receive the data compressed
        dataPack in, out;
        out.size = size;
        out.data = host_ptr;
        Recv_size_t(clientfd, &(in.size));
        in.data = malloc(in.size);
        Recv(clientfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        free(host_ptr); host_ptr=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct _cl_version version = clGetContextVersion(context->context);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_CONTEXT;
    }
    else{
        image = clCreateImage(context->context, flags, &image_format,
                              &image_desc, host_ptr, &flag);
    }
    free(host_ptr); host_ptr=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerBuffer(v, image);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &image, sizeof(cl_mem), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCreateProgramWithBuiltInKernels(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    ocland_context context;
    cl_uint num_devices;
    cl_device_id *device_list=NULL;
    size_t kernel_names_size;
    char *kernel_names=NULL;
    cl_int flag;
    cl_program program = NULL;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    Recv(clientfd,&num_devices,sizeof(cl_uint),MSG_WAITALL);
    device_list = (cl_device_id*)malloc(num_devices*sizeof(cl_device_id));
    for (i = 0; i < num_devices; i++) {
        pointer device_ptr;
        Recv(clientfd, &device_ptr, sizeof(pointer), MSG_WAITALL);
        device_list[i] = RestorePtr(device_ptr);
    }
    Recv_size_t(clientfd, &kernel_names_size);
    kernel_names = malloc(kernel_names_size);
    Recv(clientfd, kernel_names, kernel_names_size, MSG_WAITALL);
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        free(device_list); device_list=NULL;
        free(kernel_names); kernel_names=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_devices;i++){
        flag = isDevice(v, device_list[i]);
        if(flag != CL_SUCCESS){
            free(device_list); device_list=NULL;
            free(kernel_names); kernel_names=NULL;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    struct _cl_version version = clGetContextVersion(context->context);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_CONTEXT;
    }
    else{
        program = clCreateProgramWithBuiltInKernels(context->context,num_devices,device_list,
                                                    (const char*)kernel_names,
                                                    &flag);
    }
    free(device_list); device_list=NULL;
    free(kernel_names); kernel_names=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    registerProgram(v, program);
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &program, sizeof(cl_program), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clCompileProgram(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_program program;
    cl_uint num_devices;
    cl_device_id *device_list=NULL;
    size_t str_size;
    char *options=NULL;
    cl_uint num_input_headers;
    cl_program *input_headers = NULL;
    char **header_include_names = NULL;
    cl_int flag;
    // Receive the parameters
    Recv(clientfd,&program,sizeof(cl_program),MSG_WAITALL);
    Recv(clientfd,&num_devices,sizeof(cl_uint),MSG_WAITALL);
    device_list = (cl_device_id*)malloc(num_devices*sizeof(cl_device_id));
    for(i = 0; i < num_devices; i++) {
        pointer device_ptr;
        Recv(clientfd, &device_ptr, sizeof(pointer), MSG_WAITALL);
        device_list[i] = RestorePtr(device_ptr);
    }

    Recv_size_t(clientfd, &str_size);
    options = (char*)malloc(str_size);
    Recv(clientfd,options,str_size,MSG_WAITALL);
    Recv(clientfd,&num_input_headers,sizeof(cl_uint),MSG_WAITALL);
    if(num_input_headers){
        input_headers = (cl_program*)malloc(num_input_headers*sizeof(cl_program));
        header_include_names = (char**)malloc(num_input_headers*sizeof(char*));
        Recv(clientfd,input_headers,num_input_headers*sizeof(cl_program),MSG_WAITALL);
        for(i=0;i<num_input_headers;i++){
            Recv_size_t(clientfd, &str_size);
            header_include_names[i] = (char*)malloc(str_size);
            Recv(clientfd,header_include_names[i],str_size,MSG_WAITALL);
        }
    }
    // Execute the command
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        free(options); options=NULL;
        free(input_headers); input_headers=NULL;
        for(i=0;i<num_input_headers;i++){
            free(header_include_names[i]); header_include_names[i]=NULL;
        }
        free(header_include_names); header_include_names=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_devices;i++){
        flag = isDevice(v, device_list[i]);
        if(flag != CL_SUCCESS){
            free(options); options=NULL;
            free(input_headers); input_headers=NULL;
            for(i=0;i<num_input_headers;i++){
                free(header_include_names[i]); header_include_names[i]=NULL;
            }
            free(header_include_names); header_include_names=NULL;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    for(i=0;i<num_input_headers;i++){
        flag = isProgram(v, input_headers[i]);
        if(flag != CL_SUCCESS){
            free(options); options=NULL;
            free(input_headers); input_headers=NULL;
            for(i=0;i<num_input_headers;i++){
                free(header_include_names[i]); header_include_names[i]=NULL;
            }
            free(header_include_names); header_include_names=NULL;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    struct _cl_version version = clGetProgramVersion(program);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_PROGRAM;
    }
    else{
        flag = clCompileProgram(program,num_devices,device_list,options,
                                num_input_headers,input_headers,
                                (const char**)header_include_names,NULL,NULL);
    }
    free(options); options=NULL;
    free(input_headers); input_headers=NULL;
    for(i=0;i<num_input_headers;i++){
        free(header_include_names[i]); header_include_names[i]=NULL;
    }
    free(header_include_names); header_include_names=NULL;
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clLinkProgram(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    ocland_context context;
    cl_uint num_devices;
    cl_device_id *device_list=NULL;
    size_t str_size;
    char *options=NULL;
    cl_uint num_input_programs;
    cl_program *input_programs = NULL;
    cl_int flag;
    cl_program program=NULL;
    // Receive the parameters
    Recv(clientfd,&context,sizeof(ocland_context),MSG_WAITALL);
    Recv(clientfd,&num_devices,sizeof(cl_uint),MSG_WAITALL);
    device_list = (cl_device_id*)malloc(num_devices*sizeof(cl_device_id));
    for(i = 0; i < num_devices; i++) {
        pointer device_ptr;
        Recv(clientfd, &device_ptr, sizeof(pointer), MSG_WAITALL);
        device_list[i] = RestorePtr(device_ptr);
    }
    Recv_size_t(clientfd, &str_size);
    options = (char*)malloc(str_size);
    Recv(clientfd,options,str_size,MSG_WAITALL);
    Recv(clientfd,&num_input_programs,sizeof(cl_uint),MSG_WAITALL);
    if(num_input_programs){
        input_programs = (cl_program*)malloc(num_input_programs*sizeof(cl_program));
        Recv(clientfd,input_programs,num_input_programs*sizeof(cl_program),MSG_WAITALL);
    }
    // Execute the command
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        free(options); options=NULL;
        free(input_programs); input_programs=NULL;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_devices;i++){
        flag = isDevice(v, device_list[i]);
        if(flag != CL_SUCCESS){
            free(options); options=NULL;
            free(input_programs); input_programs=NULL;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    for(i=0;i<num_input_programs;i++){
        flag = isProgram(v, input_programs[i]);
        if(flag != CL_SUCCESS){
            free(options); options=NULL;
            free(input_programs); input_programs=NULL;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    struct _cl_version version = clGetContextVersion(context->context);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_CONTEXT;
    }
    else{
        program = clLinkProgram(context->context,num_devices,device_list,
                                options,num_input_programs,
                                input_programs,NULL,NULL,&flag);
    }
    free(options); options=NULL;
    free(input_programs); input_programs=NULL;
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    Send(clientfd, &program, sizeof(cl_program), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clUnloadPlatformCompiler(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_int flag;
    cl_platform_id platform;
    pointer platform_ptr;
    // Receive the parameters
    Recv(clientfd, &platform_ptr, sizeof(pointer), MSG_WAITALL);
    platform = RestorePtr(platform_ptr);
    // Execute the command
    flag = isPlatform(v, platform);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    struct _cl_version version = clGetPlatformVersion(platform);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_PLATFORM;
    }
    else{
        flag = clUnloadPlatformCompiler(platform);
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), 0);
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clGetKernelArgInfo(int* clientfd, validator v)
{
    VERBOSE_IN();
    cl_kernel kernel = NULL;
    cl_uint arg_index = 0;
    cl_kernel_arg_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    // Receive the parameters
    Recv(clientfd,&kernel,sizeof(cl_kernel),MSG_WAITALL);
    Recv(clientfd,&arg_index,sizeof(cl_uint),MSG_WAITALL);
    Recv(clientfd,&param_name,sizeof(cl_kernel_arg_info),MSG_WAITALL);
    Recv_size_t(clientfd,&param_value_size);
    // Execute the command
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        VERBOSE_OUT(flag);
        return 1;
    }
    if(param_value_size)
        param_value = malloc(param_value_size);
    struct _cl_version version = clGetKernelVersion(kernel);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_KERNEL;
    }
    else{
        flag = clGetKernelArgInfo(kernel,arg_index,param_name,
                                  param_value_size,param_value,
                                  &param_value_size_ret);
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(param_value); param_value=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(param_value){
        Send_size_t(clientfd, param_value_size_ret, MSG_MORE);
        Send(clientfd, param_value, param_value_size_ret, 0);
    }
    else{
        Send_size_t(clientfd, param_value_size_ret, 0);
    }
    free(param_value);param_value=NULL;
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueFillBuffer(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem mem;
    size_t pattern_size;
    void *pattern;
    size_t offset;
    size_t cb;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&mem,sizeof(cl_mem),MSG_WAITALL);
    Recv_size_t(clientfd,&pattern_size);
    pattern = malloc(pattern_size);
    Recv(clientfd,pattern,pattern_size,MSG_WAITALL);
    Recv_size_t(clientfd,&offset);
    Recv_size_t(clientfd,&cb);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag  = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_FILL_BUFFER;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = clEnqueueFillBuffer(command_queue,mem,
                                   pattern,pattern_size,
                                   offset,cb,0,NULL,&(event->event));
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueFillImage(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem image;
    size_t fill_color_size;
    const void *fill_color = NULL;
    size_t origin[3];
    size_t region[3];
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&image,sizeof(cl_mem),MSG_WAITALL);
    Recv_size_t(clientfd,&fill_color_size);
    fill_color = malloc(fill_color_size);
    Recv(clientfd,(void *)fill_color,fill_color_size,MSG_WAITALL);
    Recv_size_t_array(clientfd,origin,3);
    Recv_size_t_array(clientfd,region,3);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    flag  = isBuffer(v, image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_FILL_IMAGE;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = clEnqueueFillImage(command_queue,image,
                                  fill_color,
                                  origin,region,
                                  0,NULL,&(event->event));
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueMigrateMemObjects(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_uint num_mem_objects;
    cl_mem *mem_objects=NULL;
    cl_mem_migration_flags flags;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&num_mem_objects, sizeof(cl_uint), MSG_WAITALL);
    mem_objects = (void*)malloc(num_mem_objects*sizeof(cl_mem));
    Recv(clientfd,mem_objects, num_mem_objects*sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd,&flags, sizeof(cl_mem_migration_flags), MSG_WAITALL);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_mem_objects;i++){
        flag  = isBuffer(v, mem_objects[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_MIGRATE_MEM_OBJECTS;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = clEnqueueMigrateMemObjects(command_queue,num_mem_objects,
                                          mem_objects,flags,
                                          0,NULL,&(event->event));
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueMarkerWithWaitList(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_MARKER;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = clEnqueueMarkerWithWaitList(command_queue,0,NULL,&(event->event));
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

int ocland_clEnqueueBarrierWithWaitList(int* clientfd, validator v)
{
    VERBOSE_IN();
    unsigned int i;
    cl_context context;
    cl_command_queue command_queue;
    cl_uint num_events_in_wait_list;
    ocland_event *event_wait_list = NULL;
    cl_bool want_event;
    cl_int flag;
    ocland_event event = NULL;
    // Receive the parameters
    Recv(clientfd,&command_queue,sizeof(cl_command_queue),MSG_WAITALL);
    Recv(clientfd,&want_event,sizeof(cl_bool),MSG_WAITALL);
    Recv(clientfd,&num_events_in_wait_list,sizeof(cl_uint),MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        Recv(clientfd,event_wait_list,num_events_in_wait_list*sizeof(ocland_event),MSG_WAITALL);
    }
    // Ensure the provided data validity
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            free(event_wait_list); event_wait_list=NULL;
            VERBOSE_OUT(flag);
            return 1;
        }
    }
    // Build the event and the data array
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event_wait_list); event_wait_list=NULL;
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    event->event         = NULL;
    event->status        = CL_SUBMITTED;
    event->context       = context;
    event->command_queue = command_queue;
    event->command_type  = CL_COMMAND_BARRIER;
    //! @todo The events waiting and the method calling must be done asynchronously
    // We must wait manually for the events manually in order to
    // control the events generated by ocland.
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        free(event_wait_list); event_wait_list=NULL;
    }
    // Execute the command
    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = clEnqueueBarrierWithWaitList(command_queue,0,NULL,&(event->event));
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        free(event); event=NULL;
        VERBOSE_OUT(flag);
        return 1;
    }
    // Answer to the client
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
        registerEvent(v,event);
        event->status = CL_COMPLETE;
    }
    else{
        free(event); event = NULL;
    }
    VERBOSE_OUT(flag);
    return 1;
}

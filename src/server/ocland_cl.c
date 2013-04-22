/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2012  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ocland.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include <CL/cl.h>
#include <CL/cl_ext.h>

#include <ocland/common/dataExchange.h>
#include <ocland/server/ocland_cl.h>

#ifndef OCLAND_PORT
    #define OCLAND_PORT 51000u
#endif

#ifndef BUFF_SIZE
    #define BUFF_SIZE 1025u
#endif

int ocland_clGetPlatformIDs(int* clientfd, char* buffer, validator v, void* data)
{
    cl_uint num_entries;
    cl_int flag;
    cl_uint num_platforms = 0, n = 0;
    cl_platform_id *platforms = NULL;
    // Decript the received data
    num_entries = ((cl_uint*)data)[0];
    if(num_entries)
        platforms = (cl_platform_id*)malloc(num_entries*sizeof(cl_platform_id));
    flag = clGetPlatformIDs(num_entries, platforms, &num_platforms);
    // Build the package to send
    size_t msgSize  = sizeof(cl_int);                       // flag
    msgSize        += sizeof(cl_uint);                      // num_platforms
    msgSize        += num_platforms*sizeof(cl_platform_id); // platforms
    void* msg = (void*)malloc(msgSize);
    void* ptr = msg;
    ((cl_int*)ptr)[0]  = flag;          ptr = (cl_int*)ptr  + 1;
    ((cl_uint*)ptr)[0] = num_platforms; ptr = (cl_uint*)ptr + 1;
    n = (num_platforms < num_entries) ? num_platforms : num_entries;
    if(n)
        memcpy(ptr, (void*)platforms, n*sizeof(cl_platform_id));
    // Send the package (first the size, then the data)
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    if(msg) free(msg); msg=NULL;
    if(platforms) free(platforms); platforms=NULL;
    return 1;
}

int ocland_clGetPlatformInfo(int* clientfd, char* buffer, validator v, void* data)
{
    cl_int flag;
    cl_platform_id platform;
    cl_platform_info param_name;
    size_t param_value_size, param_value_size_ret=0;
    void *param_value = NULL;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    platform         = ((cl_platform_id*)data)[0];   data = (cl_platform_id*)data + 1;
    param_name       = ((cl_platform_info*)data)[0]; data = (cl_platform_info*)data + 1;
    param_value_size = ((size_t*)data)[0];
    // Ensure that platform is valid
    flag = isPlatform(v, platform);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);  // flag
        msgSize += sizeof(size_t);  // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_uint*)ptr)[0] = 0;    ptr = (cl_uint*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // For security we will look for param_value_size_ret first
    flag = clGetPlatformInfo(platform, param_name, 0, NULL, &param_value_size_ret);
    if(param_value_size && (param_value_size < param_value_size_ret)){
        flag     = CL_INVALID_VALUE;
        msgSize  = sizeof(cl_int);  // flag
        msgSize += sizeof(size_t);  // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_uint*)ptr)[0] = 0;    ptr = (cl_uint*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Get platform requested info
    param_value = (void*)malloc(param_value_size_ret);
    flag = clGetPlatformInfo(platform, param_name, param_value_size_ret, param_value, &param_value_size_ret);
    if( (param_name == CL_PLATFORM_NAME) ||
        (param_name == CL_PLATFORM_VENDOR) ||
        (param_name == CL_PLATFORM_ICD_SUFFIX_KHR) ){
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getsockname(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        param_value_size_ret += (9+strlen(inet_ntoa(adr_inet.sin_addr)))*sizeof(char);
        char *edited = (char*)malloc(param_value_size_ret);
        strcpy(edited, "ocland(");
        strcat(edited, inet_ntoa(adr_inet.sin_addr));
        strcat(edited, ") ");
        strcat(edited, (char*)param_value);
        free(param_value);
        param_value = edited;
    }
    msgSize  = sizeof(cl_int);       // flag
    msgSize += sizeof(size_t);       // param_value_size_ret
    msgSize += param_value_size_ret; // param_value
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0] = flag;                 ptr = (cl_int*)ptr + 1;
    ((size_t*)ptr)[0] = param_value_size_ret; ptr = (size_t*)ptr + 1;
    memcpy(ptr, param_value, param_value_size_ret);
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    free(param_value);param_value=NULL;
    return 1;
}

int ocland_clGetDeviceIDs(int *clientfd, char* buffer, validator v, void* data)
{
    cl_platform_id platform;
    cl_device_type device_type;
    cl_uint num_entries;
    cl_int flag;
    cl_device_id *devices = NULL;
    cl_uint num_devices = 0, n = 0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    platform    = ((cl_platform_id*)data)[0]; data = (cl_platform_id*)data + 1;
    device_type = ((cl_device_type*)data)[0]; data = (cl_device_type*)data + 1;
    num_entries = ((cl_uint*)data)[0];
    if(num_entries)
        devices = (cl_device_id*)malloc(num_entries*sizeof(cl_device_id));
    // Ensure that platform is valid
    flag = isPlatform(v, platform);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);  // flag
        msgSize += sizeof(cl_uint); // num_devices
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_uint*)ptr)[0] = 0;    ptr = (cl_uint*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clGetDeviceIDs(platform, device_type, num_entries, devices, &num_devices);
    if( devices && (flag == CL_SUCCESS) )
        registerDevices(v, num_devices, devices);
    // Build the package to send
    msgSize  = sizeof(cl_int);                   // flag
    msgSize += sizeof(cl_uint);                  // num_devices
    msgSize += num_devices*sizeof(cl_device_id); // devices
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;        ptr = (cl_int*)ptr  + 1;
    ((cl_uint*)ptr)[0] = num_devices; ptr = (cl_uint*)ptr + 1;
    n = (num_devices < num_entries) ? num_devices : num_entries;
    if(n)
        memcpy(ptr, (void*)devices, n*sizeof(cl_device_id));
    // Send the package (first the size, then the data)
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    if(msg) free(msg); msg=NULL;
    if(devices) free(devices); devices=NULL;
    return 1;
}

int ocland_clGetDeviceInfo(int* clientfd, char* buffer, validator v, void* data)
{
    cl_device_id device;
    cl_device_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value = NULL;
    size_t *param_value_size_ret = 0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    device           = ((cl_device_id*)data)[0]; data = (cl_device_id*)data + 1;
    param_name       = ((cl_device_info*)data)[0]; data = (cl_device_info*)data + 1;
    param_value_size = ((size_t*)data)[0];
    // Ensure that the device is valid
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);  // flag
        msgSize += sizeof(size_t);  // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_uint*)ptr)[0] = 0;    ptr = (cl_uint*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    flag = clGetDeviceInfo(device, param_name, param_value_size, param_value, &param_value_size_ret);
    // Build the package to send
    msgSize  = sizeof(cl_int);       // flag
    msgSize += sizeof(size_t);       // param_value_size_ret
    msgSize += param_value_size_ret; // param_value
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0] = flag;                 ptr = (cl_int*)ptr + 1;
    ((size_t*)ptr)[0] = param_value_size_ret; ptr = (size_t*)ptr + 1;
    memcpy(ptr, param_value, param_value_size_ret);
    // Send the package (first the size, then the data)
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    if(msg) free(msg); msg=NULL;
    if(param_value) free(param_value); param_value=NULL;
    return 1;
}

int ocland_clCreateContext(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_uint num_properties = 0;
    cl_context_properties *properties = NULL;
 	cl_uint num_devices = 0;
  	cl_device_id *devices = NULL;
    // pfn_notify is not supported
    cl_int flag;
    cl_context context = NULL;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    num_properties = ((cl_uint*)data)[0]; data = (cl_uint*)data + 1;
    if(num_properties){
        properties = (cl_context_properties*)malloc(num_properties*sizeof(cl_context_properties));
        for(i=0;i<num_properties;i++){
            properties[i] = ((cl_context_properties*)data)[0]; data = (cl_context_properties*)data + 1;
        }
    }
    num_devices = ((cl_uint*)data)[0]; data = (cl_uint*)data + 1;
    if(num_devices){
        devices = (cl_device_id*)malloc(num_devices*sizeof(cl_device_id));
        for(i=0;i<num_devices;i++){
            devices[i] = ((cl_device_id*)data)[0]; data = (cl_device_id*)data + 1;
        }
    }
    // Ensure that the platform provided in the properties is valid
    for(i=0;i<num_properties;i=i+2){
        if(!properties[i]){
            break;
        }
        if(properties[i] == CL_CONTEXT_PLATFORM){
            flag = isPlatform(v, (cl_platform_id)properties[i+1]);
            if(flag != CL_SUCCESS){
                msgSize  = sizeof(cl_int);      // flag
                msgSize += sizeof(cl_context);  // context
                msg      = (void*)malloc(msgSize);
                ptr      = msg;
                ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
                ((cl_context*)ptr)[0] = context;
                Send(clientfd, &msgSize, sizeof(size_t), 0);
                Send(clientfd, msg, msgSize, 0);
                free(properties);properties=NULL;
                free(devices);devices=NULL;
                free(msg);msg=NULL;
                return 1;
            }
        }
    }
    // Ensure that the devices provided are valid
    for(i=0;i<num_devices;i=i++){
        flag = isDevice(v, devices[i]);
        if(flag != CL_SUCCESS){
            msgSize  = sizeof(cl_int);      // flag
            msgSize += sizeof(cl_context);  // context
            msg      = (void*)malloc(msgSize);
            ptr      = msg;
            ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
            ((cl_context*)ptr)[0] = context;
            Send(clientfd, &msgSize, sizeof(size_t), 0);
            Send(clientfd, msg, msgSize, 0);
            free(properties);properties=NULL;
            free(devices);devices=NULL;
            free(msg);msg=NULL;
            return 1;
        }
    }
    // Create the context
    context = clCreateContext(properties, num_devices, devices, NULL, NULL, &flag);
    if(flag == CL_SUCCESS){
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getsockname(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        printf("%s has built a context\n", inet_ntoa(adr_inet.sin_addr));
        // Register the new context
        registerContext(v,context);
    }
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msgSize += sizeof(cl_context);  // context
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
    ((cl_context*)ptr)[0] = context;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(properties);properties=NULL;
    free(devices);devices=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clCreateContextFromType(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_uint num_properties = 0;
    cl_context_properties *properties = NULL;
    cl_device_type device_type;
    // pfn_notify is not supported
    cl_int flag;
    cl_context context = NULL;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    num_properties = ((cl_uint*)data)[0]; data = (cl_uint*)data + 1;
    if(num_properties){
        properties = (cl_context_properties*)malloc(num_properties*sizeof(cl_context_properties));
        for(i=0;i<num_properties;i++){
            properties[i] = ((cl_context_properties*)data)[0]; data = (cl_context_properties*)data + 1;
        }
    }
    device_type = ((cl_device_type*)data)[0]; data = (cl_device_type*)data + 1;
    // Ensure that the platform provided in the properties is valid
    for(i=0;i<num_properties;i=i+2){
        if(!properties[i]){
            break;
        }
        if(properties[i] == CL_CONTEXT_PLATFORM){
            flag = isPlatform(v, (cl_platform_id)properties[i+1]);
            if(flag != CL_SUCCESS){
                msgSize  = sizeof(cl_int);      // flag
                msgSize += sizeof(cl_context);  // context
                msg      = (void*)malloc(msgSize);
                ptr      = msg;
                ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
                ((cl_context*)ptr)[0] = context;
                Send(clientfd, &msgSize, sizeof(size_t), 0);
                Send(clientfd, msg, msgSize, 0);
                free(properties);properties=NULL;
                free(msg);msg=NULL;
                return 1;
            }
        }
    }
    // Create the context
    context = clCreateContextFromType(properties, device_type, NULL, NULL, &flag);
    if(flag == CL_SUCCESS){
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getsockname(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        printf("%s has built a context\n", inet_ntoa(adr_inet.sin_addr));
        // Register the new context
        registerContext(v,context);
    }
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msgSize += sizeof(cl_context);  // context
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
    ((cl_context*)ptr)[0] = context;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(properties);properties=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clRetainContext(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_context context = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    context = ((cl_context*)data)[0];
    // Ensure that the context is valid
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clRetainContext(context);
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clReleaseContext(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_context context = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    context = ((cl_context*)data)[0];
    // Ensure that the context is valid
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clReleaseContext(context);
    if(flag == CL_SUCCESS){
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getsockname(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        printf("%s has released a context\n", inet_ntoa(adr_inet.sin_addr));
        // unregister the context
        unregisterContext(v,context);
    }
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clGetContextInfo(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_context context = NULL;
    cl_context_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    context = ((cl_context*)data)[0];         data = (cl_context*)data + 1;
    param_name = ((cl_context_info*)data)[0]; data = (cl_context_info*)data + 1;
    param_value_size = ((size_t*)data)[0];    data = (size_t*)data + 1;
    // Ensure that the context is valid
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(size_t);      // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
        ((size_t*)ptr)[0]  = 0;    ptr = (size_t*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Build the required param_value
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    // Get the data
    flag = clGetContextInfo(context, param_name, param_value_size, param_value, &param_value_size_ret);
    // Return the package
    msgSize  = sizeof(cl_int);       // flag
    msgSize += sizeof(size_t);       // param_value_size_ret
    msgSize += param_value_size_ret; // param_value
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
    ((size_t*)ptr)[0]  = param_value_size_ret;    ptr = (size_t*)ptr + 1;
    if(param_value)
        memcpy(ptr, param_value, param_value_size_ret);
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(param_value);param_value=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clCreateCommandQueue(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_context context;
    cl_device_id device;
    cl_command_queue_properties properties;
    cl_int flag;
    cl_command_queue command_queue = NULL;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    context = ((cl_context*)data)[0];     data = (cl_context*)data + 1;
    device  = ((cl_device_id*)data)[0];   data = (cl_device_id*)data + 1;
    properties = ((cl_command_queue_properties*)data)[0]; data = (cl_command_queue_properties*)data + 1;
    // Ensure that context and device are valid
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);            // flag
        msgSize += sizeof(cl_command_queue);  // command_queue
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_command_queue*)ptr)[0] = command_queue;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(properties);properties=NULL;
        free(msg);msg=NULL;
        return 1;
    }
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);            // flag
        msgSize += sizeof(cl_command_queue);  // command_queue
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_command_queue*)ptr)[0] = command_queue;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(properties);properties=NULL;
        free(msg);msg=NULL;
        return 1;
    }
    // Create the command queue
    command_queue = clCreateCommandQueue(context, device, properties, &flag);
    if(flag == CL_SUCCESS){
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getsockname(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        printf("%s has built a command queue\n", inet_ntoa(adr_inet.sin_addr));
        // Register new command queue
        registerQueue(v, command_queue);
    }
    // Return the package
    msgSize  = sizeof(cl_int);            // flag
    msgSize += sizeof(cl_command_queue);  // command_queue
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
    ((cl_command_queue*)ptr)[0] = command_queue;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(properties);properties=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clRetainCommandQueue(int* clientfd, char* buffer, validator v, void* data)
{
    cl_command_queue command_queue = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    command_queue = ((cl_command_queue*)data)[0];
    // Ensure that the context is valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clRetainCommandQueue(command_queue);
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clReleaseCommandQueue(int* clientfd, char* buffer, validator v, void* data)
{
    cl_command_queue command_queue = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    command_queue = ((cl_command_queue*)data)[0];
    // Ensure that the context is valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clReleaseCommandQueue(command_queue);
    if(flag == CL_SUCCESS){
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getsockname(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        printf("%s has released a command queue\n", inet_ntoa(adr_inet.sin_addr));
        // unregister the command_queue
        unregisterQueue(v,command_queue);
    }
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clGetCommandQueueInfo(int* clientfd, char* buffer, validator v, void* data)
{
    cl_command_queue command_queue = NULL;
    cl_command_queue_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    command_queue = ((cl_command_queue*)data)[0];   data = (cl_command_queue*)data + 1;
    param_name = ((cl_command_queue_info*)data)[0]; data = (cl_command_queue_info*)data + 1;
    param_value_size = ((size_t*)data)[0];    data = (size_t*)data + 1;
    // Ensure that the command_queue is valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(size_t);      // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
        ((size_t*)ptr)[0]  = 0;    ptr = (size_t*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Build the required param_value
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    // Get the data
    flag = clGetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, &param_value_size_ret);
    // Return the package
    msgSize  = sizeof(cl_int);       // flag
    msgSize += sizeof(size_t);       // param_value_size_ret
    msgSize += param_value_size_ret; // param_value
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
    ((size_t*)ptr)[0]  = param_value_size_ret;    ptr = (size_t*)ptr + 1;
    if(param_value)
        memcpy(ptr, param_value, param_value_size_ret);
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(param_value);param_value=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clCreateBuffer(int* clientfd, char* buffer, validator v, void* data)
{
    cl_context context;
    cl_mem_flags flags;
    size_t size;
    cl_bool hasPtr;
    void* host_ptr = NULL;
    cl_int flag;
    cl_mem memobj = NULL;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    context = ((cl_context*)data)[0];     data = (cl_context*)data + 1;
    flags   = ((cl_mem_flags*)data)[0];   data = (cl_mem_flags*)data + 1;
    size    = ((size_t*)data)[0];         data = (size_t*)data + 1;
    hasPtr  = ((cl_bool*)data)[0];        data = (cl_bool*)data + 1;
    if(hasPtr)
        host_ptr = data;
    // Ensure that the context is valid
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);  // flag
        msgSize += sizeof(cl_mem);  // memobj
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0] = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_mem*)ptr)[0] = memobj;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Create the command queue
    memobj = clCreateBuffer(context, flags, size, host_ptr, &flag);
    if(flag == CL_SUCCESS){
        registerBuffer(v, memobj);
    }
    // Return the package
    msgSize  = sizeof(cl_int);            // flag
    msgSize += sizeof(cl_mem);  // memobj
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0] = flag; ptr = (cl_int*)ptr  + 1;
    ((cl_mem*)ptr)[0] = memobj;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clRetainMemObject(int* clientfd, char* buffer, validator v, void* data)
{
    cl_mem memobj = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    memobj = ((cl_mem*)data)[0];
    // Ensure that the context is valid
    flag = isBuffer(v, memobj);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clRetainMemObject(memobj);
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clReleaseMemObject(int* clientfd, char* buffer, validator v, void* data)
{
    cl_mem memobj = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    memobj = ((cl_mem*)data)[0];
    // Ensure that the context is valid
    flag = isBuffer(v, memobj);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clReleaseMemObject(memobj);
    if(flag == CL_SUCCESS){
        unregisterBuffer(v,memobj);
    }
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clGetSupportedImageFormats(int* clientfd, char* buffer, validator v, void* data)
{
    cl_context context;
    cl_mem_flags flags;
    cl_mem_object_type image_type;
    cl_uint num_entries;
    cl_int flag;
    cl_image_format *image_formats = NULL;
    cl_uint num_image_formats = 0, n = 0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    context     = ((cl_context*)data)[0]; data = (cl_context*)data + 1;
    flags       = ((cl_mem_flags*)data)[0]; data = (cl_mem_flags*)data + 1;
    image_type  = ((cl_mem_object_type*)data)[0]; data = (cl_mem_object_type*)data + 1;
    num_entries = ((cl_uint*)data)[0];
    if(num_entries)
        image_formats = (cl_image_format*)malloc(num_entries*sizeof(cl_image_format));
    // Ensure that context is valid
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);  // flag
        msgSize += sizeof(cl_uint); // num_image_formats
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_uint*)ptr)[0] = 0;    ptr = (cl_uint*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag =  clGetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, &num_image_formats);
    // Build the package to send
    msgSize  = sizeof(cl_int);                            // flag
    msgSize += sizeof(cl_uint);                           // num_image_formats
    msgSize += num_image_formats*sizeof(cl_image_format); // image_formats
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;        ptr = (cl_int*)ptr  + 1;
    ((cl_uint*)ptr)[0] = num_image_formats; ptr = (cl_uint*)ptr + 1;
    n = (num_image_formats < num_entries) ? num_image_formats : num_entries;
    if(n)
        memcpy(ptr, (void*)image_formats, n*sizeof(cl_image_format));
    // Send the package (first the size, then the data)
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    if(msg) free(msg); msg=NULL;
    if(image_formats) free(image_formats); image_formats=NULL;
    return 1;
}

int ocland_clGetMemObjectInfo(int* clientfd, char* buffer, validator v, void* data)
{
    cl_mem memobj = NULL;
    cl_mem_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    memobj           = ((cl_mem*)data)[0];      data = (cl_mem*)data + 1;
    param_name       = ((cl_mem_info*)data)[0]; data = (cl_mem_info*)data + 1;
    param_value_size = ((size_t*)data)[0];      data = (size_t*)data + 1;
    // Ensure that the memory object is valid
    flag = isBuffer(v, memobj);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(size_t);      // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
        ((size_t*)ptr)[0]  = 0;    ptr = (size_t*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Build the required param_value
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    // Get the data
    flag = clGetMemObjectInfo(memobj, param_name, param_value_size, param_value, &param_value_size_ret);
    // Return the package
    msgSize  = sizeof(cl_int);       // flag
    msgSize += sizeof(size_t);       // param_value_size_ret
    msgSize += param_value_size_ret; // param_value
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
    ((size_t*)ptr)[0]  = param_value_size_ret;    ptr = (size_t*)ptr + 1;
    if(param_value)
        memcpy(ptr, param_value, param_value_size_ret);
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(param_value);param_value=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clGetImageInfo(int* clientfd, char* buffer, validator v, void* data)
{
    cl_mem image = NULL;
    cl_image_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    image            = ((cl_mem*)data)[0];        data = (cl_mem*)data + 1;
    param_name       = ((cl_image_info*)data)[0]; data = (cl_image_info*)data + 1;
    param_value_size = ((size_t*)data)[0];        data = (size_t*)data + 1;
    // Ensure that the memory object is valid
    flag = isBuffer(v, image);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(size_t);      // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
        ((size_t*)ptr)[0]  = 0;    ptr = (size_t*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Build the required param_value
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    // Get the data
    flag = clGetImageInfo(image, param_name, param_value_size, param_value, &param_value_size_ret);
    // Return the package
    msgSize  = sizeof(cl_int);       // flag
    msgSize += sizeof(size_t);       // param_value_size_ret
    msgSize += param_value_size_ret; // param_value
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
    ((size_t*)ptr)[0]  = param_value_size_ret;    ptr = (size_t*)ptr + 1;
    if(param_value)
        memcpy(ptr, param_value, param_value_size_ret);
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(param_value);param_value=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clCreateSampler(int* clientfd, char* buffer, validator v, void* data)
{
    cl_context context;
    cl_bool normalized_coords;
    cl_addressing_mode addressing_mode;
    cl_filter_mode filter_mode;
    cl_int flag;
    cl_sampler sampler = NULL;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    context           = ((cl_context*)data)[0];         data = (cl_context*)data + 1;
    normalized_coords = ((cl_bool*)data)[0];            data = (cl_bool*)data + 1;
    addressing_mode   = ((cl_addressing_mode*)data)[0]; data = (cl_addressing_mode*)data + 1;
    filter_mode       = ((cl_filter_mode*)data)[0];     data = (cl_filter_mode*)data + 1;
    // Ensure that the context is valid
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(cl_sampler);  // sampler
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]     = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_sampler*)ptr)[0] = sampler;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Create the command queue
    sampler = clCreateSampler(context, normalized_coords, addressing_mode, filter_mode, &flag);
    if(flag == CL_SUCCESS){
        registerSampler(v, sampler);
    }
    // Return the package
    msgSize  = sizeof(cl_int);     // flag
    msgSize += sizeof(cl_sampler); // sampler
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]     = flag; ptr = (cl_int*)ptr  + 1;
    ((cl_sampler*)ptr)[0] = sampler;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clRetainSampler(int* clientfd, char* buffer, validator v, void* data)
{
    cl_sampler sampler = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    sampler = ((cl_sampler*)data)[0];
    // Ensure that the context is valid
    flag = isSampler(v, sampler);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clRetainSampler(sampler);
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clReleaseSampler(int* clientfd, char* buffer, validator v, void* data)
{
    cl_sampler sampler = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    sampler = ((cl_sampler*)data)[0];
    // Ensure that the context is valid
    flag = isSampler(v, sampler);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clReleaseSampler(sampler);
    if(flag == CL_SUCCESS){
        unregisterSampler(v,sampler);
    }
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clGetSamplerInfo(int* clientfd, char* buffer, validator v, void* data)
{
    cl_sampler sampler = NULL;
    cl_sampler_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    sampler          = ((cl_sampler*)data)[0];      data = (cl_sampler*)data + 1;
    param_name       = ((cl_sampler_info*)data)[0]; data = (cl_sampler_info*)data + 1;
    param_value_size = ((size_t*)data)[0];          data = (size_t*)data + 1;
    // Ensure that the sampler is valid
    flag = isSampler(v, sampler);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(size_t);      // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
        ((size_t*)ptr)[0]  = 0;    ptr = (size_t*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Build the required param_value
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    // Get the data
    flag = clGetSamplerInfo(sampler, param_name, param_value_size, param_value, &param_value_size_ret);
    // Return the package
    msgSize  = sizeof(cl_int);       // flag
    msgSize += sizeof(size_t);       // param_value_size_ret
    msgSize += param_value_size_ret; // param_value
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
    ((size_t*)ptr)[0]  = param_value_size_ret;    ptr = (size_t*)ptr + 1;
    if(param_value)
        memcpy(ptr, param_value, param_value_size_ret);
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(param_value);param_value=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clCreateProgramWithSource(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_context context;
    cl_uint count;
    size_t *lengths = NULL;
    char **strings = NULL;
    cl_int flag;
    cl_program program = NULL;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    context = ((cl_context*)data)[0]; data = (cl_context*)data + 1;
    count   = ((cl_uint*)data)[0];    data = (cl_uint*)data + 1;
    lengths = (size_t*)malloc(count * sizeof(size_t));
    strings = (char**)malloc(count * sizeof(char*));
    if(!lengths || !strings){
        flag     = CL_OUT_OF_HOST_MEMORY;
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(cl_program);  // program
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]     = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_program*)ptr)[0] = program;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    memcpy(lengths, data, count * sizeof(size_t));
    data = (size_t*)data + count;
    for(i=0;i<count;i++){
        strings[i] = (char*)malloc(lengths[i]*sizeof(char));
        if(!strings[i]){
            flag     = CL_OUT_OF_HOST_MEMORY;
            msgSize  = sizeof(cl_int);      // flag
            msgSize += sizeof(cl_program);  // program
            msg      = (void*)malloc(msgSize);
            ptr      = msg;
            ((cl_int*)ptr)[0]     = flag; ptr = (cl_int*)ptr  + 1;
            ((cl_program*)ptr)[0] = program;
            Send(clientfd, &msgSize, sizeof(size_t), 0);
            Send(clientfd, msg, msgSize, 0);
            free(msg);msg=NULL;
            return 1;
        }
        memcpy(strings[i], data, lengths[i]*sizeof(char));
        data = (char*)data + lengths[i];
    }
    // Ensure that the context is valid
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(cl_program);  // program
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]     = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_program*)ptr)[0] = program;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Create the program
    program = clCreateProgramWithSource(context, count, strings, lengths, &flag);
    if(flag == CL_SUCCESS){
        registerProgram(v, program);
    }
    // Return the package
    msgSize  = sizeof(cl_int);     // flag
    msgSize += sizeof(cl_program); // program
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]     = flag;    ptr = (cl_int*)ptr  + 1;
    ((cl_program*)ptr)[0] = program;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clCreateProgramWithBinary(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_context context;
    cl_uint num_devices;
    cl_device_id *device_list=NULL;
    size_t *lengths = NULL;
    unsigned char **binaries = NULL;
    cl_int flag;
    cl_int *binary_status = NULL;
    cl_program program = NULL;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    context     = ((cl_context*)data)[0]; data = (cl_context*)data + 1;
    num_devices = ((cl_uint*)data)[0];    data = (cl_uint*)data + 1;
    device_list   = (cl_device_id*)malloc(num_devices * sizeof(cl_device_id));
    lengths       = (size_t*)malloc(num_devices * sizeof(size_t));
    binaries      = (unsigned char**)malloc(num_devices * sizeof(unsigned char*));
    binary_status = (cl_int*)malloc(num_devices * sizeof(cl_int));
    if(!device_list || !lengths || !binaries || !binary_status){
        flag     = CL_OUT_OF_HOST_MEMORY;
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(cl_program);  // program
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]     = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_program*)ptr)[0] = program;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    memcpy(device_list, data, num_devices * sizeof(cl_device_id));
    data = (cl_device_id*)data + num_devices;
    memcpy(lengths, data, num_devices * sizeof(size_t));
    data = (size_t*)data + num_devices;
    for(i=0;i<num_devices;i++){
        binaries[i] = (char*)malloc(lengths[i]*sizeof(unsigned char));
        if(!binaries[i]){
            flag     = CL_OUT_OF_HOST_MEMORY;
            msgSize  = sizeof(cl_int);      // flag
            msgSize += sizeof(cl_program);  // program
            msg      = (void*)malloc(msgSize);
            ptr      = msg;
            ((cl_int*)ptr)[0]     = flag; ptr = (cl_int*)ptr  + 1;
            ((cl_program*)ptr)[0] = program;
            Send(clientfd, &msgSize, sizeof(size_t), 0);
            Send(clientfd, msg, msgSize, 0);
            free(msg);msg=NULL;
            return 1;
        }
        memcpy(binaries[i], data, lengths[i]*sizeof(unsigned char));
        data = (unsigned char*)data + lengths[i];
    }
    // Ensure that the context is valid
    flag = isContext(v, context);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(cl_program);  // program
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]     = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_program*)ptr)[0] = program;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Create the program
    program = clCreateProgramWithBinary(context, num_devices, device_list,
                                        lengths, binaries, binary_status, &flag);
    if(flag == CL_SUCCESS){
        registerProgram(v, program);
    }
    // Return the package
    msgSize  = sizeof(cl_int);             // flag
    msgSize += sizeof(cl_program);         // program
    msgSize += num_devices*sizeof(cl_int); // binary_status
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]     = flag;    ptr = (cl_int*)ptr  + 1;
    ((cl_program*)ptr)[0] = program; ptr = (cl_program*)ptr  + 1;
    memcpy(ptr, binary_status, num_devices*sizeof(cl_int));
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clRetainProgram(int* clientfd, char* buffer, validator v, void* data)
{
    cl_program program = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    program = ((cl_program*)data)[0];
    // Ensure that the context is valid
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clRetainProgram(program);
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clReleaseProgram(int* clientfd, char* buffer, validator v, void* data)
{
    cl_program program = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    program = ((cl_program*)data)[0];
    // Ensure that the context is valid
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clReleaseProgram(program);
    if(flag == CL_SUCCESS){
        unregisterProgram(v,program);
    }
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clBuildProgram(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_program program;
    cl_uint num_devices;
    cl_device_id *device_list=NULL;
    size_t options_size;
    char *options = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    program     = ((cl_program*)data)[0]; data = (cl_program*)data + 1;
    num_devices = ((cl_uint*)data)[0];    data = (cl_uint*)data + 1;
    device_list = (cl_device_id*)malloc(num_devices * sizeof(cl_device_id));
    if(!device_list){
        flag     = CL_OUT_OF_HOST_MEMORY;
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0] = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    memcpy(device_list, data, num_devices * sizeof(cl_device_id));
    data = (cl_device_id*)data + num_devices;
    options_size = ((size_t*)data)[0]; data = (size_t*)data + 1;
    if(options_size){
        options = (char*)malloc(options_size);
        if(!options){
            flag     = CL_OUT_OF_HOST_MEMORY;
            msgSize  = sizeof(cl_int);      // flag
            msg      = (void*)malloc(msgSize);
            ptr      = msg;
            ((cl_int*)ptr)[0] = flag;
            Send(clientfd, &msgSize, sizeof(size_t), 0);
            Send(clientfd, msg, msgSize, 0);
            free(msg);msg=NULL;
            return 1;
        }
        memcpy(options, data, options_size);
    }
    // Ensure that the program is valid
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]     = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Build the program
    flag = clBuildProgram(program, num_devices, device_list,
                          options, NULL, NULL);
    // Return the package
    msgSize  = sizeof(cl_int);             // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]     = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clGetProgramInfo(int* clientfd, char* buffer, validator v, void* data)
{
    cl_program program = NULL;
    cl_program_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    program          = ((cl_program*)data)[0];      data = (cl_program*)data + 1;
    param_name       = ((cl_program_info*)data)[0]; data = (cl_program_info*)data + 1;
    param_value_size = ((size_t*)data)[0];          data = (size_t*)data + 1;
    // Ensure that the program is valid
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(size_t);      // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
        ((size_t*)ptr)[0]  = 0;    ptr = (size_t*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Build the required param_value
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    // Get the data
    flag = clGetProgramInfo(program, param_name, param_value_size, param_value, &param_value_size_ret);
    // Return the package
    msgSize  = sizeof(cl_int);       // flag
    msgSize += sizeof(size_t);       // param_value_size_ret
    msgSize += param_value_size_ret; // param_value
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
    ((size_t*)ptr)[0]  = param_value_size_ret;    ptr = (size_t*)ptr + 1;
    if(param_value)
        memcpy(ptr, param_value, param_value_size_ret);
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(param_value);param_value=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clGetProgramBuildInfo(int* clientfd, char* buffer, validator v, void* data)
{
    cl_program program = NULL;
    cl_device_id device = NULL;
    cl_program_info param_name;
    size_t param_value_size;
    cl_int flag;
    void *param_value=NULL;
    size_t param_value_size_ret=0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    program          = ((cl_program*)data)[0];      data = (cl_program*)data + 1;
    device           = ((cl_device_id*)data)[0];    data = (cl_device_id*)data + 1;
    param_name       = ((cl_program_info*)data)[0]; data = (cl_program_info*)data + 1;
    param_value_size = ((size_t*)data)[0];          data = (size_t*)data + 1;
    // Ensure that the program is valid
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msgSize += sizeof(size_t);      // param_value_size_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
        ((size_t*)ptr)[0]  = 0;    ptr = (size_t*)ptr + 1;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Build the required param_value
    if(param_value_size)
        param_value = (void*)malloc(param_value_size);
    // Get the data
    flag = clGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, &param_value_size_ret);
    // Return the package
    msgSize  = sizeof(cl_int);       // flag
    msgSize += sizeof(size_t);       // param_value_size_ret
    msgSize += param_value_size_ret; // param_value
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr + 1;
    ((size_t*)ptr)[0]  = param_value_size_ret;    ptr = (size_t*)ptr + 1;
    if(param_value)
        memcpy(ptr, param_value, param_value_size_ret);
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(param_value);param_value=NULL;
    free(msg);msg=NULL;
    return 1;
}

int ocland_clCreateKernel(int* clientfd, char* buffer, validator v, void* data)
{
    cl_program program;
    size_t kernel_name_size;
    char* kernel_name = NULL;
    cl_int flag;
    cl_kernel kernel = NULL;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    program          = ((cl_program*)data)[0]; data = (cl_program*)data + 1;
    kernel_name_size = ((size_t*)data)[0];     data = (size_t*)data + 1;
    kernel_name = (char*)malloc(kernel_name_size);
    if(!kernel_name){
        flag     = CL_OUT_OF_HOST_MEMORY;
        msgSize  = sizeof(cl_int);     // flag
        msgSize += sizeof(cl_kernel);  // kernel
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]    = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_kernel*)ptr)[0] = kernel;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    memcpy(kernel_name, data, kernel_name_size);
    // Ensure that the program is valid
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);     // flag
        msgSize += sizeof(cl_kernel);  // kernel
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]    = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_kernel*)ptr)[0] = kernel;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Create the program
    kernel = clCreateKernel(program, kernel_name, &flag);
    if(flag == CL_SUCCESS){
        registerKernel(v, kernel);
    }
    // Return the package
    msgSize  = sizeof(cl_int);    // flag
    msgSize += sizeof(cl_kernel); // kernel
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]     = flag;    ptr = (cl_int*)ptr  + 1;
    ((cl_kernel*)ptr)[0] = kernel;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clCreateKernelsInProgram(int* clientfd, char* buffer, validator v, void* data)
{
    unsigned int i;
    cl_program program;
    cl_uint num_kernels;
    cl_int flag;
    cl_kernel *kernels = NULL;
    cl_uint num_kernels_ret = 0;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    program     = ((cl_program*)data)[0]; data = (cl_program*)data + 1;
    num_kernels = ((cl_uint*)data)[0];     data = (cl_uint*)data + 1;
    if(num_kernels){
        kernels = (cl_kernel*)malloc(num_kernels*sizeof(cl_kernel));
        if(!kernels){
            flag     = CL_OUT_OF_HOST_MEMORY;
            msgSize  = sizeof(cl_int);   // flag
            msgSize += sizeof(cl_uint);  // num_kernels_ret
            msg      = (void*)malloc(msgSize);
            ptr      = msg;
            ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
            ((cl_uint*)ptr)[0] = num_kernels_ret;
            Send(clientfd, &msgSize, sizeof(size_t), 0);
            Send(clientfd, msg, msgSize, 0);
            free(msg);msg=NULL;
            return 1;
        }
    }
    // Ensure that the program is valid
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);   // flag
        msgSize += sizeof(cl_uint);  // num_kernels_ret
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag; ptr = (cl_int*)ptr  + 1;
        ((cl_uint*)ptr)[0] = num_kernels_ret;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    // Create the program
    flag = clCreateKernelsInProgram(program, num_kernels, kernels, &num_kernels_ret);
    cl_uint n = (num_kernels < num_kernels_ret) ? num_kernels : num_kernels_ret;
    if(flag == CL_SUCCESS){
        for(i=0;i<n;i++)
            registerKernel(v, kernels[i]);
    }
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msgSize += sizeof(cl_uint);     // num_kernels_ret
    msgSize += n*sizeof(cl_kernel); // kernels
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;            ptr = (cl_int*)ptr  + 1;
    ((cl_uint*)ptr)[0] = num_kernels_ret; ptr = (cl_uint*)ptr + 1;
    memcpy(ptr, kernels, n*sizeof(cl_kernel));
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clRetainKernel(int* clientfd, char* buffer, validator v, void* data)
{
    cl_kernel kernel = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    kernel = ((cl_kernel*)data)[0];
    // Ensure that the kernel is valid
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clRetainKernel(kernel);
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clReleaseKernel(int* clientfd, char* buffer, validator v, void* data)
{
    cl_kernel kernel = NULL;
    cl_int flag;
    size_t msgSize = 0;
    void *msg = NULL, *ptr = NULL;
    // Decript the received data
    kernel = ((cl_kernel*)data)[0];
    // Ensure that the kernel is valid
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        msgSize  = sizeof(cl_int);      // flag
        msg      = (void*)malloc(msgSize);
        ptr      = msg;
        ((cl_int*)ptr)[0]  = flag;
        Send(clientfd, &msgSize, sizeof(size_t), 0);
        Send(clientfd, msg, msgSize, 0);
        free(msg);msg=NULL;
        return 1;
    }
    flag = clReleaseKernel(kernel);
    if(flag == CL_SUCCESS){
        unregisterKernel(v,kernel);
    }
    // Return the package
    msgSize  = sizeof(cl_int);      // flag
    msg      = (void*)malloc(msgSize);
    ptr      = msg;
    ((cl_int*)ptr)[0]  = flag;
    Send(clientfd, &msgSize, sizeof(size_t), 0);
    Send(clientfd, msg, msgSize, 0);
    free(msg);msg=NULL;
    return 1;
}

int ocland_clSetKernelArg(int* clientfd, char* buffer, validator v)
{
    cl_int flag;
    // Get parameters.
    cl_kernel kernel;
    cl_uint arg_index;
    size_t arg_size;
    void *arg_value;
    Recv(clientfd, &kernel, sizeof(cl_kernel), MSG_WAITALL);
    Recv(clientfd, &arg_index, sizeof(cl_uint), MSG_WAITALL);
    Recv(clientfd, &arg_size, sizeof(size_t), MSG_WAITALL);
    arg_value = (void*)malloc(arg_size);
    Recv(clientfd, arg_value, arg_size, MSG_WAITALL);
    // Ensure that pointer is valid
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }
    flag = clSetKernelArg(kernel,arg_index,arg_size,arg_value);
    Send(clientfd, &flag, sizeof(cl_int), 0);
    if(arg_value) free(arg_value); arg_value = NULL;
    return 1;
}

int ocland_clSetKernelNullArg(int* clientfd, char* buffer, validator v)
{
    cl_int flag;
    // Get parameters.
    cl_kernel kernel;
    cl_uint arg_index;
    size_t arg_size;
    void *arg_value = NULL;
    Recv(clientfd, &kernel, sizeof(cl_kernel), MSG_WAITALL);
    Recv(clientfd, &arg_index, sizeof(cl_uint), MSG_WAITALL);
    Recv(clientfd, &arg_size, sizeof(size_t), MSG_WAITALL);
    // Ensure that pointer is valid
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }
    flag = clSetKernelArg(kernel,arg_index,arg_size,arg_value);
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clGetKernelInfo(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    cl_kernel kernel;
    cl_kernel_info param_name;
    size_t param_value_size;
    Recv(clientfd, &kernel, sizeof(cl_kernel), MSG_WAITALL);
    Recv(clientfd, &param_name, sizeof(cl_kernel_info), MSG_WAITALL);
    Recv(clientfd, &param_value_size, sizeof(size_t), MSG_WAITALL);
    cl_int flag;
    size_t param_value_size_ret = 0;
    void* param_value = NULL;
    // Ensure that pointer is valid
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
        return 1;
    }
    // Build image formats if requested
    if(param_value_size){
        param_value = (void*)malloc(param_value_size);
        if(!param_value){
            flag = CL_OUT_OF_RESOURCES;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
            return 0;
        }
    }
    flag = clGetKernelInfo(kernel,param_name,param_value_size,param_value,&param_value_size_ret);
    // Write status output
    Send(clientfd, &flag, sizeof(cl_int), 0);
    Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
    if(flag != CL_SUCCESS){
        if(param_value) free(param_value); param_value=NULL;
        return 1;
    }
    // Send data
    if(param_value_size){
        Send(clientfd, param_value, param_value_size, 0);
        free(param_value); param_value = NULL;
    }
    return 1;
}

int ocland_clGetKernelWorkGroupInfo(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    cl_kernel kernel;
    cl_device_id device;
    cl_kernel_info param_name;
    size_t param_value_size;
    Recv(clientfd, &kernel, sizeof(cl_kernel), MSG_WAITALL);
    Recv(clientfd, &device, sizeof(cl_device_id), MSG_WAITALL);
    Recv(clientfd, &param_name, sizeof(cl_kernel_info), MSG_WAITALL);
    Recv(clientfd, &param_value_size, sizeof(size_t), MSG_WAITALL);
    cl_int flag;
    size_t param_value_size_ret = 0;
    void* param_value = NULL;
    // Ensure that pointers are valid
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
        return 1;
    }
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
        return 1;
    }
    // Build image formats if requested
    if(param_value_size){
        param_value = (void*)malloc(param_value_size);
        if(!param_value){
            flag = CL_OUT_OF_RESOURCES;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
            return 0;
        }
    }
    flag = clGetKernelWorkGroupInfo(kernel,device,param_name,param_value_size,param_value,&param_value_size_ret);
    // Write status output
    Send(clientfd, &flag, sizeof(cl_int), 0);
    Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
    if(flag != CL_SUCCESS){
        if(param_value) free(param_value); param_value=NULL;
        return 1;
    }
    // Send data
    if(param_value_size){
        Send(clientfd, param_value, param_value_size, 0);
        free(param_value); param_value = NULL;
    }
    return 1;
}

int ocland_clWaitForEvents(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    // Get parameters.
    cl_uint num_events=0;
    Recv(clientfd, &num_events, sizeof(cl_uint), MSG_WAITALL);
    ocland_event event_list[num_events];
    Recv(clientfd, event_list, num_events*sizeof(ocland_event), MSG_WAITALL);
    cl_int flag;
    // Ensure that pointers are valid
    for(i=0;i<num_events;i++){
        flag = isEvent(v, event_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            return 1;
        }
    }
    flag = oclandWaitForEvents(num_events, event_list);
    // Write status output
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clGetEventInfo(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    ocland_event event;
    cl_event_info param_name;
    size_t param_value_size;
    Recv(clientfd, &event, sizeof(ocland_event), MSG_WAITALL);
    Recv(clientfd, &param_name, sizeof(cl_event_info), MSG_WAITALL);
    Recv(clientfd, &param_value_size, sizeof(size_t), MSG_WAITALL);
    cl_int flag;
    size_t param_value_size_ret = 0;
    void* param_value = NULL;
    // Ensure that pointers are valid
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
        return 1;
    }
    // Build returned value if requested
    if(param_value_size){
        param_value = (void*)malloc(param_value_size);
        if(!param_value){
            flag = CL_OUT_OF_RESOURCES;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
            return 0;
        }
    }
    flag = clGetEventInfo(event->event,param_name,param_value_size,param_value,&param_value_size_ret);
    // Write status output
    Send(clientfd, &flag, sizeof(cl_int), 0);
    Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
    if(flag != CL_SUCCESS){
        if(param_value) free(param_value); param_value=NULL;
        return 1;
    }
    // Send data
    if(param_value_size){
        Send(clientfd, param_value, param_value_size, 0);
        free(param_value); param_value = NULL;
    }
    return 1;
}

int ocland_clRetainEvent(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    ocland_event event;
    Recv(clientfd, &event, sizeof(ocland_event), MSG_WAITALL);
    cl_int flag;
    // Ensure that pointer is valid
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }
    flag = clRetainEvent(event->event);
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clReleaseEvent(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    ocland_event event;
    Recv(clientfd, &event, sizeof(ocland_event), MSG_WAITALL);
    cl_int flag;
    // Ensure that pointer is valid
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }
    flag = clReleaseEvent(event->event);
    Send(clientfd, &flag, sizeof(cl_int), 0);
    // unregister the event and destroy it
    unregisterEvent(v,event);
    free(event);
    return 1;
}

int ocland_clGetEventProfilingInfo(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    ocland_event event;
    cl_event_info param_name;
    size_t param_value_size;
    Recv(clientfd, &event, sizeof(ocland_event), MSG_WAITALL);
    Recv(clientfd, &param_name, sizeof(cl_event_info), MSG_WAITALL);
    Recv(clientfd, &param_value_size, sizeof(size_t), MSG_WAITALL);
    cl_int flag;
    size_t param_value_size_ret = 0;
    void* param_value = NULL;
    // Ensure that pointers are valid
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
        return 1;
    }
    // Build returned value if requested
    if(param_value_size){
        param_value = (void*)malloc(param_value_size);
        if(!param_value){
            flag = CL_OUT_OF_RESOURCES;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
            return 0;
        }
    }
    flag = clGetEventProfilingInfo(event->event,param_name,param_value_size,param_value,&param_value_size_ret);
    // Write status output
    Send(clientfd, &flag, sizeof(cl_int), 0);
    Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
    if(flag != CL_SUCCESS){
        if(param_value) free(param_value); param_value=NULL;
        return 1;
    }
    // Send data
    if(param_value_size){
        Send(clientfd, param_value, param_value_size, 0);
        free(param_value); param_value = NULL;
    }
    return 1;
}

int ocland_clFlush(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    cl_command_queue command_queue;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    cl_int flag;
    // Ensure that pointer is valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }
    flag = clFlush(command_queue);
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clFinish(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    cl_command_queue command_queue;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    cl_int flag;
    // Ensure that pointer is valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }
    flag = clFinish(command_queue);
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clEnqueueReadBuffer(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem mem;
    cl_bool blocking_read;
    size_t offset;
    size_t cb;
    void *ptr = NULL;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &mem, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &blocking_read, sizeof(cl_bool), MSG_WAITALL);
    Recv(clientfd, &offset, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &cb, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    ptr   = malloc(cb);
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // In case of blocking simply proceed in the natural way
    if(blocking_read == CL_TRUE){
        // We may wait manually for the events provided because
        // OpenCL can only waits their events, but ocalnd event
        // can be relevant. We will not check for errors, OpenCL
        // will do it later
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            // Some OpenCL events can be stored after this method
            // has been called, due to ocland event must be
            // performed before, so we must look for now for
            // invalid events, and set the final ones.
            for(i=0;i<num_events_in_wait_list;i++){
                if(!event_wait_list[i]->event){
                    flag = CL_INVALID_EVENT_WAIT_LIST;
                    Send(clientfd, &flag, sizeof(cl_int), 0);
                    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                    return 1;
                }
                cl_event_wait_list[i] = event_wait_list[i]->event;
            }
        }
        // Call to OpenCL request
        flag = clEnqueueReadBuffer(command_queue,mem,blocking_read,
                                   offset,cb,ptr,
                                   num_events_in_wait_list,
                                   cl_event_wait_list,&(event->event));
        // Return the flag, and the event if request
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(flag != CL_SUCCESS){
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
        if(want_event == CL_TRUE){
            Send(clientfd, &event, sizeof(ocland_event), 0);
            registerEvent(v, event);
        }
        size_t buffsize = BUFF_SIZE*sizeof(char);
        Send(clientfd, &buffsize, sizeof(size_t), 0);
        // Compute the number of packages needed
        unsigned int n = cb / buffsize;
        // Send package by pieces
        for(i=0;i<n;i++){
            Send(clientfd, ptr + i*buffsize, buffsize, 0);
        }
        if(cb % buffsize){
            // Remains some data to arrive
            Send(clientfd, ptr + n*buffsize, cb % buffsize, 0);
        }
        // Mark work as done
        event->status = CL_COMPLETE;
        // Clean up
        if(want_event != CL_TRUE){
            free(event); event = NULL;
        }
        free(ptr); ptr = NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // In the non blocking case we will work in a parallel thread,
    // including the calling to clEnqueueReadBuffer method.
    flag = oclandEnqueueReadBuffer(clientfd,command_queue,mem,
                                   offset,cb,ptr,
                                   num_events_in_wait_list,event_wait_list,
                                   want_event, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
    }
    else{
        if(want_event == CL_TRUE){
            registerEvent(v, event);
        }
    }
    // event and event_wait_list must be destroyed by thread
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    return 1;
}

int ocland_clEnqueueWriteBuffer(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem mem;
    cl_bool blocking_write;
    size_t offset;
    size_t cb;
    void *ptr = NULL;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &mem, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &blocking_write, sizeof(cl_bool), MSG_WAITALL);
    Recv(clientfd, &offset, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &cb, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    ptr   = malloc(cb);
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // Send a first flag and the event before continue working
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), 0);
    if(want_event == CL_TRUE){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    // In case of blocking simply proceed in the natural way
    if(blocking_write == CL_TRUE){
        // Receive the data
        size_t buffsize = BUFF_SIZE*sizeof(char);
        Send(clientfd, &buffsize, sizeof(size_t), 0);
        // Compute the number of packages needed
        unsigned int n = cb / buffsize;
        // Receive package by pieces
        for(i=0;i<n;i++){
            Recv(clientfd, ptr + i*buffsize, buffsize, MSG_WAITALL);
        }
        if(cb % buffsize){
            // Remains some data to arrive
            Recv(clientfd, ptr + n*buffsize, cb % buffsize, MSG_WAITALL);
        }
        // We may wait manually for the events provided because
        // OpenCL can only waits their events, but ocalnd event
        // can be relevant. We will not check for errors, OpenCL
        // will do it later
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            // Some OpenCL events can be stored after this method
            // has been called, due to ocland event must be
            // performed before, so we must look now for
            // invalid events, and set the final ones.
            for(i=0;i<num_events_in_wait_list;i++){
                if(!event_wait_list[i]->event){
                    flag = CL_INVALID_EVENT_WAIT_LIST;
                    Send(clientfd, &flag, sizeof(cl_int), 0);
                    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                    return 1;
                }
                cl_event_wait_list[i] = event_wait_list[i]->event;
            }
        }
        // Call to OpenCL request
        flag = clEnqueueWriteBuffer(command_queue,mem,blocking_write,
                                   offset,cb,ptr,
                                   num_events_in_wait_list,
                                   cl_event_wait_list,&(event->event));
        // Mark work as done
        event->status = CL_COMPLETE;
        free(ptr); ptr = NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        if(want_event != CL_TRUE){
            free(event); event = NULL;
        }
        // Return the flag
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }
    // In the non blocking case we will work in a parallel thread,
    // including the calling to clEnqueueReadBuffer method.
    flag = oclandEnqueueWriteBuffer(clientfd,command_queue,mem,
                                    offset,cb,ptr,
                                    num_events_in_wait_list,event_wait_list,
                                    want_event, event);
    if(flag != CL_SUCCESS){
        event->status = CL_COMPLETE;
    }
    // event and event_wait_list must be destroyed by thread
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    return 1;
}

int ocland_clEnqueueCopyBuffer(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem src_buffer;
    cl_mem dst_buffer;
    size_t src_offset;
    size_t dst_offset;
    size_t cb;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &src_buffer, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &dst_buffer, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &src_offset, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &dst_offset, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &cb, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, src_buffer);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, dst_buffer);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }
    // Call to OpenCL request
    flag = clEnqueueCopyBuffer(command_queue,src_buffer,dst_buffer,
                               src_offset,dst_offset,cb,
                               num_events_in_wait_list,
                               cl_event_wait_list,&(event->event));
    // Mark work as done
    event->status = CL_COMPLETE;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    // Return the event
    if((flag == CL_SUCCESS) && (want_event == CL_TRUE)){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    return 1;
}

int ocland_clEnqueueReadImage(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem image;
    cl_bool blocking_read;
    size_t origin[3];
    size_t region[3];
    size_t row_pitch;
    size_t slice_pitch;
    void *ptr = NULL;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &image, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &blocking_read, sizeof(cl_bool), MSG_WAITALL);
    Recv(clientfd, origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, region, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &row_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &slice_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    ptr   = malloc(region[0] + region[1]*row_pitch + region[2]*slice_pitch);
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // In case of blocking simply send the data.
    // In rect reading process the data will send in
    // blocks of host_row_pitch size, along all the
    // region specified.
    if(blocking_read == CL_TRUE){
        // We may wait manually for the events provided because
        // OpenCL can only waits their events, but ocalnd event
        // can be relevant. We will not check for errors, OpenCL
        // will do it later
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            // Some OpenCL events can be stored after this method
            // has been called, due to ocland event must be
            // performed before, so we must look for now for
            // invalid events, and set the final ones.
            for(i=0;i<num_events_in_wait_list;i++){
                if(!event_wait_list[i]->event){
                    flag = CL_INVALID_EVENT_WAIT_LIST;
                    Send(clientfd, &flag, sizeof(cl_int), 0);
                    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                    return 1;
                }
                cl_event_wait_list[i] = event_wait_list[i]->event;
            }
        }
        // Call to OpenCL request
        flag = clEnqueueReadImage(command_queue, image, blocking_read,
                                  origin, region,
                                  row_pitch, slice_pitch,
                                  ptr, num_events_in_wait_list,
                                  cl_event_wait_list, &(event->event));
        // Return the flag, and the event if requested
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(flag != CL_SUCCESS){
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
        if(want_event == CL_TRUE){
            Send(clientfd, &event, sizeof(ocland_event), 0);
            registerEvent(v, event);
        }
        unsigned int j, k, n;
        size_t buffsize = BUFF_SIZE*sizeof(char);
        Send(clientfd, &buffsize, sizeof(size_t), 0);
        // Compute the number of packages needed
        n = row_pitch / buffsize;
        // Send the rows
        size_t host_origin = 0;
        for(j=0;j<region[1];j++){
            for(k=0;k<region[2];k++){
                // Send package by pieces
                for(i=0;i<n;i++){
                    Send(clientfd, ptr + i*buffsize + host_origin, buffsize, 0);
                }
                if(row_pitch % buffsize){
                    // Remains some data to arrive
                    Send(clientfd, ptr + n*buffsize + host_origin, row_pitch % buffsize, 0);
                }
                // Compute the new origin
                host_origin += row_pitch;
            }
        }
        // Mark work as done
        event->status = CL_COMPLETE;
        // Clean up
        if(want_event != CL_TRUE){
            free(event); event = NULL;
        }
        free(ptr); ptr = NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // In the non blocking case we will work in a parallel thread,
    // including the calling to clEnqueueReadBufferRect method.
    flag = oclandEnqueueReadImage(clientfd,command_queue,image,
                                  origin,region,
                                  row_pitch,slice_pitch,
                                  ptr,num_events_in_wait_list,event_wait_list,
                                  want_event, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
    }
    else{
        if(want_event == CL_TRUE){
            registerEvent(v, event);
        }
    }
    // event and event_wait_list must be destroyed by thread
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    return 1;
}

int ocland_clEnqueueWriteImage(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem image;
    cl_bool blocking_write;
    size_t origin[3];
    size_t region[3];
    size_t row_pitch;
    size_t slice_pitch;
    void *ptr = NULL;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &image, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &blocking_write, sizeof(cl_bool), MSG_WAITALL);
    Recv(clientfd, origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, region, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &row_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &slice_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    ptr   = malloc(region[0] + region[1]*row_pitch + region[2]*slice_pitch);
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // Send a first flag and the event before continue working
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), 0);
    if(want_event == CL_TRUE){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    // In case of blocking simply proceed in the natural way
    if(blocking_write == CL_TRUE){
        unsigned int j, k, n;
        size_t buffsize = BUFF_SIZE*sizeof(char);
        Send(clientfd, &buffsize, sizeof(size_t), 0);
        // Compute the number of packages needed
        n = row_pitch / buffsize;
        // Receive the rows
        size_t host_origin = 0;
        for(j=0;j<region[1];j++){
            for(k=0;k<region[2];k++){
                // Receive package by pieces
                for(i=0;i<n;i++){
                    Recv(clientfd, ptr + i*buffsize + host_origin, buffsize, MSG_WAITALL);
                }
                if(row_pitch % buffsize){
                    // Remains some data to arrive
                    Recv(clientfd, ptr + n*buffsize + host_origin, row_pitch % buffsize, MSG_WAITALL);
                }
                // Compute the new origin
                host_origin += row_pitch;
            }
        }
        // We may wait manually for the events provided because
        // OpenCL can only waits their events, but ocalnd event
        // can be relevant. We will not check for errors, OpenCL
        // will do it later
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            // Some OpenCL events can be stored after this method
            // has been called, due to ocland event must be
            // performed before, so we must look for now for
            // invalid events, and set the final ones.
            for(i=0;i<num_events_in_wait_list;i++){
                if(!event_wait_list[i]->event){
                    flag = CL_INVALID_EVENT_WAIT_LIST;
                    Send(clientfd, &flag, sizeof(cl_int), 0);
                    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                    return 1;
                }
                cl_event_wait_list[i] = event_wait_list[i]->event;
            }
        }
        // Call to OpenCL request
        flag = clEnqueueWriteImage(command_queue, image, blocking_write,
                                        origin, region,
                                        row_pitch, slice_pitch,
                                        ptr, num_events_in_wait_list,
                                        cl_event_wait_list, &(event->event));
        // Mark work as done
        event->status = CL_COMPLETE;
        free(ptr); ptr = NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        if(want_event != CL_TRUE){
            free(event); event = NULL;
        }
        // Return the flag
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }
    // In the non blocking case we will work in a parallel thread,
    // including the calling to clEnqueueReadBuffer method.
    flag = oclandEnqueueWriteImage(clientfd,command_queue,image,
                                   origin,region,
                                   row_pitch,slice_pitch,
                                   ptr,num_events_in_wait_list,event_wait_list,
                                   want_event, event);
    if(flag != CL_SUCCESS){
        event->status = CL_COMPLETE;
    }
    // event and event_wait_list must be destroyed by thread
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    return 1;
}

int ocland_clEnqueueCopyImage(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem src_image;
    cl_mem dst_image;
    size_t src_origin[3];
    size_t dst_origin[3];
    size_t region[3];
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &src_image, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &dst_image, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, src_origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, dst_origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, region, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, src_image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, dst_image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look for now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }
    // Call to OpenCL request
    flag = clEnqueueCopyImage(command_queue, src_image, dst_image,
                                    src_origin, dst_origin, region,
                                    num_events_in_wait_list,
                                    cl_event_wait_list, &(event->event));
    // Mark work as done
    event->status = CL_COMPLETE;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clEnqueueCopyImageToBuffer(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem src_image;
    cl_mem dst_buffer;
    size_t src_origin[3];
    size_t region[3];
    size_t dst_offset;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &src_image, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &dst_buffer, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, src_origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, region, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &dst_offset, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, src_image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, dst_buffer);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look for now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }
    // Call to OpenCL request
    flag = clEnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer,
                                    src_origin, region, dst_offset,
                                    num_events_in_wait_list,
                                    cl_event_wait_list, &(event->event));
    // Mark work as done
    event->status = CL_COMPLETE;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clEnqueueCopyBufferToImage(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem src_buffer;
    cl_mem dst_image;
    size_t src_offset;
    size_t dst_origin[3];
    size_t region[3];
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &src_buffer, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &dst_image, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &src_offset, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, dst_origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, region, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, src_buffer);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, dst_image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look for now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }
    // Call to OpenCL request
    flag = clEnqueueCopyBufferToImage(command_queue, src_buffer, dst_image,
                                    src_offset, dst_origin, region,
                                    num_events_in_wait_list,
                                    cl_event_wait_list, &(event->event));
    // Mark work as done
    event->status = CL_COMPLETE;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clEnqueueNDRangeKernel(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_kernel kernel;
    cl_uint work_dim;
    cl_bool have_global_work_offset;
    size_t *global_work_offset = NULL;
    size_t *global_work_size = NULL;
    cl_bool have_local_work_size;
    size_t *local_work_size = NULL;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &kernel, sizeof(cl_kernel), MSG_WAITALL);
    Recv(clientfd, &work_dim, sizeof(cl_uint), MSG_WAITALL);
    Recv(clientfd, &have_global_work_offset, sizeof(cl_bool), MSG_WAITALL);
    if(have_global_work_offset == CL_TRUE){
        global_work_offset = (size_t*)malloc(work_dim*sizeof(size_t));
        Recv(clientfd, global_work_offset, work_dim*sizeof(size_t), MSG_WAITALL);
    }
    global_work_size = (size_t*)malloc(work_dim*sizeof(size_t));
    Recv(clientfd, global_work_size, work_dim*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &have_local_work_size, sizeof(cl_bool), MSG_WAITALL);
    if(have_local_work_size == CL_TRUE){
        local_work_size = (size_t*)malloc(work_dim*sizeof(size_t));
        Recv(clientfd, local_work_size, work_dim*sizeof(size_t), MSG_WAITALL);
    }
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look for now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }
    // Call to OpenCL request
    flag = clEnqueueNDRangeKernel(command_queue, kernel, work_dim,
                                  global_work_offset, global_work_size,
                                  local_work_size,num_events_in_wait_list,
                                  cl_event_wait_list, &(event->event));
    // Mark work as done
    event->status = CL_COMPLETE;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

// ----------------------------------
// OpenCL 1.1
// ----------------------------------
int ocland_clCreateSubBuffer(int* clientfd, char* buffer, validator v)
{
    cl_int errcode_ret;
    cl_mem clMem = NULL;
    // Get parameters.
    cl_mem input_buffer;
    cl_mem_flags flags;
    cl_buffer_create_type buffer_create_type;
    cl_buffer_region buffer_create_info;
    Recv(clientfd, &input_buffer, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &flags, sizeof(cl_mem_flags), MSG_WAITALL);
    Recv(clientfd, &buffer_create_type, sizeof(cl_buffer_create_type), MSG_WAITALL);
    Recv(clientfd, &buffer_create_info, sizeof(cl_buffer_region), MSG_WAITALL);
    // Ensure that memory object is valid
    errcode_ret = isBuffer(v, input_buffer);
    if(errcode_ret != CL_SUCCESS){
        Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
        Send(clientfd, &clMem, sizeof(cl_mem), 0);
        return 1;
    }
    struct _cl_version version = clGetMemObjectVersion(input_buffer);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 1))){
        // OpenCL < 1.1, so this function does not exist
        errcode_ret = CL_INVALID_MEM_OBJECT;
    }
    else{
        clMem = clCreateSubBuffer(input_buffer, flags, buffer_create_type,
                                  &buffer_create_info, &errcode_ret);
    }
    // Write output
    Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
    Send(clientfd, &clMem, sizeof(cl_mem), 0);
    // Register new memory object
    registerBuffer(v, clMem);
    return 1;
}

int ocland_clCreateUserEvent(int* clientfd, char* buffer, validator v)
{
    cl_int errcode_ret;
    ocland_event event = NULL;
    // Get parameters.
    cl_context context;
    Recv(clientfd, &context, sizeof(cl_context), MSG_WAITALL);
    // Ensure that context is valid
    errcode_ret = isContext(v, context);
    if(errcode_ret != CL_SUCCESS){
        Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
        Send(clientfd, &event, sizeof(ocland_event), 0);
        return 1;
    }
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        errcode_ret = CL_OUT_OF_RESOURCES;
        Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
        Send(clientfd, &event, sizeof(ocland_event), 0);
        return 0;
    }
    event->event         = NULL;
    event->status        = CL_COMPLETE;
    event->context       = context;
    event->command_queue = NULL;

    struct _cl_version version = clGetContextVersion(context);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 1))){
        // OpenCL < 1.1, so this function does not exist
        errcode_ret = CL_INVALID_CONTEXT;
    }
    else{
        event->event = clCreateUserEvent(context, &errcode_ret);
    }

    // Write output
    Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
    Send(clientfd, &event, sizeof(ocland_event), 0);
    if(errcode_ret == CL_SUCCESS)
        free(event); event=NULL;
    // Register new memory object
    registerEvent(v, event);
    return 1;
}

int ocland_clSetUserEventStatus(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    ocland_event event;
    cl_int execution_status;
    Recv(clientfd, &event, sizeof(ocland_event), MSG_WAITALL);
    Recv(clientfd, &execution_status, sizeof(cl_int), MSG_WAITALL);
    cl_int flag;
    // Ensure that pointer is valid
    flag = isEvent(v, event);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }

    struct _cl_version version = clGetEventVersion(event->event);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 1))){
        // OpenCL < 1.1, so this function does not exist
        flag = CL_INVALID_EVENT;
    }
    else{
        flag = clSetUserEventStatus(event->event, execution_status);
    }

    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clEnqueueReadBufferRect(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem mem;
    cl_bool blocking_read;
    size_t buffer_origin[3];
    size_t host_origin[3] = {0, 0, 0};
    size_t region[3];
    size_t buffer_row_pitch;
    size_t buffer_slice_pitch;
    size_t host_row_pitch;
    size_t host_slice_pitch;
    void *ptr = NULL;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &mem, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &blocking_read, sizeof(cl_bool), MSG_WAITALL);
    Recv(clientfd, buffer_origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, region, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &buffer_row_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &buffer_slice_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &host_row_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &host_slice_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    ptr   = malloc(region[0] + region[1]*host_row_pitch + region[2]*host_slice_pitch);
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // In case of blocking simply send the data.
    // In rect reading process the data will send in
    // blocks of host_row_pitch size, along all the
    // region specified.
    if(blocking_read == CL_TRUE){
        // We may wait manually for the events provided because
        // OpenCL can only waits their events, but ocalnd event
        // can be relevant. We will not check for errors, OpenCL
        // will do it later
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            // Some OpenCL events can be stored after this method
            // has been called, due to ocland event must be
            // performed before, so we must look for now for
            // invalid events, and set the final ones.
            for(i=0;i<num_events_in_wait_list;i++){
                if(!event_wait_list[i]->event){
                    flag = CL_INVALID_EVENT_WAIT_LIST;
                    Send(clientfd, &flag, sizeof(cl_int), 0);
                    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                    return 1;
                }
                cl_event_wait_list[i] = event_wait_list[i]->event;
            }
        }

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
                                           ptr, num_events_in_wait_list,
                                           cl_event_wait_list, &(event->event));
        }

        // Return the flag, and the event if requested
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(flag != CL_SUCCESS){
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
        if(want_event == CL_TRUE){
            Send(clientfd, &event, sizeof(ocland_event), 0);
            registerEvent(v, event);
        }
        unsigned int j, k, n;
        size_t buffsize = BUFF_SIZE*sizeof(char);
        Send(clientfd, &buffsize, sizeof(size_t), 0);
        // Compute the number of packages needed
        n = host_row_pitch / buffsize;
        // Send the rows
        size_t origin = 0;
        for(j=0;j<region[1];j++){
            for(k=0;k<region[2];k++){
                // Send package by pieces
                for(i=0;i<n;i++){
                    Send(clientfd, ptr + i*buffsize + origin, buffsize, 0);
                }
                if(host_row_pitch % buffsize){
                    // Remains some data to arrive
                    Send(clientfd, ptr + n*buffsize + origin, host_row_pitch % buffsize, 0);
                }
                // Compute the new origin
                origin += host_row_pitch;
            }
        }
        // Mark work as done
        event->status = CL_COMPLETE;
        // Clean up
        if(want_event != CL_TRUE){
            free(event); event = NULL;
        }
        free(ptr); ptr = NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // In the non blocking case we will work in a parallel thread,
    // including the calling to clEnqueueReadBufferRect method.
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
    }
    else{
        if(want_event == CL_TRUE){
            registerEvent(v, event);
        }
    }
    // event and event_wait_list must be destroyed by thread
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    return 1;
}

int ocland_clEnqueueWriteBufferRect(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem mem;
    cl_bool blocking_write;
    size_t buffer_origin[3];
    size_t host_origin[3] = {0, 0, 0};
    size_t region[3];
    size_t buffer_row_pitch;
    size_t buffer_slice_pitch;
    size_t host_row_pitch;
    size_t host_slice_pitch;
    void *ptr = NULL;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &mem, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &blocking_write, sizeof(cl_bool), MSG_WAITALL);
    Recv(clientfd, buffer_origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, region, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &buffer_row_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &buffer_slice_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &host_row_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &host_slice_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    ptr   = malloc(region[0] + region[1]*host_row_pitch + region[2]*host_slice_pitch);
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if( (!ptr) || (!event) ){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // Send a first flag and the event before continue working
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), 0);
    if(want_event == CL_TRUE){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    // In case of blocking simply proceed in the natural way
    if(blocking_write == CL_TRUE){
        unsigned int j, k, n;
        size_t buffsize = BUFF_SIZE*sizeof(char);
        Send(clientfd, &buffsize, sizeof(size_t), 0);
        // Compute the number of packages needed
        n = host_row_pitch / buffsize;
        // Receive the rows
        size_t origin = 0;
        for(j=0;j<region[1];j++){
            for(k=0;k<region[2];k++){
                // Receive package by pieces
                for(i=0;i<n;i++){
                    Recv(clientfd, ptr + i*buffsize + origin, buffsize, MSG_WAITALL);
                }
                if(host_row_pitch % buffsize){
                    // Remains some data to arrive
                    Recv(clientfd, ptr + n*buffsize + origin, host_row_pitch % buffsize, MSG_WAITALL);
                }
                // Compute the new origin
                origin += host_row_pitch;
            }
        }
        // We may wait manually for the events provided because
        // OpenCL can only waits their events, but ocalnd event
        // can be relevant. We will not check for errors, OpenCL
        // will do it later
        if(num_events_in_wait_list){
            oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
            // Some OpenCL events can be stored after this method
            // has been called, due to ocland event must be
            // performed before, so we must look for now for
            // invalid events, and set the final ones.
            for(i=0;i<num_events_in_wait_list;i++){
                if(!event_wait_list[i]->event){
                    flag = CL_INVALID_EVENT_WAIT_LIST;
                    Send(clientfd, &flag, sizeof(cl_int), 0);
                    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                    return 1;
                }
                cl_event_wait_list[i] = event_wait_list[i]->event;
            }
        }

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
                                            ptr, num_events_in_wait_list,
                                            cl_event_wait_list, &(event->event));
        }

        // Mark work as done
        event->status = CL_COMPLETE;
        free(ptr); ptr = NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        if(want_event != CL_TRUE){
            free(event); event = NULL;
        }
        // Return the flag
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 1;
    }
    // In the non blocking case we will work in a parallel thread,
    // including the calling to clEnqueueReadBuffer method.
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
        event->status = CL_COMPLETE;
    }
    // event and event_wait_list must be destroyed by thread
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    return 1;
}

int ocland_clEnqueueCopyBufferRect(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem src_buffer;
    cl_mem dst_buffer;
    size_t src_origin[3];
    size_t dst_origin[3] = {0, 0, 0};
    size_t region[3];
    size_t src_row_pitch;
    size_t src_slice_pitch;
    size_t dst_row_pitch;
    size_t dst_slice_pitch;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &src_buffer, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &dst_buffer, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, src_origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, dst_origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, region, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &src_row_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &src_slice_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &dst_row_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &dst_slice_pitch, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, src_buffer);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, dst_buffer);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look for now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }

    struct _cl_version version = clGetEventVersion(event->event);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 1))){
        // OpenCL < 1.1, so this function does not exist
        flag = CL_INVALID_EVENT;
    }
    else{
        flag = clEnqueueCopyBufferRect(command_queue, src_buffer, dst_buffer,
                                        src_origin, dst_origin, region,
                                        src_row_pitch, src_slice_pitch,
                                        dst_row_pitch, dst_slice_pitch,
                                        num_events_in_wait_list,
                                        cl_event_wait_list, &(event->event));
    }

    // Mark work as done
    event->status = CL_COMPLETE;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    if((flag== CL_SUCCESS) && (want_event == CL_TRUE)){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    return 1;
}

// ----------------------------------
// OpenCL 1.2
// ----------------------------------
int ocland_clCreateSubDevices(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    cl_device_id device;
    size_t sProps;
    Recv(clientfd, &device, sizeof(cl_device_id), MSG_WAITALL);
    Recv(clientfd, &sProps, sizeof(size_t), MSG_WAITALL);
    cl_device_partition_property *properties = NULL;
    if(sProps){
        properties = (cl_device_partition_property*)malloc(sProps);
        Recv(clientfd, properties, sProps, MSG_WAITALL);
    }
    cl_uint num_entries;
    Recv(clientfd, &num_entries, sizeof(cl_uint), MSG_WAITALL);
    cl_int flag;
    cl_device_id *out_devices = NULL;
    cl_uint num_devices_ret=0;
    // Ensure that device is valid
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        Send(clientfd, &num_devices_ret, sizeof(cl_uint), 0);
        return 1;
    }
    struct _cl_version version = clGetDeviceVersion(device);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_DEVICE;
    }
    else{
        flag = clCreateSubDevices(device, properties, num_entries,
                                  out_devices, &num_devices_ret);
    }
    if(properties) free(properties); properties=NULL;
    if(!num_entries || (flag != CL_SUCCESS)){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        Send(clientfd, &num_devices_ret, sizeof(cl_uint), 0);
        return 1;
    }
    // Build devices array
    out_devices = (cl_device_id*)malloc(num_devices_ret*sizeof(cl_device_id));
    flag = clCreateSubDevices(device, properties, num_entries, out_devices, &num_devices_ret);
    // Send data
    Send(clientfd, &flag, sizeof(cl_int), 0);
    Send(clientfd, &num_devices_ret, sizeof(cl_uint), 0);
    Send(clientfd, out_devices, num_devices_ret*sizeof(cl_device_id), 0);
    // Register new devices
    registerDevices(v, num_devices_ret, out_devices);
    if(out_devices) free(out_devices); out_devices=NULL;
    return 1;
}

int ocland_clRetainDevice(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    cl_device_id device;
    Recv(clientfd, &device, sizeof(cl_device_id), MSG_WAITALL);
    cl_int flag;
    // Ensure that device is valid
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
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
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clReleaseDevice(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    cl_device_id device;
    Recv(clientfd, &device, sizeof(cl_device_id), MSG_WAITALL);
    cl_int flag;
    // Ensure that device is valid
    flag = isDevice(v, device);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
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
    Send(clientfd, &flag, sizeof(cl_int), 0);
    // Unregister it
    unregisterDevices(v, 1, &device);
    return 1;
}

int ocland_clCreateImage(int* clientfd, char* buffer, validator v)
{
    cl_int errcode_ret;
    cl_mem clMem = NULL;
    // Get parameters.
    cl_context context;
    cl_mem_flags flags;
    cl_image_format image_format;
    cl_image_desc image_desc;
    void* host_ptr = NULL;
    Recv(clientfd, &context, sizeof(cl_context), MSG_WAITALL);
    Recv(clientfd, &flags, sizeof(cl_mem_flags), MSG_WAITALL);
    Recv(clientfd, &image_format, sizeof(cl_image_format), MSG_WAITALL);
    Recv(clientfd, &image_desc, sizeof(cl_image_desc), MSG_WAITALL);
    if(flags & CL_MEM_COPY_HOST_PTR){
        // Compute the image size
        size_t size = 0;
        if( (image_desc.image_type & CL_MEM_OBJECT_IMAGE1D) ||
            (image_desc.image_type & CL_MEM_OBJECT_IMAGE1D_BUFFER) ){
            size = image_desc.image_row_pitch;
        }
        else if(image_desc.image_type & CL_MEM_OBJECT_IMAGE2D){
            size = image_desc.image_row_pitch * image_desc.image_height;
        }
        else if(image_desc.image_type & CL_MEM_OBJECT_IMAGE3D){
            size = image_desc.image_slice_pitch * image_desc.image_depth;
        }
        else if( (image_desc.image_type & CL_MEM_OBJECT_IMAGE1D_ARRAY) ||
                 (image_desc.image_type & CL_MEM_OBJECT_IMAGE2D_ARRAY) ){
            size = image_desc.image_slice_pitch * image_desc.image_array_size;
        }
        // Really large data, take care here
        host_ptr = (void*)malloc(size);
        size_t buffsize = BUFF_SIZE*sizeof(char);
        if(!host_ptr){
            buffsize = 0;
            Send(clientfd, &buffsize, sizeof(size_t), 0);
            return 0;
        }
        Send(clientfd, &buffsize, sizeof(size_t), 0);
        // Compute the number of packages needed
        unsigned int i,n;
        n = size / buffsize;
        // Receive package by pieces
        for(i=0;i<n;i++){
            Recv(clientfd, host_ptr + i*buffsize, buffsize, MSG_WAITALL);
        }
        if(size % buffsize){
            // Remains some data to arrive
            Recv(clientfd, host_ptr + n*buffsize, size % buffsize, MSG_WAITALL);
        }
    }
    // Ensure that context is valid
    errcode_ret = isContext(v, context);
    if(errcode_ret != CL_SUCCESS){
        Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
        Send(clientfd, &clMem, sizeof(cl_mem), 0);
        return 1;
    }
    struct _cl_version version = clGetContextVersion(context);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        errcode_ret = CL_INVALID_CONTEXT;
    }
    else{
        clMem = clCreateImage(context, flags, &image_format, &image_desc, host_ptr, &errcode_ret);
    }
    // Write output
    Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
    Send(clientfd, &clMem, sizeof(cl_mem), 0);
    // Register new memory object
    registerBuffer(v, clMem);
    return 1;
}

int ocland_clCreateProgramWithBuiltInKernels(int* clientfd, char* buffer, validator v)
{
    unsigned int i,n;
    cl_int errcode_ret = CL_SUCCESS;
    cl_program program = NULL;
    // Get parameters.
    cl_context context;
    cl_uint num_devices;
    size_t size;
    Recv(clientfd, &context, sizeof(cl_context), MSG_WAITALL);
    Recv(clientfd, &num_devices, sizeof(cl_uint), MSG_WAITALL);
    cl_device_id device_list[num_devices];
    Recv(clientfd, device_list, num_devices*sizeof(cl_device_id), MSG_WAITALL);
    Recv(clientfd, &size, sizeof(size_t), MSG_WAITALL);
    char* kernel_names = (char*)malloc(size);
    if( !kernel_names ){
        // We cannot stop the execution because client
        // is expecting send data
        errcode_ret = CL_OUT_OF_RESOURCES;
    }
    size_t buffsize = BUFF_SIZE*sizeof(char);
    Send(clientfd, &buffsize, sizeof(size_t), 0);
    // Compute the number of packages needed
    n = size / buffsize;
    // Receive package by pieces
    if(errcode_ret != CL_SUCCESS){
        char dummy[buffsize];
        // Receive package by pieces
        for(i=0;i<n;i++){
            Recv(clientfd, dummy, buffsize, MSG_WAITALL);
        }
        if(size % buffsize){
            // Remains some data to arrive
            Recv(clientfd, dummy, size % buffsize, MSG_WAITALL);
        }
    }
    else{
        for(i=0;i<n;i++){
            Recv(clientfd, kernel_names + i*buffsize, buffsize, MSG_WAITALL);
        }
        if(size % buffsize){
            // Remains some data to arrive
            Recv(clientfd, kernel_names + n*buffsize, size % buffsize, MSG_WAITALL);
        }
    }
    if(errcode_ret != CL_SUCCESS){
        Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
        Send(clientfd, &program, sizeof(cl_program), 0);
        if(kernel_names) free(kernel_names); kernel_names=NULL;
        return 1;
    }
    // Ensure that context is valid
    errcode_ret = isContext(v, context);
    if(errcode_ret != CL_SUCCESS){
        Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
        Send(clientfd, &program, sizeof(cl_program), 0);
        if(kernel_names) free(kernel_names); kernel_names=NULL;
        return 1;
    }
    // Ensure that devices are valid
    for(i=0;i<num_devices;i++){
        errcode_ret = isDevice(v, device_list[i]);
        if(errcode_ret != CL_SUCCESS){
            Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
            Send(clientfd, &program, sizeof(cl_program), 0);
            if(kernel_names) free(kernel_names); kernel_names=NULL;
            return 1;
        }
    }
    struct _cl_version version = clGetContextVersion(context);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        errcode_ret = CL_INVALID_CONTEXT;
    }
    else{
        program = clCreateProgramWithBuiltInKernels(context,num_devices,device_list,
                                                    (const char*)kernel_names,
                                                    &errcode_ret);
    }
    // Write output
    Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
    Send(clientfd, &program, sizeof(cl_program), 0);
    if(kernel_names) free(kernel_names); kernel_names=NULL;
    // Register new program
    registerProgram(v, program);
    return 1;
}

int ocland_clCompileProgram(int* clientfd, char* buffer, validator v)
{
    unsigned int i,j,n;
    cl_int flag = CL_SUCCESS;
    // Get parameters.
    cl_program program;
    cl_uint num_devices;
    cl_device_id *device_list = NULL;
    size_t size;
    Recv(clientfd, &program, sizeof(cl_program), MSG_WAITALL);
    Recv(clientfd, &num_devices, sizeof(cl_uint), MSG_WAITALL);
    if(num_devices){
        device_list = (cl_device_id*)malloc(num_devices*sizeof(cl_device_id));
        if(!device_list){
            // We cannot stop the execution because client
            // is expecting send data
            flag = CL_OUT_OF_RESOURCES;
            cl_device_id dummy[num_devices];
            Recv(clientfd, dummy, num_devices*sizeof(cl_device_id), MSG_WAITALL);
        }
        else{
            Recv(clientfd, device_list, num_devices*sizeof(cl_device_id), MSG_WAITALL);
        }
    }
    Recv(clientfd, &size, sizeof(size_t), MSG_WAITALL);
    char* options = (char*)malloc(size);
    if( !options ){
        // We cannot stop the execution because client
        // is expecting send data
        flag = CL_OUT_OF_RESOURCES;
    }
    size_t buffsize = BUFF_SIZE*sizeof(char);
    Send(clientfd, &buffsize, sizeof(size_t), 0);
    // Compute the number of packages needed
    n = size / buffsize;
    // Receive package by pieces
    if(flag != CL_SUCCESS){
        char dummy[buffsize];
        // Receive package by pieces
        for(i=0;i<n;i++){
            Recv(clientfd, dummy, buffsize, MSG_WAITALL);
        }
        if(size % buffsize){
            // Remains some data to arrive
            Recv(clientfd, dummy, size % buffsize, MSG_WAITALL);
        }
    }
    else{
        for(i=0;i<n;i++){
            Recv(clientfd, options + i*buffsize, buffsize, MSG_WAITALL);
        }
        if(size % buffsize){
            // Remains some data to arrive
            Recv(clientfd, options + n*buffsize, size % buffsize, MSG_WAITALL);
        }
    }
    cl_uint num_input_headers = 0;
    Recv(clientfd, &num_input_headers, sizeof(cl_uint), MSG_WAITALL);
    cl_program *input_headers = NULL;
    char **header_include_names = NULL;
    if(num_input_headers){
        input_headers = (cl_program*)malloc(num_input_headers*sizeof(cl_program));
        if(!input_headers){
            // We cannot stop the execution because client
            // is expecting send data
            flag = CL_OUT_OF_RESOURCES;
            cl_program dummy[num_input_headers];
            Recv(clientfd, dummy, num_input_headers*sizeof(cl_program), MSG_WAITALL);
        }
        else{
            Recv(clientfd, input_headers, num_input_headers*sizeof(cl_program), MSG_WAITALL);
        }
        header_include_names = (char**)malloc(num_input_headers*sizeof(char*));
        if(!header_include_names){
            // We cannot stop the execution because client
            // is expecting send data
            flag = CL_OUT_OF_RESOURCES;
            char dummy[buffsize];
            for(i=0;i<num_input_headers;i++){
                // Compute the number of packages needed
                Recv(clientfd, &size, sizeof(size_t), MSG_WAITALL);
                n = size / buffsize;
                // Receive package by pieces
                for(j=0;j<n;j++){
                    Recv(clientfd, dummy, buffsize, MSG_WAITALL);
                }
                if(size % buffsize){
                    // Remains some data to arrive
                    Recv(clientfd, dummy, size % buffsize, MSG_WAITALL);
                }
            }
        }
        else{
            for(i=0;i<num_input_headers;i++){
                // Compute the number of packages needed
                Recv(clientfd, &size, sizeof(size_t), MSG_WAITALL);
                n = size / buffsize;
                header_include_names[i] = (char*)malloc(size);
                if(!header_include_names[i]){
                    char dummy[buffsize];
                    // Receive package by pieces
                    for(j=0;j<n;j++){
                        Recv(clientfd, dummy, buffsize, MSG_WAITALL);
                    }
                    if(size % buffsize){
                        // Remains some data to arrive
                        Recv(clientfd, dummy, size % buffsize, MSG_WAITALL);
                    }
                }
                else{
                    for(j=0;j<n;j++){
                        Recv(clientfd, header_include_names[i] + j*buffsize, buffsize, MSG_WAITALL);
                    }
                    if(size % buffsize){
                        // Remains some data to arrive
                        Recv(clientfd, header_include_names[i] + n*buffsize, size % buffsize, MSG_WAITALL);
                    }
                }
            }
        }
    }
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(header_include_names){
            for(i=0;i<num_input_headers;i++){
                if(header_include_names[i]);free(header_include_names[i]);header_include_names[i]=NULL;
            }
        }
        if(device_list) free(device_list); device_list=NULL;
        if(options) free(options); options=NULL;
        if(input_headers) free(input_headers); input_headers=NULL;
        if(header_include_names) free(header_include_names); header_include_names=NULL;
        return 0;
    }
    // Ensure that program is valid
    flag = isProgram(v, program);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(header_include_names){
            for(i=0;i<num_input_headers;i++){
                if(header_include_names[i]);free(header_include_names[i]);header_include_names[i]=NULL;
            }
        }
        if(device_list) free(device_list); device_list=NULL;
        if(options) free(options); options=NULL;
        if(input_headers) free(input_headers); input_headers=NULL;
        if(header_include_names) free(header_include_names); header_include_names=NULL;
        return 0;
    }
    // Ensure that devices are valid
    for(i=0;i<num_devices;i++){
        flag = isDevice(v, device_list[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(header_include_names){
                for(i=0;i<num_input_headers;i++){
                    if(header_include_names[i]);free(header_include_names[i]);header_include_names[i]=NULL;
                }
            }
            if(device_list) free(device_list); device_list=NULL;
            if(options) free(options); options=NULL;
            if(input_headers) free(input_headers); input_headers=NULL;
            if(header_include_names) free(header_include_names); header_include_names=NULL;
            return 0;
        }
    }
    // Ensure that headers are valid
    for(i=0;i<num_input_headers;i++){
        flag = isProgram(v, input_headers[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(header_include_names){
                for(i=0;i<num_input_headers;i++){
                    if(header_include_names[i]);free(header_include_names[i]);header_include_names[i]=NULL;
                }
            }
            if(device_list) free(device_list); device_list=NULL;
            if(options) free(options); options=NULL;
            if(input_headers) free(input_headers); input_headers=NULL;
            if(header_include_names) free(header_include_names); header_include_names=NULL;
            return 0;
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
    // Write output
    Send(clientfd, &flag, sizeof(cl_int), 0);
    if(header_include_names){
        for(i=0;i<num_input_headers;i++){
            if(header_include_names[i]);free(header_include_names[i]);header_include_names[i]=NULL;
        }
    }
    if(device_list) free(device_list); device_list=NULL;
    if(options) free(options); options=NULL;
    if(input_headers) free(input_headers); input_headers=NULL;
    if(header_include_names) free(header_include_names); header_include_names=NULL;
    return 1;
}

int ocland_clLinkProgram(int* clientfd, char* buffer, validator v)
{
    unsigned int i,n;
    cl_program program = NULL;
    cl_int errcode_ret = CL_SUCCESS;
    // Get parameters.
    cl_context context;
    cl_uint num_devices;
    cl_device_id *device_list = NULL;
    size_t size;
    cl_uint num_input_programs;
    Recv(clientfd, &context, sizeof(cl_context), MSG_WAITALL);
    Recv(clientfd, &num_devices, sizeof(cl_uint), MSG_WAITALL);
    if(num_devices){
        device_list = (cl_device_id*)malloc(num_devices*sizeof(cl_device_id));
        if(!device_list){
            // We cannot stop the execution because client
            // is expecting send data
            errcode_ret = CL_OUT_OF_RESOURCES;
            cl_device_id dummy[num_devices];
            Recv(clientfd, dummy, num_devices*sizeof(cl_device_id), MSG_WAITALL);
        }
        else{
            Recv(clientfd, device_list, num_devices*sizeof(cl_device_id), MSG_WAITALL);
        }
    }
    Recv(clientfd, &size, sizeof(size_t), MSG_WAITALL);
    char* options = (char*)malloc(size);
    if( !options ){
        // We cannot stop the execution because client
        // is expecting send data
        errcode_ret = CL_OUT_OF_RESOURCES;
    }
    size_t buffsize = BUFF_SIZE*sizeof(char);
    Send(clientfd, &buffsize, sizeof(size_t), 0);
    // Compute the number of packages needed
    n = size / buffsize;
    // Receive package by pieces
    if(errcode_ret != CL_SUCCESS){
        char dummy[buffsize];
        // Receive package by pieces
        for(i=0;i<n;i++){
            Recv(clientfd, dummy, buffsize, MSG_WAITALL);
        }
        if(size % buffsize){
            // Remains some data to arrive
            Recv(clientfd, dummy, size % buffsize, MSG_WAITALL);
        }
    }
    else{
        for(i=0;i<n;i++){
            Recv(clientfd, options + i*buffsize, buffsize, MSG_WAITALL);
        }
        if(size % buffsize){
            // Remains some data to arrive
            Recv(clientfd, options + n*buffsize, size % buffsize, MSG_WAITALL);
        }
    }
    Recv(clientfd, &num_input_programs, sizeof(cl_uint), MSG_WAITALL);
    cl_program input_programs[num_input_programs];
    Recv(clientfd, input_programs, num_input_programs*sizeof(cl_program), MSG_WAITALL);
    if(errcode_ret != CL_SUCCESS){
        Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
        Send(clientfd, &program, sizeof(cl_program), 0);
        if(device_list) free(device_list); device_list=NULL;
        if(options) free(options); options=NULL;
        return 0;
    }
    // Ensure that context is valid
    errcode_ret = isContext(v, context);
    if(errcode_ret != CL_SUCCESS){
        Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
        Send(clientfd, &program, sizeof(cl_program), 0);
        if(device_list) free(device_list); device_list=NULL;
        if(options) free(options); options=NULL;
        return 0;
    }
    // Ensure that devices are valid
    for(i=0;i<num_devices;i++){
        errcode_ret = isDevice(v, device_list[i]);
        if(errcode_ret != CL_SUCCESS){
            Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
            Send(clientfd, &program, sizeof(cl_program), 0);
            if(device_list) free(device_list); device_list=NULL;
            if(options) free(options); options=NULL;
            return 0;
        }
    }
    // Ensure that programs are valid
    for(i=0;i<num_input_programs;i++){
        errcode_ret = isProgram(v, input_programs[i]);
        if(errcode_ret != CL_SUCCESS){
            Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
            Send(clientfd, &program, sizeof(cl_program), 0);
            if(device_list) free(device_list); device_list=NULL;
            if(options) free(options); options=NULL;
            return 0;
        }
    }
    struct _cl_version version = clGetContextVersion(context);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        errcode_ret = CL_INVALID_CONTEXT;
    }
    else{
        program = clLinkProgram(context,num_devices,device_list,
                                options,num_input_programs,
                                input_programs,NULL,NULL,&errcode_ret);
    }
    // Write output
    Send(clientfd, &errcode_ret, sizeof(cl_int), 0);
    Send(clientfd, &program, sizeof(cl_program), 0);
    if(device_list) free(device_list); device_list=NULL;
    if(options) free(options); options=NULL;
    // Register the new program
    if(errcode_ret == CL_SUCCESS)
        registerProgram(v, program);
    return 1;
}

int ocland_clUnloadPlatformCompiler(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    cl_platform_id platform;
    Recv(clientfd, &platform, sizeof(cl_platform_id), MSG_WAITALL);
    cl_int flag;
    // Ensure that platform is valid
    flag = isPlatform(v, platform);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        return 0;
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
    // Write status output
    Send(clientfd, &flag, sizeof(cl_int), 0);
    return 1;
}

int ocland_clGetKernelArgInfo(int* clientfd, char* buffer, validator v)
{
    // Get parameters.
    cl_kernel kernel;
    cl_uint arg_index;
    cl_kernel_arg_info param_name;
    size_t param_value_size;
    Recv(clientfd, &kernel, sizeof(cl_kernel), MSG_WAITALL);
    Recv(clientfd, &arg_index, sizeof(cl_uint), MSG_WAITALL);
    Recv(clientfd, &param_name, sizeof(cl_kernel_arg_info), MSG_WAITALL);
    Recv(clientfd, &param_value_size, sizeof(size_t), MSG_WAITALL);
    cl_int flag;
    size_t param_value_size_ret = 0;
    void* param_value = NULL;
    // Ensure that pointer is valid
    flag = isKernel(v, kernel);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
        return 1;
    }
    // Build output value if requested
    if(param_value_size){
        param_value = (void*)malloc(param_value_size);
        if(!param_value){
            flag = CL_OUT_OF_RESOURCES;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
            return 0;
        }
    }

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
    // Write status output
    Send(clientfd, &flag, sizeof(cl_int), 0);
    Send(clientfd, &param_value_size_ret, sizeof(size_t), 0);
    if(flag != CL_SUCCESS){
        if(param_value) free(param_value); param_value=NULL;
        return 1;
    }
    // Send data
    if(param_value_size){
        Send(clientfd, param_value, param_value_size, 0);
        free(param_value); param_value = NULL;
    }
    return 1;
}

int ocland_clEnqueueFillBuffer(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem mem;
    size_t pattern_size;
    void * pattern = NULL;
    size_t offset;
    size_t cb;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &mem, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &pattern_size, sizeof(size_t), MSG_WAITALL);
    pattern = (void*)malloc(pattern_size);
    Recv(clientfd, &pattern, pattern_size, MSG_WAITALL);
    Recv(clientfd, &offset, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &cb, sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(pattern) free(pattern); pattern=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, mem);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(pattern) free(pattern); pattern=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(pattern) free(pattern); pattern=NULL;
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(pattern) free(pattern); pattern=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(pattern) free(pattern); pattern=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }

    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = clEnqueueFillBuffer(command_queue,mem,
                                   pattern,pattern_size,
                                   offset,cb,
                                   num_events_in_wait_list,
                                   cl_event_wait_list,&(event->event));
    }

    // Mark work as done
    event->status = CL_COMPLETE;
    if(pattern) free(pattern); pattern=NULL;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    // Return the event
    if((flag == CL_SUCCESS) && (want_event == CL_TRUE)){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    return 1;
}

int ocland_clEnqueueFillImage(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_mem image;
    size_t fill_color_size;
    void * fill_color = NULL;
    size_t origin[3];
    size_t region[3];
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &image, sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &fill_color_size, sizeof(size_t), MSG_WAITALL);
    fill_color = (void*)malloc(fill_color_size);
    Recv(clientfd, &fill_color, fill_color_size, MSG_WAITALL);
    Recv(clientfd, origin, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, region, 3*sizeof(size_t), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(fill_color) free(fill_color); fill_color=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    flag = isBuffer(v, image);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(fill_color) free(fill_color); fill_color=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(fill_color) free(fill_color); fill_color=NULL;
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(fill_color) free(fill_color); fill_color=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(fill_color) free(fill_color); fill_color=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }

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
                                  num_events_in_wait_list,
                                  cl_event_wait_list,&(event->event));
    }

    // Mark work as done
    event->status = CL_COMPLETE;
    if(fill_color) free(fill_color); fill_color=NULL;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    // Return the event
    if((flag == CL_SUCCESS) && (want_event == CL_TRUE)){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    return 1;
}

int ocland_clEnqueueMigrateMemObjects(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_uint num_mem_objects;
    cl_mem *mem_objects=NULL;
    cl_mem_migration_flags flags;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &num_mem_objects, sizeof(cl_uint), MSG_WAITALL);
    mem_objects = (void*)malloc(num_mem_objects*sizeof(cl_mem));
    Recv(clientfd, &mem_objects, num_mem_objects*sizeof(cl_mem), MSG_WAITALL);
    Recv(clientfd, &flags, sizeof(cl_mem_migration_flags), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(mem_objects) free(mem_objects); mem_objects=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_mem_objects;i++){
        flag = isBuffer(v, mem_objects[i]);
        if(flag != CL_SUCCESS){
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(mem_objects) free(mem_objects); mem_objects=NULL;
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(mem_objects) free(mem_objects); mem_objects=NULL;
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(mem_objects) free(mem_objects); mem_objects=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(mem_objects) free(mem_objects); mem_objects=NULL;
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }

    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = clEnqueueMigrateMemObjects(command_queue,num_mem_objects,
                                          mem_objects,flags,
                                          num_events_in_wait_list,
                                          cl_event_wait_list,&(event->event));
    }

    // Mark work as done
    event->status = CL_COMPLETE;
    if(mem_objects) free(mem_objects); mem_objects=NULL;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    // Return the event
    if((flag == CL_SUCCESS) && (want_event == CL_TRUE)){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    return 1;
}

int ocland_clEnqueueMarkerWithWaitList(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }

    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = clEnqueueMarkerWithWaitList(command_queue,
                                           num_events_in_wait_list,
                                           cl_event_wait_list,&(event->event));
    }

    // Mark work as done
    event->status = CL_COMPLETE;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    // Return the event
    if((flag == CL_SUCCESS) && (want_event == CL_TRUE)){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    return 1;
}

int ocland_clEnqueueBarrierWithWaitList(int* clientfd, char* buffer, validator v)
{
    unsigned int i;
    cl_int flag;
    // Get parameters.
    cl_command_queue command_queue;
    cl_context context;
    cl_uint num_events_in_wait_list;
    cl_bool want_event;
    ocland_event event = NULL;
    ocland_event *event_wait_list = NULL;
    cl_event *cl_event_wait_list = NULL;
    Recv(clientfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    Recv(clientfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_WAITALL);
    if(num_events_in_wait_list){
        event_wait_list = (ocland_event*)malloc(num_events_in_wait_list*sizeof(ocland_event));
        cl_event_wait_list = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        Recv(clientfd, &event_wait_list, num_events_in_wait_list*sizeof(ocland_event), MSG_WAITALL);
    }
    Recv(clientfd, &want_event, sizeof(cl_bool), MSG_WAITALL);
    // Ensure that objects are valid
    flag = isQueue(v, command_queue);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        flag = isEvent(v, event_wait_list[i]);
        if(flag != CL_SUCCESS){
            flag = CL_INVALID_EVENT_WAIT_LIST;
            Send(clientfd, &flag, sizeof(cl_int), 0);
            if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
            if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
            return 1;
        }
    }
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS){
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Try to allocate memory for objects
    event = (ocland_event)malloc(sizeof(struct _ocland_event));
    if(!event){
        flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        Send(clientfd, &flag, sizeof(cl_int), 0);
        if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
        if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
        return 1;
    }
    // Set the event as uncompleted
    event->event         = NULL;
    event->status        = 1;
    event->context       = context;
    event->command_queue = command_queue;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors, OpenCL
    // will do it later
    if(num_events_in_wait_list){
        oclandWaitForEvents(num_events_in_wait_list, event_wait_list);
        // Some OpenCL events can be stored after this method
        // has been called, due to ocland event must be
        // performed before, so we must look now for
        // invalid events, and set the final ones.
        for(i=0;i<num_events_in_wait_list;i++){
            if(!event_wait_list[i]->event){
                flag = CL_INVALID_EVENT_WAIT_LIST;
                Send(clientfd, &flag, sizeof(cl_int), 0);
                if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
                if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
                return 1;
            }
            cl_event_wait_list[i] = event_wait_list[i]->event;
        }
    }

    struct _cl_version version = clGetCommandQueueVersion(command_queue);
    if(     (version.major <  1)
        || ((version.major == 1) && (version.minor < 2))){
        // OpenCL < 1.2, so this function does not exist
        flag = CL_INVALID_COMMAND_QUEUE;
    }
    else{
        flag = clEnqueueBarrierWithWaitList(command_queue,
                                            num_events_in_wait_list,
                                            cl_event_wait_list,&(event->event));
    }

    // Mark work as done
    event->status = CL_COMPLETE;
    if(event_wait_list) free(event_wait_list); event_wait_list=NULL;
    if(cl_event_wait_list) free(cl_event_wait_list); cl_event_wait_list=NULL;
    if(want_event != CL_TRUE){
        free(event); event = NULL;
    }
    // Return the flag
    Send(clientfd, &flag, sizeof(cl_int), 0);
    // Return the event
    if((flag == CL_SUCCESS) && (want_event == CL_TRUE)){
        Send(clientfd, &event, sizeof(ocland_event), 0);
        registerEvent(v, event);
    }
    return 1;
}

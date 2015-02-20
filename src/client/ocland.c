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
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include <ocland/common/dataExchange.h>
#include <ocland/common/dataPack.h>
#include <ocland/client/ocland_icd.h>
#include <ocland/client/ocland.h>
#include <ocland/client/shortcut.h>

#ifndef OCLAND_PORT
    #define OCLAND_PORT 51000u
#endif

#ifndef BUFF_SIZE
    #define BUFF_SIZE 1025u
#endif

#define THREAD_SAFE_EXIT {free(_data); _data=NULL; if(fd>0) close(fd); fd = -1; pthread_exit(NULL); return NULL;}

/// Servers data storage
static oclandServers* servers = NULL;
/// Servers initialization flag
static cl_bool initialized = CL_FALSE;

enum {
    ocland_clGetPlatformIDs,
    ocland_clGetPlatformInfo,
    ocland_clGetDeviceIDs,
    ocland_clGetDeviceInfo,
    ocland_clCreateContext,
    ocland_clCreateContextFromType,
    ocland_clRetainContext,
    ocland_clReleaseContext,
    ocland_clGetContextInfo,
    ocland_clCreateCommandQueue,
    ocland_clRetainCommandQueue,
    ocland_clReleaseCommandQueue,
    ocland_clGetCommandQueueInfo,
    ocland_clCreateBuffer,
    ocland_clRetainMemObject,
    ocland_clReleaseMemObject,
    ocland_clGetSupportedImageFormats,
    ocland_clGetMemObjectInfo,
    ocland_clGetImageInfo,
    ocland_clCreateSampler,
    ocland_clRetainSampler,
    ocland_clReleaseSampler,
    ocland_clGetSamplerInfo,
    ocland_clCreateProgramWithSource,
    ocland_clCreateProgramWithBinary,
    ocland_clRetainProgram,
    ocland_clReleaseProgram,
    ocland_clBuildProgram,
    ocland_clGetProgramBuildInfo,
    ocland_clCreateKernel,
    ocland_clCreateKernelsInProgram,
    ocland_clRetainKernel,
    ocland_clReleaseKernel,
    ocland_clSetKernelArg,
    ocland_clGetKernelInfo,
    ocland_clGetKernelWorkGroupInfo,
    ocland_clWaitForEvents,
    ocland_clGetEventInfo,
    ocland_clRetainEvent,
    ocland_clReleaseEvent,
    ocland_clGetEventProfilingInfo,
    ocland_clFlush,
    ocland_clFinish,
    ocland_clEnqueueReadBuffer,
    ocland_clEnqueueWriteBuffer,
    ocland_clEnqueueCopyBuffer,
    ocland_clEnqueueCopyImage,
    ocland_clEnqueueCopyImageToBuffer,
    ocland_clEnqueueCopyBufferToImage,
    ocland_clEnqueueNDRangeKernel,
    ocland_clCreateSubBuffer,
    ocland_clCreateUserEvent,
    ocland_clSetUserEventStatus,
    ocland_clEnqueueReadBufferRect,
    ocland_clEnqueueWriteBufferRect,
    ocland_clEnqueueCopyBufferRect,
    ocland_clEnqueueReadImage,
    ocland_clEnqueueWriteImage,
    ocland_clCreateSubDevices,
    ocland_clRetainDevice,
    ocland_clReleaseDevice,
    ocland_clCreateImage,
    ocland_clCreateProgramWithBuiltInKernels,
    ocland_clCompileProgram,
    ocland_clLinkProgram,
    ocland_clUnloadPlatformCompiler,
    ocland_clGetProgramInfo,
    ocland_clGetKernelArgInfo,
    ocland_clEnqueueFillBuffer,
    ocland_clEnqueueFillImage,
    ocland_clEnqueueMigrateMemObjects,
    ocland_clEnqueueMarkerWithWaitList,
    ocland_clEnqueueBarrierWithWaitList,
    ocland_clCreateImage2D,
    ocland_clCreateImage3D
};

/** Waits until the server is locked, and then gives access
 * and block for other instances. You may lock the servers
 * in order to avoid parallel packages send/receive that can
 * arrive to the wrong method.
 * @param socket Server socket.
 */
void lock(int socket){
    unsigned int i;
    // Find the server
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] == socket)
            break;
    }
    if(i == servers->num_servers){
        // Server not found
        return;
    }
    // Wait while server is locked
    while(servers->locked[i]){
        fflush(stdout);
    }
    // Lock the server
    servers->locked[i] = CL_TRUE;
}

/** Unlock the server for other instances.
 * @param socket Server socket. You may lock the servers
 * in order to avoid parallel packages send/receive that can
 * arrive to the wrong method.
 */
void unlock(int socket){
    unsigned int i;
    // Find the server
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] == socket)
            break;
    }
    if(i == servers->num_servers){
        // Server not found
        return;
    }
    // Unlock the server
    servers->locked[i] = CL_FALSE;
}

/** Load servers file "ocland". File must contain
 * IP address of each server, one per line.
 * @return Number of servers.
 */
unsigned int loadServers()
{
    unsigned int i;
    // Build servers struct
    servers = (oclandServers*)malloc(sizeof(oclandServers));
    servers->num_servers = 0;
    servers->address = NULL;
    servers->sockets = NULL;
    servers->locked  = NULL;
    // Load servers definition files
    FILE *fin = NULL;
    fin = fopen("ocland", "r");
    if(!fin){
        // File don't exist, ocland must be ignored
        return 0;
    }
    // Count the number of lines
    char *line = NULL;
    size_t linelen = 0;
    ssize_t read = getline(&line, &linelen, fin);
    while(read != -1) {
        if(strcmp(line, "\n"))
            servers->num_servers++;
        free(line); line = NULL; linelen = 0;
        read = getline(&line, &linelen, fin);
    }
    // Set servers
    rewind(fin);
    servers->address = (char**)malloc(servers->num_servers * sizeof(char*));
    servers->sockets = (int*)malloc(servers->num_servers * sizeof(int));
    servers->locked = (cl_bool*)malloc(servers->num_servers * sizeof(cl_bool));
    i = 0;
    line = NULL; linelen = 0;
    while((read = getline(&line, &linelen, fin)) != -1) {
        if(!strcmp(line, "\n")){
            free(line); line = NULL; linelen = 0;
            continue;
        }
        servers->address[i] = (char*)malloc((strlen(line) + 1) * sizeof(char));
        strcpy(servers->address[i], line);
        strcpy(strstr(servers->address[i], "\n"), "");
        servers->sockets[i] = -1;
        servers->locked[i] = CL_FALSE;
        free(line); line = NULL;linelen = 0;
        i++;
    }
    return servers->num_servers;
}

/** Connect to servers found on "ocland" file.
 * @return Number of active servers.
 */
unsigned int connectServers()
{
    int switch_on = 1;
    unsigned int i, n=0;
    for(i = 0; i < servers->num_servers; i++){
        // Try to connect to server
        int sockfd = 0;
        struct sockaddr_in serv_addr;
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            continue;
        memset(&serv_addr, '0', sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(OCLAND_PORT);
        if(inet_pton(AF_INET, servers->address[i], &serv_addr.sin_addr) <= 0)
            continue;
        if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            continue;
        setsockopt(sockfd,
                   IPPROTO_TCP,
                   TCP_NODELAY,
                   (char *) &switch_on,
                   sizeof(int));
        setsockopt(sockfd,
                   IPPROTO_TCP,
                   TCP_QUICKACK,
                   (char *) &switch_on,
                   sizeof(int));
        // Store socket
        servers->sockets[i] = sockfd;
        n++;
    }
    return n;
}

/** Return the server address for an specific socket
 @param sockfd Server socket.
 @return Server addresses. NULL if the server does not exist.
 */
const char* serverAddress(int socket)
{
    unsigned int i;
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] == socket){
            return (const char*)(servers->address[i]);
        }
    }
    return NULL;
}


/** Initializes ocland, loading server files
 * and connecting to servers.
 * @return Number of active servers.
 */
unsigned int oclandInit()
{
    unsigned int i,n;
    if(initialized){
        n = 0;
        for(i = 0; i < servers->num_servers; i++){
            if(servers->sockets[i] >= 0)
                n++;
        }
        return n;
    }
    initialized = CL_TRUE;
    n = loadServers();
    return connectServers();
}

cl_int oclandGetPlatformIDs(cl_uint         num_entries,
                            cl_platform_id* platforms,
                            int*            sockets,
                            cl_uint*        num_platforms)
{
    unsigned int i,j;
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clGetPlatformIDs;
    cl_uint l_num_platforms=0, t_num_platforms=0;
    cl_platform_id *l_platforms=NULL;
    if(num_platforms) *num_platforms=0;
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit())
        return CL_SUCCESS;
    // Get number of platforms from servers
    for(i=0;i<servers->num_servers;i++){
        // Ensure that the server still being active
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
       // Count the remaining number of platforms to take
        cl_uint r_num_entries = num_entries - t_num_platforms;
        if(r_num_entries < 0) r_num_entries = 0;
        // Send the command data
        Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
        Send(sockfd, &r_num_entries, sizeof(cl_uint), 0);
        // Receive the answer
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            return flag;
        }
        Recv(sockfd, &l_num_platforms, sizeof(cl_uint), MSG_WAITALL);
        // Add the platforms to the list
        cl_uint n = (l_num_platforms < r_num_entries) ? l_num_platforms : r_num_entries;
        if(!n){
            t_num_platforms += l_num_platforms;
            continue;
        }
        l_platforms = (cl_platform_id*)malloc(n*sizeof(cl_platform_id));
        Recv(sockfd, l_platforms, n*sizeof(cl_platform_id), MSG_WAITALL);
        for(j=0;j<n;j++){
            if(platforms)
                platforms[t_num_platforms + j] = l_platforms[j];
            if(sockets)
                sockets[t_num_platforms + j] = servers->sockets[i];
        }
        t_num_platforms += l_num_platforms;
        free(l_platforms); l_platforms=NULL;
    }
    if(num_platforms) *num_platforms = t_num_platforms;
    return CL_SUCCESS;
}

cl_int oclandGetPlatformInfo(cl_platform_id    platform,
                             cl_platform_info  param_name,
                             size_t            param_value_size,
                             void *            param_value,
                             size_t *          param_value_size_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret;
    void *value_ret = NULL;
    unsigned int comm = ocland_clGetPlatformInfo;
    if(param_value_size_ret) *param_value_size_ret = 0;

    int *sockfd = &(platform->socket);
    if(!sockfd){
        return CL_INVALID_PLATFORM;
    }

    // Useful data to be append to specific cl_platform_info queries
    const char* ip = serverAddress(*sockfd);
    const char* ocland_pre = "ocland(";
    const char* ocland_pos = ") ";

    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(platform->ptr), sizeof(cl_platform_id), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_platform_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size){
        value_ret = malloc(size_ret);
        Recv(sockfd, value_ret, size_ret, MSG_WAITALL);
    }
    // Modify the answer
    if((param_name == CL_PLATFORM_NAME) ||
       (param_name == CL_PLATFORM_VENDOR) ||
       (param_name == CL_PLATFORM_ICD_SUFFIX_KHR)){
        // We need to add an identifier
        size_ret += strlen(ocland_pre) + strlen(ip) + strlen(ocland_pos);
        if(value_ret){
            char* backup = value_ret;
            value_ret = malloc(size_ret);
            sprintf(value_ret, "%s%s%s%s", ocland_pre,
                                           ip,
                                           ocland_pos,
                                           backup);
            free(backup); backup = NULL;
        }
    }
    // Copy the answer to the output vars
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        if(param_value_size < size_ret){
            free(value_ret); value_ret = NULL;
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value_ret, size_ret);
        free(value_ret); value_ret = NULL;
    }

    return CL_SUCCESS;
}

cl_int oclandGetDeviceIDs(cl_platform_id   platform,
                          cl_device_type   device_type,
                          cl_uint          num_entries,
                          cl_device_id *   devices,
                          cl_uint *        num_devices)
{
    ssize_t sent;
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint n;
    unsigned int comm = ocland_clGetDeviceIDs;
    if(num_devices) *num_devices = 0;
    int *sockfd = &(platform->socket);
    if(!sockfd){
        return CL_INVALID_PLATFORM;
    }
    // Send the command data
    sent = Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    sent = Send(sockfd, &(platform->ptr), sizeof(cl_platform_id), MSG_MORE);
    sent = Send(sockfd, &device_type, sizeof(cl_device_type), MSG_MORE);
    sent = Send(sockfd, &num_entries, sizeof(cl_uint), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &n, sizeof(cl_uint), MSG_WAITALL);
    if(num_devices) *num_devices = n;
    if(num_entries < n)
        n = num_entries;
    if(devices){
        Recv(sockfd, devices, n*sizeof(cl_device_id), MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandGetDeviceInfo(cl_device_id    device,
                           cl_device_info  param_name,
                           size_t          param_value_size,
                           void *          param_value,
                           size_t *        param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret;
    unsigned int comm = ocland_clGetDeviceInfo;
    if(param_value_size_ret) *param_value_size_ret = 0;
    int *sockfd = device->socket;
    if(!sockfd){
        return CL_INVALID_DEVICE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(device->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_context_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_context oclandCreateContext(cl_platform_id                platform,
                               const cl_context_properties * properties,
                               cl_uint                       num_properties,
                               cl_uint                       num_devices,
                               const cl_device_id *          devices,
                               void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *),
                               void *                        user_data,
                               cl_int *                      errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clCreateContext;
    cl_context context = NULL;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = &(platform->socket);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
        return NULL;
    }
    // Change the local references to remote ones
    cl_context_properties *props = NULL;
    if(num_properties){
        props = (cl_context_properties*)malloc(
            num_properties * sizeof(cl_context_properties));
        memcpy(props,
               properties,
               num_properties * sizeof(cl_context_properties));
        for(i = 0; i < num_properties - 1; i = i + 2){
            if(props[i] == CL_CONTEXT_PLATFORM){
                props[i + 1] = ((cl_platform_id)(props[i + 1]))->ptr;
            }
        }
    }
    cl_device_id *devs = (cl_device_id *)malloc(num_devices*sizeof(cl_device_id));
    for(i = 0; i < num_devices; i++){
        devs[i] = devices[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &num_properties, sizeof(cl_uint), MSG_MORE);
    if(num_properties){
        Send(sockfd,
             props,
             num_properties * sizeof(cl_context_properties),
             MSG_MORE);
    }
    Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, devs, num_devices * sizeof(cl_device_id), 0);
    free(props); props=NULL;
    free(devs); devs=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &context, sizeof(cl_context), MSG_WAITALL);
    addShortcut((void*)context, sockfd);
    return context;
}

cl_context oclandCreateContextFromType(cl_platform_id                platform,
                                       const cl_context_properties * properties,
                                       cl_uint                       num_properties,
                                       cl_device_type                device_type,
                                       void (CL_CALLBACK *     pfn_notify)(const char *, const void *, size_t, void *),
                                       void *                        user_data,
                                       cl_int *                      errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clCreateContextFromType;
    cl_context context = NULL;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = &(platform->socket);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
        return NULL;
    }
    // Change the local references to remote ones
    cl_context_properties *props = NULL;
    if(num_properties){
        props = (cl_context_properties *)malloc(num_properties*sizeof(cl_context_properties));
        memcpy(props, properties, num_properties*sizeof(cl_context_properties));
        for(i=0;i<num_properties-1;i=i+2){
            if(props[i] == CL_CONTEXT_PLATFORM){
                props[i+1] = ((cl_platform_id)(props[i+1]))->ptr;
            }
        }
    }
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &num_properties, sizeof(cl_uint), MSG_MORE);
    if(num_properties)
        Send(sockfd, props, num_properties*sizeof(cl_context_properties), MSG_MORE);
    Send(sockfd, &device_type, sizeof(cl_device_type), 0);
    free(props); props=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &context, sizeof(cl_context), MSG_WAITALL);
    addShortcut((void*)context, sockfd);
    return context;
}

cl_int oclandRetainContext(cl_context context)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clRetainContext;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseContext(cl_context context)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clReleaseContext;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(context);
    return flag;
}

cl_int oclandGetContextInfo(cl_context         context,
                            cl_context_info    param_name,
                            size_t             param_value_size,
                            void *             param_value,
                            size_t *           param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetContextInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_context_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_command_queue oclandCreateCommandQueue(cl_context                     context,
                                          cl_device_id                   device,
                                          cl_command_queue_properties    properties,
                                          cl_int *                       errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_command_queue command_queue = NULL;
    unsigned int comm = ocland_clCreateCommandQueue;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &(device->ptr), sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, &properties, sizeof(cl_command_queue_properties), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &command_queue, sizeof(cl_command_queue), MSG_WAITALL);
    addShortcut((void*)command_queue, sockfd);
    return command_queue;
}

cl_int oclandRetainCommandQueue(cl_command_queue command_queue)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clRetainCommandQueue;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseCommandQueue(cl_command_queue command_queue)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clReleaseCommandQueue;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(command_queue);
    return flag;
}

cl_int oclandGetCommandQueueInfo(cl_command_queue      command_queue,
                                 cl_command_queue_info param_name,
                                 size_t                param_value_size,
                                 void *                param_value,
                                 size_t *              param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetCommandQueueInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_command_queue_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_mem oclandCreateBuffer(cl_context    context ,
                          cl_mem_flags  flags ,
                          size_t        size ,
                          void *        host_ptr ,
                          cl_int *      errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_mem mem = NULL;
    unsigned int comm = ocland_clCreateBuffer;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr)
        hasPtr = CL_TRUE;
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    Send(sockfd, &size, sizeof(size_t), MSG_MORE);
    if(flags & CL_MEM_COPY_HOST_PTR){
        // Send the data compressed
        dataPack in, out;
        in.size = size;
        in.data = host_ptr;
        out = pack(in);
        Send(sockfd, &hasPtr, sizeof(cl_bool), MSG_MORE);
        Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    else{
        Send(sockfd, &hasPtr, sizeof(cl_bool), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &mem, sizeof(cl_mem), MSG_WAITALL);
    addShortcut((void*)mem, sockfd);
    return mem;
}

cl_int oclandRetainMemObject(cl_mem mem)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clRetainMemObject;
    // Get the server
    int *sockfd = mem->socket;
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(mem->ptr), sizeof(cl_mem), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseMemObject(cl_mem mem)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clReleaseMemObject;
    // Get the server
    int *sockfd = mem->socket;
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(mem->ptr), sizeof(cl_mem), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(mem);
    return flag;
}

cl_int oclandGetSupportedImageFormats(cl_context           context,
                                      cl_mem_flags         flags,
                                      cl_mem_object_type   image_type ,
                                      cl_uint              num_entries ,
                                      cl_image_format *    image_formats ,
                                      cl_uint *            num_image_formats)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint n=0;
    unsigned int comm = ocland_clGetSupportedImageFormats;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    Send(sockfd, &image_type, sizeof(cl_mem_object_type), MSG_MORE);
    Send(sockfd, &num_entries, sizeof(cl_uint), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &n, sizeof(cl_uint), MSG_WAITALL);
    if(num_image_formats) *num_image_formats = n;
    if(image_formats){
        if(num_entries < n)
            n = num_entries;
        Recv(sockfd, image_formats, n*sizeof(cl_image_format), MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandGetMemObjectInfo(cl_mem            mem ,
                              cl_mem_info       param_name ,
                              size_t            param_value_size ,
                              void *            param_value ,
                              size_t *          param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetMemObjectInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = mem->socket;
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(mem->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_mem_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandGetImageInfo(cl_mem            image ,
                          cl_image_info     param_name ,
                          size_t            param_value_size ,
                          void *            param_value ,
                          size_t *          param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetImageInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = image->socket;
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(image->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_image_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_sampler oclandCreateSampler(cl_context           context ,
                               cl_bool              normalized_coords ,
                               cl_addressing_mode   addressing_mode ,
                               cl_filter_mode       filter_mode ,
                               cl_int *             errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_sampler sampler = NULL;
    unsigned int comm = ocland_clCreateSampler;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &normalized_coords, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, &addressing_mode, sizeof(cl_addressing_mode), MSG_MORE);
    Send(sockfd, &filter_mode, sizeof(cl_filter_mode), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &sampler, sizeof(cl_sampler), MSG_WAITALL);
    addShortcut((void*)sampler, sockfd);
    return sampler;
}

cl_int oclandRetainSampler(cl_sampler sampler)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clRetainSampler;
    // Get the server
    int *sockfd = sampler->socket;
    if(!sockfd){
        return CL_INVALID_SAMPLER;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(sampler->ptr), sizeof(cl_sampler), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseSampler(cl_sampler sampler)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clReleaseSampler;
    // Get the server
    int *sockfd = sampler->socket;
    if(!sockfd){
        return CL_INVALID_SAMPLER;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(sampler->ptr), sizeof(cl_sampler), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(sampler);
    return flag;
}

cl_int oclandGetSamplerInfo(cl_sampler          sampler ,
                            cl_sampler_info     param_name ,
                            size_t              param_value_size ,
                            void *              param_value ,
                            size_t *            param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetSamplerInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = sampler->socket;
    if(!sockfd){
        return CL_INVALID_SAMPLER;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(sampler->ptr), sizeof(cl_sampler), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_sampler_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_program oclandCreateProgramWithSource(cl_context         context ,
                                         cl_uint            count ,
                                         const char **      strings ,
                                         const size_t *     lengths ,
                                         cl_int *           errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_program program = NULL;
    unsigned int comm = ocland_clCreateProgramWithSource;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    size_t* non_zero_lengths = calloc(count, sizeof(size_t));
    // Two cases handled:
    // 1) lengths is NULL - all strings are null-terminated
    // 2) some lengths are zero - those strings are null-terminated
    if(lengths){
        memcpy(non_zero_lengths, lengths, count * sizeof(size_t));
    }
    for(i=0;i<count;i++){
        if (0 == non_zero_lengths[i]){
            non_zero_lengths[i] = strlen(strings[i]);
        }
    }

    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context->ptr, sizeof(cl_context), MSG_MORE);
    Send(sockfd, &count, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, non_zero_lengths, count*sizeof(size_t), 0);
    for(i=0;i<count;i++){
        Send(sockfd, strings[i], non_zero_lengths[i], 0);
    }
    free(non_zero_lengths);
    non_zero_lengths = NULL;

    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &program, sizeof(cl_program), MSG_WAITALL);
    addShortcut((void*)program, sockfd);
    return program;
}

cl_program oclandCreateProgramWithBinary(cl_context                      context ,
                                         cl_uint                         num_devices ,
                                         const cl_device_id *            device_list ,
                                         const size_t *                  lengths ,
                                         const unsigned char **          binaries ,
                                         cl_int *                        binary_status ,
                                         cl_int *                        errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_program program = NULL;
    cl_int* status = NULL;
    unsigned int comm = ocland_clCreateProgramWithBinary;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->ptr;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Sustitute the local references to the remote ones
    cl_device_id devices[num_devices];
    for(i=0;i<num_devices;i++){
        devices[i] = device_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, devices, num_devices*sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, lengths, num_devices*sizeof(size_t), 0);
    for(i=0;i<num_devices;i++){
        Send(sockfd, binaries[i], lengths[i], 0);
    }
    free(devices);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &program, sizeof(cl_program), MSG_WAITALL);
    addShortcut((void*)program, sockfd);
    status = (cl_int*)malloc(num_devices*sizeof(cl_int));
    if(!status){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    Recv(sockfd, status, num_devices*sizeof(cl_int), MSG_WAITALL);
    if(binary_status)
        memcpy((void*)binary_status, (void*)status, num_devices*sizeof(cl_int));
    return program;
}

cl_int oclandRetainProgram(cl_program  program)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clRetainProgram;
    // Get the server
    int *sockfd = program->socket;
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &program->socket, sizeof(cl_program), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseProgram(cl_program  program)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clReleaseProgram;
    // Get the server
    int *sockfd = program->socket;
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(program->ptr), sizeof(cl_program), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(program->ptr);
    return flag;
}

cl_int oclandBuildProgram(cl_program            program ,
                          cl_uint               num_devices ,
                          const cl_device_id *  device_list ,
                          const char *          options ,
                          void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                          void *                user_data)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clBuildProgram;
    size_t options_size = (strlen(options) + 1)*sizeof(char);
    // Get the server
    int *sockfd = program->socket;
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Sustitute the local references to the remote ones
    cl_device_id devices[num_devices];
    for(i=0;i<num_devices;i++){
        devices[i] = device_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, devices, num_devices*sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, &options_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, options, options_size, 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandGetProgramInfo(cl_program          program ,
                            cl_program_info     param_name ,
                            size_t              param_value_size ,
                            void *              param_value ,
                            size_t *            param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetProgramInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = program->socket;
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_program_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(!param_value_size){
        return CL_SUCCESS;
    }
    if(param_name != CL_PROGRAM_BINARIES){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
        return CL_SUCCESS;
    }
    for(i = 0; i < program->num_devices; i++){
        if(!program->binary_lengths[i])
            continue;
        Recv(sockfd,
             program->binaries[i],
             program->binary_lengths[i],
             MSG_WAITALL);
        memcpy(((char**)param_value)[i],
               program->binaries[i],
               program->binary_lengths[i]);
    }
    return CL_SUCCESS;
}

cl_int oclandGetProgramBuildInfo(cl_program             program ,
                                 cl_device_id           device ,
                                 cl_program_build_info  param_name ,
                                 size_t                 param_value_size ,
                                 void *                 param_value ,
                                 size_t *               param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetProgramBuildInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = program->socket;
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    Send(sockfd, &(device->ptr), sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_program_build_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_kernel oclandCreateKernel(cl_program       program ,
                             const char *     kernel_name ,
                             cl_int *         errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_kernel kernel = NULL;
    unsigned int comm = ocland_clCreateKernel;
    size_t kernel_name_size = (strlen(kernel_name)+1)*sizeof(char);
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = program->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    Send(sockfd, &kernel_name_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, kernel_name, kernel_name_size, 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &kernel, sizeof(cl_kernel), MSG_WAITALL);
    addShortcut((void*)kernel, sockfd);
    return kernel;
}

cl_int oclandCreateKernelsInProgram(cl_program      program ,
                                    cl_uint         num_kernels ,
                                    cl_kernel *     kernels ,
                                    cl_uint *       num_kernels_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint n;
    unsigned int comm = ocland_clCreateKernelsInProgram;
    if(num_kernels_ret) *num_kernels_ret=0;
    // Get the server
    int *sockfd = program->socket;
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    Send(sockfd, &num_kernels, sizeof(cl_uint), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &n, sizeof(cl_uint), MSG_WAITALL);
    if(num_kernels_ret) *num_kernels_ret=n;
    if(kernels){
        if(num_kernels < n)
            n = num_kernels;
        Recv(sockfd, kernels, n*sizeof(cl_kernel), MSG_WAITALL);
        for(i=0;i<n;i++){
            addShortcut((void*)kernels[i], sockfd);
        }
    }
    return CL_SUCCESS;
}

cl_int oclandRetainKernel(cl_kernel     kernel)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clRetainKernel;
    // Get the server
    int *sockfd = kernel->socket;
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(kernel->ptr), sizeof(cl_kernel), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseKernel(cl_kernel    kernel)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clReleaseKernel;
    // Get the server
    int *sockfd = kernel->socket;
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(kernel->ptr), sizeof(cl_kernel), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(kernel->ptr);
    return flag;
}

cl_int oclandSetKernelArg(cl_kernel     kernel ,
                          cl_uint       arg_index ,
                          size_t        arg_size ,
                          const void *  arg_value)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clSetKernelArg;
    // Get the server
    int *sockfd = kernel->socket;
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(kernel->ptr), sizeof(cl_kernel), MSG_MORE);
    Send(sockfd, &arg_index, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, &arg_size, sizeof(size_t), MSG_MORE);
    if(arg_value){
        Send(sockfd, &arg_size, sizeof(size_t), MSG_MORE);
        Send(sockfd, arg_value, arg_size, 0);
    }
    else{  // local memory
        size_t null_size=0;
        Send(sockfd, &null_size, sizeof(size_t), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandGetKernelInfo(cl_kernel        kernel ,
                           cl_kernel_info   param_name ,
                           size_t           param_value_size ,
                           void *           param_value ,
                           size_t *         param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetKernelInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = kernel->socket;
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(kernel->ptr), sizeof(cl_kernel), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_kernel_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandGetKernelWorkGroupInfo(cl_kernel                   kernel ,
                                    cl_device_id                device ,
                                    cl_kernel_work_group_info   param_name ,
                                    size_t                      param_value_size ,
                                    void *                      param_value ,
                                    size_t *                    param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetKernelWorkGroupInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = kernel->socket;
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(kernel->ptr), sizeof(cl_kernel), MSG_MORE);
    Send(sockfd, &(device->ptr), sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_kernel_work_group_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandWaitForEvents(cl_uint              num_events ,
                           const cl_event *     event_list)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clWaitForEvents;
    // Get the server
    int *sockfd = event_list[0]->socket;
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Sustitute the local references to the remote ones
    cl_event events[num_events];
    for(i=0;i<num_events;i++){
        events[i] = event_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &num_events, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, events, num_events*sizeof(cl_event), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandGetEventInfo(cl_event          event ,
                          cl_event_info     param_name ,
                          size_t            param_value_size ,
                          void *            param_value ,
                          size_t *          param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetEventInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = event->socket;
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(event->ptr), sizeof(cl_event), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_event_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandReleaseEvent(cl_event  event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clReleaseEvent;
    // Get the server
    int *sockfd = event->socket;
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(event->ptr), sizeof(cl_event), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(event->ptr);
    return flag;
}

cl_int oclandGetEventProfilingInfo(cl_event             event ,
                                   cl_profiling_info    param_name ,
                                   size_t               param_value_size ,
                                   void *               param_value ,
                                   size_t *             param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetEventProfilingInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = event->socket;
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(event->ptr), sizeof(cl_event), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_profiling_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandFlush(cl_command_queue command_queue)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clFlush;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandFinish(cl_command_queue  command_queue)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clFinish;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

/** @struct dataTransfer Vars needed for
 * an asynchronously data transfer.
 */
struct dataTransfer{
    /// Port
    unsigned int port;
    /// Socket
    int fd;
    /// Size of data
    size_t cb;
    /// Data array
    void *ptr;
};

/** Thread that receives data from server.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataRecv_thread(void *data)
{
    struct dataTransfer* _data = (struct dataTransfer*)data;
    // Connect to the received port.
    unsigned int port = _data->port;
    struct sockaddr_in serv_addr, adr_inet;
    //! @todo set SO_PRIORITY
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        printf("ERROR: Can't register a new socket for the asynchronous data transfer\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    socklen_t len_inet;
    len_inet = sizeof(serv_addr);
    const char* ip = serverAddress(_data->fd);
    if(!ip){
        printf("ERROR: Can't find the server associated with the socket\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0){
        // we can't work, disconnect from server
        printf("ERROR: Invalid address assigment (%s)\n", ip); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    while( connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ){
        if(errno == ECONNREFUSED){
            // Pobably the server is not ready yet, simply retry
            // it until the server start listening
            continue;
        }
        // we can't work, disconnect from server
        printf("ERROR: Can't connect for the asynchronous data transfer\n"); fflush(stdout);
        printf("\t%s\n", SocketsError()); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    dataPack in, out;
    out.size = _data->cb;
    out.data = _data->ptr;
    Recv(&fd, &(in.size), sizeof(size_t), MSG_WAITALL);
    if(in.size == 0){
        printf("Error uncompressing data:\n\tnull array size received"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    in.data = malloc(in.size);
    Recv(&fd, in.data, in.size, MSG_WAITALL);
    unpack(out,in);
    free(in.data); in.data=NULL;
    THREAD_SAFE_EXIT;
}

/** Performs a data reception asynchronously on a new thread and socket.
 * @param sockfd Connection socket.
 * @param data Data to transfer.
 */
void asyncDataRecv(int* sockfd, struct dataTransfer data)
{
    // Open a new thread to connect to the new port
    // and receive the data
    pthread_t thread;
    struct dataTransfer* _data = (struct dataTransfer*)malloc(sizeof(struct dataTransfer));
    _data->port  = data.port;
    _data->fd    = data.fd;
    _data->cb    = data.cb;
    _data->ptr   = data.ptr;
    int rc = pthread_create(&thread, NULL, asyncDataRecv_thread, (void *)(_data));
}

cl_int oclandEnqueueReadBuffer(cl_command_queue     command_queue ,
                               cl_mem               buffer ,
                               cl_bool              blocking_read ,
                               size_t               offset ,
                               size_t               cb ,
                               void *               ptr ,
                               cl_uint              num_events_in_wait_list ,
                               const cl_event *     event_wait_list ,
                               cl_event *           event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueReadBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(buffer->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &blocking_read, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, &offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &cb, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag, the event, and the data.
    // ------------------------------------------------------------
    if(blocking_read){
        if(event){
            Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
            addShortcut(*event, sockfd);
        }
        dataPack in, out;
        out.size = cb;
        out.data = ptr;
        Recv(sockfd, &(in.size), sizeof(size_t), MSG_WAITALL);
        in.data = malloc(in.size);
        Recv(sockfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
        return CL_SUCCESS;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    unsigned int port;
    Recv(sockfd, &port, sizeof(unsigned int), MSG_WAITALL);
    struct dataTransfer data;
    data.port  = port;
    data.fd    = *sockfd;
    data.cb    = cb;
    data.ptr   = ptr;
    asyncDataRecv(sockfd, data);
    return CL_SUCCESS;
}

/** Thread that sends data to server.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataSend_thread(void *data)
{
    struct dataTransfer* _data = (struct dataTransfer*)data;
    // Connect to the received port.
    unsigned int port = _data->port;
    struct sockaddr_in serv_addr;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        printf("ERROR: Can't register a new socket for the asynchronous data transfer\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    socklen_t len_inet;
    len_inet = sizeof(serv_addr);
    const char* ip = serverAddress(_data->fd);
    if(!ip){
        printf("ERROR: Can't find the server associated with the socket\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0){
        // we can't work, disconnect from server
        printf("ERROR: Invalid address assigment (%s)\n", ip); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    while( connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ){
        if(errno == ECONNREFUSED){
            // Pobably the server is not ready yet, simply retry
            // it until the server start listening
            continue;
        }
        // we can't work, disconnect from server
        printf("ERROR: Can't connect for the asynchronous data transfer\n"); fflush(stdout);
        printf("\t%s\n", SocketsError()); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    // Return the data (compressed) to the client
    dataPack in, out;
    in.size = _data->cb;
    in.data = _data->ptr;
    out = pack(in);
    // Since the array size is not the original one anymore, we need to
    // send the array size before to send the data
    Send(&fd, &(out.size), sizeof(size_t), MSG_MORE);
    Send(&fd, out.data, out.size, 0);
    // Clean up
    free(out.data); out.data = NULL;
    THREAD_SAFE_EXIT;
}

/** Performs a data reception asynchronously on a new thread and socket.
 * @param sockfd Connection socket.
 * @param data Data to transfer.
 */
void asyncDataSend(int* sockfd, struct dataTransfer data)
{
    // Open a new thread to connect to the new port
    // and receive the data
    pthread_t thread;
    struct dataTransfer* _data = (struct dataTransfer*)malloc(sizeof(struct dataTransfer));
    _data->port  = data.port;
    _data->fd    = data.fd;
    _data->cb    = data.cb;
    _data->ptr   = data.ptr;
    int rc = pthread_create(&thread, NULL, asyncDataSend_thread, (void *)(_data));
}

cl_int oclandEnqueueWriteBuffer(cl_command_queue    command_queue ,
                                cl_mem              buffer ,
                                cl_bool             blocking_write ,
                                size_t              offset ,
                                size_t              cb ,
                                const void *        ptr ,
                                cl_uint             num_events_in_wait_list ,
                                const cl_event *    event_wait_list ,
                                cl_event *          event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueWriteBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(buffer->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &blocking_write, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, &offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &cb, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    int ending = 0;
    if(blocking_write) ending = MSG_MORE;
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), ending);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), ending);
    }
    free(events_wait); events_wait=NULL;
    if(blocking_write){
        dataPack in, out;
        in.size = cb;
        in.data = ptr;
        out = pack(in);
        Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag and the event.
    // ------------------------------------------------------------
    if(blocking_write){
        if(event){
            Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
            addShortcut(*event, sockfd);
        }
        return CL_SUCCESS;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    unsigned int port;
    Recv(sockfd, &port, sizeof(unsigned int), MSG_WAITALL);
    struct dataTransfer data;
    data.port  = port;
    data.fd    = *sockfd;
    data.cb    = cb;
    data.ptr   = (void*)ptr;
    asyncDataSend(sockfd, data);
    return flag;
}

cl_int oclandEnqueueCopyBuffer(cl_command_queue     command_queue ,
                               cl_mem               src_buffer ,
                               cl_mem               dst_buffer ,
                               size_t               src_offset ,
                               size_t               dst_offset ,
                               size_t               cb ,
                               cl_uint              num_events_in_wait_list ,
                               const cl_event *     event_wait_list ,
                               cl_event *           event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueCopyBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(src_buffer->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &(dst_buffer->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &src_offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &dst_offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &cb, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

cl_int oclandEnqueueCopyImage(cl_command_queue      command_queue ,
                              cl_mem                src_image ,
                              cl_mem                dst_image ,
                              const size_t *        src_origin ,
                              const size_t *        dst_origin ,
                              const size_t *        region ,
                              cl_uint               num_events_in_wait_list ,
                              const cl_event *      event_wait_list ,
                              cl_event *            event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueCopyImage;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(src_image->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &(dst_image->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, src_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, dst_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

cl_int oclandEnqueueCopyImageToBuffer(cl_command_queue  command_queue ,
                                      cl_mem            src_image ,
                                      cl_mem            dst_buffer ,
                                      const size_t *    src_origin ,
                                      const size_t *    region ,
                                      size_t            dst_offset ,
                                      cl_uint           num_events_in_wait_list ,
                                      const cl_event *  event_wait_list ,
                                      cl_event *        event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueCopyImageToBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(src_image->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &(dst_buffer->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, src_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &dst_offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

cl_int oclandEnqueueCopyBufferToImage(cl_command_queue  command_queue ,
                                      cl_mem            src_buffer ,
                                      cl_mem            dst_image ,
                                      size_t            src_offset ,
                                      const size_t *    dst_origin ,
                                      const size_t *    region ,
                                      cl_uint           num_events_in_wait_list ,
                                      const cl_event *  event_wait_list ,
                                      cl_event *        event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueCopyBufferToImage;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(src_buffer->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &(dst_image->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &src_offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, dst_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

cl_int oclandEnqueueNDRangeKernel(cl_command_queue  command_queue ,
                                  cl_kernel         kernel ,
                                  cl_uint           work_dim ,
                                  const size_t *    global_work_offset ,
                                  const size_t *    global_work_size ,
                                  const size_t *    local_work_size ,
                                  cl_uint           num_events_in_wait_list ,
                                  const cl_event *  event_wait_list ,
                                  cl_event *        event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueNDRangeKernel;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    cl_bool has_global_work_offset = CL_FALSE;
    if(global_work_offset) has_global_work_offset = CL_TRUE;
    cl_bool has_local_work_size = CL_FALSE;
    if(local_work_size) has_local_work_size = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(kernel->ptr), sizeof(cl_kernel), MSG_MORE);
    Send(sockfd, &work_dim, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, &has_global_work_offset, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, &has_local_work_size, sizeof(cl_bool), MSG_MORE);
    if(has_global_work_offset)
        Send(sockfd, global_work_offset, work_dim*sizeof(size_t), MSG_MORE);
    Send(sockfd, global_work_size, work_dim*sizeof(size_t), MSG_MORE);
    if(has_local_work_size)
        Send(sockfd, local_work_size, work_dim*sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

/** @struct dataTransferRect Vars needed for
 * an asynchronously data transfer in 2D,3D
 * rectangle.
 */
struct dataTransferRect{
    /// Port
    unsigned int port;
    /// Socket
    int fd;
    /// Region to read
    const size_t *region;
    /// Size of a row
    size_t row;
    /// Size of a 2D slice (row*column)
    size_t slice;
    /// Size of data
    size_t cb;
    /// Data array (conviniently sifted with origin)
    void *ptr;
};

/** Thread that receives data from server for
 * a clEnqueueReadBufferRect specific command.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataRecvRect_thread(void *data)
{
    struct dataTransferRect* _data = (struct dataTransferRect*)data;
    // Connect to the received port.
    unsigned int port = _data->port;
    struct sockaddr_in serv_addr;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        printf("ERROR: Can't register a new socket for the asynchronous data transfer\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    socklen_t len_inet;
    len_inet = sizeof(serv_addr);
    const char* ip = serverAddress(_data->fd);
    if(!ip){
        printf("ERROR: Can't find the server associated with the socket\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0){
        // we can't work, disconnect from server
        printf("ERROR: Invalid address assigment (%s)\n", ip); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    while( connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ){
        if(errno == ECONNREFUSED){
            // Pobably the server is not ready yet, simply retry
            // it until the server start listening
            continue;
        }
        // we can't work, disconnect from server
        printf("ERROR: Can't connect for the asynchronous data transfer\n"); fflush(stdout);
        printf("\t%s\n", SocketsError()); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    dataPack in, out;
    out.size = _data->cb;
    out.data = _data->ptr;
    Recv(&fd, &(in.size), sizeof(size_t), MSG_WAITALL);
    if(in.size == 0){
        printf("Error uncompressing data:\n\tnull array size received"); fflush(stdout);
        free(_data); _data=NULL;
        THREAD_SAFE_EXIT;
    }
    in.data = malloc(in.size);
    Recv(&fd, in.data, in.size, MSG_WAITALL);
    unpack(out,in);
    free(in.data); in.data=NULL;
    THREAD_SAFE_EXIT;
}

/** Performs a data reception asynchronously on a new thread and socket, for
 * a clEnqueueReadBufferRect specific command.
 * @param sockfd Connection socket.
 * @param data Data to transfer.
 */
void asyncDataRecvRect(int* sockfd, struct dataTransferRect data)
{
    // Open a new thread to connect to the new port
    // and receive the data
    pthread_t thread;
    struct dataTransferRect* _data = (struct dataTransferRect*)malloc(sizeof(struct dataTransferRect));
    _data->port  = data.port;
    _data->fd    = data.fd;
    _data->region = data.region;
    _data->row    = data.row;
    _data->slice  = data.slice;
    _data->cb    = data.cb;
    _data->ptr   = data.ptr;
    int rc = pthread_create(&thread, NULL, asyncDataRecvRect_thread, (void *)(_data));
}

cl_int oclandEnqueueReadImage(cl_command_queue      command_queue ,
                              cl_mem                image ,
                              cl_bool               blocking_read ,
                              const size_t *        origin ,
                              const size_t *        region ,
                              size_t                row_pitch ,
                              size_t                slice_pitch ,
                              size_t                element_size ,
                              void *                ptr ,
                              cl_uint               num_events_in_wait_list ,
                              const cl_event *      event_wait_list ,
                              cl_event *            event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueReadImage;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    size_t cb = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(image->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &blocking_read, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &slice_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &element_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag, the event, and the data.
    // ------------------------------------------------------------
    if(blocking_read){
        if(event){
            Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
            addShortcut(*event, sockfd);
        }
        dataPack in, out;
        out.size = cb;
        out.data = ptr;
        Recv(sockfd, &(in.size), sizeof(size_t), MSG_WAITALL);
        in.data = malloc(in.size);
        Recv(sockfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
        return CL_SUCCESS;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    unsigned int port;
    Recv(sockfd, &port, sizeof(unsigned int), MSG_WAITALL);
    struct dataTransferRect data;
    data.port   = port;
    data.fd     = *sockfd;
    data.region = region;
    data.row    = row_pitch;
    data.slice  = slice_pitch;
    data.cb     = cb;
    data.ptr    = ptr;
    asyncDataRecvRect(sockfd, data);
    return CL_SUCCESS;
}

/** Thread that sends data to server.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataSendRect_thread(void *data)
{
    struct dataTransferRect* _data = (struct dataTransferRect*)data;
    // Connect to the received port.
    unsigned int port = _data->port;
    struct sockaddr_in serv_addr;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        printf("ERROR: Can't register a new socket for the asynchronous data transfer\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    socklen_t len_inet;
    len_inet = sizeof(serv_addr);
    const char* ip = serverAddress(_data->fd);
    if(!ip){
        printf("ERROR: Can't find the server associated with the socket\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0){
        // we can't work, disconnect from server
        printf("ERROR: Invalid address assigment (%s)\n", ip); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    while( connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ){
        if(errno == ECONNREFUSED){
            // Pobably the server is not ready yet, simply retry
            // it until the server start listening
            continue;
        }
        // we can't work, disconnect from server
        printf("ERROR: Can't connect for the asynchronous data transfer\n"); fflush(stdout);
        printf("\t%s\n", SocketsError()); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    // Return the data (compressed) to the client
    dataPack in, out;
    in.size = _data->cb;
    in.data = _data->ptr;
    out = pack(in);
    // Since the array size is not the original one anymore, we need to
    // send the array size before to send the data
    Send(&fd, &(out.size), sizeof(size_t), MSG_MORE);
    Send(&fd, out.data, out.size, 0);
    // Clean up
    free(out.data); out.data = NULL;
    THREAD_SAFE_EXIT;
}

/** Performs a data reception asynchronously on a new thread and socket.
 * @param sockfd Connection socket.
 * @param data Data to transfer.
 */
void asyncDataSendRect(int* sockfd, struct dataTransferRect data)
{
    // Open a new thread to connect to the new port
    // and receive the data
    pthread_t thread;
    struct dataTransferRect* _data = (struct dataTransferRect*)malloc(sizeof(struct dataTransferRect));
    _data->port  = data.port;
    _data->fd    = data.fd;
    _data->region = data.region;
    _data->row    = data.row;
    _data->slice  = data.slice;
    _data->cb    = data.cb;
    _data->ptr   = data.ptr;
    int rc = pthread_create(&thread, NULL, asyncDataSendRect_thread, (void *)(_data));
}

cl_int oclandEnqueueWriteImage(cl_command_queue     command_queue ,
                               cl_mem               image ,
                               cl_bool              blocking_write ,
                               const size_t *       origin ,
                               const size_t *       region ,
                               size_t               row_pitch ,
                               size_t               slice_pitch ,
                               size_t               element_size ,
                               const void *         ptr ,
                               cl_uint              num_events_in_wait_list ,
                               const cl_event *     event_wait_list ,
                               cl_event *           event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueWriteImage;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    size_t cb = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &image, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &blocking_write, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &slice_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &element_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    int ending = 0;
    if(blocking_write) ending = MSG_MORE;
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), ending);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), ending);
    }
    free(events_wait); events_wait=NULL;
    if(blocking_write){
        dataPack in, out;
        in.size = cb;
        in.data = ptr;
        out = pack(in);
        Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag and the event.
    // ------------------------------------------------------------
    if(blocking_write){
        if(event){
            Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
            addShortcut(*event, sockfd);
        }
        return CL_SUCCESS;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    unsigned int port;
    Recv(sockfd, &port, sizeof(unsigned int), MSG_WAITALL);
    struct dataTransferRect data;
    data.port  = port;
    data.fd    = *sockfd;
    data.region = region;
    data.row    = row_pitch;
    data.slice  = slice_pitch;
    data.cb    = cb;
    data.ptr   = (void*)ptr;
    asyncDataSendRect(sockfd, data);
    return flag;
}

cl_mem oclandCreateImage2D(cl_context context,
                           cl_mem_flags flags,
                           const cl_image_format *image_format,
                           size_t image_width,
                           size_t image_height,
                           size_t image_row_pitch,
                           size_t element_size,
                           void *host_ptr,
                           cl_int *errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_mem mem = NULL;
    unsigned int comm = ocland_clCreateImage2D;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr)
        hasPtr = CL_TRUE;
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    Send(sockfd, image_format, sizeof(cl_image_format), MSG_MORE);
    Send(sockfd, &image_width, sizeof(size_t), MSG_MORE);
    Send(sockfd, &image_height, sizeof(size_t), MSG_MORE);
    Send(sockfd, &image_row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &element_size, sizeof(size_t), MSG_MORE);
    if(flags & CL_MEM_COPY_HOST_PTR){
        size_t size = image_width*image_height*element_size;
        // Send the data compressed
        dataPack in, out;
        in.size = size;
        in.data = host_ptr;
        out = pack(in);
        Send(sockfd, &hasPtr, sizeof(cl_bool), MSG_MORE);
        Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    else{
        Send(sockfd, &hasPtr, sizeof(cl_bool), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &mem, sizeof(cl_mem), MSG_WAITALL);
    addShortcut((void*)mem, sockfd);
    return mem;
}

cl_mem oclandCreateImage3D(cl_context context,
                           cl_mem_flags flags,
                           const cl_image_format *image_format,
                           size_t image_width,
                           size_t image_height,
                           size_t image_depth,
                           size_t image_row_pitch,
                           size_t image_slice_pitch,
                           size_t element_size,
                           void *host_ptr,
                           cl_int *errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_mem mem = NULL;
    unsigned int comm = ocland_clCreateImage3D;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr)
        hasPtr = CL_TRUE;
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    Send(sockfd, image_format, sizeof(cl_image_format), MSG_MORE);
    Send(sockfd, &image_width, sizeof(size_t), MSG_MORE);
    Send(sockfd, &image_height, sizeof(size_t), MSG_MORE);
    Send(sockfd, &image_depth, sizeof(size_t), MSG_MORE);
    Send(sockfd, &image_row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &image_slice_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &element_size, sizeof(size_t), MSG_MORE);
    if(flags & CL_MEM_COPY_HOST_PTR){
        size_t size = image_width*image_height*image_depth*element_size;
        // Send the data compressed
        dataPack in, out;
        in.size = size;
        in.data = host_ptr;
        out = pack(in);
        Send(sockfd, &hasPtr, sizeof(cl_bool), MSG_MORE);
        Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    else{
        Send(sockfd, &hasPtr, sizeof(cl_bool), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &mem, sizeof(cl_mem), MSG_WAITALL);
    addShortcut((void*)mem, sockfd);
    return mem;
}

// -------------------------------------------- //
//                                              //
// OpenCL 1.1 methods                           //
//                                              //
// -------------------------------------------- //

cl_mem oclandCreateSubBuffer(cl_mem                    buffer ,
                             cl_mem_flags              flags ,
                             cl_buffer_create_type     buffer_create_type ,
                             const void *              buffer_create_info ,
                             cl_int *                  errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_mem mem = NULL;
    unsigned int comm = ocland_clCreateSubBuffer;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    size_t buffer_create_info_size = 0;
    if(buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION)
        buffer_create_info_size = sizeof(cl_buffer_region);
    else
        return CL_INVALID_VALUE;
    // Get the server
    int *sockfd = buffer->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_MEM_OBJECT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(buffer->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    Send(sockfd, &buffer_create_type, sizeof(cl_buffer_create_type), MSG_MORE);
    Send(sockfd, &buffer_create_info_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, buffer_create_info, buffer_create_info_size, 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &mem, sizeof(cl_mem), MSG_WAITALL);
    addShortcut((void*)mem, sockfd);
    return mem;
}

cl_event oclandCreateUserEvent(cl_context     context ,
                               cl_int *       errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_event event = NULL;
    unsigned int comm = ocland_clCreateUserEvent;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &event, sizeof(cl_event), MSG_WAITALL);
    addShortcut((void*)event, sockfd);
    return event;
}

cl_int oclandSetUserEventStatus(cl_event    event ,
                                cl_int      execution_status)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clSetUserEventStatus;
    // Get the server
    int *sockfd = event->socket;
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(event->ptr), sizeof(cl_event), 0);
    Send(sockfd, &execution_status, sizeof(cl_int), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandEnqueueReadBufferRect(cl_command_queue     command_queue ,
                                   cl_mem               mem ,
                                   cl_bool              blocking_read ,
                                   const size_t *       buffer_origin ,
                                   const size_t *       host_origin ,
                                   const size_t *       region ,
                                   size_t               buffer_row_pitch ,
                                   size_t               buffer_slice_pitch ,
                                   size_t               host_row_pitch ,
                                   size_t               host_slice_pitch ,
                                   void *               ptr ,
                                   cl_uint              num_events_in_wait_list ,
                                   const cl_event *     event_wait_list ,
                                   cl_event *           event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueReadBufferRect;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    size_t origin = host_origin[0] + host_origin[1]*host_row_pitch + host_origin[2]*host_slice_pitch;
    size_t cb = region[0] + region[1]*host_row_pitch + region[2]*host_slice_pitch;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(mem->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &blocking_read, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, buffer_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, host_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &buffer_row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &buffer_slice_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &host_row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &host_slice_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag, the event, and the data.
    // ------------------------------------------------------------
    if(blocking_read){
        if(event){
            Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
            addShortcut(*event, sockfd);
        }
        dataPack in, out;
        out.size = cb;
        out.data = ptr + origin;
        Recv(sockfd, &(in.size), sizeof(size_t), MSG_WAITALL);
        in.data = malloc(in.size);
        Recv(sockfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data=NULL;
        return CL_SUCCESS;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    unsigned int port;
    Recv(sockfd, &port, sizeof(unsigned int), MSG_WAITALL);
    struct dataTransferRect data;
    data.port   = port;
    data.fd     = *sockfd;
    data.region = region;
    data.row    = host_row_pitch;
    data.slice  = host_slice_pitch;
    data.cb     = cb;
    data.ptr    = ptr;
    asyncDataRecvRect(sockfd, data);
    return CL_SUCCESS;
}

cl_int oclandEnqueueWriteBufferRect(cl_command_queue     command_queue ,
                                    cl_mem               mem ,
                                    cl_bool              blocking_write ,
                                    const size_t *       buffer_origin ,
                                    const size_t *       host_origin ,
                                    const size_t *       region ,
                                    size_t               buffer_row_pitch ,
                                    size_t               buffer_slice_pitch ,
                                    size_t               host_row_pitch ,
                                    size_t               host_slice_pitch ,
                                    const void *         ptr ,
                                    cl_uint              num_events_in_wait_list ,
                                    const cl_event *     event_wait_list ,
                                    cl_event *           event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueWriteImage;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    size_t cb = region[2]*host_slice_pitch + region[1]*host_row_pitch + region[0];
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(mem->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &blocking_write, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, buffer_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &buffer_row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &buffer_slice_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &host_row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &host_slice_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    int ending = 0;
    if(blocking_write) ending = MSG_MORE;
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), ending);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), ending);
    }
    free(events_wait); events_wait=NULL;
    if(blocking_write){
        dataPack in, out;
        in.size = cb;
        in.data = ptr;
        out = pack(in);
        Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag and the event.
    // ------------------------------------------------------------
    if(blocking_write){
        if(event){
            Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
            addShortcut(*event, sockfd);
        }
        return CL_SUCCESS;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    unsigned int port;
    Recv(sockfd, &port, sizeof(unsigned int), MSG_WAITALL);

    struct dataTransferRect data;
    data.port   = port;
    data.fd     = *sockfd;
    data.region = region;
    data.row    = host_row_pitch;
    data.slice  = host_slice_pitch;
    data.cb     = cb;
    data.ptr    = (void*)ptr;
    asyncDataSendRect(sockfd, data);
    return flag;
}

cl_int oclandEnqueueCopyBufferRect(cl_command_queue     command_queue ,
                                   cl_mem               src_buffer ,
                                   cl_mem               dst_buffer ,
                                   const size_t *       src_origin ,
                                   const size_t *       dst_origin ,
                                   const size_t *       region ,
                                   size_t               src_row_pitch ,
                                   size_t               src_slice_pitch ,
                                   size_t               dst_row_pitch ,
                                   size_t               dst_slice_pitch ,
                                   cl_uint              num_events_in_wait_list ,
                                   const cl_event *     event_wait_list ,
                                   cl_event *           event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueCopyBufferRect;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(src_buffer->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &(dst_buffer->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, src_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, dst_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &src_row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &src_slice_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &dst_row_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &dst_slice_pitch, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

// -------------------------------------------- //
//                                              //
// OpenCL 1.2 methods                           //
//                                              //
// -------------------------------------------- //

cl_int oclandCreateSubDevices(cl_device_id                         in_device,
                              const cl_device_partition_property * properties,
                              cl_uint                              num_properties,
                              cl_uint                              num_entries,
                              cl_device_id                       * devices,
                              cl_uint                            * num_devices)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint n;
    unsigned int comm = ocland_clCreateSubDevices;
    if(num_devices) *num_devices = 0;
    int *sockfd = in_device->socket;
    if(!sockfd){
        return CL_INVALID_DEVICE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(in_device->ptr), sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, &num_properties, sizeof(cl_uint), MSG_MORE);
    if(num_properties)
        Send(sockfd, properties, num_properties*sizeof(cl_device_partition_property), MSG_MORE);
    Send(sockfd, &num_entries, sizeof(cl_uint), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &n, sizeof(cl_uint), MSG_WAITALL);
    if(num_devices) *num_devices = n;
    if(num_entries < n)
        n = num_entries;
    if(devices){
        Recv(sockfd, devices, n*sizeof(cl_device_id), MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandRetainDevice(cl_device_id device)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clRetainDevice;
    int *sockfd = device->socket;
    if(!sockfd){
        return CL_INVALID_DEVICE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(device->ptr), sizeof(cl_device_id), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    return CL_SUCCESS;
}

cl_int oclandReleaseDevice(cl_device_id device)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clReleaseDevice;
    int *sockfd = device->socket;
    if(!sockfd){
        return CL_INVALID_DEVICE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(device->ptr), sizeof(cl_device_id), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    return CL_SUCCESS;
}

cl_mem oclandCreateImage(cl_context              context,
                         cl_mem_flags            flags,
                         const cl_image_format * image_format,
                         const cl_image_desc *   image_desc,
                         size_t                  element_size,
                         void *                  host_ptr,
                         cl_int *                errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_mem image = NULL;
    unsigned int comm = ocland_clCreateImage;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Move from host references to the remote ones
    cl_image_desc descriptor;
    memcpy(&descriptor, image_desc, sizeof(cl_image_desc));
    if(descriptor.buffer)
        descriptor.buffer = descriptor.buffer->ptr;
    // Send the command data
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr)
        hasPtr = CL_TRUE;
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    Send(sockfd, image_format, sizeof(cl_image_format), MSG_MORE);
    Send(sockfd, image_desc, sizeof(cl_image_desc), MSG_MORE);
    Send(sockfd, &element_size, sizeof(size_t), MSG_MORE);
    if(flags & CL_MEM_COPY_HOST_PTR){
        size_t size = image_desc->image_width*image_desc->image_height*image_desc->image_depth*element_size;
        // Send the data compressed
        dataPack in, out;
        in.size = size;
        in.data = host_ptr;
        out = pack(in);
        Send(sockfd, &hasPtr, sizeof(cl_bool), MSG_MORE);
        Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    else{
        Send(sockfd, &hasPtr, sizeof(cl_bool), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &image, sizeof(cl_mem), MSG_WAITALL);
    addShortcut((void*)image, sockfd);
    return image;
}

cl_program oclandCreateProgramWithBuiltInKernels(cl_context             context ,
                                                 cl_uint                num_devices ,
                                                 const cl_device_id *   device_list ,
                                                 const char *           kernel_names ,
                                                 cl_int *               errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_program program = NULL;
    unsigned int comm = ocland_clCreateProgramWithBuiltInKernels;
    size_t kernel_names_size = (strlen(kernel_names) + 1)*sizeof(char);
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Sustitute the local references to the remote ones
    cl_device_id devices[num_devices];
    for(i=0;i<num_devices;i++){
        devices[i] = device_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, devices, num_devices*sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, &kernel_names_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, kernel_names, kernel_names_size, 0);
    free(devices);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    Recv(sockfd, &program, sizeof(cl_program), MSG_WAITALL);
    addShortcut((void*)program, sockfd);
    return program;
}

cl_int oclandCompileProgram(cl_program            program ,
                            cl_uint               num_devices ,
                            const cl_device_id *  device_list ,
                            const char *          options ,
                            cl_uint               num_input_headers ,
                            const cl_program *    input_headers,
                            const char **         header_include_names ,
                            void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                            void *                user_data)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clCompileProgram;
    size_t str_size = (strlen(options) + 1)*sizeof(char);
    // Get the server
    int *sockfd = getShortcut(program);
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Sustitute the local references to the remote ones
    cl_device_id devices[num_devices];
    for(i=0;i<num_devices;i++){
        devices[i] = device_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, devices, num_devices*sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, &str_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, options, str_size, MSG_MORE);
    if(num_input_headers){
        Send(sockfd, &num_input_headers, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, input_headers, num_input_headers*sizeof(cl_program), MSG_MORE);
        for(i=0;i<num_input_headers-1;i++){
            str_size = (strlen(header_include_names[i]) + 1)*sizeof(char);
            Send(sockfd, &str_size, sizeof(size_t), MSG_MORE);
            Send(sockfd, header_include_names[i], str_size, MSG_MORE);
        }
        str_size = (strlen(header_include_names[i]) + 1)*sizeof(char);
        Send(sockfd, &str_size, sizeof(size_t), MSG_MORE);
        Send(sockfd, header_include_names[i], str_size, 0);
    }
    else{
        Send(sockfd, &num_input_headers, sizeof(cl_uint), 0);
    }
    free(devices);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_program oclandLinkProgram(cl_context            context ,
                             cl_uint               num_devices ,
                             const cl_device_id *  device_list ,
                             const char *          options ,
                             cl_uint               num_input_programs ,
                             const cl_program *    input_programs ,
                             void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                             void *                user_data ,
                             cl_int *              errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_program program=NULL;
    unsigned int comm = ocland_clLinkProgram;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    size_t str_size = (strlen(options) + 1)*sizeof(char);
    // Get the server
    int *sockfd = context->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Sustitute the local references to the remote ones
    cl_device_id devices[num_devices];
    for(i=0;i<num_devices;i++){
        devices[i] = device_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(context->ptr), sizeof(cl_context), MSG_MORE);
    Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, devices, num_devices*sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, &str_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, options, str_size, MSG_MORE);
    if(num_input_programs){
        cl_program programs[num_input_programs];
        for(i=0;i<num_input_programs;i++){
            programs[i] = input_programs[i]->ptr;
        }
        Send(sockfd, &num_input_programs, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, programs, num_input_programs*sizeof(cl_program), 0);
        free(programs);
    }
    else{
        Send(sockfd, &num_input_programs, sizeof(cl_uint), 0);
    }
    free(devices);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    return program;
}

cl_int oclandUnloadPlatformCompiler(cl_platform_id  platform)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clUnloadPlatformCompiler;
    // Get the server
    int *sockfd = &(platform->socket);
    if(!sockfd){
        return CL_INVALID_PLATFORM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(platform->ptr), sizeof(cl_platform_id), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandGetKernelArgInfo(cl_kernel            kernel ,
                              cl_uint              arg_index ,
                              cl_kernel_arg_info   param_name ,
                              size_t               param_value_size ,
                              void *               param_value ,
                              size_t *             param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetKernelArgInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = kernel->socket;
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(kernel->ptr), sizeof(cl_kernel), MSG_MORE);
    Send(sockfd, &arg_index, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_kernel_arg_info), MSG_MORE);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS){
        return flag;
    }
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        Recv(sockfd, param_value, size_ret, MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandEnqueueFillBuffer(cl_command_queue    command_queue ,
                               cl_mem              mem ,
                               const void *        pattern ,
                               size_t              pattern_size ,
                               size_t              offset ,
                               size_t              cb ,
                               cl_uint             num_events_in_wait_list ,
                               const cl_event *    event_wait_list ,
                               cl_event *          event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueFillBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &(mem->ptr), sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &pattern_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, pattern, pattern_size, MSG_MORE);
    Send(sockfd, &offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &cb, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

cl_int oclandEnqueueFillImage(cl_command_queue    command_queue ,
                              cl_mem              image ,
                              size_t              fill_color_size ,
                              const void *        fill_color ,
                              const size_t *      origin ,
                              const size_t *      region ,
                              cl_uint             num_events_in_wait_list ,
                              const cl_event *    event_wait_list ,
                              cl_event *          event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueFillImage;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &image, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &fill_color_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, fill_color, fill_color_size, MSG_MORE);
    Send(sockfd, origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

cl_int oclandEnqueueMigrateMemObjects(cl_command_queue        command_queue ,
                                      cl_uint                 num_mem_objects ,
                                      const cl_mem *          mem_objects ,
                                      cl_mem_migration_flags  flags ,
                                      cl_uint                 num_events_in_wait_list ,
                                      const cl_event *        event_wait_list ,
                                      cl_event *              event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueMigrateMemObjects;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events (and mems) from the local references to the remote ones
    cl_mem *mems = (cl_mem*)malloc(num_mem_objects*sizeof(cl_mem));
    if(!mems){
        return CL_OUT_OF_HOST_MEMORY;
    }
    for(i=0;i<num_mem_objects;i++)
        mems[i] = mem_objects[i]->ptr;
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &num_mem_objects, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, mems, num_mem_objects*sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &flags, sizeof(cl_mem_migration_flags), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(mems); mems=NULL;
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

cl_int oclandEnqueueMarkerWithWaitList(cl_command_queue  command_queue ,
                                       cl_uint            num_events_in_wait_list ,
                                       const cl_event *   event_wait_list ,
                                       cl_event *         event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueMarkerWithWaitList;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

cl_int oclandEnqueueBarrierWithWaitList(cl_command_queue   command_queue ,
                                        cl_uint            num_events_in_wait_list ,
                                        const cl_event *   event_wait_list ,
                                        cl_event *         event)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueBarrierWithWaitList;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Change the events from the local references to the remote ones
    cl_event *events_wait = NULL;
    if(num_events_in_wait_list){
        events_wait = (cl_event*)malloc(num_events_in_wait_list*sizeof(cl_event));
        if(!events_wait){
            return CL_OUT_OF_HOST_MEMORY;
        }
        for(i=0;i<num_events_in_wait_list;i++)
            events_wait[i] = event_wait_list[i]->ptr;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &(command_queue->ptr), sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, events_wait, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    free(events_wait); events_wait=NULL;
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return CL_SUCCESS;
}

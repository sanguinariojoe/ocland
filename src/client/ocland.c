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
        free(line); line = NULL;linelen = 0;
        read = getline(&line, &linelen, fin);
    }
    // Set servers
    rewind(fin);
    servers->address = (char**)malloc(servers->num_servers*sizeof(char*));
    servers->sockets = (int*)malloc(servers->num_servers*sizeof(int));
    servers->locked  = (cl_bool*)malloc(servers->num_servers*sizeof(cl_bool));
    i = 0;
    line = NULL;linelen = 0;
    while((read = getline(&line, &linelen, fin)) != -1) {
        if(!strcmp(line, "\n")){
            free(line); line = NULL;linelen = 0;
            continue;
        }
        servers->address[i] = (char*)malloc((strlen(line)+1)*sizeof(char));
        strcpy(servers->address[i], line);
        strcpy(strstr(servers->address[i], "\n"), "");
        servers->sockets[i] = -1;
        servers->locked[i]  = CL_FALSE;
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
    int switch_on  = 1;
    int switch_off = 0;
    unsigned int i,n=0;
    for(i=0;i<servers->num_servers;i++){
        // Try to connect to server
        int sockfd = 0;
        struct sockaddr_in serv_addr;
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            continue;
        memset(&serv_addr, '0', sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(OCLAND_PORT);
        if(inet_pton(AF_INET, servers->address[i], &serv_addr.sin_addr)<=0)
            continue;
        if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            continue;
        setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,  (char *) &switch_on, sizeof(int));
        setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, (char *) &switch_on, sizeof(int));
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
char* serverAddress(int socket)
{
    unsigned int i;
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] == socket){
            return servers->address[i];
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
        for(i=0;i<servers->num_servers;i++){
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
                            cl_uint*        num_platforms)
{
    unsigned int i,j;
    cl_int flag;
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
            platforms[t_num_platforms + j] = l_platforms[j];
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
    cl_int flag;
    size_t size_ret;
    unsigned int comm = ocland_clGetPlatformInfo;
    if(param_value_size_ret) *param_value_size_ret = 0;
    // Try the platform in all the servers
    for(i=0;i<servers->num_servers;i++){
        // Ensure that the server still being active
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send the command data
        Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
        Send(sockfd, &platform, sizeof(cl_platform_id), MSG_MORE);
        Send(sockfd, &param_name, sizeof(cl_platform_info), MSG_MORE);
        Send(sockfd, &param_value_size, sizeof(size_t), 0);
        // Receive the answer
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            if(flag == CL_INVALID_PLATFORM){
                continue;
            }
            return flag;
        }
        Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
        if(param_value_size_ret) *param_value_size_ret = size_ret;
        if(param_value){
            Recv(sockfd, param_value, size_ret, MSG_WAITALL);
        }
        return CL_SUCCESS;
    }
    // If we reach this point, the platform was not found in any server
    return CL_INVALID_PLATFORM;
}

cl_int oclandGetDeviceIDs(cl_platform_id   platform,
                          cl_device_type   device_type,
                          cl_uint          num_entries,
                          cl_device_id *   devices,
                          cl_uint *        num_devices)
{
    unsigned int i;
    cl_int flag;
    cl_uint n;
    unsigned int comm = ocland_clGetDeviceIDs;
    if(num_devices) *num_devices = 0;
    // Test all the servers looking for this platform
    for(i=0;i<servers->num_servers;i++){
        // Ensure that the server still being active
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send the command data
        Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
        Send(sockfd, &platform, sizeof(cl_platform_id), MSG_MORE);
        Send(sockfd, &device_type, sizeof(cl_device_type), MSG_MORE);
        Send(sockfd, &num_entries, sizeof(cl_uint), 0);
        // Receive the answer
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            if(flag == CL_INVALID_PLATFORM){
                continue;
            }
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
    // The platform has not been found in any server
    return CL_INVALID_PLATFORM;
}

cl_int oclandGetDeviceInfo(cl_device_id    device,
                           cl_device_info  param_name,
                           size_t          param_value_size,
                           void *          param_value,
                           size_t *        param_value_size_ret)
{
    unsigned int i;
    cl_int flag;
    size_t size_ret;
    unsigned int comm = ocland_clGetDeviceInfo;
    if(param_value_size_ret) *param_value_size_ret = 0;
    // Test all the servers looking for this device
    for(i=0;i<servers->num_servers;i++){
        // Ensure that the server still being active
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send the command data
        Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
        Send(sockfd, &device, sizeof(cl_device_id), MSG_MORE);
        Send(sockfd, &param_name, sizeof(cl_device_info), MSG_MORE);
        Send(sockfd, &param_value_size, sizeof(size_t), 0);
        // Receive the answer
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            if(flag == CL_INVALID_PLATFORM){
                continue;
            }
            return flag;
        }
        Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
        if(param_value_size_ret) *param_value_size_ret = size_ret;
        if(param_value){
            Recv(sockfd, param_value, size_ret, MSG_WAITALL);
        }
        return CL_SUCCESS;
    }
    // The platform has not been found in any server
    return CL_INVALID_PLATFORM;
}

cl_context oclandCreateContext(const cl_context_properties * properties,
                               cl_uint                       num_properties,
                               cl_uint                       num_devices,
                               const cl_device_id *          devices,
                               void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *),
                               void *                        user_data,
                               cl_int *                      errcode_ret)
{
    unsigned int i;
    cl_int flag;
    unsigned int comm = ocland_clCreateContext;
    cl_context context = NULL;
    // Try devices in all servers
    for(i=0;i<servers->num_servers;i++){
        // Ensure that the server still being active
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send the command data
        Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
        Send(sockfd, &num_properties, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, properties, num_properties*sizeof(cl_context_properties), MSG_MORE);
        Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, devices, num_devices*sizeof(cl_device_id), 0);
        // Receive the answer
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        if(errcode_ret) *errcode_ret = flag;
        if(flag != CL_SUCCESS){
            if( (flag == CL_INVALID_DEVICE) || (flag == CL_INVALID_PLATFORM) )
                continue;
            return NULL;
        }
        Recv(sockfd, &context, sizeof(cl_context), MSG_WAITALL);
        addShortcut((void*)context, sockfd);
        return context;
    }
    // Devices can't be found at any server
    if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
    return NULL;
}

cl_context oclandCreateContextFromType(const cl_context_properties * properties,
                                       cl_uint                       num_properties,
                                       cl_device_type                device_type,
                                       void (CL_CALLBACK *     pfn_notify)(const char *, const void *, size_t, void *),
                                       void *                        user_data,
                                       cl_int *                      errcode_ret)
{
    unsigned int i;
    cl_int flag;
    unsigned int comm = ocland_clCreateContextFromType;
    cl_context context = NULL;
    // Try all the servers
    for(i=0;i<servers->num_servers;i++){
        // Ensure that the server still being active
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send the command data
        Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
        Send(sockfd, &num_properties, sizeof(cl_uint), MSG_MORE);
        Send(sockfd, properties, num_properties*sizeof(cl_context_properties), MSG_MORE);
        Send(sockfd, &device_type, sizeof(cl_device_type), 0);
        // Receive the answer
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        if(errcode_ret) *errcode_ret = flag;
        if(flag != CL_SUCCESS){
            if( (flag == CL_INVALID_DEVICE) || (flag == CL_INVALID_PLATFORM) )
                continue;
            return NULL;
        }
        Recv(sockfd, &context, sizeof(cl_context), MSG_WAITALL);
        addShortcut((void*)context, sockfd);
        return context;
    }
    // Devices can't be found at any server
    if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
    return NULL;
}

cl_int oclandRetainContext(cl_context context)
{
    cl_int flag;
    unsigned int comm = ocland_clRetainContext;
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context, sizeof(cl_context), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseContext(cl_context context)
{
    cl_int flag;
    unsigned int comm = ocland_clReleaseContext;
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context, sizeof(cl_context), 0);
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
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetContextInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context, sizeof(cl_context), MSG_MORE);
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
    cl_int flag;
    cl_command_queue command_queue = NULL;
    unsigned int comm = ocland_clCreateCommandQueue;
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context, sizeof(cl_context), MSG_MORE);
    Send(sockfd, &device, sizeof(cl_device_id), MSG_MORE);
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
    cl_int flag;
    unsigned int comm = ocland_clRetainCommandQueue;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseCommandQueue(cl_command_queue command_queue)
{
    cl_int flag;
    unsigned int comm = ocland_clReleaseCommandQueue;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
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
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetCommandQueueInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), MSG_MORE);
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
    cl_int flag;
    cl_mem mem = NULL;
    unsigned int comm = ocland_clCreateBuffer;
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr)
        hasPtr = CL_TRUE;
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context, sizeof(cl_context), MSG_MORE);
    Send(sockfd, &flags, sizeof(cl_mem_flags), MSG_MORE);
    Send(sockfd, &size, sizeof(size_t), MSG_MORE);
    if(host_ptr){
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
    cl_int flag;
    unsigned int comm = ocland_clReleaseMemObject;
    // Get the server
    int *sockfd = getShortcut(mem);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &mem, sizeof(cl_mem), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseMemObject(cl_mem mem)
{
    cl_int flag;
    unsigned int comm = ocland_clReleaseMemObject;
    // Get the server
    int *sockfd = getShortcut(mem);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &mem, sizeof(cl_mem), 0);
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
    cl_int flag;
    cl_uint n=0;
    unsigned int comm = ocland_clGetSupportedImageFormats;
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context, sizeof(cl_context), MSG_MORE);
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
        Recv(sockfd, (void*)image_formats, n*sizeof(cl_image_format), MSG_WAITALL);
    }
    return CL_SUCCESS;
}

cl_int oclandGetMemObjectInfo(cl_mem            mem ,
                              cl_mem_info       param_name ,
                              size_t            param_value_size ,
                              void *            param_value ,
                              size_t *          param_value_size_ret)
{
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetMemObjectInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(mem);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &mem, sizeof(cl_mem), MSG_MORE);
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
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetImageInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(image);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &image, sizeof(cl_mem), MSG_MORE);
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
    cl_int flag;
    cl_sampler sampler = NULL;
    unsigned int comm = ocland_clCreateSampler;
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context, sizeof(cl_context), MSG_MORE);
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
    cl_int flag;
    unsigned int comm = ocland_clRetainSampler;
    // Get the server
    int *sockfd = getShortcut(sampler);
    if(!sockfd){
        return CL_INVALID_SAMPLER;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &sampler, sizeof(cl_sampler), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseSampler(cl_sampler sampler)
{
    cl_int flag;
    unsigned int comm = ocland_clReleaseSampler;
    // Get the server
    int *sockfd = getShortcut(sampler);
    if(!sockfd){
        return CL_INVALID_SAMPLER;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &sampler, sizeof(cl_sampler), 0);
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
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetSamplerInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(sampler);
    if(!sockfd){
        return CL_INVALID_SAMPLER;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &sampler, sizeof(cl_sampler), MSG_MORE);
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
    cl_int flag;
    cl_program program = NULL;
    unsigned int comm = ocland_clCreateProgramWithSource;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context, sizeof(cl_context), MSG_MORE);
    Send(sockfd, &count, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, lengths, count*sizeof(size_t), 0);
    for(i=0;i<count;i++){
        Send(sockfd, strings[i], lengths[i], 0);
    }
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
    cl_int flag;
    cl_program program = NULL;
    cl_int* status = NULL;
    unsigned int comm = ocland_clCreateProgramWithBinary;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &context, sizeof(cl_context), MSG_MORE);
    Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, &device_list, num_devices*sizeof(cl_device_id), MSG_MORE);
    Send(sockfd, lengths, num_devices*sizeof(size_t), 0);
    for(i=0;i<num_devices;i++){
        Send(sockfd, binaries[i], lengths[i], 0);
    }
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
    cl_int flag;
    unsigned int comm = ocland_clRetainProgram;
    // Get the server
    int *sockfd = getShortcut(program);
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &program, sizeof(cl_program), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseProgram(cl_program  program)
{
    cl_int flag;
    unsigned int comm = ocland_clReleaseProgram;
    // Get the server
    int *sockfd = getShortcut(program);
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &program, sizeof(cl_program), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(program);
    return flag;
}

cl_int oclandBuildProgram(cl_program            program ,
                          cl_uint               num_devices ,
                          const cl_device_id *  device_list ,
                          const char *          options ,
                          void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                          void *                user_data)
{
    cl_int flag;
    unsigned int comm = ocland_clBuildProgram;
    size_t options_size = (strlen(options) + 1)*sizeof(char);
    // Get the server
    int *sockfd = getShortcut(program);
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &program, sizeof(cl_program), MSG_MORE);
    Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, device_list, num_devices*sizeof(cl_device_id), MSG_MORE);
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
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetProgramInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(program);
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &program, sizeof(cl_program), MSG_MORE);
    Send(sockfd, &param_name, sizeof(cl_program_info), MSG_MORE);
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

cl_int oclandGetProgramBuildInfo(cl_program             program ,
                                 cl_device_id           device ,
                                 cl_program_build_info  param_name ,
                                 size_t                 param_value_size ,
                                 void *                 param_value ,
                                 size_t *               param_value_size_ret)
{
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetProgramBuildInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(program);
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &program, sizeof(cl_program), MSG_MORE);
    Send(sockfd, &device, sizeof(cl_device_id), MSG_MORE);
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
    cl_int flag;
    cl_kernel kernel = NULL;
    unsigned int comm = ocland_clCreateKernel;
    size_t kernel_name_size = (strlen(kernel_name)+1)*sizeof(char);
    // Get the server
    int *sockfd = getShortcut(program);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &program, sizeof(cl_program), MSG_MORE);
    Send(sockfd, &kernel_name_size, sizeof(size_t), MSG_MORE);
    Send(sockfd, &kernel_name, kernel_name_size, 0);
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
    cl_int flag;
    cl_uint n;
    cl_kernel kernel = NULL;
    unsigned int comm = ocland_clCreateKernelsInProgram;
    if(num_kernels_ret) *num_kernels_ret=0;
    // Get the server
    int *sockfd = getShortcut(program);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &program, sizeof(cl_program), MSG_MORE);
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
    cl_int flag;
    unsigned int comm = ocland_clRetainKernel;
    // Get the server
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &kernel, sizeof(cl_kernel), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseKernel(cl_kernel    kernel)
{
    cl_int flag;
    unsigned int comm = ocland_clReleaseKernel;
    // Get the server
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &kernel, sizeof(cl_kernel), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(kernel);
    return flag;
}

cl_int oclandSetKernelArg(cl_kernel     kernel ,
                          cl_uint       arg_index ,
                          size_t        arg_size ,
                          const void *  arg_value)
{
    cl_int flag;
    unsigned int comm = ocland_clSetKernelArg;
    // Get the server
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &kernel, sizeof(cl_kernel), MSG_MORE);
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
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetKernelInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &kernel, sizeof(cl_kernel), MSG_MORE);
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
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetKernelWorkGroupInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &kernel, sizeof(cl_kernel), MSG_MORE);
    Send(sockfd, &device, sizeof(cl_device_id), MSG_MORE);
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
    cl_int flag;
    unsigned int comm = ocland_clWaitForEvents;
    // Get the server
    int *sockfd = getShortcut(event_list[0]);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &num_events, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, event_list, num_events*sizeof(cl_event), 0);
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
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetEventInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &event, sizeof(cl_event), MSG_MORE);
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

cl_int oclandRetainEvent(cl_event  event)
{
    cl_int flag;
    unsigned int comm = ocland_clRetainEvent;
    // Get the server
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &event, sizeof(cl_event), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseEvent(cl_event  event)
{
    cl_int flag;
    unsigned int comm = ocland_clReleaseEvent;
    // Get the server
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &event, sizeof(cl_event), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag == CL_SUCCESS)
        delShortcut(event);
    return flag;
}

cl_int oclandGetEventProfilingInfo(cl_event             event ,
                                   cl_profiling_info    param_name ,
                                   size_t               param_value_size ,
                                   void *               param_value ,
                                   size_t *             param_value_size_ret)
{
    cl_int flag;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetEventProfilingInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    // Get the server
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &event, sizeof(cl_event), MSG_MORE);
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
    cl_int flag;
    unsigned int comm = ocland_clFlush;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandFinish(cl_command_queue  command_queue)
{
    cl_int flag;
    unsigned int comm = ocland_clFinish;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
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
    char* ip = serverAddress(_data->fd);
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
    cl_int flag;
    unsigned int comm = ocland_clEnqueueReadBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &buffer, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &blocking_read, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, &offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &cb, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), MSG_MORE);
        Send(sockfd, event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), 0);
    }
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
    char* ip = serverAddress(_data->fd);
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
    cl_int flag;
    unsigned int comm = ocland_clEnqueueWriteBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &buffer, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &blocking_write, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, &offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &cb, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if( (num_events_in_wait_list) || (blocking_write) ){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), MSG_MORE);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), 0);
    }
    if(blocking_write){
        Send(sockfd, event_wait_list, num_events_in_wait_list*sizeof(cl_event), MSG_MORE);
        dataPack in, out;
        in.size = cb;
        in.data = ptr;
        out = pack(in);
        Send(sockfd, &(out.size), sizeof(size_t), MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    else{
        Send(sockfd, event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
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
    cl_int flag;
    unsigned int comm = ocland_clEnqueueCopyBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &src_buffer, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &dst_buffer, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &src_offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &dst_offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &cb, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), MSG_MORE);
        Send(sockfd, event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), 0);
    }
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
    cl_int flag;
    unsigned int comm = ocland_clEnqueueCopyImage;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &src_image, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &dst_image, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &src_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &dst_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), MSG_MORE);
        Send(sockfd, event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), 0);
    }
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
    cl_int flag;
    unsigned int comm = ocland_clEnqueueCopyImageToBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send(sockfd, &command_queue, sizeof(cl_command_queue), MSG_MORE);
    Send(sockfd, &src_image, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &dst_buffer, sizeof(cl_mem), MSG_MORE);
    Send(sockfd, &src_origin, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &region, 3*sizeof(size_t), MSG_MORE);
    Send(sockfd, &dst_offset, sizeof(size_t), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), MSG_MORE);
        Send(sockfd, event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_command_queue), 0);
    }
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
    cl_event revent = NULL;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Build the package
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    size_t msgSize  = sizeof(unsigned int);                            // Command index
    msgSize        += sizeof(cl_command_queue);                        // command_queue
    msgSize        += sizeof(cl_mem);                                  // src_buffer
    msgSize        += sizeof(cl_mem);                                  // dst_image
    msgSize        += sizeof(size_t);                                  // src_offset
    msgSize        += 3*sizeof(size_t);                                // dst_origin
    msgSize        += 3*sizeof(size_t);                                // region
    msgSize        += sizeof(cl_bool);                                 // want_event
    msgSize        += sizeof(cl_uint);                                 // num_events_in_wait_list
    msgSize        += num_events_in_wait_list*sizeof(event_wait_list); // event_wait_list
    void* msg = (void*)malloc(msgSize);
    void* mptr = msg;
    ((unsigned int*)mptr)[0]     = ocland_clEnqueueCopyBufferToImage; mptr = (unsigned int*)mptr + 1;
    ((cl_command_queue*)mptr)[0] = command_queue;                     mptr = (cl_command_queue*)mptr + 1;
    ((cl_mem*)mptr)[0]           = src_buffer;                        mptr = (cl_mem*)mptr + 1;
    ((cl_mem*)mptr)[0]           = dst_image;                         mptr = (cl_mem*)mptr + 1;
    ((size_t*)mptr)[0]           = src_offset;                        mptr = (size_t*)mptr + 1;
    memcpy(mptr, dst_origin, 3*sizeof(size_t));                       mptr = (size_t*)mptr + 3;
    memcpy(mptr, region,     3*sizeof(size_t));                       mptr = (size_t*)mptr + 3;
    ((cl_bool*)mptr)[0]          = want_event;                        mptr = (cl_bool*)mptr + 1;
    ((cl_uint*)mptr)[0]          = num_events_in_wait_list;           mptr = (cl_uint*)mptr + 1;
    memcpy(mptr, event_wait_list, num_events_in_wait_list*sizeof(cl_event));
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    mptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the flag, if CL_SUCCESS don't received, we can't
    // still working
    cl_int flag = ((cl_int*)mptr)[0]; mptr = (cl_int*)mptr + 1;
    if(flag != CL_SUCCESS)
        return flag;
    revent = ((cl_event*)mptr)[0]; mptr = (cl_event*)mptr + 1;
    if(event){
        *event = revent;
        addShortcut(*event, sockfd);
    }
    return flag;
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
    cl_event revent = NULL;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Build the package
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    cl_bool has_global_work_offset = CL_FALSE;
    if(global_work_offset) has_global_work_offset = CL_TRUE;
    cl_bool has_local_work_size = CL_FALSE;
    if(local_work_size) has_local_work_size = CL_TRUE;
    size_t msgSize  = sizeof(unsigned int);                            // Command index
    msgSize        += sizeof(cl_command_queue);                        // command_queue
    msgSize        += sizeof(cl_kernel);                               // kernel
    msgSize        += sizeof(cl_uint);                                 // work_dim
    msgSize        += sizeof(cl_bool);                                 // has_global_work_offset
    msgSize        += sizeof(cl_bool);                                 // has_local_work_size
    if(has_global_work_offset == CL_TRUE)
        msgSize    += work_dim*sizeof(size_t);                         // global_work_offset
    msgSize    += work_dim*sizeof(size_t);                             // global_work_size
    if(has_local_work_size == CL_TRUE)
        msgSize    += work_dim*sizeof(size_t);                         // local_work_size
    msgSize        += sizeof(cl_bool);                                 // want_event
    msgSize        += sizeof(cl_uint);                                 // num_events_in_wait_list
    msgSize        += num_events_in_wait_list*sizeof(event_wait_list); // event_wait_list
    void* msg = (void*)malloc(msgSize);
    void* mptr = msg;
    ((unsigned int*)mptr)[0]     = ocland_clEnqueueNDRangeKernel; mptr = (unsigned int*)mptr + 1;
    ((cl_command_queue*)mptr)[0] = command_queue;              mptr = (cl_command_queue*)mptr + 1;
    ((cl_kernel*)mptr)[0]        = kernel;                     mptr = (cl_kernel*)mptr + 1;
    ((cl_uint*)mptr)[0]          = work_dim;                   mptr = (cl_uint*)mptr + 1;
    ((cl_bool*)mptr)[0]          = has_global_work_offset;     mptr = (cl_bool*)mptr + 1;
    ((cl_bool*)mptr)[0]          = has_local_work_size;        mptr = (cl_bool*)mptr + 1;
    if(has_global_work_offset == CL_TRUE){
        memcpy(mptr, (void*)global_work_offset, work_dim*sizeof(size_t));
        mptr = (size_t*)mptr + work_dim;
    }
    memcpy(mptr, (void*)global_work_size, work_dim*sizeof(size_t));
    mptr = (size_t*)mptr + work_dim;
    if(has_local_work_size == CL_TRUE){
        memcpy(mptr, (void*)local_work_size, work_dim*sizeof(size_t));
        mptr = (size_t*)mptr + work_dim;
    }
    ((cl_bool*)mptr)[0]          = want_event;                 mptr = (cl_bool*)mptr + 1;
    ((cl_uint*)mptr)[0]          = num_events_in_wait_list;    mptr = (cl_uint*)mptr + 1;
    memcpy(mptr, event_wait_list, num_events_in_wait_list*sizeof(cl_event));
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    mptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the flag, if CL_SUCCESS don't received, we can't
    // still working
    cl_int flag = ((cl_int*)mptr)[0]; mptr = (cl_int*)mptr + 1;
    if(flag != CL_SUCCESS)
        return flag;
    revent = ((cl_event*)mptr)[0]; mptr = (cl_event*)mptr + 1;
    if(event){
        *event = revent;
        addShortcut(*event, sockfd);
    }
    return flag;
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
    char* ip = serverAddress(_data->fd);
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
    cl_event revent = NULL;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Build the package
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    size_t msgSize  = sizeof(unsigned int);                            // Command index
    msgSize        += sizeof(cl_command_queue);                        // command_queue
    msgSize        += sizeof(cl_mem);                                  // image
    msgSize        += sizeof(cl_bool);                                 // blocking_read
    msgSize        += 3*sizeof(size_t);                                // origin
    msgSize        += 3*sizeof(size_t);                                // region
    msgSize        += sizeof(size_t);                                  // row_pitch
    msgSize        += sizeof(size_t);                                  // slice_pitch
    msgSize        += sizeof(cl_bool);                                 // want_event
    msgSize        += sizeof(cl_uint);                                 // num_events_in_wait_list
    msgSize        += num_events_in_wait_list*sizeof(event_wait_list); // event_wait_list
    void* msg = (void*)malloc(msgSize);
    void* mptr = msg;
    ((unsigned int*)mptr)[0]     = ocland_clEnqueueReadImage; mptr = (unsigned int*)mptr + 1;
    ((cl_command_queue*)mptr)[0] = command_queue;             mptr = (cl_command_queue*)mptr + 1;
    ((cl_mem*)mptr)[0]           = image;                     mptr = (cl_mem*)mptr + 1;
    ((cl_bool*)mptr)[0]          = blocking_read;             mptr = (cl_bool*)mptr + 1;
    memcpy(mptr,(void*)origin,3*sizeof(size_t));              mptr = (size_t*)mptr + 3;
    memcpy(mptr,(void*)region,3*sizeof(size_t));              mptr = (size_t*)mptr + 3;
    ((size_t*)mptr)[0]           = row_pitch;                 mptr = (size_t*)mptr + 1;
    ((size_t*)mptr)[0]           = slice_pitch;               mptr = (size_t*)mptr + 1;
    ((cl_bool*)mptr)[0]          = want_event;                mptr = (cl_bool*)mptr + 1;
    ((cl_uint*)mptr)[0]          = num_events_in_wait_list;   mptr = (cl_uint*)mptr + 1;
    memcpy(mptr, event_wait_list, num_events_in_wait_list*sizeof(cl_event));
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    mptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the flag, if CL_SUCCESS don't received, we can't
    // still working
    cl_int flag = ((cl_int*)mptr)[0]; mptr = (cl_int*)mptr + 1;
    if(flag != CL_SUCCESS)
        return flag;
    size_t cb = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag, the event, and the data.
    // ------------------------------------------------------------
    if(blocking_read == CL_TRUE){
        revent = ((cl_event*)mptr)[0]; mptr = (cl_event*)mptr + 1;
        if(event){
            *event = revent;
            addShortcut(*event, sockfd);
        }
        memcpy(ptr, mptr, cb);
        return flag;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
    revent = ((cl_event*)mptr)[0]; mptr = (cl_event*)mptr + 1;
    if(event){
        *event = revent;
        addShortcut(*event, sockfd);
    }
    unsigned int port = ((unsigned int*)mptr)[0];
    struct dataTransferRect data;
    data.port   = port;
    data.fd     = *sockfd;
    data.region = region;
    data.row    = row_pitch;
    data.slice  = slice_pitch;
    data.cb     = cb;
    data.ptr    = ptr;
    asyncDataRecvRect(sockfd, data);
    return flag;
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
    char* ip = serverAddress(_data->fd);
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
    Send(&fd, &(out.size), sizeof(size_t), 0);
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
    cl_event revent = NULL;
    // Get the server
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Build the package
    size_t cb = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    cl_bool want_event = CL_FALSE;
    if(event) want_event = CL_TRUE;
    size_t msgSize  = sizeof(unsigned int);                            // Command index
    msgSize        += sizeof(cl_command_queue);                        // command_queue
    msgSize        += sizeof(cl_mem);                                  // image
    msgSize        += sizeof(cl_bool);                                 // blocking_write
    msgSize        += 3*sizeof(size_t);                                // origin
    msgSize        += 3*sizeof(size_t);                                // region
    msgSize        += sizeof(size_t);                                  // row_pitch
    msgSize        += sizeof(size_t);                                  // slice_pitch
    msgSize        += sizeof(cl_bool);                                 // want_event
    msgSize        += sizeof(cl_uint);                                 // num_events_in_wait_list
    msgSize        += num_events_in_wait_list*sizeof(event_wait_list); // event_wait_list
    if(blocking_write == CL_TRUE)
        msgSize    += cb;                                              // ptr
    void* msg = (void*)malloc(msgSize);
    void* mptr = msg;
    ((unsigned int*)mptr)[0]     = ocland_clEnqueueWriteImage; mptr = (unsigned int*)mptr + 1;
    ((cl_command_queue*)mptr)[0] = command_queue;              mptr = (cl_command_queue*)mptr + 1;
    ((cl_mem*)mptr)[0]           = image;                      mptr = (cl_mem*)mptr + 1;
    ((cl_bool*)mptr)[0]          = blocking_write;             mptr = (cl_bool*)mptr + 1;
    memcpy(mptr,(void*)origin,3*sizeof(size_t));               mptr = (size_t*)mptr + 3;
    memcpy(mptr,(void*)region,3*sizeof(size_t));               mptr = (size_t*)mptr + 3;
    ((size_t*)mptr)[0]           = row_pitch;                  mptr = (size_t*)mptr + 1;
    ((size_t*)mptr)[0]           = slice_pitch;                mptr = (size_t*)mptr + 1;
    ((cl_bool*)mptr)[0]          = want_event;                 mptr = (cl_bool*)mptr + 1;
    ((cl_uint*)mptr)[0]          = num_events_in_wait_list;    mptr = (cl_uint*)mptr + 1;
    memcpy(mptr, event_wait_list, num_events_in_wait_list*sizeof(cl_event));
    if(blocking_write == CL_TRUE){
        mptr = (cl_event*)mptr + num_events_in_wait_list;
        memcpy(mptr, ptr, cb);
    }
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    mptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the flag, if CL_SUCCESS don't received, we can't
    // still working
    cl_int flag = ((cl_int*)mptr)[0]; mptr = (cl_int*)mptr + 1;
    if(flag != CL_SUCCESS)
        return flag;
    // ------------------------------------------------------------
    // Blocking write case:
    // We may have received the flag, and the event.
    // ------------------------------------------------------------
    if(blocking_write == CL_TRUE){
        revent = ((cl_event*)mptr)[0]; mptr = (cl_event*)mptr + 1;
        if(event){
            *event = revent;
            addShortcut(*event, sockfd);
        }
        return flag;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
    revent = ((cl_event*)mptr)[0]; mptr = (cl_event*)mptr + 1;
    if(event){
        *event = revent;
        addShortcut(*event, sockfd);
    }
    unsigned int port = ((unsigned int*)mptr)[0];
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
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Build the package
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr) hasPtr = CL_TRUE;
    size_t size = image_width*image_height*element_size;
    size_t msgSize  = sizeof(unsigned int);    // Command index
    msgSize        += sizeof(cl_context);      // context
    msgSize        += sizeof(cl_mem_flags);    // flags
    msgSize        += sizeof(cl_image_format); // image_format
    msgSize        += sizeof(size_t);          // image_width
    msgSize        += sizeof(size_t);          // image_height
    msgSize        += sizeof(size_t);          // image_row_pitch
    msgSize        += sizeof(cl_bool);         // hasPtr
    if(host_ptr) msgSize += size;              // host_ptr
    void* msg = (void*)malloc(msgSize);
    void* ptr = msg;
    ((unsigned int*)ptr)[0]    = ocland_clCreateImage2D; ptr = (unsigned int*)ptr + 1;
    ((cl_context*)ptr)[0]      = context;                ptr = (cl_context*)ptr + 1;
    ((cl_mem_flags*)ptr)[0]    = flags;                  ptr = (cl_mem_flags*)ptr + 1;
    memcpy(ptr,image_format,sizeof(cl_image_format));    ptr = (cl_image_format*)ptr + 1;
    ((size_t*)ptr)[0]          = image_width;            ptr = (size_t*)ptr + 1;
    ((size_t*)ptr)[0]          = image_height;           ptr = (size_t*)ptr + 1;
    ((size_t*)ptr)[0]          = image_row_pitch;        ptr = (size_t*)ptr + 1;
    ((cl_bool*)ptr)[0]         = hasPtr;                 ptr = (cl_bool*)ptr + 1;
    if(host_ptr) memcpy(ptr, host_ptr, size);
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    ptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the data
    cl_int flag = ((cl_int*)ptr)[0]; ptr = (cl_int*)ptr  + 1;
    if(errcode_ret) *errcode_ret = flag;
    if(flag != CL_SUCCESS)
        return NULL;
    cl_mem memobj = ((cl_mem*)ptr)[0];
    addShortcut((void*)memobj, sockfd);
    return memobj;
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
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Build the package
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr) hasPtr = CL_TRUE;
    size_t size = image_width*image_height*image_depth*element_size;
    size_t msgSize  = sizeof(unsigned int);    // Command index
    msgSize        += sizeof(cl_context);      // context
    msgSize        += sizeof(cl_mem_flags);    // flags
    msgSize        += sizeof(cl_image_format); // image_format
    msgSize        += sizeof(size_t);          // image_width
    msgSize        += sizeof(size_t);          // image_height
    msgSize        += sizeof(size_t);          // image_depth
    msgSize        += sizeof(size_t);          // image_row_pitch
    msgSize        += sizeof(size_t);          // image_slice_pitch
    msgSize        += sizeof(cl_bool);         // hasPtr
    if(host_ptr) msgSize += size;              // host_ptr
    void* msg = (void*)malloc(msgSize);
    void* ptr = msg;
    ((unsigned int*)ptr)[0]    = ocland_clCreateImage3D; ptr = (unsigned int*)ptr + 1;
    ((cl_context*)ptr)[0]      = context;                ptr = (cl_context*)ptr + 1;
    ((cl_mem_flags*)ptr)[0]    = flags;                  ptr = (cl_mem_flags*)ptr + 1;
    memcpy(ptr,image_format,sizeof(cl_image_format));    ptr = (cl_image_format*)ptr + 1;
    ((size_t*)ptr)[0]          = image_width;            ptr = (size_t*)ptr + 1;
    ((size_t*)ptr)[0]          = image_height;           ptr = (size_t*)ptr + 1;
    ((size_t*)ptr)[0]          = image_depth;            ptr = (size_t*)ptr + 1;
    ((size_t*)ptr)[0]          = image_row_pitch;        ptr = (size_t*)ptr + 1;
    ((size_t*)ptr)[0]          = image_slice_pitch;      ptr = (size_t*)ptr + 1;
    ((cl_bool*)ptr)[0]         = hasPtr;                 ptr = (cl_bool*)ptr + 1;
    if(host_ptr) memcpy(ptr, host_ptr, size);
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    ptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the data
    cl_int flag = ((cl_int*)ptr)[0]; ptr = (cl_int*)ptr  + 1;
    if(errcode_ret) *errcode_ret = flag;
    if(flag != CL_SUCCESS)
        return NULL;
    cl_mem memobj = ((cl_mem*)ptr)[0];
    addShortcut((void*)memobj, sockfd);
    return memobj;
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
    // Get the server
    int *sockfd = getShortcut(buffer);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Build the package
    size_t msgSize  = sizeof(unsigned int);                  // Command index
    msgSize        += sizeof(cl_mem);                        // context
    msgSize        += sizeof(cl_mem_flags);                  // flags
    msgSize        += sizeof(cl_buffer_create_type);         // buffer_create_type
    if(buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION)
        msgSize        += sizeof(cl_buffer_region);          // buffer_create_info
    void* msg = (void*)malloc(msgSize);
    void* ptr = msg;
    ((unsigned int*)ptr)[0]          = ocland_clCreateSubBuffer; ptr = (unsigned int*)ptr + 1;
    ((cl_mem*)ptr)[0]                = buffer;                   ptr = (cl_mem*)ptr + 1;
    ((cl_buffer_create_type*)ptr)[0] = flags;                    ptr = (cl_buffer_create_type*)ptr + 1;
    if(buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION){
        memcpy(ptr,buffer_create_info,sizeof(cl_buffer_region));
    }
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    ptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the data
    cl_int flag = ((cl_int*)ptr)[0]; ptr = (cl_int*)ptr  + 1;
    if(errcode_ret) *errcode_ret = flag;
    if(flag != CL_SUCCESS)
        return NULL;
    cl_mem memobj = ((cl_mem*)ptr)[0];
    addShortcut((void*)memobj, sockfd);
    return memobj;
}

cl_event oclandCreateUserEvent(cl_context     context ,
                               cl_int *       errcode_ret)
{
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Build the package
    size_t msgSize  = sizeof(unsigned int);                  // Command index
    msgSize        += sizeof(cl_context);                    // context
    void* msg = (void*)malloc(msgSize);
    void* ptr = msg;
    ((unsigned int*)ptr)[0] = ocland_clCreateUserEvent; ptr = (unsigned int*)ptr + 1;
    ((cl_context*)ptr)[0]   = context;                  ptr = (cl_context*)ptr + 1;
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    ptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the data
    cl_int flag = ((cl_int*)ptr)[0]; ptr = (cl_int*)ptr  + 1;
    if(errcode_ret) *errcode_ret = flag;
    if(flag != CL_SUCCESS)
        return NULL;
    cl_event event = ((cl_event*)ptr)[0];
    addShortcut((void*)event, sockfd);
    return event;
}

cl_int oclandSetUserEventStatus(cl_event    event ,
                                cl_int      execution_status)
{
    // Get the server
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Build the package
    size_t msgSize  = sizeof(unsigned int);            // Command index
    msgSize        += sizeof(cl_event);                // event
    msgSize        += sizeof(cl_int);                  // execution_status
    void* msg = (void*)malloc(msgSize);
    void* ptr = msg;
    ((unsigned int*)ptr)[0] = ocland_clSetUserEventStatus; ptr = (unsigned int*)ptr + 1;
    ((cl_event*)ptr)[0]     = event;                       ptr = (cl_event*)ptr + 1;
    ((cl_int*)ptr)[0]       = execution_status;            ptr = (cl_int*)ptr + 1;
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    ptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the data
    cl_int flag = ((cl_int*)ptr)[0];
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
    char buffer[BUFF_SIZE];
    cl_bool want_event = CL_FALSE;
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clEnqueueReadBufferRect")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueReadBufferRect");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters. Host origin will be omissed for the
    // server, and all the responsability to generate store
    // the input data is rely to the client.
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &mem, sizeof(cl_mem), 0);
    Send(sockfd, &blocking_read, sizeof(cl_bool), 0);
    Send(sockfd, buffer_origin, 3*sizeof(size_t), 0);
    Send(sockfd, region, 3*sizeof(size_t), 0);
    Send(sockfd, &buffer_row_pitch, sizeof(size_t), 0);
    Send(sockfd, &buffer_slice_pitch, sizeof(size_t), 0);
    Send(sockfd, &host_row_pitch, sizeof(size_t), 0);
    Send(sockfd, &host_slice_pitch, sizeof(size_t), 0);
    Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    if(num_events_in_wait_list)
        Send(sockfd, &event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    if(event)
        want_event = CL_TRUE;
    Send(sockfd, &want_event, sizeof(cl_bool), 0);
    // And request flag, and event if needed
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    size_t origin = host_origin[0] +
                    host_origin[1]*host_row_pitch +
                    host_origin[2]*host_slice_pitch;
    // In case of blocking simply receive the data.
    // In rect reading process the data will read in
    // blocks of host_row_pitch size, along all the
    // region specified.
    if(blocking_read == CL_TRUE){
        unsigned int i, j, k, n;
        // Receive first the buffer purposed by server,
        // in order to can send data larger than the transfer
        // buffer.
        size_t buffsize;
        Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
        if(!buffsize){
            return CL_OUT_OF_HOST_MEMORY;
        }
        // Compute the number of packages needed per row
        n = host_row_pitch / buffsize;
        for(j=0;j<region[1];j++){
            for(k=0;k<region[2];k++){
                // Receive package by pieces
                for(i=0;i<n;i++){
                    Send(sockfd, ptr + i*buffsize + origin, buffsize, 0);
                }
                if(host_row_pitch % buffsize){
                    // Remains some data to transfer
                    Send(sockfd, ptr + n*buffsize + origin, host_row_pitch % buffsize, 0);
                }
                // Compute the new origin
                origin += host_row_pitch;
            }
        }
        return flag;
    }
    // In the non blocking case more complex operations are requested
    struct dataTransferRect data;
    data.region = region;
    data.row    = host_row_pitch;
    data.slice  = host_slice_pitch;
    data.ptr    = ptr + origin;
    asyncDataRecvRect(sockfd, data);
    return flag;
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
    char buffer[BUFF_SIZE];
    cl_bool want_event = CL_FALSE;
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clEnqueueWriteBufferRect")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueWriteBufferRect");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &mem, sizeof(cl_mem), 0);
    Send(sockfd, &blocking_write, sizeof(cl_bool), 0);
    Send(sockfd, buffer_origin, 3*sizeof(size_t), 0);
    Send(sockfd, region, 3*sizeof(size_t), 0);
    Send(sockfd, &buffer_row_pitch, sizeof(size_t), 0);
    Send(sockfd, &buffer_slice_pitch, sizeof(size_t), 0);
    Send(sockfd, &host_row_pitch, sizeof(size_t), 0);
    Send(sockfd, &host_slice_pitch, sizeof(size_t), 0);
    Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    if(num_events_in_wait_list)
        Send(sockfd, &event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    if(event)
        want_event = CL_TRUE;
    Send(sockfd, &want_event, sizeof(cl_bool), 0);
    // And request flag, and event if request
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    // In case of blocking simply receive the data
    if(blocking_write == CL_TRUE){
        unsigned int i,j,k,n;
        // Receive first the buffer purposed by server,
        // in order to can send data larger than the transfer
        // buffer.
        size_t buffsize;
        Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
        if(!buffsize){
            return CL_OUT_OF_HOST_MEMORY;
        }
        // Compute the number of packages needed
        n = host_row_pitch / buffsize;
        // Send package by pieces
        size_t origin=0;
        for(j=0;j<region[1];j++){
            for(k=0;k<region[2];k++){
                // Receive package by pieces
                for(i=0;i<n;i++){
                    Send(sockfd, (void*)ptr + i*buffsize + origin, buffsize, 0);
                }
                if(host_row_pitch % buffsize){
                    // Remains some data to transfer
                    Send(sockfd, (void*)ptr + n*buffsize + origin, host_row_pitch % buffsize, 0);
                }
                // Compute the new origin
                origin += host_row_pitch;
            }
        }
        // Get new flag after clEnqueueWriteBuffer has been
        // called in the server
        flag = CL_INVALID_CONTEXT;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        return flag;
    }
    // In the non blocking case more complex operations are requested
    struct dataTransferRect data;
    data.region = region;
    data.row    = host_row_pitch;
    data.slice  = host_slice_pitch;
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
    char buffer[BUFF_SIZE];
    cl_bool want_event = CL_FALSE;
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clEnqueueCopyBufferRect")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueCopyBufferRect");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &src_buffer, sizeof(cl_mem), 0);
    Send(sockfd, &dst_buffer, sizeof(cl_mem), 0);
    Send(sockfd, src_origin, 3*sizeof(size_t), 0);
    Send(sockfd, dst_origin, 3*sizeof(size_t), 0);
    Send(sockfd, region, 3*sizeof(size_t), 0);
    Send(sockfd, &src_row_pitch, sizeof(size_t), 0);
    Send(sockfd, &src_slice_pitch, sizeof(size_t), 0);
    Send(sockfd, &dst_row_pitch, sizeof(size_t), 0);
    Send(sockfd, &dst_slice_pitch, sizeof(size_t), 0);
    Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    if(num_events_in_wait_list)
        Send(sockfd, &event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    if(event)
        want_event = CL_TRUE;
    Send(sockfd, &want_event, sizeof(cl_bool), 0);
    // And request flag, and event if request
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if((flag != CL_SUCCESS) && (event)){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return flag;
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
                              cl_device_id                       * out_devices,
                              cl_uint                            * num_devices)
{
    unsigned int i;
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_DEVICE;
    }
    // Try platform in all servers
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send starting command declaration
        unsigned int commDim = strlen("clCreateSubDevices")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clCreateSubDevices");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Send parameters (we need to send size of properties)
        Send(sockfd, &in_device, sizeof(cl_platform_id), 0);
        size_t sProps = 0;
        if(properties){
            // Only CL_CONTEXT_PLATFORM will be supported, D3D ar GL can't be enabled in network
            sProps = num_properties*sizeof(cl_device_partition_property);
            Send(sockfd, &sProps, sizeof(size_t), 0);
            Send(sockfd, properties, sProps, 0);
        }
        else{
            Send(sockfd, &sProps, sizeof(size_t), 0);
        }
        Send(sockfd, &num_entries, sizeof(cl_uint), 0);
        // And request flag and real size of object
        cl_int flag = CL_INVALID_DEVICE;
        cl_uint size = 0;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        Recv(sockfd, &size, sizeof(cl_uint), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            // 2 possibilities, not right server or error
            if(flag == CL_INVALID_DEVICE)
                continue;
            return flag;
        }
        if(!num_entries){
            if(num_devices) *num_devices = size;
            if(*sockfd < 0)
                return CL_INVALID_DEVICE;
            return CL_SUCCESS;
        }
        // Get returned info
        Recv(sockfd, out_devices, size*sizeof(cl_device_id), MSG_WAITALL);
        if(num_devices) *num_devices = size;
        // A little bit special case when data transfer could failed
        if(*sockfd < 0)
            continue;
        return CL_SUCCESS;
    }
    // Device not found on any server
    return CL_INVALID_DEVICE;
}

cl_int oclandRetainDevice(cl_device_id device)
{
    unsigned int i;
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_DEVICE;
    }
    // Try platform in all servers
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send starting command declaration
        unsigned int commDim = strlen("clRetainDevice")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clRetainDevice");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Send parameters (we need to send size of properties)
        Send(sockfd, &device, sizeof(cl_platform_id), 0);
        // And request flag and real size of object
        cl_int flag = CL_INVALID_DEVICE;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            // 2 possibilities, not right server or error
            if(flag == CL_INVALID_DEVICE)
                continue;
            return flag;
        }
        // A little bit special case when data transfer could failed
        if(*sockfd < 0)
            continue;
        return CL_SUCCESS;
    }
    // Device not found on any server
    return CL_INVALID_DEVICE;
}

cl_int oclandReleaseDevice(cl_device_id device)
{
    unsigned int i;
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_DEVICE;
    }
    // Try platform in all servers
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send starting command declaration
        unsigned int commDim = strlen("clReleaseDevice")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clReleaseDevice");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Send parameters (we need to send size of properties)
        Send(sockfd, &device, sizeof(cl_platform_id), 0);
        // And request flag and real size of object
        cl_int flag = CL_INVALID_DEVICE;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            // 2 possibilities, not right server or error
            if(flag == CL_INVALID_DEVICE)
                continue;
            return flag;
        }
        // A little bit special case when data transfer could failed
        if(*sockfd < 0)
            continue;
        return CL_SUCCESS;
    }
    // Device not found on any server
    return CL_INVALID_DEVICE;
}

cl_mem oclandCreateImage(cl_context              context,
                         cl_mem_flags            flags,
                         const cl_image_format * image_format,
                         const cl_image_desc *   image_desc,
                         size_t                  element_size,
                         void *                  host_ptr,
                         cl_int *                errcode_ret)
{
    // Get the server
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Build the package
    size_t size = image_desc->image_width*image_desc->image_height*image_desc->image_depth*element_size;
    cl_bool hasPtr = CL_FALSE;
    if(host_ptr) hasPtr = CL_TRUE;
    size_t msgSize  = sizeof(unsigned int);    // Command index
    msgSize        += sizeof(cl_context);      // context
    msgSize        += sizeof(cl_mem_flags);    // flags
    msgSize        += sizeof(cl_image_format); // image_format
    msgSize        += sizeof(cl_image_desc);   // cl_image_desc
    msgSize        += sizeof(cl_bool);         // hasPtr
    if(host_ptr) msgSize += size;              // host_ptr
    void* msg = (void*)malloc(msgSize);
    void* ptr = msg;
    ((unsigned int*)ptr)[0]    = ocland_clCreateImage; ptr = (unsigned int*)ptr + 1;
    ((cl_context*)ptr)[0]      = context;              ptr = (cl_context*)ptr + 1;
    ((cl_mem_flags*)ptr)[0]    = flags;                ptr = (cl_mem_flags*)ptr + 1;
    memcpy(ptr,image_format,sizeof(cl_image_format));  ptr = (cl_image_format*)ptr + 1;
    memcpy(ptr,image_desc,sizeof(cl_image_desc));      ptr = (cl_image_desc*)ptr + 1;
    ((cl_bool*)ptr)[0]         = hasPtr;                 ptr = (cl_bool*)ptr + 1;
    if(host_ptr) memcpy(ptr, host_ptr, size);
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    ptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the data
    cl_int flag = ((cl_int*)ptr)[0]; ptr = (cl_int*)ptr  + 1;
    if(errcode_ret) *errcode_ret = flag;
    if(flag != CL_SUCCESS)
        return NULL;
    cl_mem memobj = ((cl_mem*)ptr)[0];
    addShortcut((void*)memobj, sockfd);
    return memobj;
}

cl_program oclandCreateProgramWithBuiltInKernels(cl_context             context ,
                                                 cl_uint                num_devices ,
                                                 const cl_device_id *   device_list ,
                                                 const char *           kernel_names ,
                                                 cl_int *               errcode_ret)
{
    unsigned int i,n;
    char buffer[BUFF_SIZE];
    cl_program program = NULL;
    cl_int flag = CL_INVALID_CONTEXT;
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Look for a shortcut for the context
    int *sockfd = getShortcut(context);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clCreateProgramWithBuiltInKernels")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateProgramWithBuiltInKernels");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &num_devices, sizeof(cl_uint), 0);
    Send(sockfd, device_list, num_devices*sizeof(cl_device_id), 0);
    size_t size = (strlen(kernel_names)+1)*sizeof(char);
    Send(sockfd, &size, sizeof(size_t), 0);
    // Receive first the buffer purposed by server,
    // in order to can send data larger than the transfer
    // buffer.
    size_t buffsize;
    Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
    if(!buffsize){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    // Compute the number of packages needed
    n = size / buffsize;
    // Send package by pieces
    for(i=0;i<n;i++){
        Send(sockfd, kernel_names + i*buffsize, buffsize, 0);
    }
    if(size % buffsize){
        // Remains some data to transfer
        Send(sockfd, kernel_names + n*buffsize, size % buffsize, 0);
    }
    // And request flag and result
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &program, sizeof(cl_program), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    // Register the buffer
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
    unsigned int i,j,n;
    char buffer[BUFF_SIZE];
    cl_int flag = CL_INVALID_PROGRAM;
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_PROGRAM;
    }
    // Look for a shortcut
    int *sockfd = getShortcut(program);
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clCompileProgram")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCompileProgram");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &program, sizeof(cl_program), 0);
    Send(sockfd, &num_devices, sizeof(cl_uint), 0);
    if(num_devices)
        Send(sockfd, device_list, num_devices*sizeof(cl_device_id), 0);
    size_t size = (strlen(options)+1)*sizeof(char);
    Send(sockfd, &size, sizeof(size_t), 0);
    // Receive first the buffer purposed by server,
    // in order to can send data larger than the transfer
    // buffer.
    size_t buffsize;
    Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
    if(!buffsize){
        return CL_OUT_OF_HOST_MEMORY;
    }
    // Compute the number of packages needed
    n = size / buffsize;
    // Send package by pieces
    for(i=0;i<n;i++){
        Send(sockfd, options + i*buffsize, buffsize, 0);
    }
    if(size % buffsize){
        // Remains some data to transfer
        Send(sockfd, options + n*buffsize, size % buffsize, 0);
    }
    Send(sockfd, &num_input_headers, sizeof(cl_uint), 0);
    if(num_input_headers){
        Send(sockfd, input_headers, num_input_headers*sizeof(cl_program), 0);
        for(i=0;i<num_input_headers;i++){
            size = (strlen(header_include_names[i])+1)*sizeof(char);
            Send(sockfd, &size, sizeof(size_t), 0);
            // Compute the number of packages needed
            n = size / buffsize;
            // Send package by pieces
            for(j=0;j<n;j++){
                Send(sockfd, header_include_names[i] + j*buffsize, buffsize, 0);
            }
            if(size % buffsize){
                // Remains some data to transfer
                Send(sockfd, header_include_names[i] + n*buffsize, size % buffsize, 0);
            }
        }
    }
    // And request flag and result
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
    unsigned int i,n;
    char buffer[BUFF_SIZE];
    cl_program program = NULL;
    cl_int flag = CL_INVALID_CONTEXT;
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        if(errcode_ret) *errcode_ret=CL_INVALID_CONTEXT;
        return program;
    }
    // Look for a shortcut
    int *sockfd = getShortcut(context);
    if(!sockfd){
        if(errcode_ret) *errcode_ret=CL_INVALID_CONTEXT;
        return program;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clLinkProgram")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clLinkProgram");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &num_devices, sizeof(cl_uint), 0);
    if(num_devices)
        Send(sockfd, device_list, num_devices*sizeof(cl_device_id), 0);
    size_t size = (strlen(options)+1)*sizeof(char);
    Send(sockfd, &size, sizeof(size_t), 0);
    // Receive first the buffer purposed by server,
    // in order to can send data larger than the transfer
    // buffer.
    size_t buffsize;
    Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
    if(!buffsize){
        if(errcode_ret) *errcode_ret=CL_OUT_OF_HOST_MEMORY;
        return program;
    }
    // Compute the number of packages needed
    n = size / buffsize;
    // Send package by pieces
    for(i=0;i<n;i++){
        Send(sockfd, options + i*buffsize, buffsize, 0);
    }
    if(size % buffsize){
        // Remains some data to transfer
        Send(sockfd, options + n*buffsize, size % buffsize, 0);
    }
    Send(sockfd, &num_input_programs, sizeof(cl_uint), 0);
    Send(sockfd, input_programs, num_input_programs*sizeof(cl_program), 0);
    // And request flag and result
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &program, sizeof(cl_program), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    if(flag == CL_SUCCESS){
        // Register the new pointer
        addShortcut((void *)program, sockfd);
    }
    return program;
}

cl_int oclandUnloadPlatformCompiler(cl_platform_id  platform)
{
    unsigned int i;
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_CONTEXT;
    }
    // Try platform in all servers
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send starting command declaration
        unsigned int commDim = strlen("clUnloadPlatformCompiler")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clUnloadPlatformCompiler");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Send parameters
        Send(sockfd, &platform, sizeof(cl_platform_id), 0);
        // And request flag and real size of object
        cl_int flag = CL_INVALID_PLATFORM;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            // 2 possibilities, not right server or error
            if(flag == CL_INVALID_PLATFORM)
                continue;
            return flag;
        }
        // Little bit special case when data transfer could failed
        if(*sockfd < 0)
            continue;
        return CL_SUCCESS;
    }
    // Platform not found on any server
    return CL_INVALID_PLATFORM;
}

cl_int oclandGetKernelArgInfo(cl_kernel            kernel ,
                              cl_uint              arg_index ,
                              cl_kernel_arg_info   param_name ,
                              size_t               param_value_size ,
                              void *               param_value ,
                              size_t *             param_value_size_ret)
{
    // Get the server
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Build the package
    size_t msgSize  = sizeof(unsigned int);        // Command index
    msgSize        += sizeof(cl_kernel);           // kernel
    msgSize        += sizeof(cl_uint);             // arg_index
    msgSize        += sizeof(cl_kernel_arg_info);  // param_name
    msgSize        += sizeof(size_t);              // param_value_size
    void* msg = (void*)malloc(msgSize);
    void* ptr = msg;
    ((unsigned int*)ptr)[0]   = ocland_clGetKernelArgInfo; ptr = (unsigned int*)ptr + 1;
    ((cl_kernel*)ptr)[0]      = kernel;           ptr = (cl_kernel*)ptr + 1;
    ((cl_uint*)ptr)[0]        = arg_index;        ptr = (cl_uint*)ptr + 1;
    ((cl_kernel_arg_info*)ptr)[0] = param_name;   ptr = (cl_kernel_arg_info*)ptr + 1;
    ((size_t*)ptr)[0]         = param_value_size; ptr = (size_t*)ptr + 1;
    // Send the package (first the size, and then the data)
    lock(*sockfd);
    Send(sockfd, &msgSize, sizeof(size_t), 0);
    Send(sockfd, msg, msgSize, 0);
    free(msg); msg=NULL;
    // Receive the package (first size, and then data)
    Recv(sockfd, &msgSize, sizeof(size_t), MSG_WAITALL);
    msg = (void*)malloc(msgSize);
    ptr = msg;
    Recv(sockfd, msg, msgSize, MSG_WAITALL);
    unlock(*sockfd);
    // Decript the data
    cl_int flag     = ((cl_int*)ptr)[0]; ptr = (cl_int*)ptr + 1;
    size_t size_ret = ((size_t*)ptr)[0]; ptr = (size_t*)ptr + 1;
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if( (flag == CL_SUCCESS) && param_value )
        memcpy(param_value, ptr, size_ret);
    return flag;
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
    char buffer[BUFF_SIZE];
    cl_bool want_event = CL_FALSE;
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clEnqueueFillBuffer")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueFillBuffer");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &mem, sizeof(cl_mem), 0);
    Send(sockfd, &pattern_size, sizeof(size_t), 0);
    Send(sockfd, &pattern, pattern_size, 0);
    Send(sockfd, &offset, sizeof(size_t), 0);
    Send(sockfd, &cb, sizeof(size_t), 0);
    Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    if(num_events_in_wait_list)
        Send(sockfd, &event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    if(event)
        want_event = CL_TRUE;
    Send(sockfd, &want_event, sizeof(cl_bool), 0);
    // And request flag, and event if request
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if((flag == CL_SUCCESS) && (event)){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return flag;
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
    char buffer[BUFF_SIZE];
    cl_bool want_event = CL_FALSE;
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clEnqueueFillImage")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueFillImage");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &image, sizeof(cl_mem), 0);
    Send(sockfd, &fill_color_size, sizeof(size_t), 0);
    Send(sockfd, &fill_color, fill_color_size, 0);
    Send(sockfd, origin, 3*sizeof(size_t), 0);
    Send(sockfd, region, 3*sizeof(size_t), 0);
    Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    if(num_events_in_wait_list)
        Send(sockfd, &event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    if(event)
        want_event = CL_TRUE;
    Send(sockfd, &want_event, sizeof(cl_bool), 0);
    // And request flag, and event if request
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if((flag == CL_SUCCESS) && (event)){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return flag;
}

cl_int oclandEnqueueMigrateMemObjects(cl_command_queue        command_queue ,
                                      cl_uint                 num_mem_objects ,
                                      const cl_mem *          mem_objects ,
                                      cl_mem_migration_flags  flags ,
                                      cl_uint                 num_events_in_wait_list ,
                                      const cl_event *        event_wait_list ,
                                      cl_event *              event)
{
    char buffer[BUFF_SIZE];
    cl_bool want_event = CL_FALSE;
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clEnqueueMigrateMemObjects")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueMigrateMemObjects");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &num_mem_objects, sizeof(cl_uint), 0);
    Send(sockfd, mem_objects, num_mem_objects*sizeof(cl_mem), 0);
    Send(sockfd, &flags, sizeof(cl_mem_migration_flags), 0);
    Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    if(num_events_in_wait_list)
        Send(sockfd, &event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    if(event)
        want_event = CL_TRUE;
    Send(sockfd, &want_event, sizeof(cl_bool), 0);
    // And request flag, and event if request
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if((flag == CL_SUCCESS) && (event)){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return flag;
}

cl_int oclandEnqueueMarkerWithWaitList(cl_command_queue  command_queue ,
                                       cl_uint            num_events_in_wait_list ,
                                       const cl_event *   event_wait_list ,
                                       cl_event *         event)
{
    char buffer[BUFF_SIZE];
    cl_bool want_event = CL_FALSE;
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clEnqueueMarkerWithWaitList")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueMarkerWithWaitList");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    if(num_events_in_wait_list)
        Send(sockfd, &event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    if(event)
        want_event = CL_TRUE;
    Send(sockfd, &want_event, sizeof(cl_bool), 0);
    // And request flag, and event if request
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if((flag == CL_SUCCESS) && (event)){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return flag;
}

cl_int oclandEnqueueBarrierWithWaitList(cl_command_queue  command_queue ,
                                        cl_uint            num_events_in_wait_list ,
                                        const cl_event *   event_wait_list ,
                                        cl_event *         event)
{
    char buffer[BUFF_SIZE];
    cl_bool want_event = CL_FALSE;
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clEnqueueBarrierWithWaitList")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueBarrierWithWaitList");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    if(num_events_in_wait_list)
        Send(sockfd, &event_wait_list, num_events_in_wait_list*sizeof(cl_event), 0);
    if(event)
        want_event = CL_TRUE;
    Send(sockfd, &want_event, sizeof(cl_bool), 0);
    // And request flag, and event if request
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if((flag == CL_SUCCESS) && (event)){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    return flag;
}

/*
 *  This file is part of ocland, a free CFD program based on SPH.
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

#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include <ocland/common/dataExchange.h>
#include <ocland/client/ocland.h>
#include <ocland/client/shortcut.h>

#ifndef OCLAND_PORT
    #define OCLAND_PORT 51000u
#endif

#ifndef BUFF_SIZE
    #define BUFF_SIZE 1025u
#endif

/// Servers data storage
static oclandServers* servers = NULL;
/// Servers initialization flag
static cl_bool initialized = CL_FALSE;

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
        // Store socket
        servers->sockets[i] = sockfd;
        n++;
    }
    return n;
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
    cl_uint t_num_platforms = 0;
    if(num_platforms) *num_platforms = t_num_platforms;
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit())
        return CL_SUCCESS;
    // Get number of platforms from servers
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        char buffer[BUFF_SIZE];
        // Send starting command declaration
        unsigned int commDim = strlen("clGetPlatformIDs")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clGetPlatformIDs");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Get remaining platforms to store
        unsigned int r_num_platforms = 0;
        if(t_num_platforms < num_entries){
            r_num_platforms = num_entries - t_num_platforms;
        }
        // Now we must send number of entries.
        cl_uint l_num_platforms = 0;
        cl_int flag = CL_SUCCESS;
        Send(sockfd, &r_num_platforms, sizeof(cl_uint), 0);
        // And request flag and number of platforms detected
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        Recv(sockfd, &l_num_platforms, sizeof(cl_uint), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            return flag;
        }
        // A little bit special case when data transfer could failed
        if(*sockfd < 0)
            continue;
        if(!l_num_platforms)
            continue;
        if(!r_num_platforms){
            t_num_platforms += l_num_platforms;
            continue;
        }
        // Get server platforms
        unsigned int n = r_num_platforms; if(n > l_num_platforms) n=l_num_platforms;
        cl_platform_id *l_platforms = (cl_platform_id*)malloc(n*sizeof(cl_platform_id));
        Recv(sockfd, l_platforms, n*sizeof(cl_platform_id), MSG_WAITALL);
        for(j=0;j<n;j++){
            platforms[t_num_platforms + j] = l_platforms[j];
        }
        t_num_platforms += l_num_platforms;
        free(l_platforms);
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
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_PLATFORM;
    }
    // Try platform in all servers
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send starting command declaration
        unsigned int commDim = strlen("clGetPlatformInfo")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clGetPlatformInfo");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Send parameters
        cl_platform_id p = platform;
        Send(sockfd, &p, sizeof(cl_platform_id), 0);
        Send(sockfd, &param_name, sizeof(cl_platform_info), 0);
        Send(sockfd, &param_value_size, sizeof(size_t), 0);
        // And request flag and real size of object
        cl_int flag = CL_INVALID_PLATFORM;
        size_t size = 0;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        Recv(sockfd, &size, sizeof(size_t), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            // 2 possibilities, not right server or error
            if(flag == CL_INVALID_PLATFORM)
                continue;
            return flag;
        }
        // Get returned info
        Recv(sockfd, param_value, size, MSG_WAITALL);
        if(param_value_size_ret) *param_value_size_ret = size;
        // Little bit special case when data transfer can failed
        if(*sockfd < 0)
            continue;
        return CL_SUCCESS;
    }
    // Platform not found on any server
    return CL_INVALID_PLATFORM;
}

cl_int oclandGetDeviceIDs(cl_platform_id   platform,
                          cl_device_type   device_type,
                          cl_uint          num_entries,
                          cl_device_id *   devices,
                          cl_uint *        num_devices)
{
    unsigned int i,j;
    cl_uint t_num_devices = 0;
    if(num_devices) *num_devices = t_num_devices;
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit())
        return CL_INVALID_PLATFORM;
    // Get devices from servers platforms
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        char buffer[BUFF_SIZE];
        // Send starting command declaration
        unsigned int commDim = strlen("clGetDeviceIDs")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clGetDeviceIDs");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Now we must send parameters.
        cl_int flag = CL_INVALID_PLATFORM;
        cl_platform_id p = platform - i - 1;
        Send(sockfd, &p, sizeof(cl_platform_id), 0);
        Send(sockfd, &device_type, sizeof(cl_device_type), 0);
        Send(sockfd, &num_entries, sizeof(cl_uint), 0);
        // And request flag and number of devices detected
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        Recv(sockfd, &t_num_devices, sizeof(cl_uint), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            // 2 possibilities, not right server or error
            if(flag == CL_INVALID_PLATFORM)
                continue;
            return flag;
        }
        // A little bit special case when data transfer could failed
        if(*sockfd < 0)
            continue;
        if(!t_num_devices || !num_entries){
            // We get the right platform, but no devices found
            if(num_devices) *num_devices = t_num_devices;
            return flag;
        }
        // Get devices
        unsigned int n = num_entries; if(n > t_num_devices) n=t_num_devices;
        cl_device_id *l_devices = (cl_device_id*)malloc(n*sizeof(cl_device_id));
        Recv(sockfd, l_devices, n*sizeof(cl_device_id), MSG_WAITALL);
        for(j=0;j<n;j++){
            devices[j] = l_devices[j];
        }
        if(num_devices) *num_devices = t_num_devices;
        free(l_devices);
        return CL_SUCCESS;
    }
    // No servers recognize provided platform
    return CL_INVALID_PLATFORM;
}

cl_int oclandGetDeviceInfo(cl_device_id    device,
                           cl_device_info  param_name,
                           size_t          param_value_size,
                           void *          param_value,
                           size_t *        param_value_size_ret)
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
        unsigned int commDim = strlen("clGetDeviceInfo")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clGetDeviceInfo");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Send parameters
        cl_device_id d = device;
        Send(sockfd, &d, sizeof(cl_platform_id), 0);
        Send(sockfd, &param_name, sizeof(cl_device_info), 0);
        Send(sockfd, &param_value_size, sizeof(size_t), 0);
        // And request flag and real size of object
        cl_int flag = CL_INVALID_DEVICE;
        size_t size = 0;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        Recv(sockfd, &size, sizeof(size_t), MSG_WAITALL);
        if(flag != CL_SUCCESS){
            // 2 possibilities, not right server or error
            if(flag == CL_INVALID_DEVICE)
                continue;
            return flag;
        }
        // Get returned info
        Recv(sockfd, param_value, size, MSG_WAITALL);
        if(param_value_size_ret) *param_value_size_ret = size;
        // A little bit special case when data transfer could failed
        if(*sockfd < 0)
            continue;
        return CL_SUCCESS;
    }
    // Device not found on any server
    return CL_INVALID_DEVICE;
}

cl_context oclandCreateContext(const cl_context_properties * properties,
                               cl_uint                       num_properties,
                               cl_uint                       num_devices,
                               const cl_device_id *          devices,
                               void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *),
                               void *                        user_data,
                               cl_int *                      errcode_ret)
{
    unsigned int i,j;
    char buffer[BUFF_SIZE];
    cl_device_id d[num_devices];
    cl_context context;
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        if(errcode_ret) *errcode_ret=CL_INVALID_PLATFORM;
        return NULL;
    }
    // Try devices in all servers
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send starting command declaration
        unsigned int commDim = strlen("clCreateContext")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clCreateContext");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Create corrected devices array
        for(j=0;j<num_devices;j++)
            d[j] = devices[j];
        // Send parameters (we need to send size of properties)
        size_t sProps = 0;
        if(properties){
            // Only CL_CONTEXT_PLATFORM will be supported, D3D ar GL can't be enabled in network
            sProps = num_properties*sizeof(cl_context_properties);
            Send(sockfd, &sProps, sizeof(size_t), 0);
            Send(sockfd, properties, sProps, 0);
        }
        else{
            Send(sockfd, &sProps, sizeof(size_t), 0);
        }
        Send(sockfd, &num_devices, sizeof(cl_uint), 0);
        Send(sockfd, d, num_devices*sizeof(cl_device_id), 0);
        /// pfn_notify is not implementable, so will be ignored.
        // And request flag and context
        cl_int flag = CL_INVALID_DEVICE;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        Recv(sockfd, &context, sizeof(cl_context), MSG_WAITALL);
        if(errcode_ret) *errcode_ret = flag;
        if(flag != CL_SUCCESS){
            // 2 possibilities, not right server or error
            if(flag == CL_INVALID_DEVICE)
                continue;
            return context;
        }
        // A little bit special case when data transfer could failed
        if(*sockfd < 0)
            continue;
        // Register the context as shortcut
        addShortcut((void*)context, sockfd);
        return context;
    }
    // Device not found on any server
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
    char buffer[BUFF_SIZE];
    cl_context context;
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        if(errcode_ret) *errcode_ret=CL_INVALID_PLATFORM;
        return NULL;
    }
    // Try devices in all servers
    for(i=0;i<servers->num_servers;i++){
        if(servers->sockets[i] < 0)
            continue;
        int *sockfd = &(servers->sockets[i]);
        // Send starting command declaration
        unsigned int commDim = strlen("clCreateContextFromType")+1;
        Send(sockfd, &commDim, sizeof(unsigned int), 0);
        // Send command to perform
        strcpy(buffer, "clCreateContextFromType");
        Send(sockfd, buffer, strlen(buffer)+1, 0);
        // Send parameters (we need to send size of properties)
        size_t sProps = 0;
        if(properties){
            // Only CL_CONTEXT_PLATFORM will be supported, D3D ar GL can't be enabled in network
            sProps = num_properties*sizeof(cl_context_properties);
            Send(sockfd, &sProps, sizeof(size_t), 0);
            Send(sockfd, properties, sProps, 0);
        }
        else{
            Send(sockfd, &sProps, sizeof(size_t), 0);
        }
        Send(sockfd, &device_type, sizeof(cl_device_type), 0);
        /// pfn_notify is not implementable, so will be ignored.
        // And request flag and context
        cl_int flag = CL_INVALID_DEVICE;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        Recv(sockfd, &context, sizeof(cl_context), MSG_WAITALL);
        if(errcode_ret) *errcode_ret = flag;
        if(flag != CL_SUCCESS){
            // 2 possibilities, not right server or error
            if(flag == CL_INVALID_DEVICE)
                continue;
            return context;
        }
        // A little bit special case when data transfer could failed
        if(*sockfd < 0)
            continue;
        // Register the context as shortcut
        addShortcut((void*)context, sockfd);
        return context;
    }
    // Device not found on any server
    if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
    return NULL;
}

cl_int oclandRetainContext(cl_context context)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_CONTEXT;
    }
    // Look for a shortcut for the context
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clRetainContext")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clRetainContext");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters (we need to send size of properties)
    Send(sockfd, &context, sizeof(cl_context), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandReleaseContext(cl_context context)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_CONTEXT;
    }
    // Look for a shortcut for the context
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clReleaseContext")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clReleaseContext");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters (we need to send size of properties)
    Send(sockfd, &context, sizeof(cl_context), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0){
        return flag;
    }
    // We don't need shortcut anymore
    delShortcut(context);
    return CL_SUCCESS;
}

cl_int oclandGetContextInfo(cl_context         context,
                            cl_context_info    param_name,
                            size_t             param_value_size,
                            void *             param_value,
                            size_t *           param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_CONTEXT;
    }
    // Look for a shortcut for the context
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetContextInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetContextInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &param_name, sizeof(cl_context_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_CONTEXT;
    size_t size = 0;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &size, sizeof(size_t), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // Get returned info
    Recv(sockfd, param_value, size, MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_command_queue oclandCreateCommandQueue(cl_context                     context,
                                          cl_device_id                   device,
                                          cl_command_queue_properties    properties,
                                          cl_int *                       errcode_ret)
{
    char buffer[BUFF_SIZE];
    cl_command_queue queue;
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
    unsigned int commDim = strlen("clCreateCommandQueue")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateCommandQueue");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &device, sizeof(cl_device_id), 0);
    Send(sockfd, &properties, sizeof(properties), 0);
    // And request flag and command queue
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &queue, sizeof(cl_command_queue), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    if(flag != CL_SUCCESS)
        return queue;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return queue;
    // Register the new command queue
    addShortcut((void*)queue, sockfd);
    return queue;
}

cl_int oclandRetainCommandQueue(cl_command_queue command_queue)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Look for a shortcut for the command queue
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clRetainCommandQueue")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clRetainCommandQueue");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters (we need to send size of properties)
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_COMMAND_QUEUE;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandReleaseCommandQueue(cl_command_queue command_queue)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Look for a shortcut for the command queue
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clReleaseCommandQueue")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clReleaseCommandQueue");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters (we need to send size of properties)
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_COMMAND_QUEUE;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    // We need to unregister the shortcut
    delShortcut(command_queue);
    return CL_SUCCESS;
}

cl_int oclandGetCommandQueueInfo(cl_command_queue      command_queue,
                                 cl_command_queue_info param_name,
                                 size_t                param_value_size,
                                 void *                param_value,
                                 size_t *              param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Look for a shortcut for the command queue
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    unsigned int commDim = strlen("clGetCommandQueueInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetCommandQueueInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &param_name, sizeof(cl_command_queue_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_COMMAND_QUEUE;
    size_t size = 0;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &size, sizeof(size_t), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // Get returned info
    Recv(sockfd, param_value, size, MSG_WAITALL);
    if(param_value_size_ret) *param_value_size_ret = size;
    // A little bit special case when data transfer can fail
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_mem oclandCreateBuffer(cl_context    context ,
                          cl_mem_flags  flags ,
                          size_t        size ,
                          void *        host_ptr ,
                          cl_int *      errcode_ret)
{
    char buffer[BUFF_SIZE];
    cl_mem clMem;
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
    unsigned int commDim = strlen("clCreateBuffer")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateBuffer");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &flags, sizeof(cl_mem_flags), 0);
    Send(sockfd, &size, sizeof(size_t), 0);
    if(flags & CL_MEM_COPY_HOST_PTR){
        // Really large data, take care here
        // Receive first the buffer purposed by server
        size_t buffsize;
        Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
        if(!buffsize){
            if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        // Compute the number of packages needed
        unsigned int i,n;
        n = size / buffsize;
        // Send package by pieces
        for(i=0;i<n;i++){
            Send(sockfd, host_ptr + i*buffsize, buffsize, 0);
        }
        if(size % buffsize){
            // Remains some data to transfer
            Send(sockfd, host_ptr + n*buffsize, size % buffsize, 0);
        }
    }
    // And request flag and result
    cl_int flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &clMem, sizeof(cl_mem), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    // Register the buffer
    addShortcut((void*)clMem, sockfd);
    return clMem;
}

cl_int oclandRetainMemObject(cl_mem memobj)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_MEM_OBJECT;
    }
    // Look for a shortcut for the command queue
    int *sockfd = getShortcut(memobj);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clRetainMemObject")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clRetainMemObject");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters (we need to send size of properties)
    Send(sockfd, &memobj, sizeof(cl_mem), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_MEM_OBJECT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandReleaseMemObject(cl_mem memobj)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_MEM_OBJECT;
    }
    // Look for a shortcut for the command queue
    int *sockfd = getShortcut(memobj);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clReleaseMemObject")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clReleaseMemObject");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters (we need to send size of properties)
    Send(sockfd, &memobj, sizeof(cl_mem), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_MEM_OBJECT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    // We need to unregister the shortcut
    delShortcut(memobj);
    return CL_SUCCESS;
}

cl_int oclandGetSupportedImageFormats(cl_context           context,
                                      cl_mem_flags         flags,
                                      cl_mem_object_type   image_type ,
                                      cl_uint              num_entries ,
                                      cl_image_format *    image_formats ,
                                      cl_uint *            num_image_formats)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_CONTEXT;
    }
    // Look for a shortcut for the context
    int *sockfd = getShortcut(context);
    if(!sockfd){
        return CL_INVALID_CONTEXT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetSupportedImageFormats")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetSupportedImageFormats");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &flags, sizeof(cl_mem_flags), 0);
    Send(sockfd, &image_type, sizeof(cl_mem_object_type), 0);
    Send(sockfd, &num_entries, sizeof(cl_uint), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    cl_uint num_formats;
    Recv(sockfd, &num_formats, sizeof(cl_uint), MSG_WAITALL);
    if(num_image_formats)
        *num_image_formats = num_formats;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return num_entries image formats
    // even though num_image_formats can be lower,
    // user may know how many of them are unset using
    // num_image_formats returned value.
    if(num_entries){
        Recv(sockfd, image_formats, num_entries*sizeof(cl_image_format), MSG_WAITALL);
    }
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandGetMemObjectInfo(cl_mem            memobj ,
                              cl_mem_info       param_name ,
                              size_t            param_value_size ,
                              void *            param_value ,
                              size_t *          param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_MEM_OBJECT;
    }
    // Look for a shortcut for the memobj
    int *sockfd = getShortcut(memobj);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetMemObjectInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetMemObjectInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &memobj, sizeof(cl_mem), 0);
    Send(sockfd, &param_name, sizeof(cl_mem_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_MEM_OBJECT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        Recv(sockfd, param_value, param_value_size, MSG_WAITALL);
    }
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandGetImageInfo(cl_mem            memobj ,
                          cl_mem_info       param_name ,
                          size_t            param_value_size ,
                          void *            param_value ,
                          size_t *          param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_MEM_OBJECT;
    }
    // Look for a shortcut for the memobj
    int *sockfd = getShortcut(memobj);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetImageInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetImageInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &memobj, sizeof(cl_mem), 0);
    Send(sockfd, &param_name, sizeof(cl_mem_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_MEM_OBJECT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        Recv(sockfd, param_value, param_value_size, MSG_WAITALL);
    }
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_sampler oclandCreateSampler(cl_context           context ,
                               cl_bool              normalized_coords ,
                               cl_addressing_mode   addressing_mode ,
                               cl_filter_mode       filter_mode ,
                               cl_int *             errcode_ret)
{
    char buffer[BUFF_SIZE];
    cl_sampler sampler = NULL;
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
    unsigned int commDim = strlen("clCreateSampler")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateSampler");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &normalized_coords, sizeof(cl_bool), 0);
    Send(sockfd, &addressing_mode, sizeof(cl_addressing_mode), 0);
    Send(sockfd, &filter_mode, sizeof(cl_filter_mode), 0);
    // And request flag and command queue
    cl_int flag = CL_INVALID_CONTEXT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &sampler, sizeof(cl_sampler), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    if(flag != CL_SUCCESS)
        return sampler;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return sampler;
    // Register the new sampler
    addShortcut((void*)sampler, sockfd);
    return sampler;
}

cl_int oclandRetainSampler(cl_sampler sampler)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_SAMPLER;
    }
    // Look for a shortcut
    int *sockfd = getShortcut(sampler);
    if(!sockfd){
        return CL_INVALID_SAMPLER;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clRetainSampler")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clRetainSampler");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &sampler, sizeof(cl_sampler), 0);
    // And request flag
    cl_int flag = CL_INVALID_SAMPLER;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandReleaseSampler(cl_sampler sampler)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_SAMPLER;
    }
    // Look for a shortcut
    int *sockfd = getShortcut(sampler);
    if(!sockfd){
        return CL_INVALID_SAMPLER;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clReleaseSampler")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clReleaseSampler");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &sampler, sizeof(cl_sampler), 0);
    // And request flag
    cl_int flag = CL_INVALID_SAMPLER;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    // We need to unregister the shortcut
    delShortcut(sampler);
    return CL_SUCCESS;
}

cl_int oclandGetSamplerInfo(cl_sampler          sampler ,
                            cl_sampler_info     param_name ,
                            size_t              param_value_size ,
                            void *              param_value ,
                            size_t *            param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        return CL_INVALID_MEM_OBJECT;
    }
    // Look for a shortcut
    int *sockfd = getShortcut(sampler);
    if(!sockfd){
        return CL_INVALID_MEM_OBJECT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetSamplerInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetSamplerInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &sampler, sizeof(cl_sampler), 0);
    Send(sockfd, &param_name, sizeof(cl_sampler_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_MEM_OBJECT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        Recv(sockfd, param_value, param_value_size, MSG_WAITALL);
    }
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_program oclandCreateProgramWithSource(cl_context         context ,
                                         cl_uint            count ,
                                         const char **      strings ,
                                         const size_t *     lengths ,
                                         cl_int *           errcode_ret)
{
    unsigned int i;
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
    unsigned int commDim = strlen("clCreateProgramWithSource")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateProgramWithSource");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &count, sizeof(cl_uint), 0);
    // Receive first the buffer purposed by server,
    // in order to can send data larger than the transfer
    // buffer.
    size_t buffsize;
    Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
    if(!buffsize){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    // Send the text arrays
    for(i=0;i<count;i++){
        size_t size = (strlen(strings[i])+1)*sizeof(char);
        if(lengths){
            if(lengths[i] > 0){
                size = lengths[i];
            }
        }
        // Text length
        Send(sockfd, &size, sizeof(size_t), 0);
        // Compute the number of packages needed
        unsigned int j,n;
        n = size / buffsize;
        // Send package by pieces
        for(j=0;j<n;j++){
            Send(sockfd, strings[i] + j*buffsize, buffsize, 0);
        }
        if(size % buffsize){
            // Remains some data to transfer
            Send(sockfd, strings[i] + n*buffsize, size % buffsize, 0);
        }
    }
    // And request flag and result
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &program, sizeof(cl_program), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    // Register the buffer
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
    char buffer[BUFF_SIZE];
    cl_program program = NULL;
    cl_int status[num_devices];
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
    unsigned int commDim = strlen("clCreateProgramWithBinary")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateProgramWithBinary");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &num_devices, sizeof(cl_uint), 0);
    // Receive first the buffer purposed by server,
    // in order to can send data larger than the transfer
    // buffer.
    size_t buffsize;
    Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
    if(!buffsize){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    // Send the arrays
    for(i=0;i<num_devices;i++){
        // Device
        Send(sockfd, &device_list[i], sizeof(cl_device_id), 0);
        // Binary length
        size_t size = lengths[i];
        Send(sockfd, &size, sizeof(size_t), 0);
        // Compute the number of packages needed
        unsigned int j,n;
        n = size / buffsize;
        // Send package by pieces
        for(j=0;j<n;j++){
            Send(sockfd, binaries[i] + j*buffsize, buffsize, 0);
        }
        if(size % buffsize){
            // Remains some data to transfer
            Send(sockfd, binaries[i] + n*buffsize, size % buffsize, 0);
        }
    }
    // And request flag and result
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &program, sizeof(cl_program), MSG_WAITALL);
    Recv(sockfd, &status, num_devices*sizeof(cl_int), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    // Register the buffer
    addShortcut((void*)program, sockfd);
    return program;
}

cl_int oclandRetainProgram(cl_program  program)
{
    char buffer[BUFF_SIZE];
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
    unsigned int commDim = strlen("clRetainProgram")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clRetainProgram");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters (we need to send size of properties)
    Send(sockfd, &program, sizeof(cl_program), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_PROGRAM;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandReleaseProgram(cl_program  program)
{
    char buffer[BUFF_SIZE];
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
    unsigned int commDim = strlen("clReleaseProgram")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clReleaseProgram");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters (we need to send size of properties)
    Send(sockfd, &program, sizeof(cl_program), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_PROGRAM;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0){
        return flag;
    }
    // We don't need shortcut anymore
    delShortcut(program);
    return CL_SUCCESS;
}

cl_int oclandBuildProgram(cl_program            program ,
                          cl_uint               num_devices ,
                          const cl_device_id *  device_list ,
                          const char *          options ,
                          void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                          void *                user_data)
{
    unsigned int i,n;
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
    unsigned int commDim = strlen("clBuildProgram")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clBuildProgram");
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
    // And request flag and result
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandGetProgramInfo(cl_program          program ,
                            cl_program_info     param_name ,
                            size_t              param_value_size ,
                            void *              param_value ,
                            size_t *            param_value_size_ret)
{
    char buffer[BUFF_SIZE];
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
    unsigned int commDim = strlen("clGetProgramInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetProgramInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &program, sizeof(cl_program), 0);
    Send(sockfd, &param_name, sizeof(cl_program_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_MEM_OBJECT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        Recv(sockfd, param_value, param_value_size, MSG_WAITALL);
    }
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandGetProgramBuildInfo(cl_program             program ,
                                 cl_device_id           device ,
                                 cl_program_build_info  param_name ,
                                 size_t                 param_value_size ,
                                 void *                 param_value ,
                                 size_t *               param_value_size_ret)
{
    unsigned int i,n;
    char buffer[BUFF_SIZE];
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
    unsigned int commDim = strlen("clGetProgramBuildInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetProgramBuildInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &program, sizeof(cl_program), 0);
    Send(sockfd, &device, sizeof(cl_device_id), 0);
    Send(sockfd, &param_name, sizeof(cl_program_build_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_MEM_OBJECT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        // Receive first the buffer purposed by server,
        // in order to can send data larger than the transfer
        // buffer.
        size_t buffsize;
        Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
        if(!buffsize){
            return CL_OUT_OF_HOST_MEMORY;
        }
        // Compute the number of packages needed
        n = param_value_size / buffsize;
        // Receive package by pieces
        for(i=0;i<n;i++){
            Recv(sockfd, param_value + i*buffsize, buffsize, MSG_WAITALL);
        }
        if(param_value_size % buffsize){
            // Remains some data to transfer
            Recv(sockfd, param_value + n*buffsize, param_value_size % buffsize, MSG_WAITALL);
        }
        // As special service, we will erase log content after size_ret
        if(param_value_size > size_ret){
            char *aux = param_value;
            strcpy(&aux[size_ret], "");
        }
    }
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_kernel oclandCreateKernel(cl_program       program ,
                             const char *     kernel_name ,
                             cl_int *         errcode_ret)
{
    char buffer[BUFF_SIZE];
    cl_kernel kernel = NULL;
    // Look for a shortcut
    int *sockfd = getShortcut(program);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_PROGRAM;
        return NULL;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clCreateKernel")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateKernel");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &program, sizeof(cl_program), 0);
    size_t size = (strlen(kernel_name) + 1)*sizeof(char);
    Send(sockfd, &size, sizeof(size_t), 0);
    Send(sockfd, kernel_name, size, 0);
    // And request flag and result
    cl_int flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &kernel, sizeof(cl_kernel), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    // Register the pointer
    addShortcut((void*)kernel, sockfd);
    return kernel;
}

cl_int oclandCreateKernelsInProgram(cl_program      program ,
                                    cl_uint         num_kernels ,
                                    cl_kernel *     kernels ,
                                    cl_uint *       num_kernels_ret)
{
    unsigned int i;
    char buffer[BUFF_SIZE];
    cl_uint kernels_ret = 0;
    cl_int flag;
    // Look for a shortcut
    int *sockfd = getShortcut(program);
    if(!sockfd){
        return flag;
    }
    // Correct number of kernels if is invalid
    if(!kernels)
        num_kernels = 0;
    // Execute the command on server
    unsigned int commDim = strlen("clCreateKernelsInProgram")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateKernelsInProgram");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &program, sizeof(cl_program), 0);
    Send(sockfd, &num_kernels, sizeof(cl_uint), 0);
    // And request flag and result
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &kernels_ret, sizeof(cl_uint), MSG_WAITALL);
    if(num_kernels_ret) *num_kernels_ret = kernels_ret;
    if((num_kernels) && (flag == CL_SUCCESS)){
        Recv(sockfd, kernels, num_kernels*sizeof(cl_kernel), MSG_WAITALL);
        // Register the pointers
        for(i=0;i<kernels_ret;i++)
            addShortcut((void*)kernels[i], sockfd);
    }
    return flag;
}

cl_int oclandRetainKernel(cl_kernel     kernel)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clRetainKernel")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clRetainKernel");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &kernel, sizeof(cl_kernel), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_KERNEL;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandReleaseKernel(cl_kernel    kernel)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clReleaseKernel")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clReleaseKernel");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &kernel, sizeof(cl_kernel), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_KERNEL;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0){
        return flag;
    }
    // We don't need the shortcut anymore
    delShortcut(kernel);
    return CL_SUCCESS;
}

cl_int oclandSetKernelArg(cl_kernel     kernel ,
                          cl_uint       arg_index ,
                          size_t        arg_size ,
                          const void *  arg_value)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Execute the command on server, that will be
    // different depending on arg_value
    char commName[24];
    strcpy(commName, "clSetKernelArg");
    if(arg_value == NULL)
        strcpy(commName, "clSetKernelNullArg");
    unsigned int commDim = strlen(commName)+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, commName);
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &kernel, sizeof(cl_kernel), 0);
    Send(sockfd, &arg_index, sizeof(cl_uint), 0);
    Send(sockfd, &arg_size, sizeof(size_t), 0);
    if(arg_value)
        Send(sockfd, arg_value, arg_size, 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_KERNEL;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandGetKernelInfo(cl_kernel        kernel ,
                           cl_kernel_info   param_name ,
                           size_t           param_value_size ,
                           void *           param_value ,
                           size_t *         param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetKernelInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetKernelInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &kernel, sizeof(cl_kernel), 0);
    Send(sockfd, &param_name, sizeof(cl_kernel_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_KERNEL;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        Recv(sockfd, param_value, param_value_size, MSG_WAITALL);
    }
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandGetKernelWorkGroupInfo(cl_kernel                   kernel ,
                                    cl_device_id                device ,
                                    cl_kernel_work_group_info   param_name ,
                                    size_t                      param_value_size ,
                                    void *                      param_value ,
                                    size_t *                    param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetKernelWorkGroupInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetKernelWorkGroupInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &kernel, sizeof(cl_kernel), 0);
    Send(sockfd, &device, sizeof(cl_device_id), 0);
    Send(sockfd, &param_name, sizeof(cl_kernel_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_KERNEL;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        Recv(sockfd, param_value, param_value_size, MSG_WAITALL);
    }
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}

cl_int oclandWaitForEvents(cl_uint              num_events ,
                           const cl_event *     event_list)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(event_list[0]);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clWaitForEvents")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clWaitForEvents");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &num_events, sizeof(num_events), 0);
    Send(sockfd, event_list, num_events*sizeof(cl_event), 0);
    // And request flag
    cl_int flag = CL_INVALID_EVENT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandGetEventInfo(cl_event          event ,
                          cl_event_info     param_name ,
                          size_t            param_value_size ,
                          void *            param_value ,
                          size_t *          param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetEventInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetEventInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &event, sizeof(cl_event), 0);
    Send(sockfd, &param_name, sizeof(cl_event_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_EVENT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        Recv(sockfd, param_value, param_value_size, MSG_WAITALL);
    }
    return flag;
}

cl_int oclandRetainEvent(cl_event  event)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clRetainEvent")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clRetainEvent");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &event, sizeof(cl_event), 0);
    // And request flag
    cl_int flag = CL_INVALID_EVENT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandReleaseEvent(cl_event  event)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clReleaseEvent")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clReleaseEvent");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &event, sizeof(cl_event), 0);
    // And request flag
    cl_int flag = CL_INVALID_EVENT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    // We need to unregister the shortcut
    delShortcut(event);
    return CL_SUCCESS;
}

cl_int oclandGetEventProfilingInfo(cl_event             event ,
                                   cl_profiling_info    param_name ,
                                   size_t               param_value_size ,
                                   void *               param_value ,
                                   size_t *             param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetEventProfilingInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetEventProfilingInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &event, sizeof(cl_event), 0);
    Send(sockfd, &param_name, sizeof(cl_event_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_EVENT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        Recv(sockfd, param_value, param_value_size, MSG_WAITALL);
    }
    return flag;
}

cl_int oclandFlush(cl_command_queue  command_queue)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clFlush")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clFlush");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    // And request flag
    cl_int flag = CL_INVALID_COMMAND_QUEUE;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

cl_int oclandFinish(cl_command_queue  command_queue)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(command_queue);
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clFinish")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clFinish");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    // And request flag
    cl_int flag = CL_INVALID_COMMAND_QUEUE;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

/** @struct dataTransfer Vars needed for
 * an asynchronously data transfer.
 */
struct dataTransfer{
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
    unsigned int i,n;
    struct dataTransfer* _data = (struct dataTransfer*)data;
    size_t buffsize = BUFF_SIZE*sizeof(char);
    int *fd = &(_data->fd);
    Recv(fd, &buffsize, sizeof(size_t), MSG_WAITALL);
    // Compute the number of packages needed
    n = _data->cb / buffsize;
    // Receive package by pieces
    for(i=0;i<n;i++){
        Recv(fd, _data->ptr + i*buffsize, buffsize, MSG_WAITALL);
    }
    if(_data->cb % buffsize){
        // Remains some data to arrive
        Recv(fd, _data->ptr + n*buffsize, _data->cb % buffsize, MSG_WAITALL);
    }
    free(_data); _data=NULL;
    pthread_exit(NULL);
    return NULL;
}

/** Performs a data reception asynchronously on a new thread and socket.
 * @param sockfd Connection socket.
 * @param data Data to transfer.
 */
void asyncDataRecv(int* sockfd, struct dataTransfer data)
{
    // -------------------------------------
    // Get server port and connect to it.
    // -------------------------------------
    unsigned int port;
    data.fd = -1;
    struct sockaddr_in serv_addr;
    Recv(sockfd, &port, sizeof(unsigned int), MSG_WAITALL);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        // we can't work, disconnect from server
        shutdown(*sockfd, 2);
        *sockfd = -1;
        return;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    socklen_t len_inet;
    len_inet = sizeof(serv_addr);
    getsockname(*sockfd, (struct sockaddr*)&serv_addr, &len_inet);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if( connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        // we can't work, disconnect from server
        shutdown(fd, 2);
        shutdown(*sockfd, 2);
        *sockfd = -1;
        return;
    }
    data.fd = fd;
    // -------------------------------------
    // Receive the data on another thread.
    // -------------------------------------
    pthread_t thread;
    struct dataTransfer* _data = (struct dataTransfer*)malloc(sizeof(struct dataTransfer));
    _data->cb    = data.cb;
    _data->fd    = data.fd;
    _data->ptr   = data.ptr;
    int rc = pthread_create(&thread, NULL, asyncDataRecv_thread, (void *)(_data));
    if(rc){
        // we can't work, disconnect the client
        shutdown(data.fd, 2);
        shutdown(*sockfd, 2);
        *sockfd = -1;
        return;
    }
}

cl_int oclandEnqueueReadBuffer(cl_command_queue     command_queue ,
                               cl_mem               mem ,
                               cl_bool              blocking_read ,
                               size_t               offset ,
                               size_t               cb ,
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
    unsigned int commDim = strlen("clEnqueueReadBuffer")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueReadBuffer");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &mem, sizeof(cl_mem), 0);
    Send(sockfd, &blocking_read, sizeof(cl_bool), 0);
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
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    // In case of blocking simply receive the data
    if(blocking_read == CL_TRUE){
        unsigned int i,n;
        // Receive first the buffer purposed by server,
        // in order to can send data larger than the transfer
        // buffer.
        size_t buffsize;
        Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
        if(!buffsize){
            return CL_OUT_OF_HOST_MEMORY;
        }
        // Compute the number of packages needed
        n = cb / buffsize;
        // Receive package by pieces
        for(i=0;i<n;i++){
            Recv(sockfd, ptr + i*buffsize, buffsize, MSG_WAITALL);
        }
        if(cb % buffsize){
            // Remains some data to transfer
            Recv(sockfd, ptr + n*buffsize, cb % buffsize, MSG_WAITALL);
        }
        return flag;
    }
    // In the non blocking case more complex operations are requested
    struct dataTransfer data;
    data.cb    = cb;
    data.ptr   = ptr;
    asyncDataRecv(sockfd, data);
    return flag;
}

/** Thread that sends data to server.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataSend_thread(void *data)
{
    unsigned int i,n;
    struct dataTransfer* _data = (struct dataTransfer*)data;
    size_t buffsize = BUFF_SIZE*sizeof(char);
    int *fd = &(_data->fd);
    Recv(fd, &buffsize, sizeof(size_t), MSG_WAITALL);
    // Compute the number of packages needed
    n = _data->cb / buffsize;
    // Send package by pieces
    for(i=0;i<n;i++){
        Send(fd, _data->ptr + i*buffsize, buffsize, 0);
    }
    if(_data->cb % buffsize){
        // Remains some data to arrive
        Send(fd, _data->ptr + n*buffsize, _data->cb % buffsize, 0);
    }
    free(_data); _data=NULL;
    pthread_exit(NULL);
    return NULL;
}

/** Performs a data reception asynchronously on a new thread and socket.
 * @param sockfd Connection socket.
 * @param data Data to transfer.
 */
void asyncDataSend(int* sockfd, struct dataTransfer data)
{
    // -------------------------------------
    // Get server port and connect to it.
    // -------------------------------------
    unsigned int port;
    data.fd = -1;
    struct sockaddr_in serv_addr;
    Recv(sockfd, &port, sizeof(unsigned int), MSG_WAITALL);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        // we can't work, disconnect from server
        shutdown(*sockfd, 2);
        *sockfd = -1;
        return;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    socklen_t len_inet;
    len_inet = sizeof(serv_addr);
    getsockname(*sockfd, (struct sockaddr*)&serv_addr, &len_inet);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if( connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        // we can't work, disconnect from server
        shutdown(fd, 2);
        shutdown(*sockfd, 2);
        *sockfd = -1;
        return;
    }
    data.fd = fd;
    // -------------------------------------
    // Receive the data on another thread.
    // -------------------------------------
    pthread_t thread;
    struct dataTransfer* _data = (struct dataTransfer*)malloc(sizeof(struct dataTransfer));
    _data->cb    = data.cb;
    _data->fd    = data.fd;
    _data->ptr   = data.ptr;
    int rc = pthread_create(&thread, NULL, asyncDataSend_thread, (void *)(_data));
    if(rc){
        // we can't work, disconnect the client
        shutdown(data.fd, 2);
        shutdown(*sockfd, 2);
        *sockfd = -1;
        return;
    }
}

cl_int oclandEnqueueWriteBuffer(cl_command_queue    command_queue ,
                                cl_mem              mem ,
                                cl_bool             blocking_write ,
                                size_t              offset ,
                                size_t              cb ,
                                const void *        ptr ,
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
    unsigned int commDim = strlen("clEnqueueWriteBuffer")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clEnqueueWriteBuffer");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &command_queue, sizeof(cl_command_queue), 0);
    Send(sockfd, &mem, sizeof(cl_mem), 0);
    Send(sockfd, &blocking_write, sizeof(cl_bool), 0);
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
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv(sockfd, event, sizeof(cl_event), MSG_WAITALL);
        addShortcut(*event, sockfd);
    }
    // In case of blocking simply receive the data
    if(blocking_write == CL_TRUE){
        unsigned int i,n;
        // Receive first the buffer purposed by server,
        // in order to can send data larger than the transfer
        // buffer.
        size_t buffsize;
        Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
        if(!buffsize){
            return CL_OUT_OF_HOST_MEMORY;
        }
        // Compute the number of packages needed
        n = cb / buffsize;
        // Send package by pieces
        for(i=0;i<n;i++){
            Send(sockfd, ptr + i*buffsize, buffsize, 0);
        }
        if(cb % buffsize){
            // Remains some data to transfer
            Send(sockfd, ptr + n*buffsize, cb % buffsize, 0);
        }
        // Get new flag after clEnqueueWriteBuffer has been
        // called in the server
        flag = CL_INVALID_CONTEXT;
        Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
        return flag;
    }
    // In the non blocking case more complex operations are requested
    struct dataTransfer data;
    data.cb    = cb;
    data.ptr   = (void*)ptr;
    asyncDataSend(sockfd, data);
    return flag;
}

#ifdef CL_API_SUFFIX__VERSION_1_1
cl_mem oclandCreateSubBuffer(cl_mem                    buffer ,
                             cl_mem_flags              flags ,
                             cl_buffer_create_type     buffer_create_type ,
                             const void *              buffer_create_info ,
                             cl_int *                  errcode_ret)
{
    char string[BUFF_SIZE];
    cl_mem clMem;
    // Ensure that ocland is already running
    // and exist servers to use
    if(!oclandInit()){
        if(errcode_ret) *errcode_ret = CL_INVALID_MEM_OBJECT;
        return NULL;
    }
    // Look for a shortcut for the buffer
    int *sockfd = getShortcut(buffer);
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_MEM_OBJECT;
        return NULL;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clCreateSubBuffer")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(string, "clCreateSubBuffer");
    Send(sockfd, string, strlen(string)+1, 0);
    // Send parameters
    Send(sockfd, &buffer, sizeof(cl_mem), 0);
    Send(sockfd, &flags, sizeof(cl_mem_flags), 0);
    Send(sockfd, &buffer_create_type, sizeof(cl_buffer_create_type), 0);
    Send(sockfd, buffer_create_info, sizeof(cl_buffer_region), 0);
    // And request flag and result
    cl_int flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &clMem, sizeof(cl_mem), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    return clMem;
}

cl_event oclandCreateUserEvent(cl_context     context ,
                               cl_int *       errcode_ret)
{
    char buffer[BUFF_SIZE];
    cl_event event;
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
    unsigned int commDim = strlen("clCreateUserEvent")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateUserEvent");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    // And request flag and result
    cl_int flag = CL_OUT_OF_RESOURCES;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &event, sizeof(cl_event), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    // Register the buffer
    addShortcut((void*)event, sockfd);
    return event;
}

cl_int oclandSetUserEventStatus(cl_event    event ,
                                cl_int      execution_status)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(event);
    if(!sockfd){
        return CL_INVALID_EVENT;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clSetUserEventStatus")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clSetUserEventStatus");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &event, sizeof(cl_event), 0);
    Send(sockfd, &execution_status, sizeof(cl_int), 0);
    // And request flag
    cl_int flag = CL_INVALID_EVENT;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    return flag;
}

/** @struct dataTransferRect Vars needed for
 * an asynchronously data transfer in 2D,3D
 * rectangle.
 */
struct dataTransferRect{
    /// Socket
    int fd;
    /// Region to read
    const size_t *region;
    /// Size of a row
    size_t row;
    /// Size of a 2D slice (row*column)
    size_t slice;
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
    unsigned int i, j, k, n;
    struct dataTransferRect* _data = (struct dataTransferRect*)data;
    size_t buffsize = BUFF_SIZE*sizeof(char);
    int *fd = &(_data->fd);
    Recv(fd, &buffsize, sizeof(size_t), MSG_WAITALL);
    // Compute the number of packages needed per row
    n = _data->row / buffsize;
    // Receive the rows
    size_t origin = 0;
    for(j=0;j<_data->region[1];j++){
        for(k=0;k<_data->region[2];k++){
            // Receive package by pieces
            for(i=0;i<n;i++){
                Recv(fd, _data->ptr + i*buffsize + origin, buffsize, MSG_WAITALL);
            }
            if(_data->row % buffsize){
                // Remains some data to arrive
                Recv(fd, _data->ptr + n*buffsize + origin, _data->row % buffsize, MSG_WAITALL);
            }
            // Compute the new origin
            origin += _data->row;
        }
    }
    free(_data); _data=NULL;
    pthread_exit(NULL);
    return NULL;
}

/** Performs a data reception asynchronously on a new thread and socket, for
 * a clEnqueueReadBufferRect specific command.
 * @param sockfd Connection socket.
 * @param data Data to transfer.
 */
void asyncDataRecvRect(int* sockfd, struct dataTransferRect data)
{
    // -------------------------------------
    // Get server port and connect to it.
    // -------------------------------------
    unsigned int port;
    data.fd = -1;
    struct sockaddr_in serv_addr;
    Recv(sockfd, &port, sizeof(unsigned int), MSG_WAITALL);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        // we can't work, disconnect from server
        shutdown(*sockfd, 2);
        *sockfd = -1;
        return;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    socklen_t len_inet;
    len_inet = sizeof(serv_addr);
    getsockname(*sockfd, (struct sockaddr*)&serv_addr, &len_inet);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if( connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        // we can't work, disconnect from server
        shutdown(fd, 2);
        shutdown(*sockfd, 2);
        *sockfd = -1;
        return;
    }
    data.fd = fd;
    // -------------------------------------
    // Receive the data on another thread.
    // -------------------------------------
    pthread_t thread;
    struct dataTransferRect* _data = (struct dataTransferRect*)malloc(sizeof(struct dataTransferRect));
    _data->region = data.region;
    _data->row    = data.row;
    _data->slice  = data.slice;
    _data->fd     = data.fd;
    _data->ptr    = data.ptr;
    int rc = pthread_create(&thread, NULL, asyncDataRecvRect_thread, (void *)(_data));
    if(rc){
        // we can't work, disconnect the client
        shutdown(data.fd, 2);
        shutdown(*sockfd, 2);
        *sockfd = -1;
        return;
    }
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
                    Recv(sockfd, ptr + i*buffsize + origin, buffsize, MSG_WAITALL);
                }
                if(host_row_pitch % buffsize){
                    // Remains some data to transfer
                    Recv(sockfd, ptr + n*buffsize + origin, host_row_pitch % buffsize, MSG_WAITALL);
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
#endif

#ifdef CL_API_SUFFIX__VERSION_1_2
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
                         void *                  host_ptr,
                         cl_int *                errcode_ret)
{
    char buffer[BUFF_SIZE];
    cl_mem clMem;
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
    unsigned int commDim = strlen("clCreateImage")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clCreateImage");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &context, sizeof(cl_context), 0);
    Send(sockfd, &flags, sizeof(cl_mem_flags), 0);
    Send(sockfd, image_format, sizeof(cl_image_format), 0);
    Send(sockfd, image_desc, sizeof(cl_image_desc), 0);
    if(flags & CL_MEM_COPY_HOST_PTR){
        // Really large data, take care here
        // Receive first the buffer purposed by server
        size_t buffsize;
        Recv(sockfd, &buffsize, sizeof(size_t), MSG_WAITALL);
        if(!buffsize){
            if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        // Compute the image size
        size_t size = 0;
        if( (image_desc->image_type & CL_MEM_OBJECT_IMAGE1D) ||
            (image_desc->image_type & CL_MEM_OBJECT_IMAGE1D_BUFFER) ){
            size = image_desc->image_row_pitch;
        }
        else if(image_desc->image_type & CL_MEM_OBJECT_IMAGE2D){
            size = image_desc->image_row_pitch * image_desc->image_height;
        }
        else if(image_desc->image_type & CL_MEM_OBJECT_IMAGE3D){
            size = image_desc->image_slice_pitch * image_desc->image_depth;
        }
        else if( (image_desc->image_type & CL_MEM_OBJECT_IMAGE1D_ARRAY) ||
                 (image_desc->image_type & CL_MEM_OBJECT_IMAGE2D_ARRAY) ){
            size = image_desc->image_slice_pitch * image_desc->image_array_size;
        }
        // Compute the number of packages needed
        unsigned int i,n;
        n = size / buffsize;
        // Send package by pieces
        for(i=0;i<n;i++){
            Send(sockfd, host_ptr + i*buffsize, buffsize, 0);
        }
        if(size % buffsize){
            // Remains some data to transfer
            Send(sockfd, host_ptr + n*buffsize, size % buffsize, 0);
        }
    }
    // And request flag and result
    cl_int flag = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    Recv(sockfd, &clMem, sizeof(cl_mem), MSG_WAITALL);
    if(errcode_ret) *errcode_ret = flag;
    // Register the buffer
    addShortcut((void*)clMem, sockfd);
    return clMem;
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

cl_int oclandGetKernelArgInfo(cl_kernel        kernel ,
                              cl_uint          arg_index ,
                              cl_kernel_arg_info   param_name ,
                              size_t           param_value_size ,
                              void *           param_value ,
                              size_t *         param_value_size_ret)
{
    char buffer[BUFF_SIZE];
    // Look for a shortcut
    int *sockfd = getShortcut(kernel);
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Execute the command on server
    unsigned int commDim = strlen("clGetKernelArgInfo")+1;
    Send(sockfd, &commDim, sizeof(unsigned int), 0);
    // Send command to perform
    strcpy(buffer, "clGetKernelArgInfo");
    Send(sockfd, buffer, strlen(buffer)+1, 0);
    // Send parameters
    Send(sockfd, &kernel, sizeof(cl_kernel), 0);
    Send(sockfd, &arg_index, sizeof(cl_uint), 0);
    Send(sockfd, &param_name, sizeof(cl_kernel_arg_info), 0);
    Send(sockfd, &param_value_size, sizeof(size_t), 0);
    // And request flag and real size of object
    cl_int flag = CL_INVALID_KERNEL;
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    size_t size_ret;
    Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(param_value_size_ret)
        *param_value_size_ret = size_ret;
    if(flag != CL_SUCCESS)
        return flag;
    // Server will return param_value_size bytes
    // even though size_ret can be lower than this,
    // user may know how many bytes are unset using
    // param_value_size_ret returned value.
    if(param_value_size){
        Recv(sockfd, param_value, param_value_size, MSG_WAITALL);
    }
    // A little bit special case when data transfer could failed
    if(*sockfd < 0)
        return flag;
    return CL_SUCCESS;
}
#endif

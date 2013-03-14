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

#include <ocland/common/dataExchange.h>
#include <ocland/server/ocland_mem.h>

#ifndef OCLAND_ASYNC_FIRST_PORT
    #define OCLAND_ASYNC_FIRST_PORT 51001u
#endif

#ifndef OCLAND_ASYNC_LAST_PORT
    #define OCLAND_ASYNC_LAST_PORT 51150u
#endif

#ifndef BUFF_SIZE
    #define BUFF_SIZE 1025u
#endif

/** Create a port for a parallel data transfer.
 * @param async_port Returned resulting port. Can be NULL, then
 * port data will not be returned.
 * @return Server identifier, lower than 0 if couldn't be created.
 */
int openPort(unsigned int *async_port)
{
    unsigned int port = OCLAND_ASYNC_FIRST_PORT;
    int serverfd = -1;
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverfd < 0){
        // we can't work, disconnect the client
        printf("ERROR: New socket can't be registered for asynchronous data transfer.\n"); fflush(stdout);
        return serverfd;
    }
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(port);
    while(bind(serverfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
        port++;
        serv_addr.sin_port    = htons(port);
        if(port > OCLAND_ASYNC_LAST_PORT){
            printf("ERROR: Can't find an available port for asynchronous data transfer.\n"); fflush(stdout);
            shutdown(serverfd, 2);
            serverfd = -1;
            return serverfd;
        }
    }
    if(async_port)
        *async_port = port;
    if(listen(serverfd, 1)){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on port %u binded.\n", port); fflush(stdout);
        shutdown(serverfd, 2);
        serverfd = -1;
        return serverfd;
    }
    return serverfd;
}


/** @struct dataTransfer Data needed for
 * an asynchronously transfer to client.
 */
struct dataSend{
    /// Socket
    int fd;
    /// Command queue
    cl_command_queue command_queue;
    /// Memory object
    cl_mem mem;
    /// Memory buffer origin (for clEnqueueReadBuffer)
    size_t offset;
    /// Size of data (for clEnqueueReadBuffer)
    size_t cb;
    /// Memory buffer origin (for clEnqueueReadBuffer)
    size_t *buffer_origin;
    /// Region to read (for clEnqueueReadBufferRect)
    size_t *region;
    /// Size of a buffer row (for clEnqueueReadBufferRect)
    size_t  buffer_row_pitch;
    /// Size of a buffer 2D slice (for clEnqueueReadBufferRect)
    size_t  buffer_slice_pitch;
    /// Size of a host memory row (for clEnqueueReadBufferRect)
    size_t  host_row_pitch;
    /// Size of a host memory 2D slice (for clEnqueueReadBufferRect)
    size_t  host_slice_pitch;
    /// Data array
    void *ptr;
    /// Number of events to wait
    cl_uint num_events_in_wait_list;
    /// List of events to wait
    ocland_event *event_wait_list;
    /// CL_TRUE if the event must be preserved, CL_FALSE otherwise
    cl_bool want_event;
    /// Event associated to the transmission (can be NULL)
    ocland_event event;
};

/** Test if all the objects exist on the same command queue.
 * @return CL_SUCCESS if all the objects are associated
 * with the command queue. CL_INVALID_CONTEXT if the objects
 * are associated to different command queues. Other errors
 * can be returned if some objects are invalid.
 */
cl_int testCommandQueue(cl_command_queue     command_queue ,
                        cl_mem               mem ,
                        cl_uint              num_events_in_wait_list ,
                        const ocland_event * event_wait_list)
{
    unsigned int i;
    cl_int flag;
    cl_context context, aux_context;
    flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS)
        return flag;
    flag = clGetMemObjectInfo(mem, CL_MEM_CONTEXT, sizeof(cl_context), &aux_context, NULL);
    if(flag != CL_SUCCESS)
        return flag;
    if(context != aux_context)
        return CL_INVALID_CONTEXT;
    for(i=0;i<num_events_in_wait_list;i++){
        if(event_wait_list[i]->context != context)
            return CL_INVALID_CONTEXT;
        if(event_wait_list[i]->command_queue){  // Can be NULL
            if(event_wait_list[i]->command_queue != command_queue)
                return CL_INVALID_CONTEXT;
        }
    }
    return CL_SUCCESS;
}

/** Test if the memory object has enought memory.
 * @return CL_SUCCESS if memory object has enought
 * memory allocated, CL_INVALID_VALUE if size
 * provided is out of bounds, or error if
 * unavailable data.
 */
cl_int testSize(cl_mem mem ,
                size_t cb)
{
    cl_int flag;
    size_t mem_size;
    flag = clGetMemObjectInfo(mem, CL_MEM_SIZE, sizeof(size_t), &mem_size, NULL);
    if(flag != CL_SUCCESS)
        return flag;
    if(mem_size < cb)
        return CL_INVALID_VALUE;
    return CL_SUCCESS;
}

/** Test if the memory object is readable.
 * @return CL_SUCCESS if memory object can
 * be accessed, CL_INVALID_OPERATION if is
 * not a readable object, or error if
 * unavailable data.
 */
cl_int testReadable(cl_mem mem)
{
    cl_int flag;
    cl_mem_flags flags;
    flag = clGetMemObjectInfo(mem, CL_MEM_FLAGS, sizeof(cl_mem_flags), &flags, NULL);
    if(flag != CL_SUCCESS)
        return flag;
    if(    (flags & CL_MEM_HOST_WRITE_ONLY)
        || (flags & CL_MEM_HOST_NO_ACCESS))
        return CL_INVALID_OPERATION;
    return CL_SUCCESS;
}

/** Test if the memory object is writable.
 * @return CL_SUCCESS if memory object can
 * be written, CL_INVALID_OPERATION if is
 * not accessable object, or error if
 * unavailable data.
 */
cl_int testWriteable(cl_mem mem)
{
    cl_int flag;
    cl_mem_flags flags;
    flag = clGetMemObjectInfo(mem, CL_MEM_FLAGS, sizeof(cl_mem_flags), &flags, NULL);
    if(flag != CL_SUCCESS)
        return flag;
    if(    (flags & CL_MEM_HOST_READ_ONLY)
        || (flags & CL_MEM_HOST_NO_ACCESS))
        return CL_INVALID_OPERATION;
    return CL_SUCCESS;
}

/** Thread that sends data from server to client.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataSend_thread(void *data)
{
    unsigned int i,n;
    size_t buffsize = BUFF_SIZE*sizeof(char);
    struct dataSend* _data = (struct dataSend*)data;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors,
    // assuming than events can be wrong, but is to late to
    // try to report a fail.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Call to OpenCL
    clEnqueueReadBuffer(_data->command_queue,_data->mem,CL_FALSE,
                        _data->offset,_data->cb,_data->ptr,
                        0,NULL,&(_data->event->event));
    // Start sending data to client
    int *fd = &(_data->fd);
    Send(fd, &buffsize, sizeof(size_t), 0);
    // Compute the number of packages needed
    n = _data->cb / buffsize;
    // Wait until data is copied here. We will not test
    // for errors, user can do it later
    clWaitForEvents(1,&(_data->event->event));
    // Send package by pieces
    for(i=0;i<n;i++){
        Send(fd, _data->ptr + i*buffsize, buffsize, 0);
    }
    if(_data->cb % buffsize){
        // Remains some data to arrive
        Send(fd, _data->ptr + n*buffsize, _data->cb % buffsize, 0);
    }
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    free(_data); _data=NULL;
    pthread_exit(NULL);
    return NULL;
}

cl_int oclandEnqueueReadBuffer(int *                clientfd ,
                               cl_command_queue     command_queue ,
                               cl_mem               mem ,
                               size_t               offset ,
                               size_t               cb ,
                               void *               ptr ,
                               cl_uint              num_events_in_wait_list ,
                               ocland_event *       event_wait_list ,
                               cl_bool              want_event ,
                               ocland_event         event)
{
    cl_int flag;
    // Test that the objects command queue matchs
    if(testCommandQueue(command_queue,mem,num_events_in_wait_list,event_wait_list) != CL_SUCCESS)
        return CL_INVALID_CONTEXT;
    // Test if the size is not out of bounds
    if(testSize(mem, offset+cb) != CL_SUCCESS)
        return CL_INVALID_VALUE;
    // Test if the memory can be accessed
    if(testReadable(mem) != CL_SUCCESS)
        return CL_INVALID_OPERATION;
    // Seems that data is correct, so we can proceed.
    // We need to create a new connection socket in a
    // new port in order to don't interfiere the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0){
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    // Here in after we assume that the works gone fine,
    // returning CL_SUCCESS. We need to do it in order to
    // avoid sending the port before the flag.
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), 0);
    if(want_event == CL_TRUE){
        Send(clientfd, &event, sizeof(ocland_event), 0);
    }
    // We have a new connection socket ready, reports it to
    // the client and wait until he connects with us.
    Send(clientfd, &port, sizeof(unsigned int), 0);
    int fd = accept(serverfd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on port %u binded.\n", port); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_SUCCESS;
    }
    // Now we have a new socket connected to client, so
    // hereinafter we rely the work to a new thread, that
    // will call to clEnqueueReadBuffer and will send the
    // data to the client.
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = fd;
    _data->command_queue           = command_queue;
    _data->mem                     = mem;
    _data->offset                  = offset;
    _data->cb                      = cb;
    _data->ptr                     = ptr;
    _data->num_events_in_wait_list = num_events_in_wait_list;
    _data->event_wait_list         = event_wait_list;
    _data->want_event              = want_event;
    _data->event                   = event;
    int rc = pthread_create(&thread, NULL, asyncDataSend_thread, (void *)(_data));
    if(rc){
        // we can't work, disconnect the client
        printf("ERROR: Thread creation has failed with the return code %d\n", rc); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
    }
    return CL_SUCCESS;
}

/** Thread that receives data from client.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataRecv_thread(void *data)
{
    unsigned int i,n;
    size_t buffsize = BUFF_SIZE*sizeof(char);
    struct dataSend* _data = (struct dataSend*)data;
    // Receive the data
    int *fd = &(_data->fd);
    Send(fd, &buffsize, sizeof(size_t), 0);
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
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocland event
    // can be relevant. We will not check for errors,
    // assuming than events can be wrong, but is to late to
    // try to report a fail.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Call to OpenCL
    clEnqueueWriteBuffer(_data->command_queue,_data->mem,CL_FALSE,
                        _data->offset,_data->cb,_data->ptr,
                        0,NULL,&(_data->event->event));
    // Wait until data is copied here. We will not test
    // for errors, user can do it later
    clWaitForEvents(1,&(_data->event->event));
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    free(_data); _data=NULL;
    pthread_exit(NULL);
    return NULL;
}

cl_int oclandEnqueueWriteBuffer(int *                clientfd ,
                                cl_command_queue     command_queue ,
                                cl_mem               mem ,
                                size_t               offset ,
                                size_t               cb ,
                                void *               ptr ,
                                cl_uint              num_events_in_wait_list ,
                                ocland_event *       event_wait_list ,
                                cl_bool              want_event ,
                                ocland_event         event)
{
    // Test that the objects command queue matchs
    if(testCommandQueue(command_queue,mem,num_events_in_wait_list,event_wait_list) != CL_SUCCESS)
        return CL_INVALID_CONTEXT;
    // Test if the size is not out of bounds
    if(testSize(mem, offset+cb) != CL_SUCCESS)
        return CL_INVALID_VALUE;
    // Test if the memory can be accessed
    if(testWriteable(mem) != CL_SUCCESS)
        return CL_INVALID_OPERATION;
    // Seems that data is correct, so we can proceed.
    // We need to create a new connection socket in a
    // new port in order to don't interfiere the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0){
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    // We have a new connection socket ready, reports it to
    // the client and wait until he connects with us.
    Send(clientfd, &port, sizeof(unsigned int), 0);
    int fd = accept(serverfd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on port %u binded.\n", port); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    // Now we have a new socket connected to client, so
    // hereinafter we rely the work to a new thread, that
    // will call to clEnqueueWriteBuffer and will send the
    // data to the client.
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = fd;
    _data->command_queue           = command_queue;
    _data->mem                     = mem;
    _data->offset                  = offset;
    _data->cb                      = cb;
    _data->ptr                     = ptr;
    _data->num_events_in_wait_list = num_events_in_wait_list;
    _data->event_wait_list         = event_wait_list;
    _data->want_event              = want_event;
    _data->event                   = event;
    int rc = pthread_create(&thread, NULL, asyncDataRecv_thread, (void *)(_data));
    if(rc){
        // we can't work, disconnect the client
        printf("ERROR: Thread creation has failed with the return code %d\n", rc); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    return CL_SUCCESS;
}

/** Thread that sends image from server to client.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataSendImage_thread(void *data)
{
    unsigned int i,j,k,n;
    size_t buffsize = BUFF_SIZE*sizeof(char);
    struct dataSend* _data = (struct dataSend*)data;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocland event
    // can be relevant. We will not check for errors,
    // assuming than events can be wrong, but is to late to
    // try to report a fail.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Call to OpenCL
    clEnqueueReadImage(_data->command_queue,_data->mem,CL_FALSE,
                       _data->buffer_origin,_data->region,
                       _data->buffer_row_pitch,_data->buffer_slice_pitch,
                       _data->ptr,0,NULL,&(_data->event->event));
    // Start sending data to client
    int *fd = &(_data->fd);
    Send(fd, &buffsize, sizeof(size_t), 0);
    // Compute the number of packages needed
    n = _data->host_row_pitch / buffsize;
    // Wait until data is copied here. We will not test
    // for errors, user can do it later
    clWaitForEvents(1,&(_data->event->event));
    // Send the rows
    size_t origin = 0;
    for(j=0;j<_data->region[1];j++){
        for(k=0;k<_data->region[2];k++){
            // Send package by pieces
            for(i=0;i<n;i++){
                Send(fd, _data->ptr + i*buffsize + origin, buffsize, 0);
            }
            if(_data->host_row_pitch % buffsize){
                // Remains some data to arrive
                Send(fd, _data->ptr + n*buffsize + origin, _data->host_row_pitch % buffsize, 0);
            }
            // Compute the new origin
            origin += _data->host_row_pitch;
        }
    }
    free(_data->buffer_origin); _data->buffer_origin = NULL;
    free(_data->region); _data->region = NULL;
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    free(_data); _data=NULL;
    pthread_exit(NULL);
    return NULL;
}

cl_int oclandEnqueueReadImage(int *                clientfd ,
                              cl_command_queue     command_queue ,
                              cl_mem               image ,
                              const size_t *       origin ,
                              const size_t *       region ,
                              size_t               row_pitch ,
                              size_t               slice_pitch ,
                              void *               ptr ,
                              cl_uint              num_events_in_wait_list ,
                              ocland_event *       event_wait_list ,
                              cl_bool              want_event ,
                              ocland_event         event)
{
    cl_int flag;
    // Test that the objects command queue matchs
    if(testCommandQueue(command_queue,image,num_events_in_wait_list,event_wait_list) != CL_SUCCESS)
        return flag;
    // Test if the size is not out of bounds
    size_t cb =   origin[0]
                + origin[1]*row_pitch
                + origin[2]*slice_pitch
                + region[0]
                + region[1]*row_pitch
                + region[2]*slice_pitch;
    if(testSize(image, cb) != CL_SUCCESS)
        return flag;
    // Test if the memory can be accessed
    if(testReadable(image) != CL_SUCCESS)
        return flag;
    // Seems that data is correct, so we can proceed.
    // We need to create a new connection socket in a
    // new port in order to don't interfiere the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0){
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    // Here in after we assume that the works gone fine,
    // returning CL_SUCCESS. We need to do it to avoid
    // that port is send before flag.
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), 0);
    if(want_event == CL_TRUE){
        Send(clientfd, &event, sizeof(ocland_event), 0);
    }
    // We a new connection socket ready, reports it to
    // the client and wait until he connects with us.
    Send(clientfd, &port, sizeof(unsigned int), 0);
    int fd = accept(serverfd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on port %u binded.\n", port); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_SUCCESS;
    }
    // Now we have a new socket connected to client, so
    // hereinafter we rely the work to a new thread, that
    // will call to clEnqueueReadBuffer and will send the
    // data to the client.
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = fd;
    _data->command_queue           = command_queue;
    _data->mem                     = image;
    _data->buffer_origin           = (size_t*)malloc(3*sizeof(size_t));
    _data->buffer_origin[0]        = origin[0];
    _data->buffer_origin[1]        = origin[1];
    _data->buffer_origin[2]        = origin[2];
    _data->region                  = (size_t*)malloc(3*sizeof(size_t));
    _data->region[0]               = region[0];
    _data->region[1]               = region[1];
    _data->region[2]               = region[2];
    _data->buffer_row_pitch        = row_pitch;
    _data->buffer_slice_pitch      = slice_pitch;
    _data->host_row_pitch          = row_pitch;
    _data->host_slice_pitch        = slice_pitch;
    _data->ptr                     = ptr;
    _data->num_events_in_wait_list = num_events_in_wait_list;
    _data->event_wait_list         = event_wait_list;
    _data->want_event              = want_event;
    _data->event                   = event;
    int rc = pthread_create(&thread, NULL, asyncDataSendImage_thread, (void *)(_data));
    if(rc){
        // we can't work, disconnect the client
        printf("ERROR: Thread creation has failed with the return code %d\n", rc); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
    }
    return CL_SUCCESS;
}

/** Thread that receives image from client.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataRecvImage_thread(void *data)
{
    unsigned int i,j,k,n;
    size_t buffsize = BUFF_SIZE*sizeof(char);
    struct dataSend* _data = (struct dataSend*)data;
    // Receive the data
    int *fd = &(_data->fd);
    Send(fd, &buffsize, sizeof(size_t), 0);
    // Compute the number of packages needed
    n = _data->host_row_pitch / buffsize;
    // Receive the rows
    size_t origin = 0;
    for(j=0;j<_data->region[1];j++){
        for(k=0;k<_data->region[2];k++){
            // Receive package by pieces
            for(i=0;i<n;i++){
                Recv(fd, _data->ptr + i*buffsize + origin, buffsize, MSG_WAITALL);
            }
            if(_data->host_row_pitch % buffsize){
                // Remains some data to arrive
                Recv(fd, _data->ptr + n*buffsize + origin, _data->host_row_pitch % buffsize, MSG_WAITALL);
            }
            // Compute the new origin
            origin += _data->host_row_pitch;
        }
    }
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocland event
    // can be relevant. We will not check for errors,
    // assuming than events can be wrong, but is to late to
    // try to report a fail.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Call to OpenCL
    clEnqueueWriteImage(_data->command_queue,_data->mem,CL_FALSE,
                        _data->buffer_origin,_data->region,
                        _data->buffer_row_pitch,_data->buffer_slice_pitch,
                        _data->ptr,0,NULL,&(_data->event->event));
    // Wait until data is copied here. We will not test
    // for errors, user can do it later
    clWaitForEvents(1,&(_data->event->event));
    free(_data->buffer_origin); _data->buffer_origin = NULL;
    free(_data->region); _data->region = NULL;
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    free(_data); _data=NULL;
    pthread_exit(NULL);
    return NULL;
}

cl_int oclandEnqueueWriteImage(int *                clientfd ,
                               cl_command_queue     command_queue ,
                               cl_mem               image ,
                               const size_t *       origin ,
                               const size_t *       region ,
                               size_t               row_pitch ,
                               size_t               slice_pitch ,
                               void *               ptr ,
                               cl_uint              num_events_in_wait_list ,
                               ocland_event *       event_wait_list ,
                               cl_bool              want_event ,
                               ocland_event         event)
{
    // Test that the objects command queue matchs
    if(testCommandQueue(command_queue,image,num_events_in_wait_list,event_wait_list) != CL_SUCCESS)
        return CL_INVALID_CONTEXT;
    // Test if the size is not out of bounds
    size_t cb =   origin[0]
                + origin[1]*row_pitch
                + origin[2]*slice_pitch
                + region[0]
                + region[1]*row_pitch
                + region[2]*slice_pitch;
    if(testSize(image, cb) != CL_SUCCESS)
        return CL_INVALID_VALUE;
    // Test if the memory can be accessed
    if(testWriteable(image) != CL_SUCCESS)
        return CL_INVALID_OPERATION;
    // Seems that data is correct, so we can proceed.
    // We need to create a new connection socket in a
    // new port in order to don't interfiere the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0){
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    // We have a new connection socket ready, reports it to
    // the client and wait until he connects with us.
    Send(clientfd, &port, sizeof(unsigned int), 0);
    int fd = accept(serverfd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on port %u binded.\n", port); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    // Now we have a new socket connected to client, so
    // hereinafter we rely the work to a new thread, that
    // will call to clEnqueueReadBuffer and will send the
    // data to the client.
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = fd;
    _data->command_queue           = command_queue;
    _data->mem                     = image;
    _data->buffer_origin           = (size_t*)malloc(3*sizeof(size_t));
    _data->buffer_origin[0]        = origin[0];
    _data->buffer_origin[1]        = origin[1];
    _data->buffer_origin[2]        = origin[2];
    _data->region                  = (size_t*)malloc(3*sizeof(size_t));
    _data->region[0]               = region[0];
    _data->region[1]               = region[1];
    _data->region[2]               = region[2];
    _data->buffer_row_pitch        = row_pitch;
    _data->buffer_slice_pitch      = slice_pitch;
    _data->host_row_pitch          = row_pitch;
    _data->host_slice_pitch        = slice_pitch;
    _data->ptr                     = ptr;
    _data->num_events_in_wait_list = num_events_in_wait_list;
    _data->event_wait_list         = event_wait_list;
    _data->want_event              = want_event;
    _data->event                   = event;
    int rc = pthread_create(&thread, NULL, asyncDataRecvImage_thread, (void *)(_data));
    if(rc){
        // we can't work, disconnect the client
        printf("ERROR: Thread creation has failed with the return code %d\n", rc); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    return CL_SUCCESS;
}

#ifdef CL_API_SUFFIX__VERSION_1_1

/** Thread that sends data from server to client in
 * 2D or 3D mode.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataSendRect_thread(void *data)
{
    unsigned int i,j,k,n;
    size_t buffsize = BUFF_SIZE*sizeof(char);
    struct dataSend* _data = (struct dataSend*)data;
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocalnd event
    // can be relevant. We will not check for errors,
    // assuming than events can be wrong, but is to late to
    // try to report a fail.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Call to OpenCL
    size_t host_origin[3] = {0, 0, 0};
    clEnqueueReadBufferRect(_data->command_queue,_data->mem,CL_FALSE,
                            _data->buffer_origin,host_origin,_data->region,
                            _data->buffer_row_pitch,_data->buffer_slice_pitch,
                            _data->host_row_pitch,_data->host_slice_pitch,
                            _data->ptr,0,NULL,&(_data->event->event));
    // Start sending data to client
    int *fd = &(_data->fd);
    Send(fd, &buffsize, sizeof(size_t), 0);
    // Compute the number of packages needed
    n = _data->host_row_pitch / buffsize;
    // Wait until data is copied here. We will not test
    // for errors, user can do it later
    clWaitForEvents(1,&(_data->event->event));
    // Send the rows
    size_t origin = 0;
    for(j=0;j<_data->region[1];j++){
        for(k=0;k<_data->region[2];k++){
            // Send package by pieces
            for(i=0;i<n;i++){
                Send(fd, _data->ptr + i*buffsize + origin, buffsize, 0);
            }
            if(_data->host_row_pitch % buffsize){
                // Remains some data to arrive
                Send(fd, _data->ptr + n*buffsize + origin, _data->host_row_pitch % buffsize, 0);
            }
            // Compute the new origin
            origin += _data->host_row_pitch;
        }
    }
    free(_data->buffer_origin); _data->buffer_origin = NULL;
    free(_data->region); _data->region = NULL;
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    free(_data); _data=NULL;
    pthread_exit(NULL);
    return NULL;
}

cl_int oclandEnqueueReadBufferRect(int *                clientfd ,
                                   cl_command_queue     command_queue ,
                                   cl_mem               mem ,
                                   const size_t *       buffer_origin ,
                                   const size_t *       region ,
                                   size_t               buffer_row_pitch ,
                                   size_t               buffer_slice_pitch ,
                                   size_t               host_row_pitch ,
                                   size_t               host_slice_pitch ,
                                   void *               ptr ,
                                   cl_uint              num_events_in_wait_list ,
                                   ocland_event *       event_wait_list ,
                                   cl_bool              want_event ,
                                   ocland_event         event)
{
    cl_int flag;
    // Test that the objects command queue matchs
    if(testCommandQueue(command_queue,mem,num_events_in_wait_list,event_wait_list) != CL_SUCCESS)
        return flag;
    // Test if the size is not out of bounds
    size_t cb =   buffer_origin[0]
                + buffer_origin[1]*buffer_row_pitch
                + buffer_origin[2]*buffer_slice_pitch
                + region[0]
                + region[1]*buffer_row_pitch
                + region[2]*buffer_slice_pitch;
    if(testSize(mem, cb) != CL_SUCCESS)
        return flag;
    // Test if the memory can be accessed
    if(testReadable(mem) != CL_SUCCESS)
        return flag;
    // Seems that data is correct, so we can proceed.
    // We need to create a new connection socket in a
    // new port in order to don't interfiere the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0){
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    // Here in after we assume that the works gone fine,
    // returning CL_SUCCESS. We need to do it in order to
    // avoid sending the port before the flag.
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), 0);
    if(want_event == CL_TRUE){
        Send(clientfd, &event, sizeof(ocland_event), 0);
    }
    // We have a new connection socket ready, reports it to
    // the client and wait until he connects with us.
    Send(clientfd, &port, sizeof(unsigned int), 0);
    int fd = accept(serverfd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on port %u binded.\n", port); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_SUCCESS;
    }
    // Now we have a new socket connected to client, so
    // hereinafter we rely the work to a new thread, that
    // will call to clEnqueueReadBuffer and will send the
    // data to the client.
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = fd;
    _data->command_queue           = command_queue;
    _data->mem                     = mem;
    _data->buffer_origin           = (size_t*)malloc(3*sizeof(size_t));
    _data->buffer_origin[0]        = buffer_origin[0];
    _data->buffer_origin[1]        = buffer_origin[1];
    _data->buffer_origin[2]        = buffer_origin[2];
    _data->region                  = (size_t*)malloc(3*sizeof(size_t));
    _data->region[0]               = region[0];
    _data->region[1]               = region[1];
    _data->region[2]               = region[2];
    _data->buffer_row_pitch        = buffer_row_pitch;
    _data->buffer_slice_pitch      = buffer_slice_pitch;
    _data->host_row_pitch          = host_row_pitch;
    _data->host_slice_pitch        = host_slice_pitch;
    _data->ptr                     = ptr;
    _data->num_events_in_wait_list = num_events_in_wait_list;
    _data->event_wait_list         = event_wait_list;
    _data->want_event              = want_event;
    _data->event                   = event;
    int rc = pthread_create(&thread, NULL, asyncDataSendRect_thread, (void *)(_data));
    if(rc){
        // we can't work, disconnect the client
        printf("ERROR: Thread creation has failed with the return code %d\n", rc); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
    }
    return CL_SUCCESS;
}

/** Thread that receives data from client.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataRecvRect_thread(void *data)
{
    unsigned int i,j,k,n;
    size_t buffsize = BUFF_SIZE*sizeof(char);
    struct dataSend* _data = (struct dataSend*)data;
    size_t host_origin[3] = {0, 0, 0};
    // Receive the data
    int *fd = &(_data->fd);
    Send(fd, &buffsize, sizeof(size_t), 0);
    // Compute the number of packages needed
    n = _data->host_row_pitch / buffsize;
    // Receive the rows
    size_t origin = 0;
    for(j=0;j<_data->region[1];j++){
        for(k=0;k<_data->region[2];k++){
            // Receive package by pieces
            for(i=0;i<n;i++){
                Recv(fd, _data->ptr + i*buffsize + origin, buffsize, MSG_WAITALL);
            }
            if(_data->host_row_pitch % buffsize){
                // Remains some data to arrive
                Recv(fd, _data->ptr + n*buffsize + origin, _data->host_row_pitch % buffsize, MSG_WAITALL);
            }
            // Compute the new origin
            origin += _data->host_row_pitch;
        }
    }
    // We may wait manually for the events provided because
    // OpenCL can only waits their events, but ocland event
    // can be relevant. We will not check for errors,
    // assuming than events can be wrong, but is to late to
    // try to report a fail.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Call to OpenCL
    clEnqueueWriteBufferRect(_data->command_queue,_data->mem,CL_FALSE,
                             _data->buffer_origin,host_origin,_data->region,
                             _data->buffer_row_pitch,_data->buffer_slice_pitch,
                             _data->host_row_pitch,_data->host_slice_pitch,
                             _data->ptr,0,NULL,&(_data->event->event));
    // Wait until data is copied here. We will not test
    // for errors, user can do it later
    clWaitForEvents(1,&(_data->event->event));
    free(_data->buffer_origin); _data->buffer_origin = NULL;
    free(_data->region); _data->region = NULL;
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    free(_data); _data=NULL;
    pthread_exit(NULL);
    return NULL;
}

cl_int oclandEnqueueWriteBufferRect(int *                clientfd ,
                                    cl_command_queue     command_queue ,
                                    cl_mem               mem ,
                                    const size_t *       buffer_origin ,
                                    const size_t *       region ,
                                    size_t               buffer_row_pitch ,
                                    size_t               buffer_slice_pitch ,
                                    size_t               host_row_pitch ,
                                    size_t               host_slice_pitch ,
                                    void *               ptr ,
                                    cl_uint              num_events_in_wait_list ,
                                    ocland_event *       event_wait_list ,
                                    cl_bool              want_event ,
                                    ocland_event         event)
{
    // Test that the objects command queue matchs
    if(testCommandQueue(command_queue,mem,num_events_in_wait_list,event_wait_list) != CL_SUCCESS)
        return CL_INVALID_CONTEXT;
    // Test if the size is not out of bounds
    size_t cb =   buffer_origin[0]
                + buffer_origin[1]*buffer_row_pitch
                + buffer_origin[2]*buffer_slice_pitch
                + region[0]
                + region[1]*buffer_row_pitch
                + region[2]*buffer_slice_pitch;
    if(testSize(mem, cb) != CL_SUCCESS)
        return CL_INVALID_VALUE;
    // Test if the memory can be accessed
    if(testWriteable(mem) != CL_SUCCESS)
        return CL_INVALID_OPERATION;
    // Seems that data is correct, so we can proceed.
    // We need to create a new connection socket in a
    // new port in order to don't interfiere the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0){
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    // We have a new connection socket ready, reports it to
    // the client and wait until he connects with us.
    Send(clientfd, &port, sizeof(unsigned int), 0);
    int fd = accept(serverfd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on port %u binded.\n", port); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    // Now we have a new socket connected to client, so
    // hereinafter we rely the work to a new thread, that
    // will call to clEnqueueReadBuffer and will send the
    // data to the client.
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = fd;
    _data->command_queue           = command_queue;
    _data->mem                     = mem;
    _data->buffer_origin           = (size_t*)malloc(3*sizeof(size_t));
    _data->buffer_origin[0]        = buffer_origin[0];
    _data->buffer_origin[1]        = buffer_origin[1];
    _data->buffer_origin[2]        = buffer_origin[2];
    _data->region                  = (size_t*)malloc(3*sizeof(size_t));
    _data->region[0]               = region[0];
    _data->region[1]               = region[1];
    _data->region[2]               = region[2];
    _data->buffer_row_pitch        = buffer_row_pitch;
    _data->buffer_slice_pitch      = buffer_slice_pitch;
    _data->host_row_pitch          = host_row_pitch;
    _data->host_slice_pitch        = host_slice_pitch;
    _data->ptr                     = ptr;
    _data->num_events_in_wait_list = num_events_in_wait_list;
    _data->event_wait_list         = event_wait_list;
    _data->want_event              = want_event;
    _data->event                   = event;
    int rc = pthread_create(&thread, NULL, asyncDataRecvRect_thread, (void *)(_data));
    if(rc){
        // we can't work, disconnect the client
        printf("ERROR: Thread creation has failed with the return code %d\n", rc); fflush(stdout);
        shutdown(serverfd, 2);
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return CL_OUT_OF_HOST_MEMORY;
    }
    return CL_SUCCESS;
}
#endif // CL_API_SUFFIX__VERSION_1_1

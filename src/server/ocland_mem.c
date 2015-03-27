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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <pthread.h>

#include <ocland/common/sockets.h>
#include <ocland/common/usleep.h>
#include <ocland/common/dataExchange.h>
#include <ocland/common/dataPack.h>
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

#define THREAD_SAFE_EXIT {if(fd>0) close(fd); if(_data->fd>0) close(_data->fd); free(_data); _data=NULL; pthread_exit(NULL); return NULL;}

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
        printf("ERROR: New socket can't be registered for asynchronous data transfer (%s).\n", strerror(errno)); fflush(stdout);
        return serverfd;
    }
    int resuseAddr = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &resuseAddr, sizeof(int));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(port);
    while(bind(serverfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
        port++;
        serv_addr.sin_port    = htons(port);
        if(port > OCLAND_ASYNC_LAST_PORT){
            printf("WARNING: Can't find an available port for asynchronous data transfer.\n"); fflush(stdout);
            printf("\tWaiting for an available one...\n"); fflush(stdout);
            port = OCLAND_ASYNC_FIRST_PORT;
            serv_addr.sin_port = htons(port);
            usleep(1000);
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
    struct dataSend* _data = (struct dataSend*)data;
    // Provide a server for the data transfer
    int fd = accept(_data->fd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on binded port.\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    // We may wait manually for the events generated by ocland,
    // and then we can wait for the OpenCL generated ones.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Read the buffer
    clEnqueueReadBuffer(_data->command_queue,_data->mem,CL_FALSE,
                        _data->offset,_data->cb,_data->ptr,
                        0,NULL,&(_data->event->event));
    clWaitForEvents(1,&(_data->event->event));
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
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    THREAD_SAFE_EXIT;
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
    // new port in order to don't intercept the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0)
        return CL_OUT_OF_HOST_MEMORY;
    // Here in after we assume that the works gone fine,
    // returning CL_SUCCESS. Therefore we will package
    // the flag, the event and the port to stablish the
    // connection for the asynchronous data transfer.
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
    }
    Send(clientfd, &port, sizeof(unsigned int), 0);
    // We are ready to trasfer the control to a parallel thread
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = serverfd;
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
    }
    return CL_SUCCESS;
}

/** Thread that receives data from client.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataRecv_thread(void *data)
{
    struct dataSend* _data = (struct dataSend*)data;
    // Provide a server for the data transfer
    int fd = accept(_data->fd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on binded port.\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    // We may wait manually for the events generated by ocland,
    // and then we can wait for the OpenCL generated ones.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Receive the data
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
    // Write it into the buffer
    clEnqueueWriteBuffer(_data->command_queue,_data->mem,CL_FALSE,
                        _data->offset,_data->cb,_data->ptr,
                        0,NULL,&(_data->event->event));
    // Wait until the data is copied before start cleaning up
    clWaitForEvents(1,&(_data->event->event));
    // Clean up
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    close(fd);
    close(_data->fd);
    // shutdown(fd, 2);
    // shutdown(_data->fd, 2); // Destroy the server to free the port
    THREAD_SAFE_EXIT;
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
    // new port in order to don't intercept the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0)
        return CL_OUT_OF_HOST_MEMORY;
    // Here in after we assume that the works gone fine,
    // returning CL_SUCCESS. Therefore we will package
    // the flag, the event and the port to stablish the
    // connection for the asynchronous data transfer.
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
    }
    Send(clientfd, &port, sizeof(unsigned int), 0);
    // We are ready to trasfer the control to a parallel thread
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = serverfd;
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
    }
    return CL_SUCCESS;
}

/** Thread that sends image from server to client.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataSendImage_thread(void *data)
{
    struct dataSend* _data = (struct dataSend*)data;
    // Provide a server for the data transfer
    int fd = accept(_data->fd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on binded port.\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    // We may wait manually for the events generated by ocland,
    // and then we can wait for the OpenCL generated ones.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Read the buffer
    clEnqueueReadImage(_data->command_queue,_data->mem,CL_FALSE,
                       _data->buffer_origin,_data->region,
                       _data->buffer_row_pitch,_data->buffer_slice_pitch,
                       _data->ptr,0,NULL,&(_data->event->event));
    clWaitForEvents(1,&(_data->event->event));
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
    // Clean up
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    THREAD_SAFE_EXIT;
}

cl_int oclandEnqueueReadImage(int *                clientfd ,
                              cl_command_queue     command_queue ,
                              cl_mem               image ,
                              const size_t *       origin ,
                              const size_t *       region ,
                              size_t               row_pitch ,
                              size_t               slice_pitch ,
                              size_t               element_size ,
                              void *               ptr ,
                              cl_uint              num_events_in_wait_list ,
                              ocland_event *       event_wait_list ,
                              cl_bool              want_event ,
                              ocland_event         event)
{
    cl_int flag;
    // Test that the objects command queue matchs
    if(testCommandQueue(command_queue,image,num_events_in_wait_list,event_wait_list) != CL_SUCCESS)
        return CL_INVALID_CONTEXT;
    // Test if the size is not out of bounds
    size_t offset = origin[2]*slice_pitch + origin[1]*row_pitch + origin[0]*element_size;
    size_t cb     = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    if(testSize(image, offset+cb) != CL_SUCCESS)
        return CL_INVALID_VALUE;
    // Test if the memory can be accessed
    if(testReadable(image) != CL_SUCCESS)
        return CL_INVALID_OPERATION;
    // Seems that data is correct, so we can proceed.
    // We need to create a new connection socket in a
    // new port in order to don't intercept the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0)
        return CL_OUT_OF_HOST_MEMORY;
    // Here in after we assume that the works gone fine,
    // returning CL_SUCCESS. Therefore we will package
    // the flag, the event and the port to stablish the
    // connection for the asynchronous data transfer.
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
    }
    Send(clientfd, &port, sizeof(unsigned int), 0);
    // We are ready to trasfer the control to a parallel thread
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = serverfd;
    _data->command_queue           = command_queue;
    _data->mem                     = image;
    _data->offset                  = offset;
    _data->cb                      = cb;
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
    }
    return CL_SUCCESS;
}

/** Thread that receives image from client.
 * @param data struct dataTransfer casted variable.
 * @return NULL
 */
void *asyncDataRecvImage_thread(void *data)
{
    struct dataSend* _data = (struct dataSend*)data;
    // Provide a server for the data transfer
    int fd = accept(_data->fd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on binded port.\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    // We may wait manually for the events generated by ocland,
    // and then we can wait for the OpenCL generated ones.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Receive the data
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
    // Writre it into the image
    clEnqueueWriteImage(_data->command_queue,_data->mem,CL_FALSE,
                        _data->buffer_origin,_data->region,
                        _data->buffer_row_pitch,_data->buffer_slice_pitch,
                        _data->ptr,0,NULL,&(_data->event->event));
    // Wait until the data is copied before start cleaning up
    clWaitForEvents(1,&(_data->event->event));
    // Clean up
    free(_data->ptr); _data->ptr = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    THREAD_SAFE_EXIT;
}

cl_int oclandEnqueueWriteImage(int *                clientfd ,
                               cl_command_queue     command_queue ,
                               cl_mem               image ,
                               const size_t *       origin ,
                               const size_t *       region ,
                               size_t               row_pitch ,
                               size_t               slice_pitch ,
                               size_t               element_size ,
                               void *               ptr ,
                               cl_uint              num_events_in_wait_list ,
                               ocland_event *       event_wait_list ,
                               cl_bool              want_event ,
                               ocland_event         event)
{
    cl_int flag;
    // Test that the objects command queue matchs
    if(testCommandQueue(command_queue,image,num_events_in_wait_list,event_wait_list) != CL_SUCCESS)
        return CL_INVALID_CONTEXT;
    // Test if the size is not out of bounds
    size_t offset = origin[2]*slice_pitch + origin[1]*row_pitch + origin[0]*element_size;
    size_t cb     = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    if(testSize(image, offset+cb) != CL_SUCCESS)
        return CL_INVALID_VALUE;
    // Test if the memory can be accessed
    if(testReadable(image) != CL_SUCCESS)
        return CL_INVALID_OPERATION;
    // Seems that data is correct, so we can proceed.
    // We need to create a new connection socket in a
    // new port in order to don't intercept the next
    // packets exchanged with the client (for instance
    // to call new commands).
    unsigned int port;
    int serverfd = openPort(&port);
    if(serverfd < 0)
        return CL_OUT_OF_HOST_MEMORY;
    // Here in after we assume that the works gone fine,
    // returning CL_SUCCESS. Therefore we will package
    // the flag, the event and the port to stablish the
    // connection for the asynchronous data transfer.
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
    }
    Send(clientfd, &port, sizeof(unsigned int), 0);
    // We are ready to trasfer the control to a parallel thread
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = serverfd;
    _data->command_queue           = command_queue;
    _data->mem                     = image;
    _data->offset                  = offset;
    _data->cb                      = cb;
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
    struct dataSend* _data = (struct dataSend*)data;
    // Provide a server for the data transfer
    int fd = accept(_data->fd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on binded port.\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    // We may wait manually for the events generated by ocland,
    // and then we can wait for the OpenCL generated ones.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Read the buffer
    size_t host_origin[3] = {0, 0, 0};
    clEnqueueReadBufferRect(_data->command_queue,_data->mem,CL_FALSE,
                            _data->buffer_origin,host_origin,_data->region,
                            _data->buffer_row_pitch,_data->buffer_slice_pitch,
                            _data->host_row_pitch,_data->host_slice_pitch,
                            _data->ptr,0,NULL,&(_data->event->event));
    clWaitForEvents(1,&(_data->event->event));
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
    // Clean up
    free(_data->ptr); _data->ptr = NULL;
    free(_data->buffer_origin); _data->buffer_origin = NULL;
    free(_data->region); _data->region = NULL;
    if(_data->event){
        _data->event->status = CL_COMPLETE;
    }
    if(_data->want_event != CL_TRUE){
        free(_data->event); _data->event = NULL;
    }
    if(_data->event_wait_list) free(_data->event_wait_list); _data->event_wait_list=NULL;
    THREAD_SAFE_EXIT;
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
        return CL_INVALID_CONTEXT;
    // Test if the size is not out of bounds
    size_t cb =   buffer_origin[0]
                + buffer_origin[1]*buffer_row_pitch
                + buffer_origin[2]*buffer_slice_pitch
                + region[0]
                + (region[1] - 1)*buffer_row_pitch
                + (region[2] - 1)*buffer_slice_pitch;
    if(testSize(mem, cb) != CL_SUCCESS)
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
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event == CL_TRUE){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
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
    cb = region[0] + region[1]*host_row_pitch + region[2]*host_slice_pitch;
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = fd;
    _data->command_queue           = command_queue;
    _data->mem                     = mem;
    _data->cb                      = cb;
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
    struct dataSend* _data = (struct dataSend*)data;
    // Provide a server for the data transfer
    int fd = accept(_data->fd, (struct sockaddr*)NULL, NULL);
    if(fd < 0){
        // we can't work, disconnect the client
        printf("ERROR: Can't listen on binded port.\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    // We may wait manually for the events generated by ocland,
    // and then we can wait for the OpenCL generated ones.
    if(_data->num_events_in_wait_list){
        oclandWaitForEvents(_data->num_events_in_wait_list, _data->event_wait_list);
    }
    // Receive the data
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
    // Writre it into the image
    size_t host_origin[3] = {0, 0, 0};
    clEnqueueWriteBufferRect(_data->command_queue,_data->mem,CL_FALSE,
                             _data->buffer_origin,host_origin,_data->region,
                             _data->buffer_row_pitch,_data->buffer_slice_pitch,
                             _data->host_row_pitch,_data->host_slice_pitch,
                             _data->ptr,0,NULL,&(_data->event->event));
    // Wait until the data is copied before start cleaning up
    clWaitForEvents(1,&(_data->event->event));
    // Clean up
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
    THREAD_SAFE_EXIT;
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
    cl_int flag;
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
    // Here in after we assume that the works gone fine,
    // returning CL_SUCCESS. We need to do it in order to
    // avoid sending the port before the flag.
    flag = CL_SUCCESS;
    Send(clientfd, &flag, sizeof(cl_int), MSG_MORE);
    if(want_event == CL_TRUE){
        Send(clientfd, &event, sizeof(ocland_event), MSG_MORE);
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
    cb = region[0] + region[1]*host_row_pitch + region[2]*host_slice_pitch;
    pthread_t thread;
    struct dataSend* _data = (struct dataSend*)malloc(sizeof(struct dataSend));
    _data->fd                      = fd;
    _data->command_queue           = command_queue;
    _data->mem                     = mem;
    _data->cb                      = cb;
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

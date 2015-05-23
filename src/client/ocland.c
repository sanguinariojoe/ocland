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

#include <errno.h>
#include <signal.h>

#include <pthread.h>

#include <ocland/common/sockets.h>
#include <ocland/common/dataExchange.h>
#include <ocland/common/dataPack.h>
#include <ocland/client/ocland_icd.h>
#include <ocland/client/ocland.h>
#include <ocland/client/commands_enum.h>

#ifndef OCLAND_PORT
    #define OCLAND_PORT 51000u
#endif

#ifndef BUFF_SIZE
    #define BUFF_SIZE 1025u
#endif

#define THREAD_SAFE_EXIT {free(_data); _data=NULL; if(fd>0) close(fd); fd = -1; pthread_exit(NULL); return NULL;}

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
    struct sockaddr_in serv_addr;
    //! @todo set SO_PRIORITY
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        printf("ERROR: Can't register a new socket for the asynchronous data transfer\n"); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    const char* ip = oclandServerAddress(_data->fd);
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
        printf("\t%s\n", strerror(errno)); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    dataPack in, out;
    out.size = _data->cb;
    out.data = _data->ptr;
    Recv_size_t(&fd, &(in.size));
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
    pthread_create(&thread, NULL, asyncDataRecv_thread, (void *)(_data));
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_READ_BUFFER);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, buffer->ptr_on_peer, MSG_MORE);
    Send(sockfd, &blocking_read, sizeof(cl_bool), MSG_MORE);
    Send_size_t(sockfd, offset, MSG_MORE);
    Send_size_t(sockfd, cb, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event){
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
    }
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag, the event, and the data.
    // ------------------------------------------------------------
    if(blocking_read){
        dataPack in, out;
        out.size = cb;
        out.data = ptr;
        Recv_size_t(sockfd, &(in.size));
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
    cl_uint i;
    cl_int flag;
    int socket_flag = 0;
    unsigned int comm = ocland_clEnqueueWriteBuffer;
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server to execute the command
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, buffer->ptr_on_peer, MSG_MORE);
    socket_flag |= Send_size_t(sockfd, offset, MSG_MORE);
    socket_flag |= Send_size_t(sockfd, cb, MSG_MORE);
    socket_flag |= Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
    if(num_events_in_wait_list){
        for(i = 0; i < num_events_in_wait_list; i++) {
            socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, MSG_MORE);
        }
    }
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, (*event)->ptr, 0);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }

    // Send the object data, by the upload stream socket. Theorically the server
    // has prepared the download stream to receive the object
    void *identifier = NULL;
    memcpy(&identifier, buffer->ptr_on_peer.object_ptr, sizeof(void*));
    upload_stream stream = getDataUploadStream(command_queue->server);
    if(!stream){
        return CL_OUT_OF_RESOURCES;
    }
    flag = enqueueUploadData(stream, identifier, (void*)ptr, cb);
    if(flag != CL_SUCCESS){
        return flag;
    }

    return CL_SUCCESS;
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_COPY_BUFFER);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, src_buffer->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, dst_buffer->ptr_on_peer, MSG_MORE);
    Send_size_t(sockfd, src_offset, MSG_MORE);
    Send_size_t(sockfd, dst_offset, MSG_MORE);
    Send_size_t(sockfd, cb, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_COPY_IMAGE);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, src_image->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, dst_image->ptr_on_peer, MSG_MORE);
    Send_size_t_array(sockfd, src_origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, dst_origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, region, 3, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_COPY_IMAGE_TO_BUFFER);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, src_image->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, dst_buffer->ptr_on_peer, MSG_MORE);
    Send_size_t_array(sockfd, src_origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, region, 3, MSG_MORE);
    Send_size_t(sockfd, dst_offset, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_COPY_BUFFER_TO_IMAGE);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, src_buffer->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, dst_image->ptr_on_peer, MSG_MORE);
    Send_size_t(sockfd, src_offset, MSG_MORE);
    Send_size_t_array(sockfd, dst_origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, region, 3, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_NDRANGE_KERNEL);
    }
    cl_bool has_global_work_offset = CL_FALSE;
    if(global_work_offset) has_global_work_offset = CL_TRUE;
    cl_bool has_local_work_size = CL_FALSE;
    if(local_work_size) has_local_work_size = CL_TRUE;
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_KERNEL, kernel->ptr_on_peer, MSG_MORE);
    Send(sockfd, &work_dim, sizeof(cl_uint), MSG_MORE);
    Send(sockfd, &has_global_work_offset, sizeof(cl_bool), MSG_MORE);
    Send(sockfd, &has_local_work_size, sizeof(cl_bool), MSG_MORE);
    if(has_global_work_offset)
        Send_size_t_array(sockfd, global_work_offset, work_dim, MSG_MORE);
    Send_size_t_array(sockfd, global_work_size, work_dim, MSG_MORE);
    if(has_local_work_size)
        Send_size_t_array(sockfd, local_work_size, work_dim, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
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
    const char* ip = oclandServerAddress(_data->fd);
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
        printf("\t%s\n", strerror(errno)); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    dataPack in, out;
    out.size = _data->cb;
    out.data = malloc(out.size);
    Recv_size_t(&fd, &(in.size));
    if(in.size == 0){
        printf("Error uncompressing data:\n\tnull array size received"); fflush(stdout);
        free(_data); _data=NULL;
        THREAD_SAFE_EXIT;
    }
    in.data = malloc(in.size);
    Recv(&fd, in.data, in.size, MSG_WAITALL);
    unpack(out,in);
    free(in.data); in.data=NULL;

    size_t i1, i2;
    for(i2 = 0; i2 < _data->region[2]; i2++) {
        for(i1 = 0; i1 < _data->region[1]; i1++) {
            char *dest = (char*)_data->ptr + i1 * _data->row + i2 * _data->slice;
            char *src = (char*)out.data + i1 * _data->region[0] + i2 * _data->region[0] * _data->region[1];
            memcpy(dest, src, _data->region[0]);
        }
    }
    free(out.data); out.data = NULL;
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
    pthread_create(&thread, NULL, asyncDataRecvRect_thread, (void *)(_data));
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_READ_IMAGE);
    }
    size_t cb = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, image->ptr_on_peer, MSG_MORE);
    Send(sockfd, &blocking_read, sizeof(cl_bool), MSG_MORE);
    Send_size_t_array(sockfd, origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, region, 3, MSG_MORE);
    Send_size_t(sockfd, row_pitch, MSG_MORE);
    Send_size_t(sockfd, slice_pitch, MSG_MORE);
    Send_size_t(sockfd, element_size, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
    }
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag, the event, and the data.
    // ------------------------------------------------------------
    if(blocking_read){
        dataPack in, out;
        out.size = cb;
        out.data = ptr;
        Recv_size_t(sockfd, &(in.size));
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
    const char* ip = oclandServerAddress(_data->fd);
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
        printf("\t%s\n", strerror(errno)); fflush(stdout);
        THREAD_SAFE_EXIT;
    }
    // Return the data (compressed) to the client
    dataPack in, out;
    in.size = _data->cb;
    in.data = _data->ptr;
    out = pack(in);
    // Since the array size is not the original one anymore, we need to
    // send the array size before to send the data
    Send_size_t(&fd, out.size, MSG_MORE);
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
    pthread_create(&thread, NULL, asyncDataSendRect_thread, (void *)(_data));
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_WRITE_IMAGE);
    }
    size_t cb = region[2]*slice_pitch + region[1]*row_pitch + region[0]*element_size;
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, image->ptr_on_peer, MSG_MORE);
    Send(sockfd, &blocking_write, sizeof(cl_bool), MSG_MORE);
    Send_size_t_array(sockfd, origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, region, 3, MSG_MORE);
    Send_size_t(sockfd, row_pitch, MSG_MORE);
    Send_size_t(sockfd, slice_pitch, MSG_MORE);
    Send_size_t(sockfd, element_size, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    int ending = 0;
    if(blocking_write) ending = MSG_MORE;
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : ending;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), ending);
    }
    if(blocking_write){
        dataPack in, out;
        in.size = cb;
        in.data = (void*)ptr;
        out = pack(in);
        Send_size_t(sockfd, out.size, MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
    }
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag and the event.
    // ------------------------------------------------------------
    if(blocking_write){
        return CL_SUCCESS;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
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

// -------------------------------------------- //
//                                              //
// OpenCL 1.1 methods                           //
//                                              //
// -------------------------------------------- //

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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_READ_BUFFER_RECT);
    }
    size_t origin = host_origin[0] + host_origin[1]*host_row_pitch + host_origin[2]*host_slice_pitch;
    ptr = (char*)ptr + origin;
    size_t cb = region[0] * region[1] * region[2];
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, mem->ptr_on_peer, MSG_MORE);
    Send(sockfd, &blocking_read, sizeof(cl_bool), MSG_MORE);
    Send_size_t_array(sockfd, buffer_origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, region, 3, MSG_MORE);
    Send_size_t(sockfd, buffer_row_pitch, MSG_MORE);
    Send_size_t(sockfd, buffer_slice_pitch, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
    }
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag, the event, and the data.
    // ------------------------------------------------------------
    if(blocking_read){
        dataPack in, out;
        out.size = cb;
        out.data = malloc(out.size);
        Recv_size_t(sockfd, &(in.size));
        in.data = malloc(in.size);
        Recv(sockfd, in.data, in.size, MSG_WAITALL);
        unpack(out,in);
        free(in.data); in.data = NULL;

        size_t i1, i2;
        for(i2 = 0; i2 < region[2]; i2++) {
            for(i1 = 0; i1 < region[1]; i1++) {
                char* dest = (char*)ptr + i1 * host_row_pitch + i2 * host_slice_pitch;
                char* src = (char*)out.data + i1 * region[0] + i2 * region[0] * region[1];
                memcpy(dest, src, region[0]);
            }
        }
        free(out.data); out.data = NULL;
        return CL_SUCCESS;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_WRITE_BUFFER_RECT);
    }
    size_t cb = region[2]*host_slice_pitch + region[1]*host_row_pitch + region[0];
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, mem->ptr_on_peer, MSG_MORE);
    Send(sockfd, &blocking_write, sizeof(cl_bool), MSG_MORE);
    Send_size_t_array(sockfd, buffer_origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, region, 3, MSG_MORE);
    Send_size_t(sockfd, buffer_row_pitch, MSG_MORE);
    Send_size_t(sockfd, buffer_slice_pitch, MSG_MORE);
    Send_size_t(sockfd, host_row_pitch, MSG_MORE);
    Send_size_t(sockfd, host_slice_pitch, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    int ending = 0;
    if(blocking_write) ending = MSG_MORE;
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : ending;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), ending);
    }
    if(blocking_write){
        dataPack in, out;
        in.size = cb;
        in.data = (void *)ptr;
        out = pack(in);
        Send_size_t(sockfd, out.size, MSG_MORE);
        Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
    }
    // ------------------------------------------------------------
    // Blocking read case:
    // We may have received the flag and the event.
    // ------------------------------------------------------------
    if(blocking_write){
        return CL_SUCCESS;
    }
    // ------------------------------------------------------------
    // Asynchronous read case:
    // We may have received the flag, the event, and a port to open
    // a parallel transfer channel.
    // ------------------------------------------------------------
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_COPY_BUFFER_RECT);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, src_buffer->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, dst_buffer->ptr_on_peer, MSG_MORE);
    Send_size_t_array(sockfd, src_origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, dst_origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, region, 3, MSG_MORE);
    Send_size_t(sockfd, src_row_pitch, MSG_MORE);
    Send_size_t(sockfd, src_slice_pitch, MSG_MORE);
    Send_size_t(sockfd, dst_row_pitch, MSG_MORE);
    Send_size_t(sockfd, dst_slice_pitch, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
    }
    return CL_SUCCESS;
}

// -------------------------------------------- //
//                                              //
// OpenCL 1.2 methods                           //
//                                              //
// -------------------------------------------- //

cl_int oclandUnloadPlatformCompiler(cl_platform_id  platform)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    unsigned int comm = ocland_clUnloadPlatformCompiler;
    // Get the server
    int *sockfd = platform->server->socket;
    if(!sockfd){
        return CL_INVALID_PLATFORM;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_PLATFORM, platform->ptr_on_peer, 0);
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
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
    cl_int flag = CL_OUT_OF_RESOURCES;
    cl_uint i;
    unsigned int comm = ocland_clEnqueueFillBuffer;
    cl_bool want_event = CL_FALSE;
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_FILL_BUFFER);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, mem->ptr_on_peer, MSG_MORE);
    Send_size_t(sockfd, pattern_size, MSG_MORE);
    Send(sockfd, pattern, pattern_size, MSG_MORE);
    Send_size_t(sockfd, offset, MSG_MORE);
    Send_size_t(sockfd, cb, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_FILL_IMAGE);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, image->ptr_on_peer, MSG_MORE);
    Send_size_t(sockfd, fill_color_size, MSG_MORE);
    Send(sockfd, fill_color, fill_color_size, MSG_MORE);
    Send_size_t_array(sockfd, origin, 3, MSG_MORE);
    Send_size_t_array(sockfd, region, 3, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_MIGRATE_MEM_OBJECTS);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send(sockfd, &num_mem_objects, sizeof(cl_uint), MSG_MORE);
    for (i = 0; i < num_mem_objects; i++) {
        Send_pointer_wrapper(sockfd, PTR_TYPE_MEM, mem_objects[i]->ptr_on_peer, MSG_MORE);
    }
    Send(sockfd, &flags, sizeof(cl_mem_migration_flags), MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_MARKER);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
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
    if(event) {
        want_event = CL_TRUE;
        // initEvent(event, command_queue, CL_COMMAND_BARRIER);
    }
    // Get the server
    int *sockfd = command_queue->server->socket;
    if(!sockfd){
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Send the command data
    Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    Send_pointer_wrapper(sockfd, PTR_TYPE_COMMAND_QUEUE, command_queue->ptr_on_peer, MSG_MORE);
    Send(sockfd, &want_event, sizeof(cl_bool), MSG_MORE);
    if(num_events_in_wait_list){
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), MSG_MORE);
        for(i = 0; i < num_events_in_wait_list; i++) {
            int flags = (i == num_events_in_wait_list - 1) ? 0 : MSG_MORE;
            Send_pointer_wrapper(sockfd, PTR_TYPE_EVENT, event_wait_list[i]->ptr, flags);
        }
    }
    else{
        Send(sockfd, &num_events_in_wait_list, sizeof(cl_uint), 0);
    }
    // Receive the answer
    Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(flag != CL_SUCCESS)
        return flag;
    if(event) {
        Recv_pointer_wrapper(sockfd, PTR_TYPE_EVENT, &(*event)->ptr);
    }
    return CL_SUCCESS;
}

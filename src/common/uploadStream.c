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
 * @brief Upload streamer.
 *
 * Upload streamer is the mirror of the download streamer, designed to can send
 * data in an asynchronous way.
 *
 * @see uploadStream.h
 * @see downloadStream.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/common/usleep.h>
#include<ocland/common/dataPack.h>
#include<ocland/common/dataExchange.h>
#include<ocland/common/uploadStream.h>

/** @brief Parallel thread function
 * @param in_stream Info to feed the thread.
 * @return NULL;
 */
void *uploadStreamThread(void *in_stream)
{
    upload_stream stream = (upload_stream)in_stream;
    unsigned int i;
    int socket_flag=0;
    int *sockfd = stream->socket;

    void *identifier=NULL;
    size_t info_size=0;
    void *info=NULL;

    // Work until the object should not be destroyed
    while(stream->rcount){
        identifier=NULL;
        info_size=0;
        info=NULL;

        upload_package package = stream->next_package;
        if(!package){
            usleep(10);
            continue;
        }

        // Serialize the data
        dataPack in, out;
        in.size = package->cb;
        in.data = package->data;
        out = pack(in);

        //  Send the data
        socket_flag |= Send_pointer(sockfd, PTR_TYPE_UNSET, identifier, MSG_MORE);
        socket_flag |= Send_size_t(sockfd, out.size, MSG_MORE);
        socket_flag |= Send(sockfd, out.data, out.size, 0);
        free(out.data); out.data = NULL;

        // Discard this package and start with the next one
        pthread_mutex_lock(&(stream->mutex));
        stream->next_package = package->next_package;
        free(package);
        pthread_mutex_unlock(&(stream->mutex));
    }

    pthread_exit(NULL);
}

upload_stream createUploadStream(int *socket)
{
    // Build up the object
    upload_stream stream = (upload_stream)malloc(
        sizeof(struct _upload_stream));
    if(!stream)
        return NULL;

    stream->socket = (int*)malloc(sizeof(int));
    if(!stream->socket){
        free(stream); stream = NULL;
        return NULL;
    }
    stream->socket = socket;
    stream->next_package = NULL;
    pthread_mutex_init(&(stream->mutex), NULL);
    stream->rcount = 1;

    // Launch the parallel thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&stream->thread,
                   &attr,
                   uploadStreamThread,
                   (void *)(stream));

    pthread_attr_destroy(&attr);
    return stream;
}

cl_int enqueueUploadData(upload_stream stream,
                         void* identifier,
                         void* host_ptr,
                         size_t cb)
{
    upload_package package = (upload_package)malloc(
        sizeof(struct _upload_package));
    if(!package){
        return CL_OUT_OF_HOST_MEMORY;
    }
    package->identifier = identifier;
    package->data = host_ptr;
    package->cb = cb;

    pthread_mutex_lock(&(stream->mutex));
    upload_package last_package = stream->next_package;
    if(!last_package){
        stream->next_package = package;
        pthread_mutex_unlock(&(stream->mutex));
        return CL_SUCCESS;
    }
    while(last_package->next_package){
        last_package = last_package->next_package;
    }
    last_package->next_package = package;
    pthread_mutex_unlock(&(stream->mutex));
}

cl_int retainUploadStream(upload_stream stream)
{
    stream->rcount++;
    return CL_SUCCESS;
}

cl_int releaseUploadStream(upload_stream stream)
{
    assert(stream->rcount);
    stream->rcount--;
    if(stream->rcount){
        return CL_SUCCESS;
    }

    // Wait for the thread to finish
    void *status;
    pthread_join(stream->thread, &status);

    // Clear all the pending packages
    while(stream->next_package)
    {
        upload_package old_package = stream->next_package;
        stream->next_package = old_package->next_package;
        free(old_package);
    }

    // we got this pointer in createUploadStream(), so its memory was
    // allocated in initLoadServers(). Therefore, we should not free it here
    // bacause new objects may create another streams with the same remote peer
    // socket pointer.
    //free(stream->socket); stream->socket = NULL;
    free(stream);

    return CL_SUCCESS;
}

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

#include <ocland/common/verbose.h>
#include <ocland/common/sockets.h>
#include <ocland/common/usleep.h>
#include<ocland/common/dataPack.h>
#include<ocland/common/dataExchange.h>
#include<ocland/common/uploadStream.h>

/** @brief Non-blocking event check callback function.
 *
 * Since calling clWaitForEvents is blocking the entire OpenCL API (affecting
 * therefore to other server clients), and clGetEventInfo cannot be used to get
 * a synchronization point, we must register a callback function to notify us
 * when the event has been finished.
 *
 * @see https://www.khronos.org/assets/uploads/developers/library/2011-siggraph-opencl-bof/OpenCL-BOF-NVIDIA-MultiGPU_SIGGRAPH-Aug11.pdf
 * @param event the event object for which the callback function is invoked.
 * @param event_command_exec_status the execution status of command for which
 * this callback function is invoked.
 * @param user_data A pointer to user supplied data, a cl_int status variable
 * in this case.
 */
void (CL_CALLBACK uploadStreamEventNotify) (cl_event event,
                                            cl_int event_command_exec_status,
                                            void *user_data){
	memcpy(user_data, &event_command_exec_status, sizeof(cl_int));
}

/** @brief Parallel thread function
 * @param in_stream Info to feed the thread.
 * @return NULL;
 */
void *uploadStreamThread(void *in_stream)
{
    upload_stream stream = (upload_stream)in_stream;
    int socket_flag=0;
    int *sockfd = stream->socket;

    // Work until the object should not be destroyed
    while(stream->rcount){
        pthread_mutex_lock(&(stream->mutex));
        upload_package package = stream->next_package;
        pthread_mutex_unlock(&(stream->mutex));
        if(!package){
            usleep(10);
            continue;
        }

        // Wait for the event to be completed
        if(package->event){
            // We cannot ask to clWaitForEvents, because since OpenCL 2.0 it is
            // a thread safe operation, and therefore it is blocking the entire
            // OpenCL API. Also we want to know if the event is finished
            // abnormally
            cl_int flag, status=CL_SUBMITTED;
            flag = clSetEventCallback(package->event,
            						  CL_COMPLETE,
									  &uploadStreamEventNotify,
                                      &status);
            if(flag != CL_SUCCESS){
				VERBOSE("Error waiting for an event (%p) in the upload stream:\n",
						package->event);
				VERBOSE("%s\n", OpenCLError(flag));
				break;
			}
            while(status > CL_COMPLETE){}
            if(status < CL_COMPLETE){
                VERBOSE("Event (%p) finished abnormally in the upload stream:\n",
                        package->event);
                VERBOSE("%s\n", OpenCLError(status));
            }
            // Release the event
            flag = clReleaseEvent(package->event);
            if(flag != CL_SUCCESS){
                VERBOSE("Error releasing event (%p) in the upload stream:\n",
                        package->event);
                VERBOSE("%s\n", OpenCLError(flag));
            }
        }

        // Serialize the data
        dataPack in, out;
        in.size = package->cb;
        in.data = package->data;
        out = pack(in);
        if(package->free_data == CL_TRUE){
            free(package->data);
        }

        //  Send the data
        socket_flag |= Send_pointer(sockfd, PTR_TYPE_UNSET, package->identifier, MSG_MORE);
        socket_flag |= Send_size_t(sockfd, out.size, MSG_MORE);
        socket_flag |= Send(sockfd, out.data, out.size, 0);
        if(socket_flag){
            VERBOSE("Error Sending data in the upload stream: %d\n", socket_flag);
        }
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
                         size_t cb,
                         cl_event event,
                         cl_bool free_host_ptr)
{
    upload_package package = (upload_package)malloc(
        sizeof(struct _upload_package));
    if(!package){
        return CL_OUT_OF_HOST_MEMORY;
    }
    package->identifier = identifier;
    package->data = host_ptr;
    package->cb = cb;
    package->event = event;
    package->next_package = NULL;
    package->free_data = free_host_ptr;

    pthread_mutex_lock(&(stream->mutex));
    upload_package last_package = stream->next_package;
    if(!last_package){
        stream->next_package = package;
        pthread_mutex_unlock(&(stream->mutex));
        printf("\tDONE\n");
        return CL_SUCCESS;
    }
    while(last_package->next_package){
        last_package = last_package->next_package;
    }
    last_package->next_package = package;
    pthread_mutex_unlock(&(stream->mutex));
    return CL_SUCCESS;
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
    // because new objects may create another streams with the same remote peer
    // socket pointer.
    //free(stream->socket); stream->socket = NULL;
    free(stream);

    return CL_SUCCESS;
}

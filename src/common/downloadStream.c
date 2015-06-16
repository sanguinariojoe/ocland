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
 * @brief Download streamer.
 *
 * Download streamer is just a parallel thread which is waiting for packages
 * incoming from the server in order to execute a specific task.
 *
 * @see downloadStream.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/common/usleep.h>
#include<ocland/common/downloadStream.h>
#include<ocland/common/dataPack.h>
#include<ocland/common/dataExchange.h>

#ifdef OCLAND_CLIENTSIDE
#include <ocland/client/event.h>
#endif

/*
 * Tasks management
 * ================
 */
tasks_list createTasksList()
{
    tasks_list tasks = (tasks_list)malloc(sizeof(struct _tasks_list));
    if(!tasks)
        return NULL;
    tasks->num_tasks = 0;
    tasks->tasks = NULL;
    pthread_mutex_init(&(tasks->mutex), NULL);
    return tasks;
}

cl_int releaseTasksList(tasks_list tasks)
{
    cl_int flag;

    // We don't know if any other thread is accessing this memory right now, so
    // lock the access, just in case.
    // Anyway it should be mentioned that all the threads using this stuff
    // should be finished before entering here.
    pthread_mutex_lock(&(tasks->mutex));

    // Destroy the children tasks
    while(tasks->num_tasks){
        flag = unregisterTask(tasks, tasks->tasks[0]);
        if(flag != CL_SUCCESS){
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    // We are trying to destroy the mutex, so unlock it and destroy.
    pthread_mutex_unlock(&(tasks->mutex));
    pthread_mutex_destroy(&(tasks->mutex));

    // And deallocate the memory get by the tasks list
    free(tasks);

    return CL_SUCCESS;
}

task registerTask(tasks_list         tasks,
                  void*              identifier,
                  void (CL_CALLBACK *pfn_notify)(size_t       /* info_size */,
                                               const void*  /* info */,
                                               void*        /* user_data */),
                  void*              user_data)
{
    // Create the new task
    task t = (task)malloc(sizeof(struct _task));
    if(!t)
        return NULL;
    t->identifier = identifier;
    t->pfn_notify = pfn_notify;
    t->user_data = user_data;
    t->non_propagating = 0;

    assert(tasks);
    // We must to block the tasks list to avoid someone may try to read/write
    // it while we are making the edition
    pthread_mutex_lock(&(tasks->mutex));

    // Add the new task to the list
    task *backup_tasks = tasks->tasks;
    tasks->tasks = (task*)malloc((tasks->num_tasks + 1) * sizeof(task));
    if(!tasks->tasks){
        free(t); t = NULL;
        free(backup_tasks); backup_tasks = NULL;
        pthread_mutex_unlock(&(tasks->mutex));
        return NULL;
    }
    if(backup_tasks)
        memcpy(tasks->tasks, backup_tasks, tasks->num_tasks * sizeof(task));
    tasks->tasks[tasks->num_tasks] = t;
    tasks->num_tasks++;

    // Unlock the tasks list
    pthread_mutex_unlock(&(tasks->mutex));

    return t;
}

cl_int unregisterTask(tasks_list tasks,
                      task       registered_task)
{
    // We must to block the tasks list to avoid someone may try to read/write
    // it while we are making the edition
    pthread_mutex_lock(&(tasks->mutex));

    // Locate the index of the task
    cl_uint i, index;
    for(index = 0; index < tasks->num_tasks; index++){
        if(registered_task == tasks->tasks[index]){
            break;
        }
    }
    if(index == tasks->num_tasks){
        pthread_mutex_unlock(&(tasks->mutex));
        return CL_INVALID_VALUE;
    }

    // Remove the task from the list
    assert(tasks->num_tasks > 0);
    free(tasks->tasks[index]);
    for(i = index; i < tasks->num_tasks - 1; i++){
        tasks->tasks[i] = tasks->tasks[i + 1];
    }
    tasks->num_tasks--;
    tasks->tasks[tasks->num_tasks] = NULL;

    // Unlock the tasks list
    pthread_mutex_unlock(&(tasks->mutex));

    return CL_SUCCESS;
}

/*
 * Parallel thread management
 * ==========================
 */

/// Error string
static char error_str[256];

/** @brief Report an error in the download stream.
 *
 * This function will call all the functions in _download_stream::error_tasks.
 * @param stream Download stream where the error was detected
 * @param txt Error description.
 */
void reportDownloadStreamErrors(download_stream stream,
                                const char *txt)
{
    cl_uint i;
    size_t txt_size = (strlen(txt) + 1) * sizeof(char);
    pthread_mutex_lock(&(stream->error_tasks->mutex));
    for(i = 0; i < stream->error_tasks->num_tasks; i++){
        task t = stream->error_tasks->tasks[i];
        t->pfn_notify(txt_size, (void*)txt, t->user_data);
    }
    pthread_mutex_unlock(&(stream->error_tasks->mutex));
}

/** @brief Parallel thread function
 * @param in_stream Info to feed the thread.
 * @return NULL;
 */
void *downloadStreamThread(void *in_stream)
{
    download_stream stream = (download_stream)in_stream;
    unsigned int i;
    cl_int flag;
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

        // Check if there are data waiting from server
        socket_flag = CheckDataAvailable(sockfd);
        if(socket_flag < 0){
            if(-1 == *sockfd){
                // The socket falls down in an error state
                sprintf(error_str,
                        "Connection error: %s",
                        strerror(errno));
                reportDownloadStreamErrors(stream, error_str);
                break;
            }
            // Wait for a little (10 microseconds) before checking new packages
            // incoming from the server
            usleep(10);
            continue;
        }
        if(!socket_flag){
            // Peer called to close connection
            strcpy(error_str, "Remote peer closed the connection");
            reportDownloadStreamErrors(stream, error_str);
            break;
        }

        // Capture the data from the server
        socket_flag  = Recv_pointer(sockfd, PTR_TYPE_UNSET, &identifier);
        socket_flag |= Recv_size_t(sockfd, &info_size);
        if(socket_flag){
            // This download stream is not working anymore
            strcpy(error_str, "Failure receiving task data");
            reportDownloadStreamErrors(stream, error_str);
            break;
        }
        if(info_size){
            info = malloc(info_size);
            if(!info){
                sprintf(error_str,
                        "Memory allocation error (%lu bytes)",
                        info_size);
                reportDownloadStreamErrors(stream, error_str);
                break;
            }
            socket_flag |= Recv(sockfd, info, info_size, MSG_WAITALL);
            if(socket_flag){
                // This download stream is not working anymore
                sprintf(error_str,
                        "Failure receiving %lu bytes (task info)",
                        info_size);
                reportDownloadStreamErrors(stream, error_str);
                break;
            }
        }

        // Locate and execute the function
        pthread_mutex_lock(&(stream->tasks->mutex));
        int non_propagating = 0;
        task t = NULL;
        for(i = 0; i < stream->tasks->num_tasks; i++){
            t = stream->tasks->tasks[i];
            if(identifier != t->identifier){
                continue;
            }
            t->pfn_notify(info_size, info, t->user_data);
            // Check if it is a non-propagating task
            if(t->non_propagating){
                non_propagating = 1;
                break;
            }
        }
        pthread_mutex_unlock(&(stream->tasks->mutex));
        if(non_propagating){
            flag = unregisterTask(stream->tasks, t);
            if(flag != CL_SUCCESS){
                sprintf(error_str,
                        "Failure automatically releasing a non-propagating task");
                reportDownloadStreamErrors(stream, error_str);
                break;
            }
        }
        // Info array is not required anymore
        free(info);
    }

    pthread_exit(NULL);
}

download_stream createDownloadStream(int *socket)
{
    // Build up the object
    download_stream stream = (download_stream)malloc(
        sizeof(struct _download_stream));
    if(!stream)
        return NULL;

    stream->socket = (int*)malloc(sizeof(int));
    if(!stream->socket){
        free(stream); stream = NULL;
        return NULL;
    }
    stream->socket = socket;

    stream->tasks = createTasksList();
    if(!stream->tasks){
        free(stream->socket); stream->socket = NULL;
        free(stream); stream = NULL;
        return NULL;
    }
    stream->error_tasks = createTasksList();
    if(!stream->error_tasks){
        free(stream->socket); stream->socket = NULL;
        free(stream->tasks); stream->tasks = NULL;
        free(stream); stream = NULL;
        return NULL;
    }

    stream->rcount = 1;

    // Launch the parallel thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&stream->thread,
                   &attr,
                   downloadStreamThread,
                   (void *)(stream));

    pthread_attr_destroy(&attr);

    return stream;
}

task setDownloadStreamErrorCallback(
        download_stream    stream,
        void (CL_CALLBACK *pfn_error)(size_t       /* info_size */,
                                      const void*  /* info */,
                                      void*        /* user_data */),
        void*              user_data)
{
    task t = registerTask(stream->error_tasks,
                          NULL,
                          pfn_error,
                          user_data);
    if(!t){
        return NULL;
    }
    return t;
}

cl_int retainDownloadStream(download_stream stream)
{
    stream->rcount++;
    return CL_SUCCESS;
}

cl_int releaseDownloadStream(download_stream stream)
{
    assert(stream->rcount);
    stream->rcount--;
    if(stream->rcount){
        return CL_SUCCESS;
    }

    // Wait for the thread to finish
    void *status;
    pthread_join(stream->thread, &status);

    // Release the tasks list
    releaseTasksList(stream->tasks);
    stream->tasks = NULL;
    releaseTasksList(stream->error_tasks);
    stream->error_tasks = NULL;

    // Destroy the object itself

    // we got this pointer in createDownloadStream(), so its memory was
    // allocated in initLoadServers(). Therefore, we should not free it here
    // bacause new objects may create another streams with the same remote peer
    // socket pointer.
    //free(stream->socket); stream->socket = NULL;
    free(stream);

    return CL_SUCCESS;
}

void CL_CALLBACK pfn_downloadData(size_t info_size,
                                  const void* info,
                                  void* user_data)
{
    // Extract the user data
    size_t cb;
    void* host_ptr;
    cl_event event;
    memcpy(&cb, user_data, sizeof(size_t));
    memcpy(&host_ptr, (char*)user_data + sizeof(size_t), sizeof(void*));
    memcpy(&event, (char*)user_data + sizeof(size_t) + sizeof(void*), sizeof(cl_event));
    free(user_data);

    // Get the info from the remote peer
    dataPack in, out;
    in.size = info_size;
    in.data = (void*)info;
    out.size = cb;
    out.data = host_ptr;

    // Unpack it
    unpack(out, in);

    // Report the task finalization
    if(event){
        #ifdef OCLAND_CLIENTSIDE
            setEventStatus(event, CL_COMPLETE);
        #else
            clSetUserEventStatus(event, CL_COMPLETE);
        #endif
    }
}

cl_int enqueueDownloadData(download_stream stream,
                           void* identifier,
                           void* host_ptr,
                           size_t cb,
                           cl_event event)
{
    // Build up the user_data with the data size and the memory where it should
    // be placed. We are also appending the OpenCL event.
    void* user_data = malloc(sizeof(size_t) + sizeof(void*) + sizeof(cl_event));
    if(!user_data){
        return CL_OUT_OF_HOST_MEMORY;
    }
    memcpy(user_data, &cb, sizeof(size_t));
    memcpy((char*)user_data + sizeof(size_t), &host_ptr, sizeof(void*));
    memcpy((char*)user_data + sizeof(size_t) + sizeof(void*), &event, sizeof(cl_event));

    // Register the new task
    task t = registerTask(stream->tasks,
                          identifier,
                          &pfn_downloadData,
                          user_data);
    if(!t){
        return CL_OUT_OF_HOST_MEMORY;
    }

    // Set the task as unique (non-propagating task which is self-destructing)
    t->non_propagating = 1;

    return CL_SUCCESS;
}

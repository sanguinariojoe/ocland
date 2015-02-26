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

/** @file
 * @brief Download streamer.
 *
 * Download streamer is just a parallel thread which is waiting for packages
 * incoming from the server in order to execute a specific task.
 *
 * @see downloadStream.c
 */

#ifndef DOWNLOADSTREAM_H_INCLUDED
#define DOWNLOADSTREAM_H_INCLUDED

#include <ocl_icd.h>
#include <pthread.h>

/*
 * Tasks management
 * ================
 */
/// Abstraction of _task
typedef struct _task* task;

/** @brief Task stuff.
 *
 * Each task is build upon a shared identifier between the ICD and the server,
 * and the callback function to be launched when the server is requiring it.
 */
struct _task
{
    /// Generic type of identifier, usually it is the affected object.
    void *identifier;
    /** @brief Callback function to be executed.
     *
     * This dispatching function is receiving 3 parameters:
     *     - \a info_size: The size of \a info.
     *     - \a info: Data provided by the server.
     *     - \a A pointer to the user data _task::user_data.
     */
    void (CL_CALLBACK *dispatch)(size_t       /* info_size */,
                                 const void*  /* info */,
                                 void*        /* user_data */);
    /** @brief User data for _task::dispatch.
     *
     * _task::user_data can be NULL.
     */
    void* user_data;
};

/// Abstraction of _tasks_list
typedef struct _tasks_list* tasks_list;

/** @brief Tasks list
 *
 * A list of registered tasks.
 */
struct _tasks_list
{
    /// Number of tasks
    cl_uint num_tasks;
    /// List of tasks
    task *tasks;
    /** @brief Mutex to control the access to the tasks list.
     *
     * It should be asserted that the tasks list is not modified while it is
     * read from another thread.
     */
    pthread_mutex_t mutex;
};

/** @brief Create a tasks list
 *
 * This function is threads safe.
 * @return Tasks list. NULL is returned in case of error.
 */
tasks_list createTasksList();

/** @brief Destroy the tasks list.
 *
 * It includes freeing allocated memory and destroying the mutex object.
 * @param tasks Task list to be destroyed.
 * @return CL_SUCCESS if the tasks list if successfully destroyed,
 * CL_OUT_OF_HOST_MEMORY if errors are detected.
 * @note This method will destroy all the tasks registered into.
 */
cl_int releaseTasksList(tasks_list tasks);

/** @brief Register a new task to the tasks list.
 * @param tasks Task lists where new task should be registered in.
 * @param identifier Shared identifier between the ICD and the server.
 * @param dispatch Dispatching function.
 * @param user_data User data for _task::dispatch. \a user_data can be NULL.
 * @return The generated task object. NULL if errors happened.
 * @see _task
 * @see _tasks_list
 * @note To removed the generated task use unregisterTask()
 */
task registerTask(tasks_list         tasks,
                  void*              identifier,
                  void (CL_CALLBACK *dispatch)(size_t       /* info_size */,
                                               const void*  /* info */,
                                               void*        /* user_data */),
                  void*              user_data);

/** @brief Unregister a task from the tasks list.
 *
 * The task will be destroyed, i.e. The memory will be freed.
 * @param tasks Task lists where the task should be removed from.
 * @param registered_task Registered task to be removed.
 * @return CL_SUCCESS if the task is rightly removed. CL_INVALID_VALUE if the
 * task cannot be found in the tasks list.
 */
cl_int unregisterTask(tasks_list tasks,
                      task       registered_task);

/*
 * Parallel thread management
 * ==========================
 */
/// Abstraction of _download_stream
typedef struct _download_stream* download_stream;

/** @brief Download stream
 *
 * A list of registered tasks.
 */
struct _download_stream
{
    /// Parallel thread to be waiting for server queries
    pthread_t thread;
    /// Connection socket with the server.
    int* socket;
    /// Tasks list
    tasks_list tasks;
    /** @brief References count
     *
     * The download streamer requires a parallel thread, that must be created
     * and destroyed on demand in order to avoid the user application hang
     * when trying to quit.
     *
     * The object will be removed when the reference count reach 0.
     */
    cl_uint rcount;
};

/** @brief Create a download streamer.
 *
 * This method will launch the parallel thread.
 * @param Already connected socket with the server.
 * @return Download stream. NULL is returned in case of error.
 */
download_stream createDownloadStream(int socket);

/** @brief Increments _download_stream::rcount.
 * @param stream Download stream.
 * @return CL_SUCCESS.
 */
cl_int retainDownloadStream(download_stream stream);

/** @brief Decrements _download_stream::rcount.
 *
 * When such value reachs 0, the download stream will be destroyed.
 * @param stream Download stream.
 * @return CL_SUCCESS.
 */
cl_int releaseDownloadStream(download_stream stream);

#endif // DOWNLOADSTREAM_H_INCLUDED

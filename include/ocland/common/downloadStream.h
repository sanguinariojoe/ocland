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
    void (CL_CALLBACK *pfn_notify)(size_t       /* info_size */,
                                   const void*  /* info */,
                                   void*        /* user_data */);
    /** @brief User data for _task::pfn_notify.
     *
     * _task::user_data can be NULL.
     */
    void* user_data;
    /** @brief Propagating/Unique task.
     *
     * If 0 (default value), other tasks, with the same identifier, will be
     * executed when the download stream is receiving such identifier,
     * otherwise the task is considered unique, so it is blocking the download
     * stream to don't execute other tasks until identifier is not received
     * again.
     * Unique tasks are executed just one time, selfdestructing after that.
     */
    int non_propagating;
    /** @brief info_size provided by the task event.
     *
     * This variable is conveniently provide for internal purposes, and should
     * not be used by normal users. By default it is taking the value 0
     */
    size_t _info_size;
    /** @brief info provided by the task event.
     *
     * This variable is conveniently provide for internal purposes, and should
     * not be used by normal users. By default it is taking the value NULL
     */
    void* _info;
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
 * @param pfn_notify Dispatching function.
 * @param user_data User data for _task::pfn_notify. \a user_data can be NULL.
 * @return The generated task object. NULL if errors happened.
 * @see _task
 * @see _tasks_list
 * @note To remove the generated task use unregisterTask()
 */
task registerTask(tasks_list         tasks,
                  void*              identifier,
                  void (CL_CALLBACK *pfn_notify)(size_t       /* info_size */,
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
 */
struct _download_stream
{
    /// Parallel thread to be waiting for server queries
    pthread_t thread;
    /// Connection socket with the server.
    int* socket;
    /// Registered tasks
    tasks_list tasks;
    /// Pending tasks to become dispatched
    tasks_list pending_tasks;
    /** @brief Error tasks list.
     *
     * While _download_stream::tasks are designed to capture remote peer
     * queries, this task list is designed to capture local download stream
     * errors. The info argument passed to _task::pfn_notify will be an error
     * description string.
     */
    tasks_list error_tasks;
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
download_stream createDownloadStream(int *socket);

/** @brief Set a download streamer error callback function.
 *
 * Such error callback function will be called when the downstream detects an
 * error.
 * @param pfn_error Callback function to be executed.
 * @return The generated task object. NULL if errors happened.
 */
task setDownloadStreamErrorCallback(
    download_stream    stream,
    void (CL_CALLBACK *pfn_error)(size_t       /* info_size */,
                                  const void*  /* info */,
                                  void*        /* user_data */),
    void*              user_data);

/** @brief Increments _download_stream::rcount.
 * @param stream Download stream.
 * @return CL_SUCCESS.
 */
cl_int retainDownloadStream(download_stream stream);

/** @brief Decrements _download_stream::rcount.
 *
 * When such value reachs 0, the download stream will be destroyed.
 * @note Once stream is destroyed, pointer to it must not be used any more
 * @param stream Download stream.
 * @return CL_SUCCESS.
 */
cl_int releaseDownloadStream(download_stream stream);

/** @brief Ask for a package from the download stream.
 * @param stream Download stream to be used to receive the package.
 * @param identifier Shared identifier between the server and the client.
 * @param host_ptr Pointer to the already allocated memory where the data should
 * be copied.
 * @param region Region to become downloaded and read (width in bytes, height in
 * rows, depth in slices). For a 2D rectangle copy, the depth value given by
 * region[2] should be 1. For a 1D linear copy, the height and depth values
 * given by region[1] and region[2] should be 1.
 * @param row_pitch The length of each row in bytes to be used for the memory
 * region. If row_pitch is 0, row_pitch is computed as region[0].
 * @param slice_pitch The length of each 2D slice in bytes to be used for the
 * memory region. If slice_pitch is 0, slice_pitch is computed as
 * region[1] * row_pitch. 
 * @param event OpenCL user event. It will set to CL_COMPLETE (or to the
 * appropiate error code) when the data transfer has finished. If the event is
 * NULL, it will be ignored.
 * @return CL_SUCCESS if the data is correctly enqueued to be received,
 * CL_OUT_OF_HOST_MEMORY if errors happened while memory required by the
 * implementation is allocated.
 */
cl_int enqueueDownloadData(download_stream stream,
                           void* identifier,
                           void* host_ptr,
                           const size_t* region,
                           size_t row_pitch,
                           size_t slice_pitch,
                           cl_event event);

#endif // DOWNLOADSTREAM_H_INCLUDED

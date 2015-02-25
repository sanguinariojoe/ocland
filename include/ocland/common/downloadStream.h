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
     *     - \a info: Data provided by the server, it include at least the
     *       _task::identifier.
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
 * @return Tasks list. The returned object is dynamically allocated, and
 * therefore the receiver is responsible of its destruction. NULL is returned if
 * the function fails.
 */
tasks_list createTasksList();

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
 * @param tasks Task lists where the task should be removed from.
 * @param registered_task Registered task to be removed.
 * @return CL_SUCCESS if the task is rightly removed. CL_INVALID_VALUE if the
 * task cannot be found in the tasks list.
 */
cl_int unregisterTask(tasks_list tasks,
                      task       registered_task);


#endif // DOWNLOADSTREAM_H_INCLUDED

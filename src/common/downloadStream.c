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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<ocland/common/downloadStream.h>
#include<ocland/common/dataExchange.h>

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

task registerTask(tasks_list         tasks,
                  void*              identifier,
                  void (CL_CALLBACK *dispatch)(size_t       /* info_size */,
                                               const void*  /* info */,
                                               void*        /* user_data */),
                  void*              user_data)
{
    // Create the new task
    task t = (task)malloc(sizeof(struct _task));
    if(!t)
        return NULL;
    t->identifier = identifier;
    t->dispatch = dispatch;
    t->user_data = user_data;

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
    for(i = index; i < tasks->num_tasks - 1; i++){
        tasks->tasks[i] = tasks->tasks[i + 1];
    }
    tasks->num_tasks--;
    free(tasks->tasks[tasks->num_tasks]);
    tasks->tasks[tasks->num_tasks] = NULL;

    // Unlock the tasks list
    pthread_mutex_unlock(&(tasks->mutex));

    return CL_SUCCESS;
}

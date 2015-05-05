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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ocland/server/validator.h>

void initValidator(validator v)
{
    v->socket = NULL;
    v->callbacks_socket = NULL;
    v->upload_socket = NULL;
    v->download_socket = NULL;

    v->num_devices = 0;
    v->devices = NULL;
    v->num_contexts = 0;
    v->contexts = NULL;
    v->num_queues = 0;
    v->queues = NULL;
    v->num_buffers = 0;
    v->buffers = NULL;
    v->num_samplers = 0;
    v->samplers = NULL;
    v->num_programs = 0;
    v->programs = NULL;
    v->num_kernels = 0;
    v->kernels = NULL;
    v->num_events = 0;
    v->events = NULL;
}

void closeValidator(validator v)
{
    v->socket = NULL;
    v->callbacks_socket = NULL;
    v->upload_socket = NULL;
    v->download_socket = NULL;

    v->num_devices = 0;
    if(v->devices) free(v->devices); v->devices = NULL;
    v->num_contexts = 0;
    if(v->contexts) free(v->contexts); v->contexts = NULL;
    v->num_queues = 0;
    if(v->queues) free(v->queues); v->queues = NULL;
    v->num_buffers = 0;
    if(v->buffers) free(v->buffers); v->buffers = NULL;
    v->num_samplers = 0;
    if(v->samplers) free(v->samplers); v->samplers = NULL;
    v->num_programs = 0;
    if(v->programs) free(v->programs); v->programs = NULL;
    v->num_kernels = 0;
    if(v->kernels) free(v->kernels); v->kernels = NULL;
    v->num_events = 0;
    if(v->events) free(v->events); v->events = NULL;
}

cl_int isPlatform(validator v, cl_platform_id platform)
{
    cl_uint i;
    cl_int flag;
    // Get platforms from OpenCL. Platforms can't be
    // pre-computed and stored or context generation
    // for future clients may fail if NVidia vendor
    // is used.
    cl_uint num_platforms = 0;
    cl_platform_id *platforms = NULL;
    flag = clGetPlatformIDs(0, NULL, &num_platforms);
    if( (flag != CL_SUCCESS) || (!num_platforms) ){
        return CL_INVALID_PLATFORM;
    }
    platforms = (cl_platform_id*) malloc(num_platforms*sizeof(cl_platform_id));
    if(!platforms){
        return CL_INVALID_PLATFORM;
    }
    flag = clGetPlatformIDs(num_platforms, platforms, NULL);
    if(flag != CL_SUCCESS){
        return CL_INVALID_PLATFORM;
    }
    // Compare them with the provided platform
    for(i=0;i<num_platforms;i++){
        if(platform == platforms[i]){
            if(platforms) free(platforms); platforms=NULL;
            return CL_SUCCESS;
        }
    }
    if(platforms) free(platforms); platforms=NULL;
    return CL_INVALID_PLATFORM;
}

cl_int isDevice(validator v, cl_device_id device)
{
    cl_uint i;
    // Compare provided device with all the previously registered
    for(i=0;i<v->num_devices;i++){
        if(device == v->devices[i])
            return CL_SUCCESS;
    }
    return CL_INVALID_DEVICE;
}

cl_uint registerDevices(validator v, cl_uint num_devices, cl_device_id *devices)
{
    cl_uint i,j,n=0,id=0;
    // Count the possible different devices
    for(i=0;i<num_devices;i++){
        cl_int flag = CL_SUCCESS;
        for(j=0;j<v->num_devices;j++){
            if(devices[i] == v->devices[j]){
                flag = CL_INVALID_DEVICE;
                break;
            }
        }
        if(flag == CL_SUCCESS)
            n++;
    }
    if(!n)
        return v->num_devices;
    printf("Storing %u new devices", n); fflush(stdout);
    if(!v->devices){
        v->devices = (cl_device_id*)malloc( (v->num_devices + n) * sizeof(cl_device_id));
        if(!v->devices){
            printf("...\n\tError allocating memory for devices.\n"); fflush(stdout);
            v->num_devices = 0;
            return 0;
        }
    }
    else{
        v->devices = (cl_device_id*)realloc( v->devices, (v->num_devices + n) * sizeof(cl_device_id));
        if(!v->devices){
            printf("...\n\tError reallocating memory for devices.\n"); fflush(stdout);
            v->num_devices = 0;
            return 0;
        }
    }
    // Store new devices
    for(i=0;i<num_devices;i++){
        cl_int flag = CL_SUCCESS;
        for(j=0;j<v->num_devices;j++){
            if(devices[i] == v->devices[j]){
                flag = CL_INVALID_DEVICE;
                break;
            }
        }
        if(flag == CL_SUCCESS){
            v->devices[v->num_devices+id] = devices[i];
            id++;
        }
    }
    v->num_devices += n;
    printf(", %u devices stored.\n", v->num_devices); fflush(stdout);
    return v->num_devices;
}

cl_uint unregisterDevices(validator v, cl_uint num_devices, cl_device_id *devices)
{
    cl_uint i,j,n=0,id=0;
    // Count the affected devices
    for(i=0;i<num_devices;i++){
        cl_int flag = CL_INVALID_DEVICE;
        for(j=0;j<v->num_devices;j++){
            if(devices[i] == v->devices[j]){
                flag = CL_SUCCESS;
                break;
            }
        }
        if(flag == CL_SUCCESS)
            n++;
    }
    if(!n)
        return v->num_devices;
    printf("Removing %u registered devices", n); fflush(stdout);
    if(n == v->num_devices){
        // No more devices in the list
        printf(", no more devices stored.\n"); fflush(stdout);
        v->num_devices = 0;
        if(v->devices) free(v->devices); v->devices=NULL;
        return 0;
    }
    cl_device_id *backup = v->devices;
    v->devices = (cl_device_id*)malloc( (v->num_devices - n) * sizeof(cl_device_id));
    if(!v->devices){
        printf("...\n\tError allocating memory for devices.\n"); fflush(stdout);
        if(backup) free(backup); backup=NULL;
        v->num_devices = 0;
        return 0;
    }
    // Store devices not affected
    for(i=0;i<num_devices;i++){
        cl_int flag = CL_INVALID_DEVICE;
        for(j=0;j<v->num_devices;j++){
            if(devices[i] == backup[j]){
                flag = CL_SUCCESS;
                break;
            }
        }
        if(flag == CL_SUCCESS){
            v->devices[id] = devices[i];
            id++;
        }
    }
    v->num_devices -= n;
    printf(", %u devices remain stored.\n", v->num_devices); fflush(stdout);
    if(backup) free(backup); backup=NULL;
    return v->num_devices;
}

cl_int isContext(validator v, ocland_context context)
{
    cl_uint i;
    // Compare provided context with all the previously registered
    for(i=0;i<v->num_contexts;i++){
        if(context == v->contexts[i])
            return CL_SUCCESS;
    }
    return CL_INVALID_CONTEXT;
}

cl_uint registerContext(validator v, ocland_context context)
{
    // Look if the context already exist
    if(isContext(v,context) == CL_SUCCESS)
        return v->num_contexts;
    printf("Storing new context"); fflush(stdout);
    if(!v->contexts){
        v->contexts = (ocland_context*)malloc( (v->num_contexts + 1) * sizeof(ocland_context));
        if(!v->contexts){
            printf("...\n\tError allocating memory for contexts.\n"); fflush(stdout);
            v->num_contexts = 0;
            return 0;
        }
    }
    else{
        v->contexts = (ocland_context*)realloc( v->contexts, (v->num_contexts + 1) * sizeof(ocland_context));
        if(!v->contexts){
            printf("...\n\tError reallocating memory for contexts.\n"); fflush(stdout);
            v->num_contexts = 0;
            return 0;
        }
    }
    // Store new context
    v->contexts[v->num_contexts] = context;
    v->num_contexts++;
    printf(", %u contexts stored.\n", v->num_contexts); fflush(stdout);
    return v->num_contexts;
}

cl_uint unregisterContext(validator v, ocland_context context)
{
    cl_uint i,id=0;
    // Look if the context don't exist
    if(isContext(v,context) != CL_SUCCESS)
        return v->num_contexts;
    printf("Removing registered context"); fflush(stdout);
    if(v->num_contexts == 1){
        // No more contexts in the list
        printf(", no more contexts generated.\n"); fflush(stdout);
        v->num_contexts = 0;
        if(v->contexts) free(v->contexts); v->contexts=NULL;
        return 0;
    }
    ocland_context *backup = v->contexts;
    v->contexts = (ocland_context*)malloc( (v->num_contexts - 1) * sizeof(ocland_context));
    if(!v->contexts){
        printf("...\n\tError allocating memory for contexts.\n"); fflush(stdout);
        if(backup) free(backup); backup=NULL;
        v->num_contexts = 0;
        return 0;
    }
    // Store contexts not affected
    for(i=0;i<v->num_contexts;i++){
        if(context == backup[i]){
            continue;
        }
        v->contexts[id] = backup[i];
        id++;
    }
    v->num_contexts--;
    printf(", %u contexts remain stored.\n", v->num_contexts); fflush(stdout);
    if(backup) free(backup); backup=NULL;
    return v->num_contexts;
}

cl_int isQueue(validator v, cl_command_queue queue)
{
    cl_uint i;
    // Compare provided queue with all the previously registered
    for(i=0;i<v->num_queues;i++){
        if(queue == v->queues[i])
            return CL_SUCCESS;
    }
    return CL_INVALID_COMMAND_QUEUE;
}

cl_uint registerQueue(validator v, cl_command_queue queue)
{
    // Look if the queue already exist
    if(isQueue(v,queue) == CL_SUCCESS)
        return v->num_queues;
    printf("Storing new command queue"); fflush(stdout);
    if(!v->queues){
        v->queues = (cl_command_queue*)malloc( (v->num_queues + 1) * sizeof(cl_command_queue));
        if(!v->queues){
            printf("...\n\tError allocating memory for command queues.\n"); fflush(stdout);
            v->num_queues = 0;
            return 0;
        }
    }
    else{
        v->queues = (cl_command_queue*)realloc( v->queues, (v->num_queues + 1) * sizeof(cl_command_queue));
        if(!v->queues){
            printf("...\n\tError reallocating memory for command queues.\n"); fflush(stdout);
            v->num_queues = 0;
            return 0;
        }
    }
    // Store new queue
    v->queues[v->num_queues] = queue;
    v->num_queues++;
    printf(", %u command queues stored.\n", v->num_queues); fflush(stdout);
    return v->num_queues;
}

cl_uint unregisterQueue(validator v, cl_command_queue queue)
{
    cl_uint i,id=0;
    // Look if the queue don't exist
    if(isQueue(v,queue) != CL_SUCCESS)
        return v->num_queues;
    printf("Removing registered command queue"); fflush(stdout);
    if(v->num_queues == 1){
        // No more queues in the list
        printf(", no more command queues stored.\n"); fflush(stdout);
        v->num_queues = 0;
        if(v->queues) free(v->queues); v->queues=NULL;
        return 0;
    }
    cl_command_queue *backup = v->queues;
    v->queues = (cl_command_queue*)malloc( (v->num_queues - 1) * sizeof(cl_command_queue));
    if(!v->queues){
        printf("...\n\tError allocating memory for command queues.\n"); fflush(stdout);
        if(backup) free(backup); backup=NULL;
        v->num_queues = 0;
        return 0;
    }
    // Store queues not affected
    for(i=0;i<v->num_queues;i++){
        if(queue == backup[i]){
            continue;
        }
        v->queues[id] = backup[i];
        id++;
    }
    v->num_queues--;
    printf(", %u command queues remain stored.\n", v->num_queues); fflush(stdout);
    if(backup) free(backup); backup=NULL;
    return v->num_queues;
}

cl_int isBuffer(validator v, cl_mem buffer)
{
    cl_uint i;
    // Compare provided buffer with all the previously registered
    for(i=0;i<v->num_buffers;i++){
        if(buffer == v->buffers[i])
            return CL_SUCCESS;
    }
    return CL_INVALID_MEM_OBJECT;
}

cl_uint registerBuffer(validator v, cl_mem buffer)
{
    // Look if the buffer already exist
    if(isBuffer(v,buffer) == CL_SUCCESS)
        return v->num_buffers;
    printf("Storing new buffer"); fflush(stdout);
    if(!v->buffers){
        v->buffers = (cl_mem*)malloc( (v->num_buffers + 1) * sizeof(cl_mem));
        if(!v->buffers){
            printf("...\n\tError allocating memory for buffers.\n"); fflush(stdout);
            v->num_buffers = 0;
            return 0;
        }
    }
    else{
        v->buffers = (cl_mem*)realloc( v->buffers, (v->num_buffers + 1) * sizeof(cl_mem));
        if(!v->buffers){
            printf("...\n\tError reallocating memory for buffers.\n"); fflush(stdout);
            v->num_buffers = 0;
            return 0;
        }
    }
    // Store new buffer
    v->buffers[v->num_buffers] = buffer;
    v->num_buffers++;
    printf(", %u buffers stored.\n", v->num_buffers); fflush(stdout);
    return v->num_buffers;
}

cl_uint unregisterBuffer(validator v, cl_mem buffer)
{
    cl_uint i,id=0;
    // Look if the buffer don't exist
    if(isBuffer(v,buffer) != CL_SUCCESS)
        return v->num_buffers;
    printf("Removing registered buffer"); fflush(stdout);
    if(v->num_buffers == 1){
        // No more buffers in the list
        printf(", no more buffers stored.\n"); fflush(stdout);
        v->num_buffers = 0;
        if(v->buffers) free(v->buffers); v->buffers=NULL;
        return 0;
    }
    cl_mem *backup = v->buffers;
    v->buffers = (cl_mem*)malloc( (v->num_buffers - 1) * sizeof(cl_mem));
    if(!v->buffers){
        printf("...\n\tError allocating memory for buffers.\n"); fflush(stdout);
        if(backup) free(backup); backup=NULL;
        v->num_buffers = 0;
        return 0;
    }
    // Store buffers not affected
    for(i=0;i<v->num_buffers;i++){
        if(buffer == backup[i]){
            continue;
        }
        v->buffers[id] = backup[i];
        id++;
    }
    v->num_buffers--;
    printf(", %u buffers remain stored.\n", v->num_buffers); fflush(stdout);
    if(backup) free(backup); backup=NULL;
    return v->num_buffers;
}

cl_int isSampler(validator v, cl_sampler sampler)
{
    cl_uint i;
    // Compare provided sampler with all the previously registered
    for(i=0;i<v->num_samplers;i++){
        if(sampler == v->samplers[i])
            return CL_SUCCESS;
    }
    return CL_INVALID_SAMPLER;
}

cl_uint registerSampler(validator v, cl_sampler sampler)
{
    // Look if the sampler already exist
    if(isSampler(v,sampler) == CL_SUCCESS)
        return v->num_samplers;
    printf("Storing new sampler"); fflush(stdout);
    if(!v->samplers){
        v->samplers = (cl_sampler*)malloc( (v->num_samplers + 1) * sizeof(cl_sampler));
        if(!v->samplers){
            printf("...\n\tError allocating memory for samplers.\n"); fflush(stdout);
            v->num_samplers = 0;
            return 0;
        }
    }
    else{
        v->samplers = (cl_sampler*)realloc( v->samplers, (v->num_samplers + 1) * sizeof(cl_sampler));
        if(!v->samplers){
            printf("...\n\tError reallocating memory for samplers.\n"); fflush(stdout);
            v->num_samplers = 0;
            return 0;
        }
    }
    // Store new sampler
    v->samplers[v->num_samplers] = sampler;
    v->num_samplers++;
    printf(", %u samplers stored.\n", v->num_samplers); fflush(stdout);
    return v->num_samplers;
}

cl_uint unregisterSampler(validator v, cl_sampler sampler)
{
    cl_uint i,id=0;
    // Look if the sampler don't exist
    if(isSampler(v,sampler) != CL_SUCCESS)
        return v->num_samplers;
    printf("Removing registered sampler"); fflush(stdout);
    if(v->num_samplers == 1){
        // No more samplers in the list
        printf(", no more samplers stored.\n"); fflush(stdout);
        v->num_samplers = 0;
        if(v->samplers) free(v->samplers); v->samplers=NULL;
        return 0;
    }
    cl_sampler *backup = v->samplers;
    v->samplers = (cl_sampler*)malloc( (v->num_samplers - 1) * sizeof(cl_sampler));
    if(!v->samplers){
        printf("...\n\tError allocating memory for samplers.\n"); fflush(stdout);
        if(backup) free(backup); backup=NULL;
        v->num_samplers = 0;
        return 0;
    }
    // Store samplers not affected
    for(i=0;i<v->num_samplers;i++){
        if(sampler == backup[i]){
            continue;
        }
        v->samplers[id] = backup[i];
        id++;
    }
    v->num_samplers--;
    printf(", %u samplers remain stored.\n", v->num_samplers); fflush(stdout);
    if(backup) free(backup); backup=NULL;
    return v->num_samplers;
}

cl_int isProgram(validator v, cl_program program)
{
    cl_uint i;
    // Compare provided program with all the previously registered
    for(i=0;i<v->num_programs;i++){
        if(program == v->programs[i])
            return CL_SUCCESS;
    }
    return CL_INVALID_PROGRAM;
}

cl_uint registerProgram(validator v, cl_program program)
{
    // Look if the program already exist
    if(isProgram(v,program) == CL_SUCCESS)
        return v->num_programs;
    printf("Storing new program"); fflush(stdout);
    if(!v->programs){
        v->programs = (cl_program*)malloc( (v->num_programs + 1) * sizeof(cl_program));
        if(!v->programs){
            printf("...\n\tError allocating memory for programs.\n"); fflush(stdout);
            v->num_programs = 0;
            return 0;
        }
    }
    else{
        v->programs = (cl_program*)realloc( v->programs, (v->num_programs + 1) * sizeof(cl_program));
        if(!v->programs){
            printf("...\n\tError reallocating memory for programs.\n"); fflush(stdout);
            v->num_programs = 0;
            return 0;
        }
    }
    // Store new program
    v->programs[v->num_programs] = program;
    v->num_programs++;
    printf(", %u programs stored.\n", v->num_programs); fflush(stdout);
    return v->num_programs;
}

cl_uint unregisterProgram(validator v, cl_program program)
{
    cl_uint i,id=0;
    // Look if the program don't exist
    if(isProgram(v,program) != CL_SUCCESS)
        return v->num_programs;
    printf("Removing registered program"); fflush(stdout);
    if(v->num_programs == 1){
        // No more programs in the list
        printf(", no more programs stored.\n"); fflush(stdout);
        v->num_programs = 0;
        if(v->programs) free(v->programs); v->programs=NULL;
        return 0;
    }
    cl_program *backup = v->programs;
    v->programs = (cl_program*)malloc( (v->num_programs - 1) * sizeof(cl_program));
    if(!v->programs){
        printf("...\n\tError allocating memory for programs.\n"); fflush(stdout);
        if(backup) free(backup); backup=NULL;
        v->num_programs = 0;
        return 0;
    }
    // Store programs not affected
    for(i=0;i<v->num_programs;i++){
        if(program == backup[i]){
            continue;
        }
        v->programs[id] = backup[i];
        id++;
    }
    v->num_programs--;
    printf(", %u programs remain stored.\n", v->num_programs); fflush(stdout);
    if(backup) free(backup); backup=NULL;
    return v->num_programs;
}

cl_int isKernel(validator v, cl_kernel kernel)
{
    cl_uint i;
    // Compare provided kernel with all the previously registered
    for(i=0;i<v->num_kernels;i++){
        if(kernel == v->kernels[i])
            return CL_SUCCESS;
    }
    return CL_INVALID_KERNEL;
}

cl_uint registerKernel(validator v, cl_kernel kernel)
{
    // Look if the kernel already exist
    if(isKernel(v,kernel) == CL_SUCCESS)
        return v->num_kernels;
    printf("Storing new kernel"); fflush(stdout);
    if(!v->kernels){
        v->kernels = (cl_kernel*)malloc( (v->num_kernels + 1) * sizeof(cl_kernel));
        if(!v->kernels){
            printf("...\n\tError allocating memory for kernels.\n"); fflush(stdout);
            v->num_kernels = 0;
            return 0;
        }
    }
    else{
        v->kernels = (cl_kernel*)realloc( v->kernels, (v->num_kernels + 1) * sizeof(cl_kernel));
        if(!v->kernels){
            printf("...\n\tError reallocating memory for kernels.\n"); fflush(stdout);
            v->num_kernels = 0;
            return 0;
        }
    }
    // Store new kernel
    v->kernels[v->num_kernels] = kernel;
    v->num_kernels++;
    printf(", %u kernels stored.\n", v->num_kernels); fflush(stdout);
    return v->num_kernels;
}

cl_uint unregisterKernel(validator v, cl_kernel kernel)
{
    cl_uint i,id=0;
    // Look if the kernel don't exist
    if(isKernel(v,kernel) != CL_SUCCESS)
        return v->num_kernels;
    printf("Removing registered kernel"); fflush(stdout);
    if(v->num_kernels == 1){
        // No more kernels in the list
        printf(", no more kernels stored.\n"); fflush(stdout);
        v->num_kernels = 0;
        if(v->kernels) free(v->kernels); v->kernels=NULL;
        return 0;
    }
    cl_kernel *backup = v->kernels;
    v->kernels = (cl_kernel*)malloc( (v->num_kernels - 1) * sizeof(cl_kernel));
    if(!v->kernels){
        printf("...\n\tError allocating memory for kernels.\n"); fflush(stdout);
        if(backup) free(backup); backup=NULL;
        v->num_kernels = 0;
        return 0;
    }
    // Store kernels not affected
    for(i=0;i<v->num_kernels;i++){
        if(kernel == backup[i]){
            continue;
        }
        v->kernels[id] = backup[i];
        id++;
    }
    v->num_kernels--;
    printf(", %u kernels remain stored.\n", v->num_kernels); fflush(stdout);
    if(backup) free(backup); backup=NULL;
    return v->num_kernels;
}

cl_int isEvent(validator v, ocland_event event)
{
    cl_uint i;
    // Compare provided event with all the previously registered
    for(i=0;i<v->num_events;i++){
        if(event == v->events[i])
            return CL_SUCCESS;
    }
    return CL_INVALID_EVENT;
}

cl_uint registerEvent(validator v, ocland_event event)
{
    // Look if the event already exist
    if(isEvent(v,event) == CL_SUCCESS)
        return v->num_events;
    printf("Storing new event"); fflush(stdout);
    if(!v->events){
        v->events = (ocland_event*)malloc( (v->num_events + 1) * sizeof(ocland_event));
        if(!v->events){
            printf("...\n\tError allocating memory for events.\n"); fflush(stdout);
            v->num_events = 0;
            return 0;
        }
    }
    else{
        v->events = (ocland_event*)realloc( v->events, (v->num_events + 1) * sizeof(ocland_event));
        if(!v->events){
            printf("...\n\tError reallocating memory for events.\n"); fflush(stdout);
            v->num_events = 0;
            return 0;
        }
    }
    // Store new event
    v->events[v->num_events] = event;
    v->num_events++;
    printf(", %u events stored.\n", v->num_events); fflush(stdout);
    return v->num_events;
}

cl_uint unregisterEvent(validator v, ocland_event event)
{
    cl_uint i,id=0;
    // Look if the event don't exist
    if(isEvent(v,event) != CL_SUCCESS)
        return v->num_events;
    printf("Removing registered event"); fflush(stdout);
    if(v->num_events == 1){
        // No more events in the list
        printf(", no more events stored.\n"); fflush(stdout);
        v->num_events = 0;
        if(v->events) free(v->events); v->events=NULL;
        return 0;
    }
    ocland_event *backup = v->events;
    v->events = (ocland_event*)malloc( (v->num_events - 1) * sizeof(ocland_event));
    if(!v->events){
        printf("...\n\tError allocating memory for events.\n"); fflush(stdout);
        if(backup) free(backup); backup=NULL;
        v->num_events = 0;
        return 0;
    }
    // Store events not affected
    for(i=0;i<v->num_events;i++){
        if(event == backup[i]){
            continue;
        }
        v->events[id] = backup[i];
        id++;
    }
    v->num_events--;
    printf(", %u events remain stored.\n", v->num_events); fflush(stdout);
    if(backup) free(backup); backup=NULL;
    return v->num_events;
}

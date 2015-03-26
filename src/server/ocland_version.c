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
#include <string.h>

#include <ocland/server/ocland_version.h>

struct _cl_version clGetPlatformVersion(cl_platform_id platform)
{
    struct _cl_version version;
    version.major = 0;
    version.minor = 0;
    size_t param_value_size = 0;
    cl_int flag = clGetPlatformInfo(platform, CL_PLATFORM_VERSION,
                                    0, NULL, &param_value_size);
    if(flag != CL_SUCCESS)
        return version;
    char* param_value = (char*)malloc(param_value_size);
    if(!param_value)
        return version;
    flag = clGetPlatformInfo(platform, CL_PLATFORM_VERSION,
                             param_value_size, param_value, NULL);
    if(flag != CL_SUCCESS){
        free(param_value); param_value=NULL;
        return version;
    }
    // The platform version may start with "OpenCL " string, so discard it
    char* str = strstr(param_value,"OpenCL ");
    if(!str){
        free(param_value); param_value=NULL;
        return version;
    }
    str += 7;
    // Get the major version
    version.major = atoi(str);
    // Look for the minor version (is exist)
    str = strstr(str,".");
    if(str)
        version.minor = atoi(str + 1);
    free(param_value); param_value=NULL;
    return version;
}

struct _cl_version clGetDeviceVersion(cl_device_id device)
{
    struct _cl_version version;
    version.major = 0;
    version.minor = 0;
    cl_platform_id platform = NULL;
    cl_int flag = clGetDeviceInfo(device, CL_DEVICE_PLATFORM,
                                  sizeof(cl_platform_id), &platform, NULL);
    if(flag != CL_SUCCESS)
        return version;
    return clGetPlatformVersion(platform);
}

struct _cl_version clGetContextVersion(cl_context context)
{
    struct _cl_version version;
    version.major = 0;
    version.minor = 0;
    cl_device_id *devices = NULL;
    size_t param_value_size = 0;
    cl_int flag = clGetContextInfo(context, CL_CONTEXT_DEVICES,
                                   0, NULL, &param_value_size);
    if(flag != CL_SUCCESS)
        return version;
    devices = (cl_device_id*)malloc(param_value_size);
    if(!devices)
        return version;
    flag = clGetContextInfo(context, CL_CONTEXT_DEVICES,
                            param_value_size, devices, NULL);
    cl_device_id device = devices[0];
    free(devices); devices=NULL;
    if(flag != CL_SUCCESS)
        return version;

    return clGetDeviceVersion(device);
}

struct _cl_version clGetCommandQueueVersion(cl_command_queue command_queue)
{
    struct _cl_version version;
    version.major = 0;
    version.minor = 0;
    cl_device_id device = NULL;
    cl_int flag = clGetCommandQueueInfo(command_queue, CL_QUEUE_DEVICE,
                                        sizeof(cl_device_id), &device, NULL);
    if(flag != CL_SUCCESS)
        return version;
    return clGetDeviceVersion(device);
}

struct _cl_version clGetMemObjectVersion(cl_mem memobj)
{
    struct _cl_version version;
    version.major = 0;
    version.minor = 0;
    cl_context context = NULL;
    cl_int flag = clGetMemObjectInfo(memobj, CL_MEM_CONTEXT,
                                     sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS)
        return version;
    return clGetContextVersion(context);
}

struct _cl_version clGetSamplerVersion(cl_sampler sampler)
{
    struct _cl_version version;
    version.major = 0;
    version.minor = 0;
    cl_context context = NULL;
    cl_int flag = clGetSamplerInfo(sampler, CL_SAMPLER_CONTEXT,
                                   sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS)
        return version;
    return clGetContextVersion(context);
}

struct _cl_version clGetProgramVersion(cl_program program)
{
    struct _cl_version version;
    version.major = 0;
    version.minor = 0;
    cl_context context = NULL;
    cl_int flag =  clGetProgramInfo(program, CL_PROGRAM_CONTEXT,
                                    sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS)
        return version;
    return clGetContextVersion(context);
}

struct _cl_version clGetKernelVersion(cl_kernel kernel)
{
    struct _cl_version version;
    version.major = 0;
    version.minor = 0;
    cl_context context = NULL;
    cl_int flag = clGetKernelInfo(kernel, CL_KERNEL_CONTEXT,
                                  sizeof(cl_context), &context, NULL);
    if(flag != CL_SUCCESS)
        return version;
    return clGetContextVersion(context);
}

struct _cl_version clGetEventVersion(cl_event event)
{
    struct _cl_version version;
    version.major = 0;
    version.minor = 0;
    cl_command_queue command_queue = NULL;
    cl_int flag = clGetEventInfo(event, CL_EVENT_COMMAND_QUEUE,
                                 sizeof(cl_command_queue), &command_queue, NULL);
    if(flag != CL_SUCCESS)
        return version;
    return clGetCommandQueueVersion(command_queue);
}

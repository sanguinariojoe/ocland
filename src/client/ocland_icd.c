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

#include <ocland/client/ocland_opencl.h>
#include <ocland/common/verbose.h>

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#ifndef MAX_N_PLATFORMS
    #define MAX_N_PLATFORMS 1<<16 //   65536
#endif
#ifndef MAX_N_DEVICES
    #define MAX_N_DEVICES 1<<16   //   65536
#endif

// Forward declaration for the icd_clSetKernelArg method
CL_API_ENTRY cl_int CL_API_CALL
icd_clGetKernelArgInfo(cl_kernel           kernel ,
                       cl_uint             arg_indx ,
                       cl_kernel_arg_info  param_name ,
                       size_t              param_value_size ,
                       void *              param_value ,
                       size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_2;

cl_uint num_master_events = 0;
cl_event *master_events = NULL;

/** Check for event validity
 * @param event Event to check
 * @return 1 if the event is a known event, 0 otherwise.
 */
int isEvent(cl_event event){
    cl_uint i;
    for(i=0;i<num_master_events;i++){
        if(event == master_events[i])
            return 1;
    }
    return 0;
}

// --------------------------------------------------------------
// Platforms
// --------------------------------------------------------------

// This function must be __stdcall on windows to make icd loader happy with
// value of ESP properly saved across the function call.
static cl_int CL_API_CALL
__GetPlatformIDs(cl_uint num_entries,
                 cl_platform_id *platforms,
                 cl_uint *num_platforms)
{
    VERBOSE_IN();
    if((!platforms   && !num_platforms) ||
       (num_entries && !platforms) ||
       (!num_entries &&  platforms)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }

    cl_uint num_master_platforms = 0;
    cl_platform_id *master_platforms = NULL;

    cl_int err_code;
    // Init platforms array
    if(!num_master_platforms){
        err_code = getPlatformIDs(0,
                                  &master_dispatch,
                                  NULL,
                                  &num_master_platforms);
        if(err_code != CL_SUCCESS){
            VERBOSE_OUT(err_code);
            return err_code;
        }
        if(!num_master_platforms){
            VERBOSE_OUT(CL_PLATFORM_NOT_FOUND_KHR);
            return CL_PLATFORM_NOT_FOUND_KHR;
        }
        master_platforms = (cl_platform_id*)malloc(
            num_master_platforms * sizeof(cl_platform_id));
        if(!master_platforms){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        err_code = getPlatformIDs(num_master_platforms,
                                  &master_dispatch,
                                  master_platforms,
                                  NULL);
        if(err_code != CL_SUCCESS){
            VERBOSE_OUT(err_code);
            return err_code;
        }
    }
    if( num_platforms )
        *num_platforms = num_master_platforms;
    // Answer to the user
    if(!num_master_platforms){
        if(master_platforms)
            free(master_platforms);
        master_platforms = NULL;
        VERBOSE_OUT(CL_PLATFORM_NOT_FOUND_KHR);
        return CL_PLATFORM_NOT_FOUND_KHR;
    }
    if(platforms) {
        cl_uint n = num_master_platforms < num_entries ? num_master_platforms:num_entries;
        memcpy(platforms, master_platforms, n * sizeof(cl_platform_id));
    }
    if(master_platforms)
        free(master_platforms);
    master_platforms = NULL;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetPlatformIDs(cl_uint           num_entries ,
                     cl_platform_id *  platforms ,
                     cl_uint *         num_platforms) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    cl_int err_code = __GetPlatformIDs(num_entries, platforms, num_platforms);
    if(err_code == CL_PLATFORM_NOT_FOUND_KHR){
        err_code = CL_SUCCESS;
    }
    VERBOSE_OUT(err_code);
    return err_code;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clIcdGetPlatformIDsKHR(cl_uint num_entries,
                           cl_platform_id *platforms,
                           cl_uint *num_platforms) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    cl_int flag = __GetPlatformIDs(num_entries, platforms, num_platforms);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetPlatformInfo(cl_platform_id   platform,
                      cl_platform_info param_name,
                      size_t           param_value_size,
                      void *           param_value,
                      size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasPlatform(platform)){
        VERBOSE_OUT(CL_INVALID_PLATFORM);
        return CL_INVALID_PLATFORM;
    }
    if((!param_value_size &&  param_value) ||
       ( param_value_size && !param_value)) {
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    // Connect to servers to get info
    cl_int flag = getPlatformInfo(platform,
                                  param_name,
                                  param_value_size,
                                  param_value,
                                  param_value_size_ret);
    VERBOSE_OUT(flag);
    return flag;
}

// --------------------------------------------------------------
// Devices
// --------------------------------------------------------------

static cl_int CL_API_CALL
icd_clGetDeviceIDs(cl_platform_id   platform,
                   cl_device_type   device_type,
                   cl_uint          num_entries,
                   cl_device_id *   devices,
                   cl_uint *        num_devices)
{
    VERBOSE_IN();
    if(!hasPlatform(platform)){
        VERBOSE_OUT(CL_INVALID_PLATFORM);
        return CL_INVALID_PLATFORM;
    }
    if(    (!num_entries &&  devices)
        || ( num_entries && !devices)
        || (!devices && !num_devices) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = getDeviceIDs(platform,
                               device_type,
                               num_entries,
                               devices,
                               num_devices);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetDeviceInfo(cl_device_id    device,
                    cl_device_info  param_name,
                    size_t          param_value_size,
                    void *          param_value,
                    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasDevice(device)){
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return CL_INVALID_DEVICE;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }

    size_t size_ret = 0;
    void* value = NULL;
    if(param_name == CL_DEVICE_PLATFORM){
        size_ret = sizeof(cl_platform_id);
        value = &(device->platform);
    }
    else if(param_name == CL_DEVICE_PARENT_DEVICE){
        size_ret = sizeof(cl_device_id);
        value = &(device->parent_device);
    }
    else if(param_name == CL_DEVICE_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(device->rcount);
    }
    else{
        cl_int flag = getDeviceInfo(device,
                                    param_name,
                                    param_value_size,
                                    param_value,
                                    param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clCreateSubDevices(cl_device_id                         in_device,
                       const cl_device_partition_property * properties,
                       cl_uint                              num_entries,
                       cl_device_id                       * out_devices,
                       cl_uint                            * num_devices) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    if(!hasDevice(in_device)){
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return CL_INVALID_DEVICE;
    }
    if(    ( !out_devices && !num_devices )
        || ( !out_devices &&  num_entries )
        || (  out_devices && !num_entries )){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    // Count the number of properties
    cl_uint num_properties = 0;
    if(properties){
        while(properties[num_properties] != 0)
            num_properties++;
        num_properties++;   // Trailing zero must be counted in
    }
    cl_int flag = createSubDevices(in_device,
                                   properties,
                                   num_properties,
                                   num_entries,
                                   out_devices,
                                   num_devices);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    if(!hasDevice(device)){
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return CL_INVALID_DEVICE;
    }
    cl_int flag = retainDevice(device);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    if(!hasDevice(device)){
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return CL_INVALID_DEVICE;
    }
    cl_int flag = releaseDevice(device);
    VERBOSE_OUT(flag);
    return flag;
}

// --------------------------------------------------------------
// Context
// --------------------------------------------------------------

CL_API_ENTRY cl_context CL_API_CALL
icd_clCreateContext(const cl_context_properties * properties,
                    cl_uint                       num_devices ,
                    const cl_device_id *          devices,
                    void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *),
                    void *                        user_data,
                    cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    cl_uint num_properties = 0;
    VERBOSE_IN();
    cl_platform_id platform = NULL;
    if(   !num_devices || !devices
       || (!pfn_notify &&  user_data)
       || ( pfn_notify && !user_data)){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    // Count the number of properties
    if(properties){
        while(properties[num_properties] != 0)
            num_properties++;
        num_properties++;   // Final zero must be counted

        // Look for the platform in the properties
        for(i = 0; i < num_properties - 1; i = i + 2){
            if(properties[i] == CL_CONTEXT_PLATFORM){
                platform = (cl_platform_id)(properties[i+1]);
            }
        }
    }
    if(platform){
        // Ensure that is a right platform
        if(!hasPlatform(platform)){
            if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
            VERBOSE_OUT(CL_INVALID_PLATFORM);
            return NULL;
        }
    }

    for(i = 0; i < num_devices; i++){
        // Check that the devices are valid
        if(!hasDevice(devices[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return NULL;
        }
        if(platform){
            // Check that their belongs to a common platform
            if(devices[i]->platform != platform){
                if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
                VERBOSE_OUT(CL_INVALID_DEVICE);
                return NULL;
            }
        }
        else{
            // If the platform has not been specified in the properties, get
            // the first available one.
            platform = devices[i]->platform;
        }
    }

    if(!platform){
        if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
        VERBOSE_OUT(CL_INVALID_PLATFORM);
        return NULL;
    }

    cl_int flag;
    cl_context context = createContext(platform,
                                       properties,
                                       num_properties,
                                       num_devices,
                                       devices,
                                       pfn_notify,
                                       user_data,
                                       &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);

    if(flag != CL_SUCCESS){
        return NULL;
    }
    return context;
}

CL_API_ENTRY cl_context CL_API_CALL
icd_clCreateContextFromType(const cl_context_properties * properties,
                            cl_device_type                device_type,
                            void (CL_CALLBACK *     pfn_notify)(const char *, const void *, size_t, void *),
                            void *                        user_data,
                            cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    cl_uint num_properties = 0;
    VERBOSE_IN();
    cl_platform_id platform = NULL;
    if(   (!pfn_notify &&  user_data)
       || ( pfn_notify && !user_data)){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    // Count the number of properties
    if(properties){
        while(properties[num_properties] != 0)
            num_properties++;
        num_properties++;   // Final zero must be counted

        // Look for the platform in the properties
        for(i = 0; i < num_properties - 1; i = i + 2){
            if(properties[i] == CL_CONTEXT_PLATFORM){
                platform = (cl_platform_id)(properties[i+1]);
            }
        }
    }
    if(platform){
        // Ensure that is a right platform
        if(!hasPlatform(platform)){
            if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
            VERBOSE_OUT(CL_INVALID_PLATFORM);
            return NULL;
        }
    }

    if(!platform){
        if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
        VERBOSE_OUT(CL_INVALID_PLATFORM);
        return NULL;
    }

    cl_int flag;
    cl_context context = createContextFromType(platform,
                                               properties,
                                               num_properties,
                                               device_type,
                                               pfn_notify,
                                               user_data,
                                               &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return context;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    cl_int flag;
    VERBOSE_IN();
    if(!hasContext(context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }

    flag = retainContext(context);

    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    cl_int flag;
    VERBOSE_IN();
    if(!hasContext(context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }

    flag = releaseContext(context);

    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetContextInfo(cl_context         context,
                     cl_context_info    param_name,
                     size_t             param_value_size,
                     void *             param_value,
                     size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasContext(context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    size_t size_ret = 0;
    void* value = NULL;
    if(param_name == CL_CONTEXT_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(context->rcount);
    }
    else if(param_name == CL_CONTEXT_NUM_DEVICES){
        size_ret = sizeof(cl_uint);
        value = &(context->num_devices);
    }
    else if(param_name == CL_CONTEXT_DEVICES){
        size_ret = context->num_devices * sizeof(cl_device_id);
        value = context->devices;
    }
    else if(param_name == CL_CONTEXT_PROPERTIES){
        size_ret = context->num_properties * sizeof(cl_context_properties);
        value = context->properties;
    }
    else{
        // Let's see if server knows something we already can't
        cl_int flag = getContextInfo(context,
                                     param_name,
                                     param_value_size,
                                     param_value,
                                     param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

// --------------------------------------------------------------
// Command Queue
// --------------------------------------------------------------

CL_API_ENTRY cl_command_queue CL_API_CALL
icd_clCreateCommandQueue(cl_context                     context,
                         cl_device_id                   device,
                         cl_command_queue_properties    properties,
                         cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    if(!hasDevice(device)){
        if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return NULL;
    }

    cl_int flag;
    cl_command_queue command_queue = createCommandQueue(context,
                                                        device,
                                                        properties,
                                                        &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return command_queue;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    cl_int flag;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    flag = retainCommandQueue(command_queue);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    cl_int flag;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    flag = releaseCommandQueue(command_queue);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetCommandQueueInfo(cl_command_queue      command_queue,
                          cl_command_queue_info param_name,
                          size_t                param_value_size,
                          void *                param_value,
                          size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    size_t size_ret = 0;
    void* value = NULL;
    if(param_name == CL_QUEUE_CONTEXT){
        size_ret = sizeof(cl_context);
        value = &(command_queue->context);
    }
    else if(param_name == CL_QUEUE_DEVICE){
        size_ret = sizeof(cl_device_id);
        value = &(command_queue->device);
    }
    else if(param_name == CL_QUEUE_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(command_queue->rcount);
    }
    else if(param_name == CL_QUEUE_PROPERTIES){
        size_ret = sizeof(cl_command_queue_properties);
        value = &(command_queue->properties);
    }
    else{
        cl_int flag = getCommandQueueInfo(command_queue,
                                          param_name,
                                          param_value_size,
                                          param_value,
                                          param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clSetCommandQueueProperty(cl_command_queue             command_queue,
                              cl_command_queue_properties  properties,
                              cl_bool                      enable,
                              cl_command_queue_properties *old_properties) CL_EXT_SUFFIX__VERSION_1_0_DEPRECATED
{
    // It is a deprecated invalid method
    VERBOSE_IN();
    VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
    return CL_INVALID_COMMAND_QUEUE;
}



// --------------------------------------------------------------
// Memory objects
// --------------------------------------------------------------

CL_API_ENTRY cl_mem CL_API_CALL
icd_clCreateBuffer(cl_context    context ,
                   cl_mem_flags  flags ,
                   size_t        size ,
                   void *        host_ptr ,
                   cl_int *      errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    if( (flags & CL_MEM_ALLOC_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR) ){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    if( (flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR) ){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    if(host_ptr && ( !(flags & CL_MEM_COPY_HOST_PTR) && !(flags & CL_MEM_USE_HOST_PTR) )){
        if(errcode_ret) *errcode_ret=CL_INVALID_HOST_PTR;
        VERBOSE_OUT(CL_INVALID_HOST_PTR);
        return NULL;
    }
    else if(!host_ptr && ( (flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_COPY_HOST_PTR) )){
        if(errcode_ret) *errcode_ret=CL_INVALID_HOST_PTR;
        VERBOSE_OUT(CL_INVALID_HOST_PTR);
        return NULL;
    }

    cl_int flag;
    cl_mem mem = createBuffer(context,
                              flags,
                              size,
                              host_ptr,
                              &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return mem;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    cl_int flag;
    VERBOSE_IN();
    if(!hasMem(memobj)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    flag = retainMemObject(memobj);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    cl_int flag;
    VERBOSE_IN();
    if(!hasMem(memobj)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    flag = releaseMemObject(memobj);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetSupportedImageFormats(cl_context           context,
                               cl_mem_flags         flags,
                               cl_mem_object_type   image_type ,
                               cl_uint              num_entries ,
                               cl_image_format *    image_formats ,
                               cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0
{
    cl_int flag;
    VERBOSE_IN();
    if(!hasContext(context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    (!num_entries &&  image_formats)
        || ( num_entries && !image_formats)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    flag = getSupportedImageFormats(context,
                                    flags,
                                    image_type,
                                    num_entries,
                                    image_formats,
                                    num_image_formats);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetMemObjectInfo(cl_mem            memobj ,
                       cl_mem_info       param_name ,
                       size_t            param_value_size ,
                       void *            param_value ,
                       size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasMem(memobj)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    size_t size_ret = 0;
    void* value = NULL;
    if(param_name == CL_MEM_TYPE){
        size_ret = sizeof(cl_mem_object_type);
        value = &(memobj->type);
    }
    else if(param_name == CL_MEM_FLAGS){
        size_ret = sizeof(cl_mem_flags);
        value = &(memobj->flags);
    }
    else if(param_name == CL_MEM_SIZE){
        size_ret = sizeof(size_t);
        value = &(memobj->size);
    }
    else if(param_name == CL_MEM_HOST_PTR){
        size_ret = sizeof(void*);
        value = &(memobj->host_ptr);
    }
    else if(param_name == CL_MEM_MAP_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(memobj->map_count);
    }
    else if(param_name == CL_MEM_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(memobj->rcount);
    }
    else if(param_name == CL_MEM_CONTEXT){
        size_ret = sizeof(cl_context);
        value = &(memobj->context);
    }
    else if(param_name == CL_MEM_ASSOCIATED_MEMOBJECT){
        size_ret = sizeof(cl_mem);
        value = &(memobj->mem_associated);
    }
    else if(param_name == CL_MEM_OFFSET){
        size_ret = sizeof(size_t);
        value = &(memobj->offset);
    }
    else{
        cl_int flag = getMemObjectInfo(memobj,
                                       param_name,
                                       param_value_size,
                                       param_value,
                                       param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetImageInfo(cl_mem            image ,
                   cl_image_info     param_name ,
                   size_t            param_value_size ,
                   void *            param_value ,
                   size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasMem(image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    (image->type != CL_MEM_OBJECT_IMAGE1D)
        && (image->type != CL_MEM_OBJECT_IMAGE1D_BUFFER)
        && (image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY)
        && (image->type != CL_MEM_OBJECT_IMAGE2D)
        && (image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY)
        && (image->type != CL_MEM_OBJECT_IMAGE3D)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    size_t size_ret = 0;
    void* value = NULL;
    if(param_name == CL_IMAGE_FORMAT){
        size_ret = sizeof(cl_image_format);
        value = image->image_format;
    }
    else if(param_name == CL_IMAGE_ELEMENT_SIZE){
        size_ret = sizeof(size_t);
        value = &(image->element_size);
    }
    else if(param_name == CL_IMAGE_ROW_PITCH){
        size_ret = sizeof(size_t);
        value = &(image->image_desc->image_row_pitch);
    }
    else if(param_name == CL_IMAGE_SLICE_PITCH){
        size_ret = sizeof(size_t);
        value = &(image->image_desc->image_slice_pitch);
    }
    else if(param_name == CL_IMAGE_WIDTH){
        size_ret = sizeof(size_t);
        value = &(image->image_desc->image_width);
    }
    else if(param_name == CL_IMAGE_HEIGHT){
        size_ret = sizeof(size_t);
        value = &(image->image_desc->image_height);
    }
    else if(param_name == CL_IMAGE_DEPTH){
        size_ret = sizeof(size_t);
        value = &(image->image_desc->image_depth);
    }
    else if(param_name == CL_IMAGE_ARRAY_SIZE){
        size_ret = sizeof(size_t);
        value = &(image->image_desc->image_array_size);
    }
    else if(param_name == CL_IMAGE_BUFFER){
        size_ret = sizeof(size_t);
        value = &(image->image_desc->buffer);
    }
    else if(param_name == CL_IMAGE_NUM_MIP_LEVELS){
        size_ret = sizeof(size_t);
        value = &(image->image_desc->num_mip_levels);
    }
    else if(param_name == CL_IMAGE_NUM_SAMPLES){
        size_ret = sizeof(size_t);
        value = &(image->image_desc->num_samples);
    }
    else{
        cl_int flag = getImageInfo(image,
                                   param_name,
                                   param_value_size,
                                   param_value,
                                   param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_mem CL_API_CALL
icd_clCreateSubBuffer(cl_mem                    buffer ,
                      cl_mem_flags              flags ,
                      cl_buffer_create_type     buffer_create_type ,
                      const void *              buffer_create_info ,
                      cl_int *                  errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    VERBOSE_IN();
    if(!hasMem(buffer)){
        if(errcode_ret) *errcode_ret = CL_INVALID_MEM_OBJECT;
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return NULL;
    }
    if((flags & CL_MEM_USE_HOST_PTR) ||
       (flags & CL_MEM_ALLOC_HOST_PTR) ||
       (flags & CL_MEM_COPY_HOST_PTR)){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }

    cl_int flag;
    cl_mem mem = createSubBuffer(buffer,
                                 flags,
                                 buffer_create_type,
                                 buffer_create_info,
                                 &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return mem;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clSetMemObjectDestructorCallback(cl_mem  memobj ,
                                     void (CL_CALLBACK * pfn_notify)(cl_mem  memobj,
                                                                     void* user_data),
                                     void * user_data)             CL_API_SUFFIX__VERSION_1_1
{
    VERBOSE_IN();
    if(!hasMem(memobj)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!pfn_notify){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = setMemObjectDestructorCallback(memobj,
                                                 pfn_notify,
                                                 user_data);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_mem CL_API_CALL
icd_clCreateImage(cl_context              context,
                  cl_mem_flags            flags,
                  const cl_image_format * image_format,
                  const cl_image_desc *   image_desc,
                  void *                  host_ptr,
                  cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    unsigned int element_n = 1;
    size_t element_size = 0;
    VERBOSE_IN();
    if(!hasContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    if((flags & CL_MEM_ALLOC_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR)){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    if((flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR)){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    if(!image_format){
        if(errcode_ret) *errcode_ret=CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        VERBOSE_OUT(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
        return NULL;
    }
    if(!image_desc){
        if(errcode_ret) *errcode_ret=CL_INVALID_IMAGE_DESCRIPTOR;
        VERBOSE_OUT(CL_INVALID_IMAGE_DESCRIPTOR);
        return NULL;
    }
    if((image_desc->buffer) && !hasMem(image_desc->buffer)){
        if(errcode_ret) *errcode_ret=CL_INVALID_IMAGE_DESCRIPTOR;
        VERBOSE_OUT(CL_INVALID_IMAGE_DESCRIPTOR);
        return NULL;
    }
    if(host_ptr && ( !(flags & CL_MEM_COPY_HOST_PTR) && !(flags & CL_MEM_USE_HOST_PTR) )){
        if(errcode_ret) *errcode_ret=CL_INVALID_HOST_PTR;
        VERBOSE_OUT(CL_INVALID_HOST_PTR);
        return NULL;
    }
    else if(!host_ptr && ( (flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_COPY_HOST_PTR) )){
        if(errcode_ret) *errcode_ret=CL_INVALID_HOST_PTR;
        VERBOSE_OUT(CL_INVALID_HOST_PTR);
        return NULL;
    }

    // Compute the element size of the image (which depends on the provided
    // format)
    if(    (image_format->image_channel_order == CL_RG)
        || (image_format->image_channel_order == CL_RGx)
        || (image_format->image_channel_order == CL_RA) )
    {
        element_n = 2;
    }
    else if(    (image_format->image_channel_order == CL_RGB)
             || (image_format->image_channel_order == CL_RGBx) )
    {
        element_n = 3;
    }
    else if(    (image_format->image_channel_order == CL_RGBA)
             || (image_format->image_channel_order == CL_ARGB)
             || (image_format->image_channel_order == CL_BGRA) )
    {
        element_n = 4;
    }
    if(    (image_format->image_channel_data_type == CL_SNORM_INT8)
        || (image_format->image_channel_data_type == CL_UNORM_INT8)
        || (image_format->image_channel_data_type == CL_SIGNED_INT8)
        || (image_format->image_channel_data_type == CL_UNSIGNED_INT8) )
    {
        element_size = 1*element_n;
    }
    else if(    (image_format->image_channel_data_type == CL_SNORM_INT16)
             || (image_format->image_channel_data_type == CL_UNORM_INT16)
             || (image_format->image_channel_data_type == CL_SIGNED_INT16)
             || (image_format->image_channel_data_type == CL_UNSIGNED_INT16)
             || (image_format->image_channel_data_type == CL_HALF_FLOAT) )
    {
        element_size = 2*element_n;
    }
    else if(    (image_format->image_channel_data_type == CL_SIGNED_INT32)
             || (image_format->image_channel_data_type == CL_UNSIGNED_INT32)
             || (image_format->image_channel_data_type == CL_FLOAT) )
    {
        element_size = 4*element_n;
    }
    else if(image_format->image_channel_data_type == CL_UNORM_SHORT_565)
    {
        element_size = (5+6+5)/8;
    }
    else if(image_format->image_channel_data_type == CL_UNORM_SHORT_555)
    {
        element_size = (1+5+5+5)/8;
    }
    else if(image_format->image_channel_data_type == CL_UNORM_INT_101010)
    {
        element_size = (2+10+10+10)/8;
    }
    cl_int flag;
    cl_mem mem = createImage(context,
                             flags,
                             image_format,
                             image_desc,
                             element_size,
                             host_ptr,
                             &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return mem;
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
icd_clCreateImage2D(cl_context              context ,
                    cl_mem_flags            flags ,
                    const cl_image_format * image_format ,
                    size_t                  image_width ,
                    size_t                  image_height ,
                    size_t                  image_row_pitch ,
                    void *                  host_ptr ,
                    cl_int *                errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    cl_int flag;
    cl_image_desc image_desc;
    VERBOSE_IN();
    image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
    image_desc.image_width = image_width;
    image_desc.image_height = image_height;
    image_desc.image_depth = 1;
    image_desc.image_array_size = 1;
    image_desc.image_row_pitch = image_row_pitch;
    image_desc.image_slice_pitch = image_row_pitch * image_height;
    image_desc.num_mip_levels = 0;
    image_desc.num_samples = 0;
    image_desc.buffer = NULL;
    cl_mem mem = icd_clCreateImage(context,
                                   flags,
                                   image_format,
                                   &image_desc,
                                   host_ptr,
                                   &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return mem;
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
icd_clCreateImage3D(cl_context              context,
                    cl_mem_flags            flags,
                    const cl_image_format * image_format,
                    size_t                  image_width,
                    size_t                  image_height ,
                    size_t                  image_depth ,
                    size_t                  image_row_pitch ,
                    size_t                  image_slice_pitch ,
                    void *                  host_ptr ,
                    cl_int *                errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    cl_int flag;
    cl_image_desc image_desc;
    VERBOSE_IN();
    image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
    image_desc.image_width = image_width;
    image_desc.image_height = image_height;
    image_desc.image_depth = image_depth;
    image_desc.image_array_size = 1;
    image_desc.image_row_pitch = image_row_pitch;
    image_desc.image_slice_pitch = image_slice_pitch;
    image_desc.num_mip_levels = 0;
    image_desc.num_samples = 0;
    image_desc.buffer = NULL;
    cl_mem mem = icd_clCreateImage(context,
                                   flags,
                                   image_format,
                                   &image_desc,
                                   host_ptr,
                                   &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return mem;
}

// --------------------------------------------------------------
// Samplers
// --------------------------------------------------------------

CL_API_ENTRY cl_sampler CL_API_CALL
icd_clCreateSampler(cl_context           context ,
                    cl_bool              normalized_coords ,
                    cl_addressing_mode   addressing_mode ,
                    cl_filter_mode       filter_mode ,
                    cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }

    cl_int flag;
    cl_sampler sampler = createSampler(context,
                                       normalized_coords,
                                       addressing_mode,
                                       filter_mode,
                                       &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return sampler;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainSampler(cl_sampler  sampler) CL_API_SUFFIX__VERSION_1_0
{
    cl_int flag;
    VERBOSE_IN();
    if(!hasSampler(sampler)){
        VERBOSE_OUT(CL_INVALID_SAMPLER);
        return CL_INVALID_SAMPLER;
    }
    flag = retainSampler(sampler);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseSampler(cl_sampler  sampler) CL_API_SUFFIX__VERSION_1_0
{
    cl_int flag;
    VERBOSE_IN();
    if(!hasSampler(sampler)){
        VERBOSE_OUT(CL_INVALID_SAMPLER);
        return CL_INVALID_SAMPLER;
    }
    flag = releaseSampler(sampler);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetSamplerInfo(cl_sampler          sampler ,
                     cl_sampler_info     param_name ,
                     size_t              param_value_size ,
                     void *              param_value ,
                     size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasSampler(sampler)){
        VERBOSE_OUT(CL_INVALID_SAMPLER);
        return CL_INVALID_SAMPLER;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    size_t size_ret = 0;
    void* value = NULL;
    if(param_name == CL_SAMPLER_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(sampler->rcount);
    }
    else if(param_name == CL_SAMPLER_CONTEXT){
        size_ret = sizeof(cl_context);
        value = &(sampler->context);
    }
    else if(param_name == CL_SAMPLER_NORMALIZED_COORDS){
        size_ret = sizeof(cl_bool);
        value = &(sampler->normalized_coords);
    }
    else if(param_name == CL_SAMPLER_ADDRESSING_MODE){
        size_ret = sizeof(cl_addressing_mode);
        value = &(sampler->addressing_mode);
    }
    else if(param_name == CL_SAMPLER_FILTER_MODE){
        size_ret = sizeof(cl_filter_mode);
        value = &(sampler->filter_mode);
    }
    else{
        cl_int flag = getSamplerInfo(sampler,
                                     param_name,
                                     param_value_size,
                                     param_value,
                                     param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

// --------------------------------------------------------------
// Programs
// --------------------------------------------------------------
CL_API_ENTRY cl_program CL_API_CALL
icd_clCreateProgramWithSource(cl_context         context ,
                              cl_uint            count ,
                              const char **      strings ,
                              const size_t *     lengths ,
                              cl_int *           errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    if((!count) || (!strings)){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    for(i = 0; i < count; i++){
        if(!strings[i]){
            if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
            VERBOSE_OUT(CL_INVALID_VALUE);
            return NULL;
        }
    }

    cl_int flag;
    cl_program program = createProgramWithSource(context,
                                                 count,
                                                 strings,
                                                 lengths,
                                                 &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return program;
}

CL_API_ENTRY cl_program CL_API_CALL
icd_clCreateProgramWithBinary(cl_context                      context ,
                              cl_uint                         num_devices ,
                              const cl_device_id *            device_list ,
                              const size_t *                  lengths ,
                              const unsigned char **          binaries ,
                              cl_int *                        binary_status ,
                              cl_int *                        errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    if(    (!num_devices) || (!device_list)
        || (!lengths) || (!binaries) ){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    for(i=0;i<num_devices;i++){
        if((!lengths[i]) || (!binaries[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
            VERBOSE_OUT(CL_INVALID_VALUE);
            return NULL;
        }
        if(!hasDevice(device_list[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return NULL;
        }
    }

    cl_int flag;
    cl_program program = createProgramWithBinary(context,
                                                 num_devices,
                                                 device_list,
                                                 lengths,
                                                 binaries,
                                                 binary_status,
                                                 &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return program;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainProgram(cl_program  program) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    cl_int flag;
    if(!hasProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    flag = retainProgram(program);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseProgram(cl_program  program) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    cl_int flag;
    if(!hasProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    flag = releaseProgram(program);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clBuildProgram(cl_program            program ,
                   cl_uint               num_devices ,
                   const cl_device_id *  device_list ,
                   const char *          options ,
                   void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                   void *                user_data) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    if(    ((!num_devices) && ( device_list))
        || (( num_devices) && (!device_list)) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    for(i = 0; i < num_devices; i++){
        if(!hasDevice(device_list[i])){
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return CL_INVALID_DEVICE;
        }
    }
    /** @todo Implement callbacks.
     *
     * For the moment we are calling the method in a blocking way, calling the
     * callback function immediately after that.
     * Implementing callback functions may be hard due to the requirement of
     * calling the update method (which requires the main stream to communicate
     * with the server).
     * An intermediate solution may be setting a flag which is reporting if an
     * update is required, calling the update method when the info is required.
     */
    if(pfn_notify || user_data){
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return CL_OUT_OF_HOST_MEMORY;
    }
    if((!pfn_notify  &&  user_data  ) ||
       ( num_devices && !device_list) ||
       (!num_devices &&  device_list) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = buildProgram(program,
                               num_devices,
                               device_list,
                               options,
                               NULL,
                               NULL);

    if(pfn_notify){
        pfn_notify(program, user_data);
    }

    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
icd_clUnloadCompiler(void) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    // Deprecated function which, according to old specifications, is always
    // returning CL_SUCCESS
    VERBOSE_IN();
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetProgramInfo(cl_program          program ,
                     cl_program_info     param_name ,
                     size_t              param_value_size ,
                     void *              param_value ,
                     size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    // The kernel already have all the requested data available
    size_t size_ret = 0;
    void* value = NULL;
    if(param_name == CL_PROGRAM_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(program->rcount);
    }
    else if(param_name == CL_PROGRAM_CONTEXT){
        size_ret = sizeof(cl_context);
        value = &(program->context);
    }
    else if(param_name == CL_PROGRAM_NUM_DEVICES){
        size_ret = sizeof(cl_uint);
        value = &(program->num_devices);
    }
    else if(param_name == CL_PROGRAM_DEVICES){
        size_ret = sizeof(cl_device_id) * program->num_devices;
        value = program->devices;
    }
    else if(param_name == CL_PROGRAM_SOURCE){
        size_ret = sizeof(char) * (strlen(program->source) + 1);
        value = program->source;
    }
    else if(param_name == CL_PROGRAM_BINARY_SIZES){
        size_ret = sizeof(size_t) * program->num_devices;
        value = program->binary_lengths;
    }
    else if(param_name == CL_PROGRAM_BINARIES){
        size_ret = sizeof(unsigned char*) * program->num_devices;
        value = program->binaries;
    }
    else if(param_name == CL_PROGRAM_NUM_KERNELS){
        if(!program->kernels){
            return CL_INVALID_PROGRAM_EXECUTABLE;
        }
        size_ret = sizeof(cl_uint);
        value = &(program->num_kernels);
    }
    else if(param_name == CL_PROGRAM_KERNEL_NAMES){
        if(!program->kernels){
            return CL_INVALID_PROGRAM_EXECUTABLE;
        }
        size_ret = sizeof(unsigned char*)*program->num_kernels;
        value = program->kernels;
    }
    else{
        // What are you asking for?? Anyway, lets see if the server knows the
        // answer
        cl_int flag = getProgramInfo(program,
                                     param_name,
                                     param_value_size,
                                     param_value,
                                     param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetProgramBuildInfo(cl_program             program ,
                          cl_device_id           device ,
                          cl_program_build_info  param_name ,
                          size_t                 param_value_size ,
                          void *                 param_value ,
                          size_t *               param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    if(!hasDevice(device)){
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return CL_INVALID_DEVICE;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = getProgramBuildInfo(program,
                                      device,
                                      param_name,
                                      param_value_size,
                                      param_value,
                                      param_value_size_ret);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_program CL_API_CALL
icd_clCreateProgramWithBuiltInKernels(cl_context             context ,
                                      cl_uint                num_devices ,
                                      const cl_device_id *   device_list ,
                                      const char *           kernel_names ,
                                      cl_int *               errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    cl_uint i;
    if(!hasContext(context)){
        if(errcode_ret) *errcode_ret=CL_INVALID_PROGRAM;
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return NULL;
    }
    if(!num_devices || !device_list || !kernel_names){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    for(i=0;i<num_devices;i++){
        if(!kernel_names[i]){
            if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
            VERBOSE_OUT(CL_INVALID_VALUE);
            return NULL;
        }
        if(!hasDevice(device_list[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return NULL;
        }
    }
    cl_int flag;
    cl_program program = createProgramWithBuiltInKernels(context,
                                                         num_devices,
                                                         device_list,
                                                         kernel_names,
                                                         &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return program;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clCompileProgram(cl_program            program ,
                     cl_uint               num_devices ,
                     const cl_device_id *  device_list ,
                     const char *          options ,
                     cl_uint               num_input_headers ,
                     const cl_program *    input_headers,
                     const char **         header_include_names ,
                     void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                     void *                user_data) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    cl_uint i;
    if(!hasProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    if(    ((!num_devices) && ( device_list))
        || (( num_devices) && (!device_list)) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(    ((!num_input_headers) && ( ( input_headers) || ( header_include_names)))
        || (( num_input_headers) && ( (!input_headers) || (!header_include_names)))){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    for(i = 0; i < num_devices; i++){
        if(!hasDevice(device_list[i])){
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return CL_INVALID_DEVICE;
        }
    }
    for(i = 0; i < num_input_headers; i++){
        if(!hasProgram(input_headers[i])){
            VERBOSE_OUT(CL_INVALID_PROGRAM);
            return CL_INVALID_PROGRAM;
        }
    }

    /** @todo Implement callbacks.
     *
     * For the moment we are calling the method in a blocking way, calling the
     * callback function immediately after that.
     * Implementing callback functions may be hard due to the requirement of
     * calling the update method (which requires the main stream to communicate
     * with the server).
     * An intermediate solution may be setting a flag which is reporting if an
     * update is required, calling the update method when the info is required.
     */
    if(pfn_notify || user_data){
        VERBOSE_OUT(CL_OUT_OF_RESOURCES);
        return CL_OUT_OF_RESOURCES;
    }
    if((!pfn_notify  &&  user_data  ) ||
       ( num_devices && !device_list) ||
       (!num_devices &&  device_list) ||
       (!num_input_headers && ( input_headers ||  header_include_names) ) ||
       ( num_input_headers && (!input_headers || !header_include_names) ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = compileProgram(program,
                                 num_devices,
                                 device_list,
                                 options,
                                 num_input_headers,
                                 input_headers,
                                 header_include_names,
                                 NULL,
                                 NULL);
    if(pfn_notify){
        pfn_notify(program, user_data);
    }
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_program CL_API_CALL
icd_clLinkProgram(cl_context            context ,
                  cl_uint               num_devices ,
                  const cl_device_id *  device_list ,
                  const char *          options ,
                  cl_uint               num_input_programs ,
                  const cl_program *    input_programs ,
                  void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                  void *                user_data ,
                  cl_int *              errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    cl_int flag;
    cl_uint i;
    if(!hasContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    if(    ((!num_devices) && ( device_list))
        || (( num_devices) && (!device_list)) ){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    if(    ((!num_input_programs) && ( input_programs))
        || ((!num_input_programs) && ( input_programs))){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    for(i = 0; i < num_devices; i++){
        if(!hasDevice(device_list[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return NULL;
        }
    }
    for(i = 0; i < num_input_programs; i++){
        if(!hasProgram(input_programs[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_PROGRAM;
            VERBOSE_OUT(CL_INVALID_PROGRAM);
            return NULL;
        }
    }

    /** @todo Implement callbacks.
     *
     * For the moment we are calling the method in a blocking way, calling the
     * callback function immediately after that.
     * Implementing callback functions may be hard due to the requirement of
     * calling the update method (which requires the main stream to communicate
     * with the server).
     * An intermediate solution may be setting a flag which is reporting if an
     * update is required, calling the update method when the info is required.
     */
    if(pfn_notify || user_data){
        if(errcode_ret) *errcode_ret=CL_OUT_OF_RESOURCES;
        VERBOSE_OUT(CL_OUT_OF_RESOURCES);
        return NULL;
    }
    if((!pfn_notify  &&  user_data  ) ||
       ( num_devices && !device_list) ||
       (!num_devices &&  device_list) ||
       (!num_input_programs || !input_programs) ){
        if(errcode_ret) *errcode_ret=CL_OUT_OF_RESOURCES;
        VERBOSE_OUT(CL_OUT_OF_RESOURCES);
        return NULL;
    }
    cl_program program = linkProgram(context,
                                     num_devices,
                                     device_list,
                                     options,
                                     num_input_programs,
                                     input_programs,
                                     NULL,
                                     NULL,
                                     &flag);
    if(errcode_ret) *errcode_ret = flag;
    if(pfn_notify){
        pfn_notify(program, user_data);
    }
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return program;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clUnloadPlatformCompiler(cl_platform_id  platform) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    cl_int flag = oclandUnloadPlatformCompiler(platform);
    VERBOSE_OUT(flag);
    return flag;
}

// --------------------------------------------------------------
// Kernels
// --------------------------------------------------------------

CL_API_ENTRY cl_kernel CL_API_CALL
icd_clCreateKernel(cl_program       program ,
                   const char *     kernel_name ,
                   cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasProgram(program)){
        if(errcode_ret) *errcode_ret = CL_INVALID_PROGRAM;
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return NULL;
    }
    if(!kernel_name){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }

    cl_int flag;
    cl_kernel kernel = createKernel(program, kernel_name, &flag);
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    if(flag != CL_SUCCESS){
        return NULL;
    }
    return kernel;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clCreateKernelsInProgram(cl_program      program ,
                             cl_uint         num_kernels ,
                             cl_kernel *     kernels ,
                             cl_uint *       num_kernels_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint n;
    VERBOSE_IN();
    if(!hasProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    if(    ( !kernels && !num_kernels_ret )
        || ( !kernels &&  num_kernels )
        || (  kernels && !num_kernels ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = createKernelsInProgram(program,
                                         num_kernels,
                                         kernels,
                                         &n);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    if(num_kernels_ret)
        *num_kernels_ret = n;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainKernel(cl_kernel     kernel) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    cl_int flag = retainKernel(kernel);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseKernel(cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    cl_int flag = releaseKernel(kernel);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clSetKernelArg(cl_kernel     kernel ,
                   cl_uint       arg_index ,
                   size_t        arg_size ,
                   const void *  arg_value) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    if(arg_index >= kernel->num_args){
        VERBOSE_OUT(CL_INVALID_ARG_INDEX);
        return CL_INVALID_ARG_INDEX;
    }
    void *val = (void*)arg_value;

    // Test if the passed argument is the same memory already set
    cl_kernel_arg arg = kernel->args[arg_index];
    if((arg->is_set == CL_TRUE) && (arg_size == arg->bytes)){
        if(!val && !arg->value){
            // Local memory (or NULL general object), already set
            VERBOSE_OUT(CL_SUCCESS);
            return CL_SUCCESS;
        }
    }
    // We are not heuristically checking if the argument is either a cl_mem or a
    // cl_sampler anymore, but we are using the arguments recovered data, which
    // are making OpenCL 1.2 non-supported platforms fail. However it is not
    // safe to try to do it in a different way.
    cl_kernel_arg_address_qualifier address;
    cl_int flag = icd_clGetKernelArgInfo(kernel,
                                         arg_index,
                                         CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                                         sizeof(cl_kernel_arg_address_qualifier),
                                         &address,
                                         NULL);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(CL_OUT_OF_RESOURCES);
        VERBOSE("ERROR: clGetKernelArgInfo (CL_KERNEL_ARG_ADDRESS_QUALIFIER) failed!\n");
        return CL_OUT_OF_RESOURCES;
    }
    char *arg_type_name = NULL;
    size_t arg_type_name_size = 0;
    flag = icd_clGetKernelArgInfo(kernel,
                                  arg_index,
                                  CL_KERNEL_ARG_TYPE_NAME,
                                  0,
                                  NULL,
                                  &arg_type_name_size);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(CL_OUT_OF_RESOURCES);
        VERBOSE("ERROR: clGetKernelArgInfo (CL_KERNEL_ARG_TYPE_NAME) failed!\n");
        return CL_OUT_OF_RESOURCES;
    }
    arg_type_name = (char*)malloc(arg_type_name_size);
    if(!arg_type_name){
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return CL_OUT_OF_HOST_MEMORY;
    }
    flag = icd_clGetKernelArgInfo(kernel,
                                  arg_index,
                                  CL_KERNEL_ARG_TYPE_NAME,
                                  arg_type_name_size,
                                  arg_type_name,
                                  NULL);
    if(flag != CL_SUCCESS){
        free(arg_type_name);
        VERBOSE_OUT(CL_OUT_OF_RESOURCES);
        VERBOSE("ERROR: clGetKernelArgInfo (CL_KERNEL_ARG_TYPE_NAME) failed!\n");
        return CL_OUT_OF_RESOURCES;
    }

    // There are 3 special cases where the references to the object should be
    // replaced:
    // cl_sampler, detected by the CL_KERNEL_ARG_TYPE_NAME
    // __constant cl_mem object, detected by the CL_KERNEL_ARG_ADDRESS_QUALIFIER
    // __global cl_mem object, detected by the CL_KERNEL_ARG_ADDRESS_QUALIFIER
    if(!strcmp(arg_type_name, "sampler_t")){
        if(arg_size != sizeof(cl_sampler)){
            free(arg_type_name);
            VERBOSE_OUT(CL_INVALID_ARG_SIZE);
            return CL_INVALID_ARG_SIZE;
        }
        cl_sampler sampler = *(cl_sampler*)(arg_value);
        if(hasSampler(sampler)){
            val = (void*)(&(sampler->ptr));
        }
    }
    else if((address == CL_KERNEL_ARG_ADDRESS_GLOBAL) ||
            (address == CL_KERNEL_ARG_ADDRESS_CONSTANT))
    {
        if(arg_size != sizeof(cl_mem)){
            free(arg_type_name);
            VERBOSE_OUT(CL_INVALID_ARG_SIZE);
            return CL_INVALID_ARG_SIZE;
        }
        cl_mem mem_obj = *(cl_mem*)(arg_value);
        if(hasMem(mem_obj)){
            val = (void*)(&(mem_obj->ptr));
        }
    }
    free(arg_type_name);

    // Have real sampler and buffer objects pointer now, check if the passed
    // argument is the same already set
    if((arg->is_set == CL_TRUE) && (arg_size == arg->bytes)){
        if(!memcmp(val, arg->value, arg_size)){
            // already set
            VERBOSE_OUT(CL_SUCCESS);
            return CL_SUCCESS;
        }
    }

    flag = setKernelArg(kernel,
                        arg_index,
                        arg_size,
                        val);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    arg->bytes = arg_size;
    if(!val){
        // Local memory
        arg->value = (void *)val;
        arg->is_set = CL_TRUE;
        VERBOSE_OUT(flag);
        return flag;
    }
    if (arg->is_set && arg->value) {
        // release previous set argument value
        free(arg->value); arg->value = NULL;
        arg->is_set = CL_FALSE;
    }
    // And set the new one
    arg->value = malloc(arg_size);
    if(!arg->value){
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return CL_OUT_OF_HOST_MEMORY;
    }
    memcpy(arg->value, val, arg_size);
    arg->is_set = CL_TRUE;

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetKernelInfo(cl_kernel        kernel ,
                    cl_kernel_info   param_name ,
                    size_t           param_value_size ,
                    void *           param_value ,
                    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    // The kernel already have all the requested data available
    size_t size_ret = 0;
    void* value = NULL;
    if(param_name == CL_KERNEL_FUNCTION_NAME){
        size_ret = sizeof(char)*(strlen(kernel->func_name)+1);
        value = kernel->func_name;
    }
    else if(param_name == CL_KERNEL_NUM_ARGS){
        size_ret = sizeof(cl_uint);
        value = &(kernel->num_args);
    }
    else if(param_name == CL_KERNEL_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(kernel->rcount);
    }
    else if(param_name == CL_KERNEL_CONTEXT){
        size_ret = sizeof(cl_context);
        value = &(kernel->context);
    }
    else if(param_name == CL_KERNEL_PROGRAM){
        size_ret = sizeof(cl_program);
        value = &(kernel->program);
    }
    else if(param_name == CL_KERNEL_ATTRIBUTES){
        if(!kernel->attributes){
            return CL_INVALID_VALUE;
        }
        size_ret = sizeof(char) * (strlen(kernel->attributes) + 1);
        value = kernel->attributes;
    }
    else{
        // What are you asking for?
        // Anyway, let's see if the server knows it (>1.2 compatibility)
        cl_int flag = getKernelInfo(kernel,
                                    param_name,
                                    param_value_size,
                                    param_value,
                                    param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetKernelWorkGroupInfo(cl_kernel                   kernel ,
                             cl_device_id                device ,
                             cl_kernel_work_group_info   param_name ,
                             size_t                      param_value_size ,
                             void *                      param_value ,
                             size_t *                    param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    if(!hasDevice(device)){
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return CL_INVALID_DEVICE;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = getKernelWorkGroupInfo(kernel,
                                         device,
                                         param_name,
                                         param_value_size,
                                         param_value,
                                         param_value_size_ret);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetKernelArgInfo(cl_kernel        kernel ,
                       cl_uint          arg_indx ,
                       cl_kernel_arg_info   param_name ,
                       size_t           param_value_size ,
                       void *           param_value ,
                       size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    if(!hasKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    if(arg_indx >= kernel->num_args){
        VERBOSE_OUT(CL_INVALID_ARG_INDEX);
        return CL_INVALID_ARG_INDEX;
    }
    if(    (  param_value_size && !param_value )
        || ( !param_value_size &&  param_value ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!param_value && !param_value_size_ret ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    size_t size_ret = 0;
    void* value = NULL;
    cl_kernel_arg arg = kernel->args[arg_indx];
    if(param_name == CL_KERNEL_ARG_ADDRESS_QUALIFIER){
        // Address qualifier is mandatory data, and therefore we have discarded
        // all non-complaint kernels before
        size_ret = sizeof(cl_kernel_arg_address_qualifier);
        value = &(arg->address);
    }
    else if(param_name == CL_KERNEL_ARG_ACCESS_QUALIFIER){
        if(arg->access_available == CL_FALSE){
            VERBOSE_OUT(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }
        size_ret = sizeof(cl_kernel_arg_access_qualifier);
        value = &(arg->access);
    }
    else if(param_name == CL_KERNEL_ARG_TYPE_NAME){
        if(arg->type_name_available == CL_FALSE){
            VERBOSE_OUT(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }
        size_ret = sizeof(char)*(strlen(arg->type_name) + 1);
        value = arg->type_name;
    }
    else if(param_name == CL_KERNEL_ARG_TYPE_QUALIFIER){
        if(arg->type_available == CL_FALSE){
            VERBOSE_OUT(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }
        size_ret = sizeof(cl_kernel_arg_type_qualifier);
        value = &(arg->type);
    }
    else if(param_name == CL_KERNEL_ARG_NAME){
        if(arg->name_available == CL_FALSE){
            VERBOSE_OUT(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }
        size_ret = sizeof(char) * (strlen(arg->name) + 1);
        value = arg->name;
    }
    else{
        // What are you asking for?
        // Anyway, let's see if the server knows it (>1.2 compatibility)
        cl_int flag = getKernelArgInfo(kernel,
                                       arg_indx,
                                       param_name,
                                       param_value_size,
                                       param_value,
                                       param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

// --------------------------------------------------------------
// Events
// --------------------------------------------------------------

CL_API_ENTRY cl_int CL_API_CALL
icd_clWaitForEvents(cl_uint              num_events ,
                    const cl_event *     event_list) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    cl_uint i;
    if(!num_events || !event_list){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    for(i=0;i<num_events;i++){
        if(!isEvent(event_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT);
            return CL_INVALID_EVENT;
        }
        if(event_list[i]->context != event_list[0]->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandWaitForEvents(num_events,event_list);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetEventInfo(cl_event          event ,
                   cl_event_info     param_name ,
                   size_t            param_value_size ,
                   void *            param_value ,
                   size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isEvent(event)){
        VERBOSE_OUT(CL_INVALID_EVENT);
        return CL_INVALID_EVENT;
    }
    // Directly answer for known data
    if(param_name == CL_EVENT_COMMAND_QUEUE){
        if( (param_value_size < sizeof(cl_command_queue)) && (param_value)){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        if(param_value_size_ret) *param_value_size_ret = sizeof(cl_command_queue);
        if(param_value) memcpy(param_value, &(event->command_queue), sizeof(cl_command_queue));
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    if(param_name == CL_EVENT_CONTEXT){
        if( (param_value_size < sizeof(cl_context)) && (param_value)){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        if(param_value_size_ret) *param_value_size_ret = sizeof(cl_context);
        if(param_value) memcpy(param_value, &(event->context), sizeof(cl_context));
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    if(param_name == CL_EVENT_COMMAND_TYPE){
        if( (param_value_size < sizeof(cl_command_type)) && (param_value)){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        if(param_value_size_ret) *param_value_size_ret = sizeof(cl_command_type);
        if(param_value) memcpy(param_value, &(event->command_type), sizeof(cl_command_type));
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    if(param_name == CL_EVENT_REFERENCE_COUNT){
        if( (param_value_size < sizeof(cl_uint)) && (param_value)){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        if(param_value_size_ret) *param_value_size_ret = sizeof(cl_uint);
        if(param_value) memcpy(param_value, &(event->rcount), sizeof(cl_uint));
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    // Ask the server
    cl_int flag = oclandGetEventInfo(event,param_name,param_value_size,param_value,param_value_size_ret);

    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainEvent(cl_event  event) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isEvent(event)){
        VERBOSE_OUT(CL_INVALID_EVENT);
        return CL_INVALID_EVENT;
    }
    // Simply increase the number of references to this object
    pthread_mutex_lock(&(event->rcount_mutex));
    event->rcount++;
    pthread_mutex_unlock(&(event->rcount_mutex));
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseEvent(cl_event  event) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isEvent(event)){
        VERBOSE_OUT(CL_INVALID_EVENT);
        return CL_INVALID_EVENT;
    }
    // Decrease the number of references to this object
    pthread_mutex_lock(&(event->rcount_mutex));
    event->rcount--;
    pthread_mutex_unlock(&(event->rcount_mutex));
    if(event->rcount){
        // There are some active references to the object, so we must retain it
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }

    cl_uint i;
    cl_int flag = oclandReleaseEvent(event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    free(event);
    for(i=0;i<num_master_events;i++){
        if(master_events[i] == event){
            // Create a new array removing the selected one
            cl_event *backup = master_events;
            master_events = NULL;
            if(num_master_events-1)
                master_events = (cl_event*)malloc((num_master_events-1)*sizeof(cl_event));
            memcpy(master_events, backup, i*sizeof(cl_event));
            memcpy(master_events+i, backup+i+1, (num_master_events-1-i)*sizeof(cl_event));
            free(backup);
            break;
        }
    }
    num_master_events--;
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetEventProfilingInfo(cl_event             event ,
                            cl_profiling_info    param_name ,
                            size_t               param_value_size ,
                            void *               param_value ,
                            size_t *             param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isEvent(event)){
        VERBOSE_OUT(CL_INVALID_EVENT);
        return CL_INVALID_EVENT;
    }
    cl_int flag = oclandGetEventProfilingInfo(event,param_name,param_value_size,param_value,param_value_size_ret);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_event CL_API_CALL
icd_clCreateUserEvent(cl_context     context,
                      cl_int *       errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    VERBOSE_IN();
    if(!hasContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    cl_event event = (cl_event)malloc(sizeof(struct _cl_event));
    if(!event){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    cl_int flag;
    event->dispatch = &master_dispatch;
    event->ptr = oclandCreateUserEvent(context,&flag);
    event->rcount = 1;
    pthread_mutex_init(&(event->rcount_mutex), NULL);
    event->socket = context->server->socket;
    event->command_queue = NULL;
    event->context = context;
    event->command_type = CL_COMMAND_USER;
    // Create a new array appending the new one
    cl_event *backup = master_events;
    num_master_events++;
    master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
    memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
    free(backup);
    master_events[num_master_events-1] = event;
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return event;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clSetUserEventStatus(cl_event    event ,
                         cl_int      execution_status) CL_API_SUFFIX__VERSION_1_1
{
    VERBOSE_IN();
    if(!isEvent(event)){
        VERBOSE_OUT(CL_INVALID_EVENT);
        return CL_INVALID_EVENT;
    }
    cl_int flag = oclandSetUserEventStatus(event,execution_status);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clSetEventCallback(cl_event     event ,
                       cl_int       command_exec_callback_type ,
                       void (CL_CALLBACK *  pfn_notify)(cl_event, cl_int, void *),
                       void *       user_data) CL_API_SUFFIX__VERSION_1_1
{
    VERBOSE_IN();
    if(!isEvent(event)){
        VERBOSE_OUT(CL_INVALID_EVENT);
        return CL_INVALID_EVENT;
    }
    if(!pfn_notify || (command_exec_callback_type != CL_COMPLETE)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    /** Callbacks can't be registered in ocland due
     * to the implicit network interface, so this
     * operation may fail ever.
     */
    VERBOSE_OUT(CL_INVALID_EVENT);
    return CL_INVALID_EVENT;
}

// --------------------------------------------------------------
// Enqueues
// --------------------------------------------------------------

CL_API_ENTRY cl_int CL_API_CALL
icd_clFlush(cl_command_queue  command_queue) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    cl_int flag = oclandFlush(command_queue);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clFinish(cl_command_queue  command_queue) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    cl_int flag = oclandFinish(command_queue);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueReadBuffer(cl_command_queue     command_queue ,
                        cl_mem               buffer ,
                        cl_bool              blocking_read ,
                        size_t               offset ,
                        size_t               cb ,
                        void *               ptr ,
                        cl_uint              num_events_in_wait_list ,
                        const cl_event *     event_wait_list ,
                        cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if( (!ptr) || (buffer->size < offset+cb) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(command_queue->context != buffer->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueReadBuffer(command_queue,buffer,
                                          blocking_read,offset,cb,ptr,
                                          num_events_in_wait_list,
                                          event_wait_list, event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_READ_BUFFER;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueWriteBuffer(cl_command_queue    command_queue ,
                         cl_mem              buffer ,
                         cl_bool             blocking_write ,
                         size_t              offset ,
                         size_t              cb ,
                         const void *        ptr ,
                         cl_uint             num_events_in_wait_list ,
                         const cl_event *    event_wait_list ,
                         cl_event *          event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if( (!ptr) || (buffer->size < offset+cb) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(command_queue->context != buffer->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueWriteBuffer(command_queue,buffer,
                                           blocking_write,offset,cb,ptr,
                                           num_events_in_wait_list,
                                           event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_WRITE_BUFFER;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueCopyBuffer(cl_command_queue     command_queue ,
                        cl_mem               src_buffer ,
                        cl_mem               dst_buffer ,
                        size_t               src_offset ,
                        size_t               dst_offset ,
                        size_t               cb ,
                        cl_uint              num_events_in_wait_list ,
                        const cl_event *     event_wait_list ,
                        cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(src_buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!hasMem(dst_buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    (src_buffer->size < src_offset+cb)
        || (dst_buffer->size < dst_offset+cb)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(    (command_queue->context != src_buffer->context)
        || (command_queue->context != dst_buffer->context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueCopyBuffer(command_queue, src_buffer,dst_buffer,
                                          src_offset,dst_offset,cb,
                                          num_events_in_wait_list,
                                          event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_COPY_BUFFER;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueReadImage(cl_command_queue      command_queue ,
                       cl_mem                image ,
                       cl_bool               blocking_read ,
                       const size_t *        origin ,
                       const size_t *        region ,
                       size_t                row_pitch ,
                       size_t                slice_pitch ,
                       void *                ptr ,
                       cl_uint               num_events_in_wait_list ,
                       const cl_event *      event_wait_list ,
                       cl_event *            event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    (!ptr)
        || (!origin)
        || (!region) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(command_queue->context != image->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    // Correct some values if they are not provided
    if(!row_pitch)
        row_pitch   = region[0];
    if(!slice_pitch)
        slice_pitch = region[1]*row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (row_pitch   < region[0])
       || (slice_pitch < region[1]*row_pitch)
       || (slice_pitch % row_pitch)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = oclandEnqueueReadImage(command_queue,image,
                                         blocking_read,origin,region,
                                         row_pitch,slice_pitch,
                                         image->element_size,ptr,
                                         num_events_in_wait_list,
                                         event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_READ_IMAGE;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueWriteImage(cl_command_queue     command_queue ,
                        cl_mem               image ,
                        cl_bool              blocking_write ,
                        const size_t *       origin ,
                        const size_t *       region ,
                        size_t               row_pitch ,
                        size_t               slice_pitch ,
                        const void *         ptr ,
                        cl_uint              num_events_in_wait_list ,
                        const cl_event *     event_wait_list ,
                        cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    (!ptr)
        || (!origin)
        || (!region) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(command_queue->context != image->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    // Correct some values if they are not provided
    if(!row_pitch)
        row_pitch   = region[0];
    if(!slice_pitch)
        slice_pitch = region[1]*row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (row_pitch   < region[0])
       || (slice_pitch < region[1]*row_pitch)
       || (slice_pitch % row_pitch)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = oclandEnqueueWriteImage(command_queue,image,
                                          blocking_write,origin,region,
                                          row_pitch,slice_pitch,
                                          image->element_size,ptr,
                                          num_events_in_wait_list,
                                          event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_WRITE_IMAGE;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueCopyImage(cl_command_queue      command_queue ,
                       cl_mem                src_image ,
                       cl_mem                dst_image ,
                       const size_t *        src_origin ,
                       const size_t *        dst_origin ,
                       const size_t *        region ,
                       cl_uint               num_events_in_wait_list ,
                       const cl_event *      event_wait_list ,
                       cl_event *            event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(src_image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!hasMem(dst_image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    (!src_origin)
        || (!dst_origin)
        || (!region)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(    (command_queue->context != src_image->context)
        || (command_queue->context != dst_image->context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueCopyImage(command_queue,
                                         src_image,dst_image,
                                         src_origin,dst_origin,region,
                                         num_events_in_wait_list,
                                         event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_COPY_IMAGE;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueCopyImageToBuffer(cl_command_queue  command_queue ,
                               cl_mem            src_image ,
                               cl_mem            dst_buffer ,
                               const size_t *    src_origin ,
                               const size_t *    region ,
                               size_t            dst_offset ,
                               cl_uint           num_events_in_wait_list ,
                               const cl_event *  event_wait_list ,
                               cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(src_image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!hasMem(dst_buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(   (!src_origin)
       || (!region)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(    (command_queue->context != src_image->context)
        || (command_queue->context != dst_buffer->context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueCopyImageToBuffer(command_queue,
                                                 src_image,dst_buffer,
                                                 src_origin,region,dst_offset,
                                                 num_events_in_wait_list,
                                                 event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_COPY_IMAGE_TO_BUFFER;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueCopyBufferToImage(cl_command_queue  command_queue ,
                               cl_mem            src_buffer ,
                               cl_mem            dst_image ,
                               size_t            src_offset ,
                               const size_t *    dst_origin ,
                               const size_t *    region ,
                               cl_uint           num_events_in_wait_list ,
                               const cl_event *  event_wait_list ,
                               cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(src_buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!hasMem(dst_image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(   (!dst_origin)
       || (!region)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(    (command_queue->context != src_buffer->context)
        || (command_queue->context != dst_image->context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueCopyBufferToImage(command_queue,
                                                 src_buffer,dst_image,
                                                 src_offset,dst_origin,region,
                                                 num_events_in_wait_list,
                                                 event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_COPY_BUFFER_TO_IMAGE;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY void * CL_API_CALL
icd_clEnqueueMapBuffer(cl_command_queue  command_queue ,
                       cl_mem            buffer ,
                       cl_bool           blocking_map ,
                       cl_map_flags      map_flags ,
                       size_t            offset ,
                       size_t            cb ,
                       cl_uint           num_events_in_wait_list ,
                       const cl_event *  event_wait_list ,
                       cl_event *        event ,
                       cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        if(errcode_ret) *errcode_ret = CL_INVALID_COMMAND_QUEUE;
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return NULL;
    }
    if(!hasMem(buffer)){
        if(errcode_ret) *errcode_ret = CL_INVALID_MEM_OBJECT;
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return NULL;
    }
    if(buffer->size < offset+cb){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    if(    !(map_flags & CL_MAP_READ)
        && !(map_flags & CL_MAP_WRITE)
        && !(map_flags & CL_MAP_WRITE_INVALIDATE_REGION)){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    if(command_queue->context != buffer->context){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        if(errcode_ret) *errcode_ret = CL_INVALID_EVENT_WAIT_LIST;
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return NULL;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_EVENT_WAIT_LIST;
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return NULL;
        }
        if(event_wait_list[i]->context != command_queue->context){
            if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return NULL;
        }
    }
    void *host_ptr = NULL;
    if(buffer->host_ptr)
        host_ptr = (char*)(buffer->host_ptr) + offset;
    if(!host_ptr){
        host_ptr = malloc(cb);
        if(!host_ptr){
            if(errcode_ret) *errcode_ret = CL_MAP_FAILURE;
            VERBOSE_OUT(CL_MAP_FAILURE);
            return NULL;
        }
    }

    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    if(map_flags & CL_MAP_WRITE_INVALIDATE_REGION){
        // If CL_MAP_WRITE_INVALIDATE_REGION has been set, we must not guarantee
        // that the data inside the returned pointer matchs with the data allocated
        // in the device, and therefore we dont need to do nothing but to create an
        // event if the user has requested it
        if(event){
            cl_int flag;
            if(num_events_in_wait_list){
                flag = clWaitForEvents(num_events_in_wait_list, event_wait_list);
                if(flag != CL_SUCCESS){
                    if(errcode_ret) *errcode_ret = flag;
                    VERBOSE_OUT(flag);
                    return NULL;
                }
            }
            *event = clCreateUserEvent(command_queue->context, &flag);
            if(flag != CL_SUCCESS){
                if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
                VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
                return NULL;
            }
            flag = oclandSetUserEventStatus(*event,CL_COMPLETE);
            if(flag != CL_SUCCESS){
                if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
                VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
                return NULL;
            }
        }
    }
    else{
        // Otherwise the memory must be start readed right now. Since a lot of
        // network traffic is required for this operation, it could be expected to
        // be several times slower than the device data reading, so we can use
        // clEnqueueReadBuffer without a significative performance lost.
        cl_int flag;
        flag = clEnqueueReadBuffer(command_queue, buffer, blocking_map, offset,
                                   cb, host_ptr, num_events_in_wait_list,
                                   event_wait_list, event);
        if(flag != CL_SUCCESS){
            if(errcode_ret) *errcode_ret = flag;
            VERBOSE_OUT(flag);
            return NULL;
        }
    }

    cl_map mapobj = (cl_map)malloc(sizeof(struct _cl_map));
    mapobj->map_flags = map_flags;
    mapobj->blocking = blocking_map;
    mapobj->type = CL_MEM_OBJECT_BUFFER;
    mapobj->mapped_ptr = host_ptr;
    mapobj->offset = offset;
    mapobj->cb = cb;
    mapobj->origin[0] = 0; mapobj->origin[1] = 0; mapobj->origin[2] = 0;
    mapobj->region[0] = 0; mapobj->region[1] = 0; mapobj->region[2] = 0;
    mapobj->row_pitch = 0;
    mapobj->slice_pitch = 0;

    cl_map *backup = buffer->maps;
    buffer->map_count++;
    buffer->maps = (cl_map*)malloc(buffer->map_count*sizeof(cl_map));
    memcpy(buffer->maps, backup, (buffer->map_count-1)*sizeof(cl_map));
    free(backup);
    buffer->maps[buffer->map_count-1] = mapobj;

    VERBOSE_OUT(CL_SUCCESS);
    return mapobj->mapped_ptr;
}

CL_API_ENTRY void * CL_API_CALL
icd_clEnqueueMapImage(cl_command_queue   command_queue ,
                      cl_mem             image ,
                      cl_bool            blocking_map ,
                      cl_map_flags       map_flags ,
                      const size_t *     origin ,
                      const size_t *     region ,
                      size_t *           image_row_pitch ,
                      size_t *           image_slice_pitch ,
                      cl_uint            num_events_in_wait_list ,
                      const cl_event *   event_wait_list ,
                      cl_event *         event ,
                      cl_int *           errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    size_t ptr_orig = 0, ptr_size = 0;
    size_t row_pitch = 0;
    size_t slice_pitch = 0;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        if(errcode_ret) *errcode_ret = CL_INVALID_COMMAND_QUEUE;
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return NULL;
    }
    if(!hasMem(image)){
        if(errcode_ret) *errcode_ret = CL_INVALID_MEM_OBJECT;
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return NULL;
    }
    if(    (image->type != CL_MEM_OBJECT_IMAGE1D)
        && (image->type != CL_MEM_OBJECT_IMAGE1D_BUFFER)
        && (image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY)
        && (image->type != CL_MEM_OBJECT_IMAGE2D)
        && (image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY)
        && (image->type != CL_MEM_OBJECT_IMAGE3D)){
        if(errcode_ret) *errcode_ret = CL_INVALID_MEM_OBJECT;
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return NULL;
    }
    if(    !(map_flags & CL_MAP_READ)
        && !(map_flags & CL_MAP_WRITE)
        && !(map_flags & CL_MAP_WRITE_INVALIDATE_REGION)){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    if(!image_row_pitch){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    if(command_queue->context != image->context){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        if(errcode_ret) *errcode_ret = CL_INVALID_EVENT_WAIT_LIST;
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return NULL;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_EVENT_WAIT_LIST;
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return NULL;
        }
        if(event_wait_list[i]->context != command_queue->context){
            if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return NULL;
        }
    }

    row_pitch = *image_row_pitch;
    if(image_slice_pitch)
        slice_pitch = *image_slice_pitch;

    ptr_orig = slice_pitch * origin[2] + row_pitch * origin[1] + origin[0];
    ptr_size = slice_pitch * region[2] + row_pitch * region[1] + region[0];

    void *host_ptr = NULL;
    if(image->host_ptr)
        host_ptr = (char*)(image->host_ptr) + ptr_orig;
    if(!host_ptr){
        host_ptr = malloc(ptr_size);
        if(!host_ptr){
            if(errcode_ret) *errcode_ret = CL_MAP_FAILURE;
            VERBOSE_OUT(CL_MAP_FAILURE);
            return NULL;
        }
    }

    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    if(map_flags & CL_MAP_WRITE_INVALIDATE_REGION){
        // If CL_MAP_WRITE_INVALIDATE_REGION has been set, we must not guarantee
        // that the data inside the returned pointer matchs with the data allocated
        // in the device, and therefore we dont need to do nothing but to create an
        // event if the user has requested it
        if(event){
            cl_int flag;
            if(num_events_in_wait_list){
                flag = clWaitForEvents(num_events_in_wait_list, event_wait_list);
                if(flag != CL_SUCCESS){
                    if(errcode_ret) *errcode_ret = flag;
                    VERBOSE_OUT(flag);
                    return NULL;
                }
            }
            *event = clCreateUserEvent(command_queue->context, &flag);
            if(flag != CL_SUCCESS){
                if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
                VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
                return NULL;
            }
            flag = oclandSetUserEventStatus(*event,CL_COMPLETE);
            if(flag != CL_SUCCESS){
                if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
                VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
                return NULL;
           }
        }
    }
    else{
        // Otherwise the memory must be start readed right now. Since a lot of
        // network traffic is required for this operation, it could be expected to
        // be several times slower than the device data reading, so we can use
        // clEnqueueReadImage without a significative performance lost.
        cl_int flag;
        flag =  clEnqueueReadImage(command_queue, image, blocking_map, origin,
                                   region, row_pitch, slice_pitch, host_ptr,
                                   num_events_in_wait_list, event_wait_list,
                                   event);
        if(flag != CL_SUCCESS){
            if(errcode_ret) *errcode_ret = flag;
            VERBOSE_OUT(flag);
            return NULL;
        }
    }

    cl_map mapobj = (cl_map)malloc(sizeof(struct _cl_map));
    mapobj->map_flags = map_flags;
    mapobj->blocking = blocking_map;
    mapobj->type = CL_MEM_OBJECT_IMAGE3D;
    mapobj->mapped_ptr = host_ptr;
    mapobj->offset = 0;
    mapobj->cb = 0;
    memcpy(mapobj->origin,origin,3*sizeof(size_t));
    memcpy(mapobj->region,region,3*sizeof(size_t));
    mapobj->row_pitch = row_pitch;
    mapobj->slice_pitch = slice_pitch;

    cl_map *backup = image->maps;
    image->map_count++;
    image->maps = (cl_map*)malloc(image->map_count*sizeof(cl_map));
    memcpy(image->maps, backup, (image->map_count-1)*sizeof(cl_map));
    free(backup);
    image->maps[image->map_count-1] = mapobj;

    VERBOSE_OUT(CL_SUCCESS);
    return mapobj->mapped_ptr;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueUnmapMemObject(cl_command_queue  command_queue ,
                            cl_mem            memobj ,
                            void *            mapped_ptr ,
                            cl_uint           num_events_in_wait_list ,
                            const cl_event *   event_wait_list ,
                            cl_event *         event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    cl_map mapobj = NULL;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(memobj)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(command_queue->context != memobj->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    for(i=0;i<memobj->map_count;i++){
        if(mapped_ptr == memobj->maps[i]->mapped_ptr){
            mapobj = memobj->maps[i];
            break;
        }
    }
    if(!mapobj){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }

    cl_map_flags map_flags = mapobj->map_flags;
    // In the reading case the work has been already done, so we just need to
    // wait for the requested events, and generate a completed event if it has
    // been requested by the user.
    if( (map_flags & CL_MAP_READ) && event){
        // If writing is added to the map flags too, the reading event will be
        // ever less restrictive than the writting one, otherwise we must
        // generate a completed event
        if(!(map_flags & CL_MAP_WRITE)){
            cl_int flag;
            if(num_events_in_wait_list){
                flag = clWaitForEvents(num_events_in_wait_list, event_wait_list);
                if(flag != CL_SUCCESS){
                    VERBOSE_OUT(flag);
                    return flag;
                }
            }
            *event = clCreateUserEvent(command_queue->context, &flag);
            if(flag != CL_SUCCESS){
                VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
                return CL_OUT_OF_HOST_MEMORY;
            }
            flag = oclandSetUserEventStatus(*event,CL_COMPLETE);
            if(flag != CL_SUCCESS){
                VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
                return CL_OUT_OF_HOST_MEMORY;
            }
        }
    }

    // In the writing case we need to send the data to the server. Since
    // a lot of network traffic is required for this operation, it could be
    // expected to be several times slower than the device data reading, so
    // we can use clEnqueueReadBuffer without a significative performance lost.
    if(    (map_flags & CL_MAP_WRITE)
        || (map_flags & CL_MAP_WRITE_INVALIDATE_REGION) ){
        cl_int flag;
        if(mapobj->type == CL_MEM_OBJECT_BUFFER){
            flag = clEnqueueWriteBuffer(command_queue, memobj, mapobj->blocking,
                                        mapobj->offset, mapobj->cb,
                                        mapobj->mapped_ptr,
                                        num_events_in_wait_list,
                                        event_wait_list, event);
        }
        else{
            flag =  clEnqueueWriteImage(command_queue, memobj, mapobj->blocking,
                                        mapobj->origin, mapobj->region,
                                        mapobj->row_pitch, mapobj->slice_pitch,
                                        mapobj->mapped_ptr,
                                        num_events_in_wait_list,
                                        event_wait_list, event);
        }
        if(flag != CL_SUCCESS){
            VERBOSE_OUT(flag);
            return flag;
        }
    }

    // Release the map object
    if(!memobj->host_ptr){
        // The mapped memory has been allocated just for this map operation
        free(mapped_ptr);
    }
    for(i=0;i<memobj->map_count;i++){
        if(mapped_ptr == memobj->maps[i]->mapped_ptr){
            // Create a new array removing the selected one
            cl_map *backup = memobj->maps;
            memobj->maps = NULL;
            if(memobj->map_count-1)
                memobj->maps = (cl_map*)malloc((memobj->map_count-1)*sizeof(cl_map));
            memcpy(memobj->maps, backup, i*sizeof(cl_map));
            memcpy(memobj->maps+i, backup+i+1, (memobj->map_count-1-i)*sizeof(cl_map));
            free(backup);
            break;
        }
    }
    memobj->map_count--;

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueNDRangeKernel(cl_command_queue  command_queue ,
                           cl_kernel         kernel ,
                           cl_uint           work_dim ,
                           const size_t *    global_work_offset ,
                           const size_t *    global_work_size ,
                           const size_t *    local_work_size ,
                           cl_uint           num_events_in_wait_list ,
                           const cl_event *  event_wait_list ,
                           cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    if((work_dim < 1) || (work_dim > 3)){
        VERBOSE_OUT(CL_INVALID_WORK_DIMENSION);
        return CL_INVALID_WORK_DIMENSION;
    }
    if(!global_work_size)
        return CL_INVALID_WORK_GROUP_SIZE;
    if(command_queue->context != kernel->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueNDRangeKernel(command_queue,kernel,
                                             work_dim,global_work_offset,
                                             global_work_size,local_work_size,
                                             num_events_in_wait_list,
                                             event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_NDRANGE_KERNEL;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueTask(cl_command_queue   command_queue ,
                  cl_kernel          kernel ,
                  cl_uint            num_events_in_wait_list ,
                  const cl_event *   event_wait_list ,
                  cl_event *         event) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    /** Following OpenCL specification, this method is equivalent
     * to call clEnqueueNDRangeKernel with: \n
     * work_dim = 1
     * global_work_offset = NULL
     * global_work_size[0] = 1
     * local_work_size[0] = 1
     */
    cl_uint work_dim = 1;
    size_t *global_work_offset=NULL;
    size_t global_work_size=1;
    size_t local_work_size=1;
    cl_int flag = icd_clEnqueueNDRangeKernel(command_queue, kernel,work_dim,
                                             global_work_offset,&global_work_size,
                                             &local_work_size,
                                             num_events_in_wait_list,event_wait_list,
                                             event);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueNativeKernel(cl_command_queue   command_queue ,
                          void (CL_CALLBACK *user_func)(void *),
                          void *             args ,
                          size_t             cb_args ,
                          cl_uint            num_mem_objects ,
                          const cl_mem *     mem_list ,
                          const void **      args_mem_loc ,
                          cl_uint            num_events_in_wait_list ,
                          const cl_event *   event_wait_list ,
                          cl_event *         event) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    /// Native kernels cannot be supported by remote applications
    VERBOSE_OUT(CL_INVALID_OPERATION);
    return CL_INVALID_OPERATION;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueReadBufferRect(cl_command_queue     command_queue ,
                            cl_mem               buffer ,
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
                            cl_event *           event) CL_API_SUFFIX__VERSION_1_1
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    (!ptr)
        || (!buffer_origin)
        || (!host_origin)
        || (!region) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(command_queue->context != buffer->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    // Correct some values if they are not provided
    if(!buffer_row_pitch)
        buffer_row_pitch = region[0];
    if(!host_row_pitch)
        host_row_pitch = region[0];
    if(!buffer_slice_pitch)
        buffer_slice_pitch = region[1]*buffer_row_pitch;
    if(!host_slice_pitch)
        host_slice_pitch = region[1]*host_row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (buffer_row_pitch   < region[0])
       || (buffer_slice_pitch < region[1]*buffer_row_pitch)
       || (buffer_slice_pitch % buffer_row_pitch)
       || (host_row_pitch   < region[0])
       || (host_slice_pitch < region[1]*host_row_pitch)
       || (host_slice_pitch % host_row_pitch)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = oclandEnqueueReadBufferRect(command_queue,buffer,blocking_read,
                                              buffer_origin,host_origin,region,
                                              buffer_row_pitch,buffer_slice_pitch,
                                              host_row_pitch,host_slice_pitch,ptr,
                                              num_events_in_wait_list,
                                              event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_READ_BUFFER_RECT;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueWriteBufferRect(cl_command_queue     command_queue ,
                             cl_mem               buffer ,
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
                             cl_event *           event) CL_API_SUFFIX__VERSION_1_1
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    (!ptr)
        || (!buffer_origin)
        || (!host_origin)
        || (!region) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(command_queue->context != buffer->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    // Correct some values if they are not provided
    if(!buffer_row_pitch)
        buffer_row_pitch = region[0];
    if(!host_row_pitch)
        host_row_pitch = region[0];
    if(!buffer_slice_pitch)
        buffer_slice_pitch = region[1]*buffer_row_pitch;
    if(!host_slice_pitch)
        host_slice_pitch = region[1]*host_row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (buffer_row_pitch   < region[0])
       || (buffer_slice_pitch < region[1]*buffer_row_pitch)
       || (buffer_slice_pitch % buffer_row_pitch)
       || (host_row_pitch   < region[0])
       || (host_slice_pitch < region[1]*host_row_pitch)
       || (host_slice_pitch % host_row_pitch)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = oclandEnqueueWriteBufferRect(command_queue,buffer,blocking_write,
                                               buffer_origin,host_origin,region,
                                               buffer_row_pitch,buffer_slice_pitch,
                                               host_row_pitch,host_slice_pitch,ptr,
                                               num_events_in_wait_list,
                                               event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_WRITE_BUFFER_RECT;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueCopyBufferRect(cl_command_queue     command_queue ,
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
                            cl_event *           event) CL_API_SUFFIX__VERSION_1_1
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(src_buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!hasMem(dst_buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(   (!src_origin)
       || (!dst_origin)
       || (!region)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(    (command_queue->context != src_buffer->context)
        || (command_queue->context != dst_buffer->context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    // Correct some values if they are not provided
    if(!src_row_pitch)
        src_row_pitch   = region[0];
    if(!dst_row_pitch)
        dst_row_pitch   = region[0];
    if(!src_slice_pitch)
        src_slice_pitch = region[1]*src_row_pitch;
    if(!dst_slice_pitch)
        dst_slice_pitch = region[1]*dst_row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (src_row_pitch   < region[0])
       || (dst_row_pitch   < region[0])
       || (src_slice_pitch < region[1]*src_row_pitch)
       || (dst_slice_pitch < region[1]*dst_row_pitch)
       || (src_slice_pitch % src_row_pitch)
       || (dst_slice_pitch % dst_row_pitch)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = oclandEnqueueCopyBufferRect(command_queue,src_buffer,dst_buffer,
                                              src_origin,dst_origin,region,
                                              src_row_pitch,src_slice_pitch,
                                              dst_row_pitch,dst_slice_pitch,
                                              num_events_in_wait_list,
                                              event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_COPY_BUFFER_RECT;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueFillBuffer(cl_command_queue    command_queue ,
                        cl_mem              buffer ,
                        const void *        pattern ,
                        size_t              pattern_size ,
                        size_t              offset ,
                        size_t              cb ,
                        cl_uint             num_events_in_wait_list ,
                        const cl_event *    event_wait_list ,
                        cl_event *          event) CL_API_SUFFIX__VERSION_1_2
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if((!pattern) || (!pattern_size)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if((offset % pattern_size) || (cb % pattern_size)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(command_queue->context != buffer->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueFillBuffer(command_queue,buffer,
                                          pattern,pattern_size,offset,cb,
                                          num_events_in_wait_list,
                                          event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_FILL_BUFFER;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueFillImage(cl_command_queue    command_queue ,
                       cl_mem              image ,
                       const void *        fill_color ,
                       const size_t *      origin ,
                       const size_t *      region ,
                       cl_uint             num_events_in_wait_list ,
                       const cl_event *    event_wait_list ,
                       cl_event *          event) CL_API_SUFFIX__VERSION_1_2
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!hasMem(image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(   (!fill_color)
       || (!origin)
       || (!region)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(   (!region[0]) || (!region[1]) || (!region[2]) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(command_queue->context != image->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    // Get the size of the filling color fill_color
    size_t fill_color_size = 4*sizeof(float);
    cl_image_format image_format;
    cl_int flag = clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &image_format, NULL);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(    image_format.image_channel_data_type == CL_SIGNED_INT8
        || image_format.image_channel_data_type == CL_SIGNED_INT16
        || image_format.image_channel_data_type == CL_SIGNED_INT32 ){
            fill_color_size = 4*sizeof(int);
    }
    if(    image_format.image_channel_data_type == CL_UNSIGNED_INT8
        || image_format.image_channel_data_type == CL_UNSIGNED_INT16
        || image_format.image_channel_data_type == CL_UNSIGNED_INT32 ){
            fill_color_size = 4*sizeof(unsigned int);
    }

    flag = oclandEnqueueFillImage(command_queue,image,
                                  fill_color_size,fill_color,origin,region,
                                  num_events_in_wait_list,event_wait_list,
                                  event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_FILL_IMAGE;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueMigrateMemObjects(cl_command_queue        command_queue ,
                               cl_uint                 num_mem_objects ,
                               const cl_mem *          mem_objects ,
                               cl_mem_migration_flags  flags ,
                               cl_uint                 num_events_in_wait_list ,
                               const cl_event *        event_wait_list ,
                               cl_event *              event) CL_API_SUFFIX__VERSION_1_2
{
    cl_uint i;
    VERBOSE_IN();
    // Test for valid flags
    if(    (!flags)
        || (    (flags != CL_MIGRATE_MEM_OBJECT_HOST)
             && (flags != CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED))){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    // Test for other invalid values
    if(    (!num_mem_objects)
        || (!mem_objects)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    for(i=0;i<num_mem_objects;i++){
        if(!hasMem(mem_objects[i])){
            VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
            return CL_INVALID_MEM_OBJECT;
        }
        if(command_queue->context != mem_objects[i]->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueMigrateMemObjects(command_queue,
                                                 num_mem_objects,mem_objects,
                                                 flags,num_events_in_wait_list,
                                                 event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_MIGRATE_MEM_OBJECTS;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueMarkerWithWaitList(cl_command_queue  command_queue ,
                                cl_uint            num_events_in_wait_list ,
                                const cl_event *   event_wait_list ,
                                cl_event *         event) CL_API_SUFFIX__VERSION_1_2
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueMarkerWithWaitList(command_queue,
                                                  num_events_in_wait_list,
                                                  event_wait_list,event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_MARKER;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueBarrierWithWaitList(cl_command_queue  command_queue ,
                                 cl_uint            num_events_in_wait_list ,
                                 const cl_event *   event_wait_list ,
                                 cl_event *         event) CL_API_SUFFIX__VERSION_1_2
{
    cl_uint i;
    VERBOSE_IN();
    if(!hasCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list)){
        VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    for(i=0;i<num_events_in_wait_list;i++){
        if(!isEvent(event_wait_list[i])){
            VERBOSE_OUT(CL_INVALID_EVENT_WAIT_LIST);
            return CL_INVALID_EVENT_WAIT_LIST;
        }
        if(event_wait_list[i]->context != command_queue->context){
            VERBOSE_OUT(CL_INVALID_CONTEXT);
            return CL_INVALID_CONTEXT;
        }
    }
    cl_int flag = oclandEnqueueBarrierWithWaitList(command_queue,
                                                   num_events_in_wait_list,
                                                   event_wait_list, event);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Setup the output event
    if(event){
        cl_event e = (cl_event)malloc(sizeof(struct _cl_event));
        if(!e){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        e->dispatch = &master_dispatch;
        e->ptr = *event;
        e->rcount = 1;
        pthread_mutex_init(&(e->rcount_mutex), NULL);
        e->socket = command_queue->server->socket;
        e->command_queue = command_queue;
        e->context = command_queue->context;
        e->command_type = CL_COMMAND_BARRIER;
        *event = e;
        // Create a new array appending the new one
        cl_event *backup = master_events;
        num_master_events++;
        master_events = (cl_event*)malloc(num_master_events*sizeof(cl_event));
        memcpy(master_events, backup, (num_master_events-1)*sizeof(cl_event));
        free(backup);
        master_events[num_master_events-1] = e;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
icd_clEnqueueMarker(cl_command_queue    command_queue ,
                    cl_event *          event) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    VERBOSE_IN();
    cl_int flag = icd_clEnqueueMarkerWithWaitList(command_queue, 0, NULL, event);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
icd_clEnqueueWaitForEvents(cl_command_queue command_queue ,
                           cl_uint          num_events ,
                           const cl_event * event_list ) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    VERBOSE_IN();
    cl_int flag = icd_clWaitForEvents(num_events, event_list);
    VERBOSE_OUT(flag);
    return flag;
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
icd_clEnqueueBarrier(cl_command_queue command_queue ) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    VERBOSE_IN();
    cl_int flag = icd_clEnqueueBarrierWithWaitList(command_queue, 0, NULL, NULL);
    VERBOSE_OUT(flag);
    return flag;
}

// --------------------------------------------------------------
// OpenGL
// --------------------------------------------------------------

CL_API_ENTRY cl_mem CL_API_CALL
icd_clCreateFromGLBuffer(cl_context     context ,
                         cl_mem_flags   flags ,
                         cl_GLuint      bufobj ,
                         int *          errcode_ret ) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    VERBOSE_OUT(CL_INVALID_GL_OBJECT);
    return NULL;
}

CL_API_ENTRY cl_mem CL_API_CALL
icd_clCreateFromGLTexture(cl_context      context ,
                          cl_mem_flags    flags ,
                          cl_GLenum       target ,
                          cl_GLint        miplevel ,
                          cl_GLuint       texture ,
                          cl_int *        errcode_ret ) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    VERBOSE_OUT(CL_INVALID_GL_OBJECT);
    return NULL;
}

CL_API_ENTRY cl_mem CL_API_CALL
icd_clCreateFromGLRenderbuffer(cl_context   context ,
                               cl_mem_flags flags ,
                               cl_GLuint    renderbuffer ,
                               cl_int *     errcode_ret ) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    VERBOSE_OUT(CL_INVALID_GL_OBJECT);
    return NULL;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetGLObjectInfo(cl_mem                memobj ,
                      cl_gl_object_type *   gl_object_type ,
                      cl_GLuint *           gl_object_name ) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    /** If have not been possible to generate GL memory
     * objects, is impossible that the memory object is
     * associated to a GL object.
     */
    VERBOSE_OUT(CL_INVALID_GL_OBJECT);
    return CL_INVALID_GL_OBJECT;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetGLTextureInfo(cl_mem               memobj ,
                       cl_gl_texture_info   param_name ,
                       size_t               param_value_size ,
                       void *               param_value ,
                       size_t *             param_value_size_ret ) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    /** If have not been possible to generate GL memory
     * objects, is impossible that the memory object is
     * associated to a GL object.
     */
    VERBOSE_OUT(CL_INVALID_GL_OBJECT);
    return CL_INVALID_GL_OBJECT;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueAcquireGLObjects(cl_command_queue      command_queue ,
                              cl_uint               num_objects ,
                              const cl_mem *        mem_objects ,
                              cl_uint               num_events_in_wait_list ,
                              const cl_event *      event_wait_list ,
                              cl_event *            event ) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    /** If have not been possible to generate GL memory
     * objects, is impossible that the memory object is
     * associated to a GL object.
     */
    VERBOSE_OUT(CL_INVALID_GL_OBJECT);
    return CL_INVALID_GL_OBJECT;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueReleaseGLObjects(cl_command_queue      command_queue ,
                              cl_uint               num_objects ,
                              const cl_mem *        mem_objects ,
                              cl_uint               num_events_in_wait_list ,
                              const cl_event *      event_wait_list ,
                              cl_event *            event ) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    /** If have not been possible to generate GL memory
     * objects, is impossible that the memory object is
     * associated to a GL object.
     */
    VERBOSE_OUT(CL_INVALID_GL_OBJECT);
    return CL_INVALID_GL_OBJECT;
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
icd_clCreateFromGLTexture2D(cl_context      context ,
                            cl_mem_flags    flags ,
                            cl_GLenum       target ,
                            cl_GLint        miplevel ,
                            cl_GLuint       texture ,
                            cl_int *        errcode_ret ) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    VERBOSE_IN();
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    VERBOSE_OUT(CL_INVALID_GL_OBJECT);
    return NULL;
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
icd_clCreateFromGLTexture3D(cl_context      context ,
                            cl_mem_flags    flags ,
                            cl_GLenum       target ,
                            cl_GLint        miplevel ,
                            cl_GLuint       texture ,
                            cl_int *        errcode_ret ) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    VERBOSE_IN();
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    VERBOSE_OUT(CL_INVALID_GL_OBJECT);
    return NULL;
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetGLContextInfoKHR(const cl_context_properties * properties ,
                          cl_gl_context_info            param_name ,
                          size_t                        param_value_size ,
                          void *                        param_value ,
                          size_t *                      param_value_size_ret ) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    VERBOSE_OUT(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR);
    return CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR;
}

// --------------------------------------------------------------
// Extensions, only used at the start of icd_loader
// --------------------------------------------------------------

CL_API_ENTRY void * CL_API_CALL
icd_clGetExtensionFunctionAddress(const char *func_name) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if( func_name != NULL &&  strcmp("clIcdGetPlatformIDsKHR", func_name) == 0 ){
        VERBOSE_OUT(CL_SUCCESS);
        return (void *)__GetPlatformIDs;
    }
    else if( func_name != NULL &&  strcmp("clGetPlatformInfo", func_name) == 0 ){
        VERBOSE_OUT(CL_SUCCESS);
        return (void *)icd_clGetPlatformInfo;
    }
    else if( func_name != NULL &&  strcmp("clGetDeviceInfo", func_name) == 0 ){
        VERBOSE_OUT(CL_SUCCESS);
        return (void *)icd_clGetDeviceInfo;
    }
    VERBOSE_OUT(CL_OUT_OF_RESOURCES);
    return NULL;
}

CL_API_ENTRY void * CL_API_CALL
icd_clGetExtensionFunctionAddressForPlatform(cl_platform_id platform,
                                             const char *   func_name) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    return icd_clGetExtensionFunctionAddress(func_name);
}

/// Dummy function to parse non-implemented methods
CL_API_ENTRY cl_int CL_API_CALL dummyFunc(void)
{
    VERBOSE_IN();
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}

// Some old functions which are redirecting to others
CL_API_ENTRY cl_int CL_API_CALL
icd_clCreateSubDevicesEXT(cl_device_id                             in_device,
                          const cl_device_partition_property_ext * properties,
                          cl_uint                                  num_entries,
                          cl_device_id *                           out_devices,
                          cl_uint *                                num_devices) CL_EXT_SUFFIX__VERSION_1_1
{
    return icd_clCreateSubDevices(in_device,
                                  (const cl_device_partition_property*)properties,
                                  num_entries,
                                  out_devices,
                                  num_devices);
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainDeviceEXT(cl_device_id device) CL_EXT_SUFFIX__VERSION_1_1
{
    return icd_clRetainDevice(device);
}

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseDeviceEXT(cl_device_id device) CL_EXT_SUFFIX__VERSION_1_1
{
    return icd_clReleaseDevice(device);
}

CL_API_ENTRY cl_event CL_API_CALL
icd_clCreateEventFromGLsyncKHR(cl_context           context,
                           cl_GLsync            cl_GLsync,
                           cl_int *             errcode_ret) CL_EXT_SUFFIX__VERSION_1_1
{
    if(errcode_ret)
        *errcode_ret = CL_INVALID_CONTEXT;
    return NULL;
}

struct _cl_icd_dispatch master_dispatch = {
  &icd_clGetPlatformIDs,
  &icd_clGetPlatformInfo,
  &icd_clGetDeviceIDs,
  &icd_clGetDeviceInfo,
  &icd_clCreateContext,
  &icd_clCreateContextFromType,
  &icd_clRetainContext,
  &icd_clReleaseContext,
  &icd_clGetContextInfo,
  &icd_clCreateCommandQueue,
  &icd_clRetainCommandQueue,
  &icd_clReleaseCommandQueue,
  &icd_clGetCommandQueueInfo,
  &icd_clSetCommandQueueProperty,  // DEPRECATED
  &icd_clCreateBuffer,
  &icd_clCreateImage2D,
  &icd_clCreateImage3D,
  &icd_clRetainMemObject,
  &icd_clReleaseMemObject,
  &icd_clGetSupportedImageFormats,
  &icd_clGetMemObjectInfo,
  &icd_clGetImageInfo,
  &icd_clCreateSampler,
  &icd_clRetainSampler,
  &icd_clReleaseSampler,
  &icd_clGetSamplerInfo,
  &icd_clCreateProgramWithSource,
  &icd_clCreateProgramWithBinary,
  &icd_clRetainProgram,
  &icd_clReleaseProgram,
  &icd_clBuildProgram,
  &icd_clUnloadCompiler,  // DEPRECATED
  &icd_clGetProgramInfo,
  &icd_clGetProgramBuildInfo,
  &icd_clCreateKernel,
  &icd_clCreateKernelsInProgram,
  &icd_clRetainKernel,
  &icd_clReleaseKernel,
  &icd_clSetKernelArg,
  &icd_clGetKernelInfo,
  &icd_clGetKernelWorkGroupInfo,
  &icd_clWaitForEvents,
  &icd_clGetEventInfo,
  &icd_clRetainEvent,
  &icd_clReleaseEvent,
  &icd_clGetEventProfilingInfo,
  &icd_clFlush,
  &icd_clFinish,
  &icd_clEnqueueReadBuffer,
  &icd_clEnqueueWriteBuffer,
  &icd_clEnqueueCopyBuffer,
  &icd_clEnqueueReadImage,
  &icd_clEnqueueWriteImage,
  &icd_clEnqueueCopyImage,
  &icd_clEnqueueCopyImageToBuffer,
  &icd_clEnqueueCopyBufferToImage,
  &icd_clEnqueueMapBuffer,
  &icd_clEnqueueMapImage,
  &icd_clEnqueueUnmapMemObject,
  &icd_clEnqueueNDRangeKernel,
  &icd_clEnqueueTask,
  &icd_clEnqueueNativeKernel,
  &icd_clEnqueueMarker,
  &icd_clEnqueueWaitForEvents,
  &icd_clEnqueueBarrier,
  &icd_clGetExtensionFunctionAddress,
  &icd_clCreateFromGLBuffer,
  &icd_clCreateFromGLTexture2D,
  &icd_clCreateFromGLTexture3D,
  &icd_clCreateFromGLRenderbuffer,
  &icd_clGetGLObjectInfo,
  &icd_clGetGLTextureInfo,
  &icd_clEnqueueAcquireGLObjects,
  &icd_clEnqueueReleaseGLObjects,
  &icd_clGetGLContextInfoKHR,
  &dummyFunc,    // clUnknown75
  &dummyFunc,    // clUnknown76
  &dummyFunc,    // clUnknown77
  &dummyFunc,    // clUnknown78
  &dummyFunc,    // clUnknown79
  &dummyFunc,    // clUnknown80
  &icd_clSetEventCallback,
  &icd_clCreateSubBuffer,
  &icd_clSetMemObjectDestructorCallback,
  &icd_clCreateUserEvent,
  &icd_clSetUserEventStatus,
  &icd_clEnqueueReadBufferRect,
  &icd_clEnqueueWriteBufferRect,
  &icd_clEnqueueCopyBufferRect,
  &icd_clCreateSubDevicesEXT,
  &icd_clRetainDeviceEXT,
  &icd_clReleaseDeviceEXT,
  &icd_clCreateEventFromGLsyncKHR,
  &icd_clCreateSubDevices,
  &icd_clRetainDevice,
  &icd_clReleaseDevice,
  &icd_clCreateImage,
  &icd_clCreateProgramWithBuiltInKernels,
  &icd_clCompileProgram,
  &icd_clLinkProgram,
  &icd_clUnloadPlatformCompiler,
  &icd_clGetKernelArgInfo,
  &icd_clEnqueueFillBuffer,
  &icd_clEnqueueFillImage,
  &icd_clEnqueueMigrateMemObjects,
  &icd_clEnqueueMarkerWithWaitList,
  &icd_clEnqueueBarrierWithWaitList,
  &icd_clGetExtensionFunctionAddressForPlatform,
  &icd_clCreateFromGLTexture,
  &dummyFunc,    // clUnknown109
  &dummyFunc,    // clUnknown110
  &dummyFunc,    // clUnknown111
  &dummyFunc,    // clUnknown112
  &dummyFunc,    // clUnknown113
  &dummyFunc,    // clUnknown114
  &dummyFunc,    // clUnknown115
  &dummyFunc,    // clUnknown116
  &dummyFunc,    // clUnknown117
  &dummyFunc,    // clUnknown118
  &dummyFunc,    // clUnknown119
  &dummyFunc,    // clUnknown120
  &dummyFunc,    // clUnknown121
  &dummyFunc,    // clUnknown122
};

#ifdef WIN32
#pragma GCC visibility push(default)
// __stdcall must be used but without _function@argsize naming convention,
// this is done using separate cl* functions exported by def file
// TODO

// Platform API
CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformIDs(cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetPlatformIDs(num_entries, platforms, num_platforms);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformInfo(cl_platform_id   platform,
                  cl_platform_info param_name,
                  size_t           param_value_size,
                  void *           param_value,
                  size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);
}

// Device APIs
CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id platform,
               cl_device_type device_type,
               cl_uint        num_entries,
               cl_device_id * devices,
               cl_uint *      num_devices) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id   device,
                cl_device_info param_name,
                size_t         param_value_size,
                void *         param_value,
                size_t *       param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateSubDevices(cl_device_id                         in_device,
                   const cl_device_partition_property * properties,
                   cl_uint                              num_devices,
                   cl_device_id *                       out_devices,
                   cl_uint *                            num_devices_ret) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clCreateSubDevices(in_device, properties, num_devices, out_devices, num_devices_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clRetainDevice(device);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clReleaseDevice(device);
}

// Context APIs
CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(const cl_context_properties * properties,
                cl_uint                 num_devices,
                const cl_device_id *    devices,
                void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *),
                void *                  user_data,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
}

CL_API_ENTRY cl_context CL_API_CALL
clCreateContextFromType(const cl_context_properties * properties,
                        cl_device_type          device_type,
                        void (CL_CALLBACK *     pfn_notify)(const char *, const void *, size_t, void *),
                        void *                  user_data,
                        cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clRetainContext(context);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clReleaseContext(context);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetContextInfo(cl_context         context,
                 cl_context_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);
}

// Command Queue APIs
CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(cl_context                     context,
                     cl_device_id                   device,
                     cl_command_queue_properties    properties,
                     cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateCommandQueue(context, device, properties, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clRetainCommandQueue(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clReleaseCommandQueue(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetCommandQueueInfo(cl_command_queue      command_queue,
                      cl_command_queue_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
}

// Memory Object APIs
CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context   context,
               cl_mem_flags flags,
               size_t       size,
               void *       host_ptr,
               cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateBuffer(context, flags, size, host_ptr, errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateSubBuffer(cl_mem                   buffer,
                  cl_mem_flags             flags,
                  cl_buffer_create_type    buffer_create_type,
                  const void *             buffer_create_info,
                  cl_int *                 errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    return icd_clCreateSubBuffer(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage(cl_context              context,
              cl_mem_flags            flags,
              const cl_image_format * image_format,
              const cl_image_desc *   image_desc,
              void *                  host_ptr,
              cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clCreateImage(context, flags, image_format, image_desc, host_ptr, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clRetainMemObject(memobj);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clReleaseMemObject(memobj);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSupportedImageFormats(cl_context           context,
                           cl_mem_flags         flags,
                           cl_mem_object_type   image_type,
                           cl_uint              num_entries,
                           cl_image_format *    image_formats,
                           cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectInfo(cl_mem      memobj,
                   cl_mem_info param_name,
                   size_t      param_value_size,
                   void *      param_value,
                   size_t *    param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetImageInfo(cl_mem           image,
               cl_image_info    param_name,
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetImageInfo(image, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetMemObjectDestructorCallback(cl_mem memobj,
                                 void (CL_CALLBACK * pfn_notify)( cl_mem memobj, void* user_data),
                                 void * user_data)             CL_API_SUFFIX__VERSION_1_1
{
    return icd_clSetMemObjectDestructorCallback(memobj, pfn_notify, user_data);
}

// Sampler APIs
CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSampler(cl_context          context,
                cl_bool             normalized_coords,
                cl_addressing_mode  addressing_mode,
                cl_filter_mode      filter_mode,
                cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateSampler(context, normalized_coords, addressing_mode, filter_mode, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainSampler(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clRetainSampler(sampler);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseSampler(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clReleaseSampler(sampler);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSamplerInfo(cl_sampler      sampler,
                 cl_sampler_info param_name,
                 size_t          param_value_size,
                 void *          param_value,
                 size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetSamplerInfo(sampler, param_name, param_value_size, param_value, param_value_size_ret);
}

// Program Object APIs
CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(cl_context     context,
                          cl_uint        count,
                          const char **  strings,
                          const size_t * lengths,
                          cl_int *       errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateProgramWithSource(context, count, strings, lengths, errcode_ret);
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(cl_context             context,
                          cl_uint                num_devices,
                          const cl_device_id *   device_list,
                          const size_t *         lengths,
                          const unsigned char ** binaries,
                          cl_int *               binary_status,
                          cl_int *               errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBuiltInKernels(cl_context           context,
                                  cl_uint              num_devices,
                                  const cl_device_id * device_list,
                                  const char *         kernel_names,
                                  cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clCreateProgramWithBuiltInKernels(context, num_devices, device_list, kernel_names, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clRetainProgram(program);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clReleaseProgram(program);
}

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program           program,
               cl_uint              num_devices,
               const cl_device_id * device_list,
               const char *         options,
               void (CL_CALLBACK *  pfn_notify)(cl_program program, void * user_data),
               void *               user_data) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clBuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);
}

CL_API_ENTRY cl_int CL_API_CALL
clCompileProgram(cl_program           program,
                 cl_uint              num_devices,
                 const cl_device_id * device_list,
                 const char *         options,
                 cl_uint              num_input_headers,
                 const cl_program *   input_headers,
                 const char **        header_include_names,
                 void (CL_CALLBACK *  pfn_notify)(cl_program program, void * user_data),
                 void *               user_data) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clCompileProgram(program, num_devices, device_list, options, num_input_headers, input_headers, header_include_names, pfn_notify, user_data);
}

CL_API_ENTRY cl_program CL_API_CALL
clLinkProgram(cl_context           context,
              cl_uint              num_devices,
              const cl_device_id * device_list,
              const char *         options,
              cl_uint              num_input_programs,
              const cl_program *   input_programs,
              void (CL_CALLBACK *  pfn_notify)(cl_program program, void * user_data),
              void *               user_data,
              cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clLinkProgram(context, num_devices, device_list, options, num_input_programs, input_programs, pfn_notify, user_data, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clUnloadPlatformCompiler(cl_platform_id platform) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clUnloadPlatformCompiler(platform);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramInfo(cl_program         program,
                 cl_program_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(cl_program            program,
                      cl_device_id          device,
                      cl_program_build_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);
}

// Kernel Object APIs
CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program      program,
               const char *    kernel_name,
               cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateKernel(program, kernel_name, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateKernelsInProgram(cl_program     program,
                         cl_uint        num_kernels,
                         cl_kernel *    kernels,
                         cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clRetainKernel(kernel);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clReleaseKernel(kernel);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel    kernel,
               cl_uint      arg_index,
               size_t       arg_size,
               const void * arg_value) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clSetKernelArg(kernel, arg_index, arg_size, arg_value);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelInfo(cl_kernel       kernel,
                cl_kernel_info  param_name,
                size_t          param_value_size,
                void *          param_value,
                size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelArgInfo(cl_kernel       kernel,
                   cl_uint         arg_indx,
                   cl_kernel_arg_info param_name,
                   size_t          param_value_size,
                   void *          param_value,
                   size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clGetKernelArgInfo(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelWorkGroupInfo(cl_kernel                 kernel,
                         cl_device_id              device,
                         cl_kernel_work_group_info param_name,
                         size_t                    param_value_size,
                         void *                    param_value,
                         size_t *                  param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
}

// Event Object APIs
CL_API_ENTRY cl_int CL_API_CALL
clWaitForEvents(cl_uint          num_events,
                const cl_event * event_list) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clWaitForEvents(num_events, event_list);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetEventInfo(cl_event      event,
               cl_event_info param_name,
               size_t        param_value_size,
               void *        param_value,
               size_t *      param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetEventInfo(event, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_event CL_API_CALL
clCreateUserEvent(cl_context context,
                  cl_int *   errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    return icd_clCreateUserEvent(context, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clRetainEvent(event);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clReleaseEvent(event);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetUserEventStatus(cl_event event,
                     cl_int   execution_status) CL_API_SUFFIX__VERSION_1_1
{
    return icd_clSetUserEventStatus(event, execution_status);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetEventCallback(cl_event    event,
                   cl_int      command_exec_callback_type,
                   void (CL_CALLBACK * pfn_notify)(cl_event, cl_int, void *),
                   void *      user_data) CL_API_SUFFIX__VERSION_1_1
{
    return icd_clSetEventCallback(event, command_exec_callback_type, pfn_notify, user_data);
}

// Profiling APIs
CL_API_ENTRY cl_int CL_API_CALL
clGetEventProfilingInfo(cl_event          event,
                        cl_profiling_info param_name,
                        size_t            param_value_size,
                        void *            param_value,
                        size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret);
}

// Flush and Finish APIs
CL_API_ENTRY cl_int CL_API_CALL
clFlush(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clFlush(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clFinish(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clFinish(command_queue);
}

// Enqueued Commands APIs
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue command_queue,
                    cl_mem           buffer,
                    cl_bool          blocking_read,
                    size_t           offset,
                    size_t           size,
                    void *           ptr,
                    cl_uint          num_events_in_wait_list,
                    const cl_event * event_wait_list,
                    cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueReadBuffer(command_queue, buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBufferRect(cl_command_queue command_queue,
                        cl_mem           buffer,
                        cl_bool          blocking_read,
                        const size_t *   buffer_offset,
                        const size_t *   host_offset,
                        const size_t *   region,
                        size_t           buffer_row_pitch,
                        size_t           buffer_slice_pitch,
                        size_t           host_row_pitch,
                        size_t           host_slice_pitch,
                        void *           ptr,
                        cl_uint          num_events_in_wait_list,
                        const cl_event * event_wait_list,
                        cl_event *       event) CL_API_SUFFIX__VERSION_1_1
{
    return icd_clEnqueueReadBufferRect(command_queue, buffer, blocking_read, buffer_offset, host_offset, region, buffer_row_pitch, buffer_slice_pitch,
            host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue command_queue,
                     cl_mem           buffer,
                     cl_bool          blocking_write,
                     size_t           offset,
                     size_t           size,
                     const void *     ptr,
                     cl_uint          num_events_in_wait_list,
                     const cl_event * event_wait_list,
                     cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBufferRect(cl_command_queue command_queue,
                         cl_mem           buffer,
                         cl_bool          blocking_write,
                         const size_t *   buffer_offset,
                         const size_t *   host_offset,
                         const size_t *   region,
                         size_t           buffer_row_pitch,
                         size_t           buffer_slice_pitch,
                         size_t           host_row_pitch,
                         size_t           host_slice_pitch,
                         const void *     ptr,
                         cl_uint          num_events_in_wait_list,
                         const cl_event * event_wait_list,
                         cl_event *       event) CL_API_SUFFIX__VERSION_1_1
{
    return icd_clEnqueueWriteBufferRect(command_queue, buffer, blocking_write, buffer_offset, host_offset, region, buffer_row_pitch, buffer_slice_pitch,
            host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillBuffer(cl_command_queue command_queue,
                    cl_mem           buffer,
                    const void *     pattern,
                    size_t           pattern_size,
                    size_t           offset,
                    size_t           size,
                    cl_uint          num_events_in_wait_list,
                    const cl_event * event_wait_list,
                    cl_event *       event) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clEnqueueFillBuffer(command_queue, buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue command_queue,
                    cl_mem           src_buffer,
                    cl_mem           dst_buffer,
                    size_t           src_offset,
                    size_t           dst_offset,
                    size_t           size,
                    cl_uint          num_events_in_wait_list,
                    const cl_event * event_wait_list,
                    cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, size, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferRect(cl_command_queue command_queue,
                        cl_mem           src_buffer,
                        cl_mem           dst_buffer,
                        const size_t *   src_origin,
                        const size_t *   dst_origin,
                        const size_t *   region,
                        size_t           src_row_pitch,
                        size_t           src_slice_pitch,
                        size_t           dst_row_pitch,
                        size_t           dst_slice_pitch,
                        cl_uint          num_events_in_wait_list,
                        const cl_event * event_wait_list,
                        cl_event *       event) CL_API_SUFFIX__VERSION_1_1
{
    return icd_clEnqueueCopyBufferRect(command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, src_slice_pitch,
            dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadImage(cl_command_queue command_queue,
                   cl_mem           image,
                   cl_bool          blocking_read,
                   const size_t *   origin,
                   const size_t *   region,
                   size_t           row_pitch,
                   size_t           slice_pitch,
                   void *           ptr,
                   cl_uint          num_events_in_wait_list,
                   const cl_event * event_wait_list,
                   cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr,
            num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteImage(cl_command_queue command_queue,
                    cl_mem           image,
                    cl_bool          blocking_write,
                    const size_t *   origin,
                    const size_t *   region,
                    size_t           input_row_pitch,
                    size_t           input_slice_pitch,
                    const void *     ptr,
                    cl_uint          num_events_in_wait_list,
                    const cl_event * event_wait_list,
                    cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr,
            num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillImage(cl_command_queue command_queue,
                   cl_mem           image,
                   const void *     fill_color,
                   const size_t *   origin,
                   const size_t *   region,
                   cl_uint          num_events_in_wait_list,
                   const cl_event * event_wait_list,
                   cl_event *       event) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clEnqueueFillImage(command_queue, image, fill_color, origin, region, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImage(cl_command_queue command_queue,
                   cl_mem           src_image,
                   cl_mem           dst_image,
                   const size_t *   src_origin,
                   const size_t *   dst_origin,
                   const size_t *   region,
                   cl_uint          num_events_in_wait_list,
                   const cl_event * event_wait_list,
                   cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueCopyImage(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImageToBuffer(cl_command_queue command_queue,
                           cl_mem           src_image,
                           cl_mem           dst_buffer,
                           const size_t *   src_origin,
                           const size_t *   region,
                           size_t           dst_offset,
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, src_origin, region, dst_offset,
            num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue command_queue,
                           cl_mem           src_buffer,
                           cl_mem           dst_image,
                           size_t           src_offset,
                           const size_t *   dst_origin,
                           const size_t *   region,
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueCopyBufferToImage(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapBuffer(cl_command_queue command_queue,
                   cl_mem           buffer,
                   cl_bool          blocking_map,
                   cl_map_flags     map_flags,
                   size_t           offset,
                   size_t           size,
                   cl_uint          num_events_in_wait_list,
                   const cl_event * event_wait_list,
                   cl_event *       event,
                   cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, size, num_events_in_wait_list, event_wait_list, event, errcode_ret);
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapImage(cl_command_queue command_queue,
                  cl_mem           image,
                  cl_bool          blocking_map,
                  cl_map_flags     map_flags,
                  const size_t *   origin,
                  const size_t *   region,
                  size_t *         image_row_pitch,
                  size_t *         image_slice_pitch,
                  cl_uint          num_events_in_wait_list,
                  const cl_event * event_wait_list,
                  cl_event *       event,
                  cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueMapImage(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch,
            num_events_in_wait_list, event_wait_list, event, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(cl_command_queue command_queue,
                        cl_mem           memobj,
                        void *           mapped_ptr,
                        cl_uint          num_events_in_wait_list,
                        const cl_event * event_wait_list,
                        cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMigrateMemObjects(cl_command_queue       command_queue,
                           cl_uint                num_mem_objects,
                           const cl_mem *         mem_objects,
                           cl_mem_migration_flags flags,
                           cl_uint                num_events_in_wait_list,
                           const cl_event *       event_wait_list,
                           cl_event *             event) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clEnqueueMigrateMemObjects(command_queue, num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue command_queue,
                       cl_kernel        kernel,
                       cl_uint          work_dim,
                       const size_t *   global_work_offset,
                       const size_t *   global_work_size,
                       const size_t *   local_work_size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size,
            num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueTask(cl_command_queue command_queue,
              cl_kernel        kernel,
              cl_uint          num_events_in_wait_list,
              const cl_event * event_wait_list,
              cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueTask(command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNativeKernel(cl_command_queue  command_queue,
                      void (CL_CALLBACK * user_func)(void *),
                      void *            args,
                      size_t            cb_args,
                      cl_uint           num_mem_objects,
                      const cl_mem *    mem_list,
                      const void **     args_mem_loc,
                      cl_uint           num_events_in_wait_list,
                      const cl_event *  event_wait_list,
                      cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueNativeKernel(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc,
            num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarkerWithWaitList(cl_command_queue command_queue,
                            cl_uint          num_events_in_wait_list,
                            const cl_event * event_wait_list,
                            cl_event *       event) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clEnqueueMarkerWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrierWithWaitList(cl_command_queue command_queue,
                             cl_uint          num_events_in_wait_list,
                             const cl_event * event_wait_list,
                             cl_event *       event) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clEnqueueBarrierWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event);
}


CL_API_ENTRY void * CL_API_CALL
clGetExtensionFunctionAddressForPlatform(cl_platform_id platform,
                                         const char *   func_name) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clGetExtensionFunctionAddressForPlatform(platform, func_name);
}


// Deprecated OpenCL 1.1 APIs
CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
clCreateImage2D(cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_row_pitch,
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return icd_clCreateImage2D(context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
clCreateImage3D(cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_depth,
                size_t                  image_row_pitch,
                size_t                  image_slice_pitch,
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return icd_clCreateImage3D(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
clEnqueueMarker(cl_command_queue    command_queue,
                cl_event *          event) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return icd_clEnqueueMarker(command_queue, event);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
clEnqueueWaitForEvents(cl_command_queue  command_queue,
                        cl_uint          num_events,
                        const cl_event * event_list) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return icd_clEnqueueWaitForEvents(command_queue, num_events, event_list);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
clEnqueueBarrier(cl_command_queue command_queue) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return icd_clEnqueueBarrier(command_queue);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
clUnloadCompiler(void) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return icd_clUnloadCompiler();
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED void * CL_API_CALL
clGetExtensionFunctionAddress(const char * func_name) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return icd_clGetExtensionFunctionAddress(func_name);
}

// Khronos extensions
CL_API_ENTRY cl_int CL_API_CALL
clIcdGetPlatformIDsKHR(cl_uint          num_entries,
                       cl_platform_id * platforms,
                       cl_uint *        num_platforms)
{
    return icd_clIcdGetPlatformIDsKHR(num_entries, platforms, num_platforms);
}

/* unsuported by ocland currently
CL_API_ENTRY cl_int CL_API_CALL clTerminateContextKHR(cl_context context) CL_EXT_SUFFIX__VERSION_1_2
{
    return icd_clTerminateContextKHR(context);
}
*/

CL_API_ENTRY cl_int CL_API_CALL
clReleaseDeviceEXT( cl_device_id device) CL_EXT_SUFFIX__VERSION_1_1
{
    return icd_clReleaseDeviceEXT(device);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainDeviceEXT( cl_device_id device) CL_EXT_SUFFIX__VERSION_1_1
{
    return icd_clRetainDeviceEXT(device);
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateSubDevicesEXT(cl_device_id   in_device,
                      const cl_device_partition_property_ext * properties,
                      cl_uint        num_entries,
                      cl_device_id * out_devices,
                      cl_uint *      num_devices) CL_EXT_SUFFIX__VERSION_1_1
{
    return icd_clCreateSubDevicesEXT(in_device, properties, num_entries, out_devices, num_devices);
}

// OpenCL + OpenGL API
CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLBuffer(cl_context   context,
                     cl_mem_flags flags,
                     cl_GLuint    bufobj,
                     int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateFromGLBuffer(context, flags, bufobj, errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLTexture(cl_context   context,
                      cl_mem_flags flags,
                      cl_GLenum    target,
                      cl_GLint     miplevel,
                      cl_GLuint    texture,
                      cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    return icd_clCreateFromGLTexture(context, flags, target, miplevel, texture, errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLRenderbuffer(cl_context   context,
                           cl_mem_flags flags,
                           cl_GLuint    renderbuffer,
                           cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clCreateFromGLRenderbuffer(context, flags, renderbuffer, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetGLObjectInfo(cl_mem              memobj,
                  cl_gl_object_type * gl_object_type,
                  cl_GLuint *         gl_object_name) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetGLObjectInfo(memobj, gl_object_type, gl_object_name);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetGLTextureInfo(cl_mem             memobj,
                   cl_gl_texture_info param_name,
                   size_t             param_value_size,
                   void *             param_value,
                   size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetGLTextureInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAcquireGLObjects(cl_command_queue command_queue,
                          cl_uint          num_objects,
                          const cl_mem *   mem_objects,
                          cl_uint          num_events_in_wait_list,
                          const cl_event * event_wait_list,
                          cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueAcquireGLObjects(command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReleaseGLObjects(cl_command_queue command_queue,
                          cl_uint          num_objects,
                          const cl_mem *   mem_objects,
                          cl_uint          num_events_in_wait_list,
                          const cl_event * event_wait_list,
                          cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clEnqueueReleaseGLObjects(command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event);
}

// Deprecated OpenCL 1.1 APIs
CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
clCreateFromGLTexture2D(cl_context      context,
                        cl_mem_flags    flags,
                        cl_GLenum       target,
                        cl_GLint        miplevel,
                        cl_GLuint       texture,
                        cl_int *        errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return icd_clCreateFromGLTexture2D(context, flags, target, miplevel, texture, errcode_ret);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
clCreateFromGLTexture3D(cl_context   context,
                        cl_mem_flags flags,
                        cl_GLenum    target,
                        cl_GLint     miplevel,
                        cl_GLuint    texture,
                        cl_int *     errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return icd_clCreateFromGLTexture3D(context, flags, target, miplevel, texture, errcode_ret);
}

// Khronos OpenGL sharing extension
CL_API_ENTRY cl_int CL_API_CALL
clGetGLContextInfoKHR(const cl_context_properties * properties,
                      cl_gl_context_info            param_name,
                      size_t                        param_value_size,
                      void *                        param_value,
                      size_t *                      param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return icd_clGetGLContextInfoKHR(properties, param_name, param_value_size, param_value, param_value_size_ret);
}

// Khronos OpenGL event extension
CL_API_ENTRY cl_event CL_API_CALL
clCreateEventFromGLsyncKHR(cl_context context,
                           cl_GLsync  cl_GLsync,
                           cl_int *   errcode_ret) CL_EXT_SUFFIX__VERSION_1_1
{
    return icd_clCreateEventFromGLsyncKHR(context, cl_GLsync, errcode_ret);
}
#pragma GCC visibility pop
#else
    #define SYMB(f) typeof(icd_##f) f __attribute__ ((alias ("icd_" #f), visibility("default")))
    SYMB(clGetPlatformIDs);
    SYMB(clGetPlatformInfo);
    SYMB(clGetDeviceIDs);
    SYMB(clGetDeviceInfo);
    SYMB(clCreateSubDevices);
    SYMB(clRetainDevice);
    SYMB(clReleaseDevice);
    SYMB(clCreateContext);
    SYMB(clCreateContextFromType);
    SYMB(clRetainContext);
    SYMB(clReleaseContext);
    SYMB(clGetContextInfo);
    SYMB(clCreateCommandQueue);
    SYMB(clRetainCommandQueue);
    SYMB(clReleaseCommandQueue);
    SYMB(clGetCommandQueueInfo);
    SYMB(clCreateBuffer);
    SYMB(clCreateSubBuffer);
    SYMB(clCreateImage);
    SYMB(clRetainMemObject);
    SYMB(clReleaseMemObject);
    SYMB(clGetSupportedImageFormats);
    SYMB(clGetMemObjectInfo);
    SYMB(clGetImageInfo);
    SYMB(clSetMemObjectDestructorCallback);
    SYMB(clCreateSampler);
    SYMB(clRetainSampler);
    SYMB(clReleaseSampler);
    SYMB(clGetSamplerInfo);
    SYMB(clCreateProgramWithSource);
    SYMB(clCreateProgramWithBinary);
    SYMB(clCreateProgramWithBuiltInKernels);
    SYMB(clRetainProgram);
    SYMB(clReleaseProgram);
    SYMB(clBuildProgram);
    SYMB(clCompileProgram);
    SYMB(clLinkProgram);
    SYMB(clUnloadPlatformCompiler);
    SYMB(clGetProgramInfo);
    SYMB(clGetProgramBuildInfo);
    SYMB(clCreateKernel);
    SYMB(clCreateKernelsInProgram);
    SYMB(clRetainKernel);
    SYMB(clReleaseKernel);
    SYMB(clSetKernelArg);
    SYMB(clGetKernelInfo);
    SYMB(clGetKernelArgInfo);
    SYMB(clGetKernelWorkGroupInfo);
    SYMB(clWaitForEvents);
    SYMB(clGetEventInfo);
    SYMB(clCreateUserEvent);
    SYMB(clRetainEvent);
    SYMB(clReleaseEvent);
    SYMB(clSetUserEventStatus);
    SYMB(clSetEventCallback);
    SYMB(clGetEventProfilingInfo);
    SYMB(clFlush);
    SYMB(clFinish);
    SYMB(clEnqueueReadBuffer);
    SYMB(clEnqueueReadBufferRect);
    SYMB(clEnqueueWriteBuffer);
    SYMB(clEnqueueWriteBufferRect);
    SYMB(clEnqueueFillBuffer);
    SYMB(clEnqueueCopyBuffer);
    SYMB(clEnqueueCopyBufferRect);
    SYMB(clEnqueueReadImage);
    SYMB(clEnqueueWriteImage);
    SYMB(clEnqueueFillImage);
    SYMB(clEnqueueCopyImage);
    SYMB(clEnqueueCopyImageToBuffer);
    SYMB(clEnqueueCopyBufferToImage);
    SYMB(clEnqueueMapBuffer);
    SYMB(clEnqueueMapImage);
    SYMB(clEnqueueUnmapMemObject);
    SYMB(clEnqueueMigrateMemObjects);
    SYMB(clEnqueueNDRangeKernel);
    SYMB(clEnqueueTask);
    SYMB(clEnqueueNativeKernel);
    SYMB(clEnqueueMarkerWithWaitList);
    SYMB(clEnqueueBarrierWithWaitList);
    SYMB(clGetExtensionFunctionAddressForPlatform);
    SYMB(clCreateImage2D);
    SYMB(clCreateImage3D);
    SYMB(clEnqueueMarker);
    SYMB(clEnqueueWaitForEvents);
    SYMB(clEnqueueBarrier);
    SYMB(clUnloadCompiler);
    SYMB(clGetExtensionFunctionAddress);
    SYMB(clIcdGetPlatformIDsKHR);
    //SYMB(clTerminateContextKHR);
    SYMB(clReleaseDeviceEXT);
    SYMB(clRetainDeviceEXT);
    SYMB(clCreateSubDevicesEXT);
    SYMB(clCreateFromGLBuffer);
    SYMB(clCreateFromGLTexture);
    SYMB(clCreateFromGLRenderbuffer);
    SYMB(clGetGLObjectInfo);
    SYMB(clGetGLTextureInfo);
    SYMB(clEnqueueAcquireGLObjects);
    SYMB(clEnqueueReleaseGLObjects);
    SYMB(clCreateFromGLTexture2D);
    SYMB(clCreateFromGLTexture3D);
    SYMB(clGetGLContextInfoKHR);
    SYMB(clCreateEventFromGLsyncKHR);
#endif

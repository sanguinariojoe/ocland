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

#include <ocland/client/ocland_opencl.h>

#include <stdio.h>
#include <string.h>

// Log macros
static char err_str[64];
const char* OpenCLError(cl_int err_code)
{
    switch(err_code){
    case CL_SUCCESS:
        strcpy(err_str, "CL_SUCCESS");
        break;
    case CL_DEVICE_NOT_FOUND:
        strcpy(err_str, "CL_DEVICE_NOT_FOUND");
        break;
    case CL_DEVICE_NOT_AVAILABLE:
        strcpy(err_str, "CL_DEVICE_NOT_AVAILABLE");
        break;
    case CL_COMPILER_NOT_AVAILABLE:
        strcpy(err_str, "CL_COMPILER_NOT_AVAILABLE");
        break;
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        strcpy(err_str, "CL_MEM_OBJECT_ALLOCATION_FAILURE");
        break;
    case CL_OUT_OF_RESOURCES:
        strcpy(err_str, "CL_OUT_OF_RESOURCES");
        break;
    case CL_OUT_OF_HOST_MEMORY:
        strcpy(err_str, "CL_OUT_OF_HOST_MEMORY");
        break;
    case CL_PROFILING_INFO_NOT_AVAILABLE:
        strcpy(err_str, "CL_PROFILING_INFO_NOT_AVAILABLE");
        break;
    case CL_MEM_COPY_OVERLAP:
        strcpy(err_str, "CL_MEM_COPY_OVERLAP");
        break;
    case CL_IMAGE_FORMAT_MISMATCH:
        strcpy(err_str, "CL_IMAGE_FORMAT_MISMATCH");
        break;
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        strcpy(err_str, "CL_IMAGE_FORMAT_NOT_SUPPORTED");
        break;
    case CL_BUILD_PROGRAM_FAILURE:
        strcpy(err_str, "CL_BUILD_PROGRAM_FAILURE");
        break;
    case CL_MAP_FAILURE:
        strcpy(err_str, "CL_MAP_FAILURE");
        break;
    case CL_MISALIGNED_SUB_BUFFER_OFFSET:
        strcpy(err_str, "CL_MISALIGNED_SUB_BUFFER_OFFSET");
        break;
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
        strcpy(err_str, "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST");
        break;
    case CL_COMPILE_PROGRAM_FAILURE:
        strcpy(err_str, "CL_COMPILE_PROGRAM_FAILURE");
        break;
    case CL_LINKER_NOT_AVAILABLE:
        strcpy(err_str, "CL_LINKER_NOT_AVAILABLE");
        break;
    case CL_LINK_PROGRAM_FAILURE:
        strcpy(err_str, "CL_LINK_PROGRAM_FAILURE");
        break;
    case CL_DEVICE_PARTITION_FAILED:
        strcpy(err_str, "CL_DEVICE_PARTITION_FAILED");
        break;
    case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
        strcpy(err_str, "CL_KERNEL_ARG_INFO_NOT_AVAILABLE");
        break;
    case CL_INVALID_VALUE:
        strcpy(err_str, "CL_INVALID_VALUE");
        break;
    case CL_INVALID_DEVICE_TYPE:
        strcpy(err_str, "CL_INVALID_DEVICE_TYPE");
        break;
    case CL_INVALID_PLATFORM:
        strcpy(err_str, "CL_INVALID_PLATFORM");
        break;
    case CL_INVALID_DEVICE:
        strcpy(err_str, "CL_INVALID_DEVICE");
        break;
    case CL_INVALID_CONTEXT:
        strcpy(err_str, "CL_INVALID_CONTEXT");
        break;
    case CL_INVALID_QUEUE_PROPERTIES:
        strcpy(err_str, "CL_INVALID_QUEUE_PROPERTIES");
        break;
    case CL_INVALID_COMMAND_QUEUE:
        strcpy(err_str, "CL_INVALID_COMMAND_QUEUE");
        break;
    case CL_INVALID_HOST_PTR:
        strcpy(err_str, "CL_INVALID_HOST_PTR");
        break;
    case CL_INVALID_MEM_OBJECT:
        strcpy(err_str, "CL_INVALID_MEM_OBJECT");
        break;
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        strcpy(err_str, "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR");
        break;
    case CL_INVALID_IMAGE_SIZE:
        strcpy(err_str, "CL_INVALID_IMAGE_SIZE");
        break;
    case CL_INVALID_SAMPLER:
        strcpy(err_str, "CL_INVALID_SAMPLER");
        break;
    case CL_INVALID_BINARY:
        strcpy(err_str, "CL_INVALID_BINARY");
        break;
    case CL_INVALID_BUILD_OPTIONS:
        strcpy(err_str, "CL_INVALID_BUILD_OPTIONS");
        break;
    case CL_INVALID_PROGRAM:
        strcpy(err_str, "CL_INVALID_PROGRAM");
        break;
    case CL_INVALID_PROGRAM_EXECUTABLE:
        strcpy(err_str, "CL_INVALID_PROGRAM_EXECUTABLE");
        break;
    case CL_INVALID_KERNEL_NAME:
        strcpy(err_str, "CL_INVALID_KERNEL_NAME");
        break;
    case CL_INVALID_KERNEL_DEFINITION:
        strcpy(err_str, "CL_INVALID_KERNEL_DEFINITION");
        break;
    case CL_INVALID_KERNEL:
        strcpy(err_str, "CL_INVALID_KERNEL");
        break;
    case CL_INVALID_ARG_INDEX:
        strcpy(err_str, "CL_INVALID_ARG_INDEX");
        break;
    case CL_INVALID_ARG_VALUE:
        strcpy(err_str, "CL_INVALID_ARG_VALUE");
        break;
    case CL_INVALID_ARG_SIZE:
        strcpy(err_str, "CL_INVALID_ARG_SIZE");
        break;
    case CL_INVALID_KERNEL_ARGS:
        strcpy(err_str, "CL_INVALID_KERNEL_ARGS");
        break;
    case CL_INVALID_WORK_DIMENSION:
        strcpy(err_str, "CL_INVALID_WORK_DIMENSION");
        break;
    case CL_INVALID_WORK_GROUP_SIZE:
        strcpy(err_str, "CL_INVALID_WORK_GROUP_SIZE");
        break;
    case CL_INVALID_WORK_ITEM_SIZE:
        strcpy(err_str, "CL_INVALID_WORK_ITEM_SIZE");
        break;
    case CL_INVALID_GLOBAL_OFFSET:
        strcpy(err_str, "CL_INVALID_GLOBAL_OFFSET");
        break;
    case CL_INVALID_EVENT_WAIT_LIST:
        strcpy(err_str, "CL_INVALID_EVENT_WAIT_LIST");
        break;
    case CL_INVALID_EVENT:
        strcpy(err_str, "CL_INVALID_EVENT");
        break;
    case CL_INVALID_OPERATION:
        strcpy(err_str, "CL_INVALID_OPERATION");
        break;
    case CL_INVALID_GL_OBJECT:
        strcpy(err_str, "CL_INVALID_GL_OBJECT");
        break;
    case CL_INVALID_BUFFER_SIZE:
        strcpy(err_str, "CL_INVALID_BUFFER_SIZE");
        break;
    case CL_INVALID_MIP_LEVEL:
        strcpy(err_str, "CL_INVALID_MIP_LEVEL");
        break;
    case CL_INVALID_GLOBAL_WORK_SIZE:
        strcpy(err_str, "CL_INVALID_GLOBAL_WORK_SIZE");
        break;
    case CL_INVALID_PROPERTY:
        strcpy(err_str, "CL_INVALID_PROPERTY");
        break;
    case CL_INVALID_IMAGE_DESCRIPTOR:
        strcpy(err_str, "CL_INVALID_IMAGE_DESCRIPTOR");
        break;
    case CL_INVALID_COMPILER_OPTIONS:
        strcpy(err_str, "CL_INVALID_COMPILER_OPTIONS");
        break;
    case CL_INVALID_LINKER_OPTIONS:
        strcpy(err_str, "CL_INVALID_LINKER_OPTIONS");
        break;
    case CL_INVALID_DEVICE_PARTITION_COUNT:
        strcpy(err_str, "CL_INVALID_DEVICE_PARTITION_COUNT");
        break;
    default:
        sprintf(err_str, "%d", err_code);
        break;
    }
    return err_str;
}

#define WHERESTR  "[file %s, line %d]: "
#define WHEREARG  __FILE__, __LINE__
#define DEBUGPRINT2(...)       fprintf(stderr, __VA_ARGS__)
#define DEBUGPRINT(_fmt, ...)  DEBUGPRINT2(WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#ifdef OCLAND_CLIENT_VERBOSE
    #define VERBOSE_IN() {printf("[line %d]: %s...\n", __LINE__, __func__); fflush(stdout);}
    #define VERBOSE_OUT(flag) {printf("\t%s -> %s\n", __func__, OpenCLError(flag)); fflush(stdout);}
    #define VERBOSE(...) {printf(__VA_ARGS__); fflush(stdout);}
#else
    #define VERBOSE_IN()
    #define VERBOSE_OUT(flag)
    #define VERBOSE(...)
#endif

#ifndef MAX_N_PLATFORMS
    #define MAX_N_PLATFORMS 1<<16 //   65536
#endif
#ifndef MAX_N_DEVICES
    #define MAX_N_DEVICES 1<<16   //   65536
#endif

#define SYMB(f) \
typeof(icd_##f) f __attribute__ ((alias ("icd_" #f), visibility("default")))

#pragma GCC visibility push(hidden)

/// Number of known platforms
cl_uint num_master_platforms = 0;
/// List of known platforms
cl_platform_id *master_platforms = NULL;
/// Number of known devices
cl_uint num_master_devices = 0;
/// List of known devices
cl_device_id *master_devices = NULL;

cl_uint num_master_contexts = 0;
cl_context *master_contexts = NULL;
cl_uint num_master_queues = 0;
cl_command_queue *master_queues = NULL;
cl_uint num_master_mems = 0;
cl_mem *master_mems = NULL;
cl_uint num_master_samplers = 0;
cl_mem *master_samplers = NULL;
cl_uint num_master_programs = 0;
cl_program *master_programs = NULL;
cl_uint num_master_kernels = 0;
cl_kernel *master_kernels = NULL;
cl_uint num_master_events = 0;
cl_event *master_events = NULL;

/** Check for platforms validity
 * @param platform Platform to check
 * @return 1 if the platform is a known platform, 0 otherwise.
 */
int isPlatform(cl_platform_id platform){
    cl_uint i;
    for(i = 0; i < num_master_platforms; i++){
        if(platform == master_platforms[i])
            return 1;
    }
    return 0;
}

/** Check for devices validity
 * @param device Device to check
 * @return 1 if the device is a known device, 0 otherwise.
 */
int isDevice(cl_device_id device){
    cl_uint i;
    for(i=0;i<num_master_devices;i++){
        if(device == master_devices[i])
            return 1;
    }
    return 0;
}

/** Check for context validity
 * @param context Context to check
 * @return 1 if the context is a known context, 0 otherwise.
 */
int isContext(cl_context context){
    cl_uint i;
    for(i=0;i<num_master_contexts;i++){
        if(context == master_contexts[i])
            return 1;
    }
    return 0;
}

/** Check for command queue validity
 * @param command_queue Command queue to check
 * @return 1 if command_queue is a known command queue, 0 otherwise.
 */
int isCommandQueue(cl_command_queue command_queue){
    cl_uint i;
    for(i=0;i<num_master_queues;i++){
        if(command_queue == master_queues[i])
            return 1;
    }
    return 0;
}

/** Check for memory object validity
 * @param mem_obj Memory object to check
 * @return 1 if mem_obj is a known memory object, 0 otherwise.
 */
int isMemObject(cl_mem mem_obj){
    cl_uint i;
    for(i=0;i<num_master_mems;i++){
        if(mem_obj == master_mems[i])
            return 1;
    }
    return 0;
}

/** Check for sampler validity
 * @param sampler Sampler to check
 * @return 1 if the sampler is a known sampler, 0 otherwise.
 */
int isSampler(cl_sampler sampler){
    cl_uint i;
    for(i=0;i<num_master_samplers;i++){
        if(sampler == master_samplers[i])
            return 1;
    }
    return 0;
}

/** Check for program validity
 * @param program Program to check
 * @return 1 if the program is a known program, 0 otherwise.
 */
int isProgram(cl_program program){
    cl_uint i;
    for(i=0;i<num_master_programs;i++){
        if(program == master_programs[i])
            return 1;
    }
    return 0;
}

/** Check for kernel validity
 * @param kernel Kernel to check
 * @return 1 if the kernel is a known kernel, 0 otherwise.
 */
int isKernel(cl_kernel kernel){
    cl_uint i;
    for(i=0;i<num_master_kernels;i++){
        if(kernel == master_kernels[i])
            return 1;
    }
    return 0;
}

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

static cl_int
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

    cl_uint i, j;
    cl_int err_code;
    // Init platforms array
    if(!num_master_platforms){
        err_code = oclandGetPlatformIDs(0,
                                        NULL,
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
        cl_platform_id *server_platforms = (cl_platform_id*)malloc(
            num_master_platforms * sizeof(cl_platform_id));
        int *server_sockets = (int*)malloc(
            num_master_platforms * sizeof(int));
        if(!master_platforms || !server_platforms || !server_sockets){
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return CL_OUT_OF_HOST_MEMORY;
        }
        err_code = oclandGetPlatformIDs(num_master_platforms,
                                        server_platforms,
                                        server_sockets,
                                        NULL);
        if(err_code != CL_SUCCESS){
            VERBOSE_OUT(err_code);
            return err_code;
        }
        // Send data to master_platforms
        for(i =0 ; i < num_master_platforms; i++){
            master_platforms[i] = (cl_platform_id)malloc(
                sizeof(struct _cl_platform_id));
            master_platforms[i]->dispatch = &master_dispatch;
            master_platforms[i]->ptr      = server_platforms[i];
            master_platforms[i]->socket   = server_sockets[i];
        }
        free(server_platforms); server_platforms=NULL;
        free(server_sockets); server_sockets=NULL;
    }
    // Send the requested data
    if(!num_master_platforms){
        VERBOSE_OUT(CL_PLATFORM_NOT_FOUND_KHR);
        return CL_PLATFORM_NOT_FOUND_KHR;
    }
    if( num_platforms )
        *num_platforms = num_master_platforms;
    if( platforms ) {
        cl_uint i;
        for(i=0; i<(num_master_platforms<num_entries?num_master_platforms:num_entries); i++){
            platforms[i] = master_platforms[i];
        }
    }
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
SYMB(clGetPlatformIDs);

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
SYMB(clIcdGetPlatformIDsKHR);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetPlatformInfo(cl_platform_id   platform,
                      cl_platform_info param_name,
                      size_t           param_value_size,
                      void *           param_value,
                      size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isPlatform(platform)){
        VERBOSE_OUT(CL_INVALID_PLATFORM);
        return CL_INVALID_PLATFORM;
    }
    if((!param_value_size &&  param_value) ||
       ( param_value_size && !param_value)) {
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    // Connect to servers to get info
    cl_int flag = oclandGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clGetPlatformInfo);

// --------------------------------------------------------------
// Devices
// --------------------------------------------------------------

static cl_int
icd_clGetDeviceIDs(cl_platform_id   platform,
                   cl_device_type   device_type,
                   cl_uint          num_entries,
                   cl_device_id *   devices,
                   cl_uint *        num_devices)
{
    cl_uint i,j;
    VERBOSE_IN();
    if(!isPlatform(platform)){
        VERBOSE_OUT(CL_INVALID_PLATFORM);
        return CL_INVALID_PLATFORM;
    }
    if(    (!num_entries &&  devices)
        || ( num_entries && !devices)
        || (!devices && !num_devices) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = oclandGetDeviceIDs(platform, device_type, num_entries,
                                     devices, num_devices);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }

    for(i=0;i<num_entries;i++){
        // Test if device has been already stored
        flag = CL_SUCCESS;
        for(j = 0; j < num_master_devices; j++){
            if(master_devices[j]->ptr == devices[i]){
                devices[i] = master_devices[j];
                flag = CL_INVALID_DEVICE;
                break;
            }
        }
        if(flag != CL_SUCCESS)
            continue;
        // Add the new device
        cl_device_id* backup = master_devices;
        master_devices = (cl_device_id*)malloc(
            (num_master_devices + 1) * sizeof(cl_device_id));
        for(j = 0; j < num_master_devices; j++){
            master_devices[j] = backup[j];
        }
        free(backup); backup = NULL;

        master_devices[num_master_devices] = (cl_device_id)malloc(
            sizeof(struct _cl_device_id));
        master_devices[num_master_devices]->dispatch = &master_dispatch;
        master_devices[num_master_devices]->ptr = devices[i];
        master_devices[num_master_devices]->rcount = 1;
        master_devices[num_master_devices]->socket = &(platform->socket);
        master_devices[num_master_devices]->platform = platform;

        devices[i] = master_devices[num_master_devices];
        num_master_devices++;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clGetDeviceIDs);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetDeviceInfo(cl_device_id    device,
                    cl_device_info  param_name,
                    size_t          param_value_size,
                    void *          param_value,
                    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isDevice(device)){
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
    else{
        cl_int flag = oclandGetDeviceInfo(device, param_name, param_value_size,
                                          param_value, param_value_size_ret);
        VERBOSE_OUT(flag);
        return flag;
    }

    if(param_value){
        if(param_value_size < size_ret){
            VERBOSE_OUT(CL_INVALID_VALUE);
            return CL_INVALID_VALUE;
        }
        memcpy(param_value, value, size_ret);
        cl_platform_id *plat = *((cl_platform_id*)param_value);
    }
    if(param_value_size_ret){
        *param_value_size_ret = size_ret;
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clGetDeviceInfo);

CL_API_ENTRY cl_int CL_API_CALL
icd_clCreateSubDevices(cl_device_id                         in_device,
                       const cl_device_partition_property * properties,
                       cl_uint                              num_entries,
                       cl_device_id                       * out_devices,
                       cl_uint                            * num_devices) CL_API_SUFFIX__VERSION_1_2
{
    cl_uint i, j;
    VERBOSE_IN();
    if(!isDevice(in_device)){
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
        num_properties++;   // Final zero must be counted
    }
    cl_int flag = oclandCreateSubDevices(in_device, properties, num_properties,
                                         num_entries, out_devices, num_devices);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }

    for(i=0;i<num_entries;i++){
        // Add the new device
        cl_device_id* backup = master_devices;
        master_devices = (cl_device_id*)malloc(
            (num_master_devices + 1) * sizeof(cl_device_id));
        for(j = 0; j < num_master_devices; j++){
            master_devices[j] = backup[j];
        }
        free(backup); backup = NULL;

        master_devices[num_master_devices] = (cl_device_id)malloc(
            sizeof(struct _cl_device_id));
        master_devices[num_master_devices]->dispatch = &master_dispatch;
        master_devices[num_master_devices]->ptr = out_devices[i];
        master_devices[num_master_devices]->rcount = 1;
        master_devices[num_master_devices]->socket = in_device->socket;
        master_devices[num_master_devices]->platform = in_device->platform;

        out_devices[i] = master_devices[num_master_devices];
        num_master_devices++;
    }

    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clCreateSubDevices);

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    if(!isDevice(device)){
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return CL_INVALID_DEVICE;
    }
    // return oclandRetainDevice(device);
    device->rcount++;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clRetainDevice);

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    if(!isDevice(device)){
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return CL_INVALID_DEVICE;
    }
    // Ensure that the object can be destroyed
    device->rcount--;
    if(device->rcount)
        return CL_SUCCESS;
    // Reference count has reached 0, object should be destroyed
    cl_uint i,j;
    cl_int flag = oclandReleaseDevice(device);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    free(device);
    for(i = 0; i < num_master_devices; i++){
        if(master_devices[i] == device){
            for(j = i + 1; j < num_master_devices; j++){
                master_devices[j - 1] = master_devices[j];
            }
            break;
        }
    }
    num_master_devices--;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clReleaseDevice);

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
    }
    // Look for the platform in the properties
    for(i = 0; i < num_properties - 1; i = i + 2){
        if(properties[i] == CL_CONTEXT_PLATFORM){
            platform = (cl_platform_id)(properties[i+1]);
        }
    }
    if(platform){
        if(!isPlatform(platform)){
            if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
            VERBOSE_OUT(CL_INVALID_PLATFORM);
            return NULL;
        }
    }

    for(i = 0; i < num_devices; i++){
        if(!isDevice(devices[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return NULL;
        }
        if(platform){
            if(devices[i]->platform != platform){
                if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
                VERBOSE_OUT(CL_INVALID_DEVICE);
                return NULL;
            }
        }
        else{
            platform = devices[i]->platform;
        }
    }
    if(!platform){
        if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
        VERBOSE_OUT(CL_INVALID_PLATFORM);
        return NULL;
    }

    /** @todo Implement callbacks to control and report errors.
     * The callbacks should, at least, check the conection with
     * the server.
     * For the time being, the callback is just ignored
     */
    cl_int flag;
    cl_context ptr = oclandCreateContext(platform, properties, num_properties,
                                         num_devices, devices, NULL,
                                         NULL, &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_context context = (cl_context)malloc(sizeof(struct _cl_context));
    if(!context){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    context->dispatch = &master_dispatch;
    context->ptr = ptr;
    context->rcount = 1;
    context->socket = &(platform->socket);
    context->platform = platform;
    context->properties =  NULL;
    context->num_properties = num_properties;
    if(properties){
        context->properties = (cl_context_properties*)malloc(num_properties*sizeof(cl_context_properties));
        if(!context->properties){
            if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return NULL;
        }
        memcpy(context->properties, properties, num_properties*sizeof(cl_context_properties));
    }
    context->num_devices = num_devices;
    context->devices = (cl_device_id*)malloc(num_devices*sizeof(cl_device_id));
    if(!context->devices){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    memcpy(context->devices, devices, num_devices*sizeof(cl_device_id));

    // Expand the memory objects array appending the new one
    cl_context *backup = master_contexts;
    num_master_contexts++;
    master_contexts = (cl_context*)malloc(num_master_contexts*sizeof(cl_context));
    memcpy(master_contexts, backup, (num_master_contexts-1)*sizeof(cl_context));
    free(backup);
    master_contexts[num_master_contexts-1] = context;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return context;
}
SYMB(clCreateContext);

CL_API_ENTRY cl_context CL_API_CALL
icd_clCreateContextFromType(const cl_context_properties * properties,
                            cl_device_type                device_type,
                            void (CL_CALLBACK *     pfn_notify)(const char *, const void *, size_t, void *),
                            void *                        user_data,
                            cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i,j;
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
    }
    // Look for platform in the properties
    for(i=0;i<num_properties-1;i=i+2){
        if(properties[i] == CL_CONTEXT_PLATFORM){
            platform = (cl_platform_id)(properties[i+1]);
        }
    }
    if(platform){
        if(!isPlatform(platform)){
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
    /** @todo Implement callbacks to control and report errors.
     * The callbacks should, at least, check the conection with
     * the server.
     * For the time being, the callback is just ignored
     */
    cl_int flag;
    cl_context ptr = oclandCreateContextFromType(platform, properties,
                                                 num_properties, device_type,
                                                 NULL, NULL, &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_context context = (cl_context)malloc(sizeof(struct _cl_context));
    if(!context){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    context->dispatch = &master_dispatch;
    context->ptr = ptr;
    context->rcount = 1;
    context->socket = &(platform->socket);
    context->platform = platform;
    context->properties =  NULL;
    context->num_properties = num_properties;
    if(properties){
        context->properties = (cl_context_properties*)malloc(num_properties*sizeof(cl_context_properties));
        if(!context->properties){
            if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
            return NULL;
        }
        memcpy(context->properties, properties, num_properties*sizeof(cl_context_properties));
    }

    size_t devices_size = 0;
    flag = oclandGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &devices_size);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    context->devices = (cl_device_id*)malloc(devices_size);
    if(!context->devices){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    flag = oclandGetContextInfo(context, CL_CONTEXT_DEVICES, devices_size, context->devices, NULL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    cl_uint n = devices_size / sizeof(cl_device_id);
    context->num_devices = n;
    for(i=0;i<n;i++){
        for(j = 0; j < num_master_devices; j++){
            if(master_devices[j]->ptr == context->devices[i]){
                context->devices[i] = master_devices[j];
                break;
            }
        }
    }

    // Expand the devices array appending the new one
    cl_context *backup = master_contexts;
    num_master_contexts++;
    master_contexts = (cl_context*)malloc(num_master_contexts*sizeof(cl_context));
    memcpy(master_contexts, backup, (num_master_contexts-1)*sizeof(cl_context));
    free(backup);
    master_contexts[num_master_contexts-1] = context;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return context;
}
SYMB(clCreateContextFromType);

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isContext(context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    // return oclandRetainContext(context);
    context->rcount++;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clRetainContext);

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isContext(context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    // Ensure that the object can be destroyed
    context->rcount--;
    if(context->rcount){
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    // Reference count has reached 0, so the object should be destroyed
    cl_uint i,j;
    cl_int flag = oclandReleaseContext(context);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    free(context->devices); context->devices = NULL;
    free(context->properties); context->properties = NULL;
    free(context);
    for(i=0;i<num_master_contexts;i++){
        if(master_contexts[i] == context){
            // Create a new array removing the selected one
            cl_context *backup = master_contexts;
            master_contexts = NULL;
            if(num_master_contexts-1)
                master_contexts = (cl_context*)malloc((num_master_contexts-1)*sizeof(cl_context));
            memcpy(master_contexts, backup, i*sizeof(cl_context));
            memcpy(master_contexts+i, backup+i+1, (num_master_contexts-1-i)*sizeof(cl_context));
            free(backup);
            break;
        }
    }
    num_master_contexts--;
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clReleaseContext);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetContextInfo(cl_context         context,
                     cl_context_info    param_name,
                     size_t             param_value_size,
                     void *             param_value,
                     size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isContext(context)){
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
    else if(param_name == CL_CONTEXT_DEVICES){
        size_ret = context->num_devices * sizeof(cl_device_id);
        value = context->devices;
    }
    else if(param_name == CL_CONTEXT_PROPERTIES){
        size_ret = context->num_properties * sizeof(cl_context_properties);
        value = context->properties;
    }
    else{
        cl_int flag = oclandGetContextInfo(context, param_name,
                                           param_value_size, param_value,
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
SYMB(clGetContextInfo);

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
    if(!isContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    if(!isDevice(device)){
        if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
        VERBOSE_OUT(CL_INVALID_DEVICE);
        return NULL;
    }

    cl_int flag;
    cl_command_queue ptr = oclandCreateCommandQueue(context,
                                                    device,
                                                    properties,
                                                    &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_command_queue command_queue = (cl_command_queue)malloc(
        sizeof(struct _cl_command_queue));
    if(!command_queue){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    command_queue->dispatch = &master_dispatch;
    command_queue->ptr = ptr;
    command_queue->rcount = 1;
    command_queue->socket = context->socket;
    command_queue->context = context;
    command_queue->device = device;
    command_queue->properties = properties;
    // Expand the memory objects array appending the new one
    cl_command_queue *backup = master_queues;
    num_master_queues++;
    master_queues = (cl_command_queue*)malloc(
        num_master_queues * sizeof(cl_command_queue));
    memcpy(master_queues, backup,
        (num_master_queues - 1) * sizeof(cl_command_queue));
    free(backup); backup = NULL;
    master_queues[num_master_queues-1] = command_queue;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return command_queue;
}
SYMB(clCreateCommandQueue);

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    // return oclandRetainCommandQueue(command_queue);
    command_queue->rcount++;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clRetainCommandQueue);

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    // Ensure that the object can be destroyed
    command_queue->rcount--;
    if(command_queue->rcount){
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    // Reference count has reached 0, object should be destroyed
    cl_uint i, j;
    cl_int flag = oclandReleaseCommandQueue(command_queue);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    free(command_queue);
    for(i = 0; i < num_master_queues; i++){
        if(master_queues[i] == command_queue){
            // Create a new array removing the selected one
            cl_command_queue *backup = master_queues;
            master_queues = NULL;
            if(num_master_queues - 1)
                master_queues = (cl_command_queue*)malloc(
                    (num_master_queues - 1) * sizeof(cl_command_queue));
            memcpy(master_queues, backup, i * sizeof(cl_command_queue));
            memcpy(master_queues + i,
                   backup + i + 1,
                   (num_master_queues - 1 - i) * sizeof(cl_command_queue));
            free(backup);
            break;
        }
    }
    num_master_queues--;
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clReleaseCommandQueue);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetCommandQueueInfo(cl_command_queue      command_queue,
                          cl_command_queue_info param_name,
                          size_t                param_value_size,
                          void *                param_value,
                          size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isCommandQueue(command_queue)){
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
    if(param_name == CL_QUEUE_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(command_queue->rcount);
    }
    else if(param_name == CL_QUEUE_CONTEXT){
        size_ret = sizeof(cl_context);
        value = &(command_queue->context);
    }
    else if(param_name == CL_QUEUE_DEVICE){
        size_ret = sizeof(cl_device_id);
        value = &(command_queue->device);
    }
    else if(param_name == CL_QUEUE_PROPERTIES){
        size_ret = sizeof(cl_command_queue_properties);
        value = &(command_queue->properties);
    }
    else{
        cl_int flag = oclandGetCommandQueueInfo(command_queue,
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
SYMB(clGetCommandQueueInfo);

CL_API_ENTRY cl_int CL_API_CALL
icd_clSetCommandQueueProperty(cl_command_queue             command_queue,
                              cl_command_queue_properties  properties,
                              cl_bool                      enable,
                              cl_command_queue_properties  old_properties) CL_EXT_SUFFIX__VERSION_1_0_DEPRECATED
{
    // It is a deprecated method which should not be used
    VERBOSE_IN();
    VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
    return CL_INVALID_COMMAND_QUEUE;
}
SYMB(clSetCommandQueueProperty);



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
    if(!isContext(context)){
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
    cl_mem ptr = (void*)oclandCreateBuffer(context, flags, size, host_ptr,
                                           &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_mem mem = (cl_mem)malloc(sizeof(struct _cl_mem));
    if(!mem){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    mem->dispatch = &master_dispatch;
    mem->ptr = ptr;
    mem->rcount = 1;
    mem->socket = context->socket;
    mem->context = context;
    mem->type = CL_MEM_OBJECT_BUFFER;
    // Buffer data
    mem->flags = flags;
    mem->size = size;
    mem->host_ptr = NULL;
    mem->map_count = 0;
    mem->maps = NULL;
    // Image data
    mem->image_format = NULL;
    mem->element_size = 0;
    mem->row_pitch = 0;
    mem->slice_pitch = 0;
    mem->width = 0;
    mem->height = 0;
    mem->depth = 0;
    // SubBuffer data
    mem->mem_associated = NULL;
    mem->offset = 0;

    if(flags & CL_MEM_ALLOC_HOST_PTR){
        mem->host_ptr = malloc(size);
        if(!mem->host_ptr){
            if(errcode_ret) *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
            VERBOSE_OUT(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            return NULL;
        }
    }
    else if(flags & CL_MEM_USE_HOST_PTR){
        mem->host_ptr = host_ptr;
    }
    // Expand the memory objects array appending the new one
    cl_mem *backup = master_mems;
    num_master_mems++;
    master_mems = (cl_mem*)malloc(num_master_mems*sizeof(cl_mem));
    memcpy(master_mems, backup, (num_master_mems-1)*sizeof(cl_mem));
    free(backup);
    master_mems[num_master_mems-1] = mem;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return mem;
}
SYMB(clCreateBuffer);

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isMemObject(memobj)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    // return oclandRetainMemObject(memobj);
    memobj->rcount++;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clRetainMemObject);

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isMemObject(memobj)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    // Ensure that the object can be destroyed
    memobj->rcount--;
    if(memobj->rcount){
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    // Reference count has reached 0, so the object should be destroyed
    cl_uint i,j;
    cl_int flag = oclandReleaseMemObject(memobj);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    if(memobj->flags & CL_MEM_ALLOC_HOST_PTR) free(memobj->host_ptr); memobj->host_ptr = NULL;
    if(memobj->image_format) free(memobj->image_format); memobj->image_format = NULL;
    if(memobj->maps) free(memobj->maps); memobj->maps = NULL;
    free(memobj);
    for(i=0;i<num_master_mems;i++){
        if(master_mems[i] == memobj){
            // Create a new array removing the selected one
            cl_mem *backup = master_mems;
            master_mems = NULL;
            if(num_master_mems-1)
                master_mems = (cl_mem*)malloc((num_master_mems-1)*sizeof(cl_mem));
            memcpy(master_mems, backup, i*sizeof(cl_mem));
            memcpy(master_mems+i, backup+i+1, (num_master_mems-1-i)*sizeof(cl_mem));
            free(backup);
            break;
        }
    }
    num_master_mems--;
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clReleaseMemObject);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetSupportedImageFormats(cl_context           context,
                               cl_mem_flags         flags,
                               cl_mem_object_type   image_type ,
                               cl_uint              num_entries ,
                               cl_image_format *    image_formats ,
                               cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isContext(context)){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    if(    (!num_entries &&  image_formats)
        || ( num_entries && !image_formats)){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag;
    flag = oclandGetSupportedImageFormats(context->ptr,flags,image_type,num_entries,image_formats,num_image_formats);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clGetSupportedImageFormats);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetMemObjectInfo(cl_mem            memobj ,
                       cl_mem_info       param_name ,
                       size_t            param_value_size ,
                       void *            param_value ,
                       size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isMemObject(memobj)){
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
    if(param_name == CL_MEM_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(memobj->rcount);
    }
    else if(param_name == CL_MEM_CONTEXT){
        size_ret = sizeof(cl_context);
        value = &(memobj->context);
    }
    else if(param_name == CL_MEM_TYPE){
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
    else if(param_name == CL_MEM_ASSOCIATED_MEMOBJECT){
        size_ret = sizeof(cl_mem);
        value = &(memobj->mem_associated);
    }
    else if(param_name == CL_MEM_OFFSET){
        size_ret = sizeof(size_t);
        value = &(memobj->offset);
    }
    else{
        cl_int flag = oclandGetMemObjectInfo(memobj,
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
SYMB(clGetMemObjectInfo);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetImageInfo(cl_mem            image ,
                   cl_image_info     param_name ,
                   size_t            param_value_size ,
                   void *            param_value ,
                   size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isMemObject(image)){
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
        value = &(image->row_pitch);
    }
    else if(param_name == CL_IMAGE_SLICE_PITCH){
        size_ret = sizeof(size_t);
        value = &(image->slice_pitch);
    }
    else if(param_name == CL_IMAGE_WIDTH){
        size_ret = sizeof(size_t);
        value = &(image->width);
    }
    else if(param_name == CL_IMAGE_HEIGHT){
        size_ret = sizeof(size_t);
        value = &(image->height);
    }
    else if(param_name == CL_IMAGE_DEPTH){
        size_ret = sizeof(size_t);
        value = &(image->depth);
    }
    else{
        cl_int flag = oclandGetImageInfo(image, param_name,
                                         param_value_size, param_value,
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
SYMB(clGetImageInfo);

CL_API_ENTRY cl_mem CL_API_CALL
icd_clCreateSubBuffer(cl_mem                    buffer ,
                      cl_mem_flags              flags ,
                      cl_buffer_create_type     buffer_create_type ,
                      const void *              buffer_create_info ,
                      cl_int *                  errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    VERBOSE_IN();
    if(!isMemObject(buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if( (flags & CL_MEM_ALLOC_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    if( (flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }

    cl_int flag;
    cl_mem ptr = oclandCreateSubBuffer(buffer,
                                       flags,
                                       buffer_create_type,
                                       buffer_create_info,
                                       &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_mem mem = (cl_mem)malloc(sizeof(struct _cl_mem));
    if(!mem){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    mem->dispatch = &master_dispatch;
    mem->ptr = ptr;
    mem->rcount = 1;
    mem->socket = buffer->socket;
    mem->context = buffer->context;
    mem->type = CL_MEM_OBJECT_BUFFER;
    // Buffer data
    mem->flags = flags;
    mem->size = ((cl_buffer_region*)buffer_create_info)->size;
    mem->host_ptr = NULL;
    mem->map_count = 0;
    mem->maps = NULL;
    // Image data
    mem->image_format = NULL;
    mem->element_size = 0;
    mem->row_pitch = 0;
    mem->slice_pitch = 0;
    mem->width = 0;
    mem->height = 0;
    mem->depth = 0;
    // SubBuffer data
    mem->mem_associated = buffer;
    mem->offset = ((cl_buffer_region*)buffer_create_info)->origin;

    if(flags & CL_MEM_ALLOC_HOST_PTR){
        mem->host_ptr = malloc(((cl_buffer_region*)buffer_create_info)->size);
        if(!mem->host_ptr){
            if(errcode_ret) *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
            VERBOSE_OUT(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            return NULL;
        }
    }
    else if(flags & CL_MEM_USE_HOST_PTR){
        mem->host_ptr =
            buffer->host_ptr + ((cl_buffer_region*)buffer_create_info)->origin;
    }
    // Expand the memory objects array appending the new one
    cl_mem *backup = master_mems;
    num_master_mems++;
    master_mems = (cl_mem*)malloc(num_master_mems*sizeof(cl_mem));
    memcpy(master_mems, backup, (num_master_mems-1)*sizeof(cl_mem));
    free(backup);
    master_mems[num_master_mems-1] = mem;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return mem;
}
SYMB(clCreateSubBuffer);

CL_API_ENTRY cl_int CL_API_CALL
icd_clSetMemObjectDestructorCallback(cl_mem  memobj ,
                                 void (CL_CALLBACK * pfn_notify)(cl_mem  memobj , void* user_data),
                                 void * user_data)             CL_API_SUFFIX__VERSION_1_1
{
    VERBOSE_IN();
    if(!isMemObject(memobj)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!pfn_notify){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    /** Callbacks can't be registered in ocland due
     * to the implicit network interface, so this
     * operation may fail ever.
     */
    VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
    return CL_OUT_OF_HOST_MEMORY;
}
SYMB(clSetMemObjectDestructorCallback);

CL_API_ENTRY cl_mem CL_API_CALL
icd_clCreateImage(cl_context              context,
                  cl_mem_flags            flags,
                  const cl_image_format * image_format,
                  const cl_image_desc *   image_desc,
                  void *                  host_ptr,
                  cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    size_t image_size = 0;
    unsigned int element_n = 1;
    size_t element_size = 0;
    VERBOSE_IN();
    if(!isContext(context)){
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

    // Compute the sizes of the image
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
    image_size = image_desc->image_depth*image_desc->image_height*image_desc->image_width * element_size;


    cl_int flag;
    cl_mem ptr = (void*)oclandCreateImage(context, flags, image_format,
                                          image_desc, element_size, host_ptr,
                                          &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_mem mem = (cl_mem)malloc(sizeof(struct _cl_mem));
    if(!mem){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    mem->dispatch = &master_dispatch;
    mem->ptr = ptr;
    mem->rcount = 1;
    mem->socket = context->socket;
    mem->context = context;
    mem->type = image_desc->image_type;
    // Buffer data
    mem->flags = flags;
    mem->size = image_size;
    mem->host_ptr = NULL;
    mem->map_count = 0;
    mem->maps = NULL;
    // Image data
    mem->image_format = (cl_image_format*)malloc(sizeof(cl_image_format));
    memcpy(mem->image_format, image_format, sizeof(cl_image_format));
    mem->element_size = element_size;
    mem->row_pitch = image_desc->image_row_pitch;
    mem->slice_pitch = image_desc->image_slice_pitch;
    mem->width = image_desc->image_width;
    mem->height = image_desc->image_height;
    mem->depth = image_desc->image_depth;
    // SubBuffer data
    mem->mem_associated = NULL;
    mem->offset = 0;
    if(flags & CL_MEM_ALLOC_HOST_PTR){
        mem->host_ptr = malloc(image_size);
        if(!mem->host_ptr){
            if(errcode_ret) *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
            VERBOSE_OUT(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            return NULL;
        }
    }
    else if(flags & CL_MEM_USE_HOST_PTR){
        mem->host_ptr = host_ptr;
    }
    // Expand the memory objects array appending the new one
    cl_mem *backup = master_mems;
    num_master_mems++;
    master_mems = (cl_mem*)malloc(num_master_mems*sizeof(cl_mem));
    memcpy(master_mems, backup, (num_master_mems-1)*sizeof(cl_mem));
    free(backup);
    master_mems[num_master_mems-1] = mem;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return mem;
}
SYMB(clCreateImage);

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
    size_t image_size = 0;
    unsigned int element_n = 1;
    size_t element_size = 0;
    VERBOSE_IN();
    if(!isContext(context)){
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
    if(!image_format){
        if(errcode_ret) *errcode_ret=CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        VERBOSE_OUT(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
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

    // Compute the sizes of the image
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
    image_size = image_height*image_width * element_size;


    cl_int flag;
    cl_mem ptr = oclandCreateImage2D(context, flags, image_format, image_width,
                                     image_height, image_row_pitch,
                                     element_size, host_ptr, &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_mem mem = (cl_mem)malloc(sizeof(struct _cl_mem));
    if(!mem){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    mem->dispatch = &master_dispatch;
    mem->ptr = ptr;
    mem->rcount = 1;
    mem->socket = context->socket;
    mem->context = context;
    mem->type = CL_MEM_OBJECT_IMAGE2D;
    // Buffer data
    mem->flags = flags;
    mem->size = image_size;
    mem->host_ptr = NULL;
    mem->map_count = 0;
    mem->maps = NULL;
    // Image data
    mem->image_format = (cl_image_format*)malloc(sizeof(cl_image_format));
    memcpy(mem->image_format, image_format, sizeof(cl_image_format));
    mem->element_size = element_size;
    mem->row_pitch = image_row_pitch;
    mem->slice_pitch = 0;
    mem->width = image_width;
    mem->height = image_height;
    mem->depth = 1;
    // SubBuffer data
    mem->mem_associated = NULL;
    mem->offset = 0;
    if(flags & CL_MEM_ALLOC_HOST_PTR){
        mem->host_ptr = malloc(image_size);
        if(!mem->host_ptr){
            if(errcode_ret) *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
            VERBOSE_OUT(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            return NULL;
        }
    }
    else if(flags & CL_MEM_USE_HOST_PTR){
        mem->host_ptr = host_ptr;
    }
    // Expand the memory objects array appending the new one
    cl_mem *backup = master_mems;
    num_master_mems++;
    master_mems = (cl_mem*)malloc(num_master_mems*sizeof(cl_mem));
    memcpy(master_mems, backup, (num_master_mems-1)*sizeof(cl_mem));
    free(backup);
    master_mems[num_master_mems-1] = mem;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return mem;
}
SYMB(clCreateImage2D);

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
    size_t image_size = 0;
    unsigned int element_n = 1;
    size_t element_size = 0;
    VERBOSE_IN();
    if(!isContext(context)){
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
    if(!image_format){
        if(errcode_ret) *errcode_ret=CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        VERBOSE_OUT(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
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

    // Compute the sizes of the image
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
    image_size = image_depth*image_height*image_width * element_size;

    cl_int flag;
    cl_mem ptr = oclandCreateImage3D(context, flags, image_format, image_width,
                                     image_height,image_depth, image_row_pitch,
                                     image_slice_pitch, element_size, host_ptr,
                                     &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_mem mem = (cl_mem)malloc(sizeof(struct _cl_mem));
    if(!mem){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    mem->dispatch = &master_dispatch;
    mem->ptr = ptr;
    mem->rcount = 1;
    mem->socket = context->socket;
    mem->context = context;
    mem->type = CL_MEM_OBJECT_IMAGE3D;
    // Buffer data
    mem->flags = flags;
    mem->size = image_size;
    mem->host_ptr = NULL;
    mem->map_count = 0;
    mem->maps = NULL;
    // Image data
    mem->image_format = (cl_image_format*)malloc(sizeof(cl_image_format));
    memcpy(mem->image_format, image_format, sizeof(cl_image_format));
    mem->element_size = element_size;
    mem->row_pitch = image_row_pitch;
    mem->slice_pitch = 1;
    mem->width = image_width;
    mem->height = image_height;
    mem->depth = 0;
    // SubBuffer data
    mem->mem_associated = NULL;
    mem->offset = 0;
    if(flags & CL_MEM_ALLOC_HOST_PTR){
        mem->host_ptr = malloc(image_size);
        if(!mem->host_ptr){
            if(errcode_ret) *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
            VERBOSE_OUT(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            return NULL;
        }
    }
    else if(flags & CL_MEM_USE_HOST_PTR){
        mem->host_ptr = host_ptr;
    }
    // Expand the memory objects array appending the new one
    cl_mem *backup = master_mems;
    num_master_mems++;
    master_mems = (cl_mem*)malloc(num_master_mems*sizeof(cl_mem));
    memcpy(master_mems, backup, (num_master_mems-1)*sizeof(cl_mem));
    free(backup);
    master_mems[num_master_mems-1] = mem;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return mem;
}
SYMB(clCreateImage3D);

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
    if(!isContext(context)){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }

    cl_int flag;
    cl_sampler ptr = (void*)oclandCreateSampler(context, normalized_coords,
                                                addressing_mode, filter_mode,
                                                &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_sampler sampler = (cl_sampler)malloc(sizeof(struct _cl_sampler));
    if(!sampler){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    sampler->dispatch = &master_dispatch;
    sampler->ptr = ptr;
    sampler->rcount = 1;
    sampler->socket = context->socket;
    sampler->context = context;
    sampler->normalized_coords = normalized_coords;
    sampler->addressing_mode = addressing_mode;
    sampler->filter_mode = filter_mode;
    // Expand the samplers array appending the new one
    cl_sampler *backup = master_samplers;
    num_master_samplers++;
    master_samplers = (cl_sampler*)malloc(num_master_samplers*sizeof(cl_sampler));
    memcpy(master_samplers, backup, (num_master_samplers-1)*sizeof(cl_sampler));
    free(backup);
    master_samplers[num_master_samplers-1] = sampler;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return sampler;
}
SYMB(clCreateSampler);

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainSampler(cl_sampler  sampler) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isSampler(sampler)){
        VERBOSE_OUT(CL_INVALID_SAMPLER);
        return CL_INVALID_SAMPLER;
    }
    // return oclandRetainSampler(sampler);
    sampler->rcount++;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clRetainSampler);

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseSampler(cl_sampler  sampler) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isSampler(sampler)){
        VERBOSE_OUT(CL_INVALID_SAMPLER);
        return CL_INVALID_SAMPLER;
    }
    // Ensure that the object can be destroyed
    sampler->rcount--;
    if(sampler->rcount){
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    // Reference count has reached 0, so the object should be destroyed
    cl_uint i,j;
    cl_int flag = oclandReleaseSampler(sampler);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    free(sampler);
    for(i=0;i<num_master_samplers;i++){
        if(master_samplers[i] == sampler){
            // Create a new array removing the selected one
            cl_sampler *backup = master_samplers;
            master_samplers = NULL;
            if(num_master_samplers-1)
                master_samplers = (cl_sampler*)malloc((num_master_samplers-1)*sizeof(cl_sampler));
            memcpy(master_samplers, backup, i*sizeof(cl_sampler));
            memcpy(master_samplers+i, backup+i+1, (num_master_samplers-1-i)*sizeof(cl_sampler));
            free(backup);
            break;
        }
    }
    num_master_samplers--;
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clReleaseSampler);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetSamplerInfo(cl_sampler          sampler ,
                     cl_sampler_info     param_name ,
                     size_t              param_value_size ,
                     void *              param_value ,
                     size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isSampler(sampler)){
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
    if(param_name == CL_PROGRAM_REFERENCE_COUNT){
        size_ret = sizeof(cl_uint);
        value = &(sampler->rcount);
    }
    else if(param_name == CL_SAMPLER_CONTEXT){
        size_ret = sizeof(cl_context);
        value = &(sampler->context);
    }
    else if(param_name == CL_SAMPLER_ADDRESSING_MODE){
        size_ret = sizeof(cl_addressing_mode);
        value = &(sampler->addressing_mode);
    }
    else if(param_name == CL_SAMPLER_FILTER_MODE){
        size_ret = sizeof(cl_filter_mode);
        value = &(sampler->filter_mode);
    }
    else if(param_name == CL_SAMPLER_NORMALIZED_COORDS){
        size_ret = sizeof(cl_bool);
        value = &(sampler->normalized_coords);
    }
    else{
        cl_int flag = oclandGetSamplerInfo(sampler, param_name,
                                           param_value_size, param_value,
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
SYMB(clGetSamplerInfo);

// --------------------------------------------------------------
// Programs
// --------------------------------------------------------------

cl_int setupProgram(cl_program program)
{
    cl_uint i;
    cl_int flag;
    size_t ret_size;

    // Set the program as unbuilt
    if(program->source) free(program->source);
    if(program->binary_lengths) free(program->binary_lengths);
    if(program->binaries){
        for(i = 0; i < program->num_devices; i++){
            if(program->binaries[i]) free(program->binaries[i]);
            program->binaries[i]=NULL;
        }
        free(program->binaries);
    }
    if(program->kernels) free(program->kernels);
    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->kernels = NULL;
    program->num_kernels = 0;

    // Allocate new memory
    program->binary_lengths = (size_t*)malloc(
        program->num_devices*sizeof(size_t));
    program->binaries = (unsigned char**)malloc(
        program->num_devices*sizeof(unsigned char*));
    if((!program->binary_lengths) ||
       (!program->binaries) ){
        return CL_OUT_OF_HOST_MEMORY;
    }
    for(i = 0; i < program->num_devices; i++){
        program->binary_lengths[i] = NULL;
        program->binaries[i] = NULL;
    }

    // Get the source code
    flag = oclandGetProgramInfo(program,
                                CL_PROGRAM_SOURCE,
                                0,
                                NULL,
                                &ret_size);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    if(ret_size){
        program->source = (char*)malloc(ret_size);
        if(!program->source){
            return CL_OUT_OF_HOST_MEMORY;
        }
        flag = oclandGetProgramInfo(program,
                                    CL_PROGRAM_SOURCE,
                                    ret_size,
                                    program->source,
                                    NULL);
        if(flag != CL_SUCCESS){
            return CL_OUT_OF_RESOURCES;
        }
    }

    // It is not safe to use ocland in OpenCL < 1.2 due to the impossibility
    // to know the memory address of each argument, and therefore we are not
    // trying to avoid failures anymore

    // Get the compiled binaries (if they are available)
    flag = oclandGetProgramInfo(program,
                                CL_PROGRAM_BINARY_SIZES,
                                program->num_devices * sizeof(size_t),
                                program->binary_lengths,
                                NULL);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }

    for(i = 0; i < program->num_devices; i++){
        if(!program->binary_lengths[i]){
            continue;
        }
        program->binaries[i] = (unsigned char*)malloc(
            program->binary_lengths[i]);
        if(!program->binaries[i]){
            return CL_OUT_OF_HOST_MEMORY;
        }
    }
    flag = oclandGetProgramInfo(program,
                                CL_PROGRAM_BINARIES,
                                program->num_devices * sizeof(unsigned char*),
                                program->binaries,
                                NULL);

    // Get the list of available kernels
    flag = oclandGetProgramInfo(program,
                                CL_PROGRAM_KERNEL_NAMES,
                                0,
                                NULL,
                                &ret_size);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    if(ret_size){
        program->kernels = (char*)malloc(ret_size);
        if(!program->kernels){
            return CL_OUT_OF_HOST_MEMORY;
        }
        flag = oclandGetProgramInfo(program,
                                    CL_PROGRAM_KERNEL_NAMES,
                                    ret_size,
                                    program->kernels,
                                    NULL);
        if(flag != CL_SUCCESS){
            return CL_OUT_OF_RESOURCES;
        }
        // Count the number of kernels
        cl_uint n_chars = ret_size / sizeof(char);
        program->num_kernels = 1;
        for(i = 0; i < n_chars; i++){
            if(program->kernels[i] == ';')
                program->num_kernels++;
        }
        if(program->kernels[n_chars - 1] == ';')
            program->num_kernels--;
    }

    return CL_SUCCESS;
}


CL_API_ENTRY cl_program CL_API_CALL
icd_clCreateProgramWithSource(cl_context         context ,
                              cl_uint            count ,
                              const char **      strings ,
                              const size_t *     lengths ,
                              cl_int *           errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    VERBOSE_IN();
    if(!isContext(context)){
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
    cl_program ptr = oclandCreateProgramWithSource(context,
                                                   count,
                                                   strings,
                                                   lengths,
                                                   &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_program program = (cl_program)malloc(sizeof(struct _cl_program));
    if(!program){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    program->dispatch = &master_dispatch;
    program->ptr = ptr;
    program->rcount = 1;
    program->socket = context->socket;
    program->context = context;

    // With clCreateProgramWithSource the devices list is the associated with
    // the context
    size_t devices_size;
    flag = icd_clGetContextInfo(context,
                                CL_CONTEXT_DEVICES,
                                0,
                                NULL,
                                &devices_size);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }
    program->num_devices = devices_size / sizeof(cl_device_id);
    program->devices = (cl_device_id*)malloc(devices_size);
    if(!program->devices){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    flag = icd_clGetContextInfo(context,
                                CL_CONTEXT_DEVICES,
                                devices_size,
                                program->devices,
                                NULL);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return NULL;
    }

    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->kernels = NULL;
    flag = setupProgram(program);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    // Expand the programs array appending the new one
    cl_program *backup = master_programs;
    num_master_programs++;
    master_programs = (cl_program*)malloc(
        num_master_programs * sizeof(cl_program));
    memcpy(master_programs,
           backup,
           (num_master_programs - 1) * sizeof(cl_program));
    free(backup);
    master_programs[num_master_programs - 1] = program;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return program;
}
SYMB(clCreateProgramWithSource);

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
    if(!isContext(context)){
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
        if(!isDevice(device_list[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return NULL;
        }
    }

    cl_int flag;
    cl_program ptr = oclandCreateProgramWithBinary(context->ptr,
                                                   num_devices,
                                                   device_list,
                                                   lengths,
                                                   binaries,
                                                   binary_status,
                                                   &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_program program = (cl_program)malloc(sizeof(struct _cl_program));
    if(!program){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    program->dispatch = &master_dispatch;
    program->ptr = ptr;
    program->rcount = 1;
    program->socket = context->socket;
    program->context = context;

    // With clCreateProgramWithBinary the devices list is the provided one
    program->num_devices = num_devices;
    program->devices = (cl_device_id*)malloc(
        num_devices * sizeof(cl_device_id));
    if(!program->devices){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    memcpy(program->devices, device_list, num_devices * sizeof(cl_device_id));

    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->kernels = NULL;
    flag = setupProgram(program);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    // Expand the programs array appending the new one
    cl_program *backup = master_programs;
    num_master_programs++;
    master_programs = (cl_program*)malloc(
        num_master_programs * sizeof(cl_program));
    memcpy(master_programs,
           backup,
           (num_master_programs - 1) * sizeof(cl_program));
    free(backup);
    master_programs[num_master_programs - 1] = program;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return program;
}
SYMB(clCreateProgramWithBinary);

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainProgram(cl_program  program) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    // return oclandRetainKernel(kernel->ptr);
    program->rcount++;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clRetainProgram);

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseProgram(cl_program  program) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    // Ensure that the object can be destroyed
    program->rcount--;
    if(program->rcount){
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    // Reference count has reached 0, so the object should be destroyed
    cl_uint i,j;
    cl_int flag = oclandReleaseProgram(program);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    if(program->devices) free(program->devices);
    if(program->source) free(program->source);
    if(program->binary_lengths) free(program->binary_lengths);
    for(i=0;i<program->num_devices;i++){
        if(program->binaries[i]) free(program->binaries[i]);
    }
    if(program->binaries) free(program->binaries);
    if(program->kernels) free(program->kernels);
    free(program);
    for(i=0;i<num_master_programs;i++){
        if(master_programs[i] == program){
            // Create a new array removing the selected one
            cl_program *backup = master_programs;
            master_programs = NULL;
            if(num_master_programs - 1)
                master_programs = (cl_program*)malloc(
                    (num_master_programs - 1) * sizeof(cl_program));
            memcpy(master_programs, backup, i * sizeof(cl_program));
            memcpy(master_programs + i,
                   backup + i + 1,
                   (num_master_programs - 1 - i) * sizeof(cl_program));
            free(backup);
            break;
        }
    }
    num_master_programs--;
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clReleaseProgram);

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
    if(!isProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    if(    ((!num_devices) && ( device_list))
        || (( num_devices) && (!device_list)) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    for(i=0;i<num_devices;i++){
        if(!isDevice(device_list[i])){
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return CL_INVALID_DEVICE;
        }
    }
    /** callbacks can't be implemented trought network, so
     * if you request a callback CL_OUT_OF_RESOURCES will
     * be reported.
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
    cl_int flag = oclandBuildProgram(program,
                                     num_devices,
                                     device_list,
                                     options,
                                     NULL,
                                     NULL);
    setupProgram(program);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clBuildProgram);

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
icd_clUnloadCompiler(void) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    // Deprecated function which, according to old specifications, is allways
    // returning CL_SUCCESS
    VERBOSE_IN();
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clUnloadCompiler);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetProgramInfo(cl_program          program ,
                     cl_program_info     param_name ,
                     size_t              param_value_size ,
                     void *              param_value ,
                     size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isProgram(program)){
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
        size_ret = sizeof(cl_device_id)*program->num_devices;
        value = program->devices;
    }
    else if(param_name == CL_PROGRAM_SOURCE){
        size_ret = sizeof(char)*(strlen(program->source)+1);
        value = program->source;
    }
    else if(param_name == CL_PROGRAM_BINARY_SIZES){
        size_ret = sizeof(size_t)*program->num_devices;
        value = program->binary_lengths;
    }
    else if(param_name == CL_PROGRAM_BINARIES){
        size_ret = sizeof(unsigned char*)*program->num_devices;
        value = program->binaries;
    }
    else if(param_name == CL_PROGRAM_BINARIES){
        size_ret = sizeof(unsigned char*)*program->num_devices;
        value = program->binaries;
    }
    else if(param_name == CL_PROGRAM_NUM_KERNELS){
        size_ret = sizeof(cl_uint);
        value = &(program->num_kernels);
    }
    else if(param_name == CL_PROGRAM_KERNEL_NAMES){
        size_ret = sizeof(unsigned char*)*program->num_kernels;
        value = program->kernels;
    }
    else{
        // What are you asking for?? Anyway, lets see if the server knows the
        // answer
        cl_int flag = oclandGetProgramInfo(program,
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
SYMB(clGetProgramInfo);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetProgramBuildInfo(cl_program             program ,
                          cl_device_id           device ,
                          cl_program_build_info  param_name ,
                          size_t                 param_value_size ,
                          void *                 param_value ,
                          size_t *               param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    if(!isDevice(device)){
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
    cl_int flag = oclandGetProgramBuildInfo(program,
                                            device,
                                            param_name,
                                            param_value_size,
                                            param_value,
                                            param_value_size_ret);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clGetProgramBuildInfo);

CL_API_ENTRY cl_program CL_API_CALL
icd_clCreateProgramWithBuiltInKernels(cl_context             context ,
                                      cl_uint                num_devices ,
                                      const cl_device_id *   device_list ,
                                      const char *           kernel_names ,
                                      cl_int *               errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    cl_uint i;
    if(!isContext(context)){
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
        if(!isDevice(device_list[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return NULL;
        }
    }
    cl_int flag;
    cl_program ptr = oclandCreateProgramWithBuiltInKernels(context->ptr,
                                                           num_devices,
                                                           device_list,
                                                           kernel_names,
                                                           &flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_program program = (cl_program)malloc(sizeof(struct _cl_program));
    if(!program){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    program->dispatch = &master_dispatch;
    program->ptr = ptr;
    program->rcount = 1;
    program->socket = context->socket;
    program->context = context;

    // With clCreateProgramWithBuiltInKernels the devices list is the provided one
    program->num_devices = num_devices;
    program->devices = (cl_device_id*)malloc(
        num_devices * sizeof(cl_device_id));
    if(!program->devices){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    memcpy(program->devices, device_list, num_devices * sizeof(cl_device_id));

    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->kernels = NULL;
    flag = setupProgram(program);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    // Expand the programs array appending the new one
    cl_program *backup = master_programs;
    num_master_programs++;
    master_programs = (cl_program*)malloc(
        num_master_programs * sizeof(cl_program));
    memcpy(master_programs,
           backup,
           (num_master_programs - 1) * sizeof(cl_program));
    free(backup);
    master_programs[num_master_programs - 1] = program;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return program;
}
SYMB(clCreateProgramWithBuiltInKernels);

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
    if(!isProgram(program)){
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
    for(i=0;i<num_devices;i++){
        if(!isDevice(device_list[i])){
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return CL_INVALID_DEVICE;
        }
    }
    /** callbacks can't be implemented trought network, so
     * if you request a callback CL_OUT_OF_RESOURCES will
     * be reported.
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
    cl_int flag = oclandCompileProgram(program,
                                       num_devices,
                                       device_list,
                                       options,
                                       num_input_headers,
                                       input_headers,
                                       header_include_names,
                                       NULL,
                                       NULL);
    setupProgram(program);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clCompileProgram);

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
    if(!isContext(context)){
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
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    for(i=0;i<num_devices;i++){
        if(!isDevice(device_list[i])){
            if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
            VERBOSE_OUT(CL_INVALID_DEVICE);
            return NULL;
        }
    }
    /** callbacks can't be implemented trought network, so
     * if you request a callback CL_OUT_OF_RESOURCES will
     * be reported.
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
    cl_program ptr = oclandLinkProgram(context,num_devices,device_list,options,
                                       num_input_programs,input_programs,NULL,
                                       NULL,&flag);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    cl_program program = (cl_program)malloc(sizeof(struct _cl_program));
    if(!program){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }

    program->dispatch = &master_dispatch;
    program->ptr = ptr;
    program->rcount = 1;
    program->socket = context->socket;
    program->context = context;

    // With clLinkProgram the devices list is the provided one
    program->num_devices = num_devices;
    program->devices = (cl_device_id*)malloc(
        num_devices * sizeof(cl_device_id));
    if(!program->devices){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    memcpy(program->devices, device_list, num_devices * sizeof(cl_device_id));

    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->kernels = NULL;
    flag = setupProgram(program);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    // Expand the programs array appending the new one
    cl_program *backup = master_programs;
    num_master_programs++;
    master_programs = (cl_program*)malloc(
        num_master_programs * sizeof(cl_program));
    memcpy(master_programs,
           backup,
           (num_master_programs - 1) * sizeof(cl_program));
    free(backup);
    master_programs[num_master_programs - 1] = program;

    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return program;
}
SYMB(clLinkProgram);

CL_API_ENTRY cl_int CL_API_CALL
icd_clUnloadPlatformCompiler(cl_platform_id  platform) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    cl_int flag = oclandUnloadPlatformCompiler(platform);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clUnloadPlatformCompiler);

// --------------------------------------------------------------
// Kernels
// --------------------------------------------------------------

cl_int setupKernelArg(cl_kernel kernel, cl_kernel_arg arg)
{
    cl_int flag;
    // Set initial data
    arg->address = 0;
    arg->access = 0;
    arg->type_name = NULL;
    arg->type = 0;
    arg->name = NULL;
    arg->bytes = 0;
    arg->value = NULL;
    // Get the available data
    size_t ret_size;
    flag = oclandGetKernelArgInfo(kernel,
                                  arg->index,
                                  CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                                  sizeof(cl_uint),
                                  &(arg->address),
                                  NULL);
    flag = oclandGetKernelArgInfo(kernel,
                                  arg->index,
                                  CL_KERNEL_ARG_ACCESS_QUALIFIER,
                                  sizeof(cl_uint),
                                  &(arg->access),
                                  NULL);
    flag = oclandGetKernelArgInfo(kernel,
                                  arg->index,
                                  CL_KERNEL_ARG_TYPE_NAME,
                                  0,
                                  NULL,
                                  &ret_size);
    if(flag == CL_SUCCESS){
        arg->type_name = (char*)malloc(ret_size);
        if(!arg->type_name){
            return CL_OUT_OF_HOST_MEMORY;
        }
        flag = oclandGetKernelArgInfo(kernel,
                                      arg->index,
                                      CL_KERNEL_ARG_TYPE_NAME,
                                      ret_size,
                                      arg->type_name,
                                      NULL);
    }
    flag = oclandGetKernelArgInfo(kernel,
                                  arg->index,
                                  CL_KERNEL_ARG_TYPE_QUALIFIER,
                                  sizeof(cl_uint),
                                  &(arg->type),
                                  NULL);
    flag = oclandGetKernelArgInfo(kernel,
                                  arg->index,
                                  CL_KERNEL_ARG_NAME,
                                  0,
                                  NULL,
                                  &ret_size);
    if(flag == CL_SUCCESS){
        arg->name = (char*)malloc(ret_size);
        if(!arg->name){
            return CL_OUT_OF_HOST_MEMORY;
        }
        flag = oclandGetKernelArgInfo(kernel,
                                      arg->index,
                                      CL_KERNEL_ARG_NAME,
                                      ret_size,
                                      arg->name,
                                      NULL);
    }
    return CL_SUCCESS;
}

cl_int setupKernel(cl_kernel kernel)
{
    cl_uint i,j;
    cl_int flag;
    // Set the kernel as unbuilt
    kernel->args = NULL;
    kernel->func_name = (char*)malloc(sizeof(char));
    strcpy(kernel->func_name, "");
    kernel->num_args = 0;
    // Get the kernel info
    size_t ret_size;
    flag = oclandGetKernelInfo(kernel,
                               CL_KERNEL_FUNCTION_NAME,
                               0,
                               NULL,
                               &ret_size);
    if(flag != CL_SUCCESS)
        return CL_OUT_OF_HOST_MEMORY;
    free(kernel->func_name);
    kernel->func_name = (char*)malloc(ret_size);
    if(!kernel->func_name)
        return CL_OUT_OF_HOST_MEMORY;
    flag = oclandGetKernelInfo(kernel,
                               CL_KERNEL_FUNCTION_NAME,
                               ret_size,
                               kernel->func_name,
                               NULL);
    if(flag != CL_SUCCESS)
        return CL_OUT_OF_HOST_MEMORY;
    flag = oclandGetKernelInfo(kernel,
                               CL_KERNEL_NUM_ARGS,
                               sizeof(cl_uint),
                               &(kernel->num_args),
                               NULL);
    if(flag != CL_SUCCESS)
        return CL_OUT_OF_HOST_MEMORY;
    VERBOSE("\tfunction: %s, arguments: %u\n",
            kernel->func_name,
            kernel->num_args);
    // Get the arguments data
    if(kernel->num_args){
        kernel->args = (cl_kernel_arg*)malloc(
            kernel->num_args * sizeof(cl_kernel_arg));
        if(!kernel->args)
            return CL_OUT_OF_HOST_MEMORY;
    }
    for(i = 0; i < kernel->num_args; i++){
        kernel->args[i] = (cl_kernel_arg)malloc(sizeof(struct _cl_kernel_arg));
        if(!kernel->args[i]){
            for(j = 0; j < i; j++){
                free(kernel->args[j]);
                kernel->args[j] = NULL;
            }
            free(kernel->args); kernel->args = NULL;
            return CL_OUT_OF_HOST_MEMORY;
        }
        kernel->args[i]->index = i;
        flag = setupKernelArg(kernel, kernel->args[i]);
        if(flag != CL_SUCCESS){
            for(j = 0; j < i; j++){
                free(kernel->args[j]);
                kernel->args[j] = NULL;
            }
            free(kernel->args); kernel->args = NULL;
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    return CL_SUCCESS;
}

CL_API_ENTRY cl_kernel CL_API_CALL
icd_clCreateKernel(cl_program       program ,
                   const char *     kernel_name ,
                   cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isProgram(program)){
        if(errcode_ret) *errcode_ret = CL_INVALID_PROGRAM;
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return NULL;
    }
    if(!kernel_name){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        VERBOSE_OUT(CL_INVALID_VALUE);
        return NULL;
    }
    // Create the remote kernel instance
    cl_int flag;
    cl_kernel ptr = oclandCreateKernel(program, kernel_name, &flag);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    // Build the new kernel instance
    cl_kernel kernel = (cl_kernel)malloc(sizeof(struct _cl_kernel));
    if(!kernel){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
        return NULL;
    }
    kernel->dispatch = &master_dispatch;
    kernel->ptr = ptr;
    kernel->rcount = 1;
    kernel->socket = program->socket;
    kernel->context = program->context;
    kernel->program = program;
    // Setup the kernel data
    flag = setupKernel(kernel);
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        VERBOSE_OUT(flag);
        return NULL;
    }

    // Expand the kernels array appending the new one
    cl_kernel *backup = master_kernels;
    num_master_kernels++;
    master_kernels = (cl_kernel*)malloc(num_master_kernels*sizeof(cl_kernel));
    memcpy(master_kernels, backup, (num_master_kernels-1)*sizeof(cl_kernel));
    free(backup);
    master_kernels[num_master_kernels-1] = kernel;
    if(errcode_ret) *errcode_ret = flag;
    VERBOSE_OUT(flag);
    return kernel;
}
SYMB(clCreateKernel);

CL_API_ENTRY cl_int CL_API_CALL
icd_clCreateKernelsInProgram(cl_program      program ,
                             cl_uint         num_kernels ,
                             cl_kernel *     kernels ,
                             cl_uint *       num_kernels_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i, j, n;
    VERBOSE_IN();
    if(!isProgram(program)){
        VERBOSE_OUT(CL_INVALID_PROGRAM);
        return CL_INVALID_PROGRAM;
    }
    if(    ( !kernels && !num_kernels_ret )
        || ( !kernels &&  num_kernels )
        || (  kernels && !num_kernels ) ){
        VERBOSE_OUT(CL_INVALID_VALUE);
        return CL_INVALID_VALUE;
    }
    cl_int flag = oclandCreateKernelsInProgram(program,
                                               num_kernels,
                                               kernels,
                                               &n);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    if(num_kernels_ret)
        *num_kernels_ret = n;
    if(num_kernels){
        n = num_kernels < n ? num_kernels:n;
        for(i = 0; i < n; i++){
            cl_kernel kernel = (cl_kernel)malloc(sizeof(struct _cl_kernel));
            if(!kernel){
                VERBOSE_OUT(CL_OUT_OF_HOST_MEMORY);
                return CL_OUT_OF_HOST_MEMORY;
            }

            kernel->dispatch = &master_dispatch;
            kernel->ptr      = kernels[i];
            kernel->rcount   = 1;
            kernel->socket   = program->socket;
            kernel->context  = program->context;
            kernel->program  = program;
            flag = setupKernel(kernel);
            if(flag != CL_SUCCESS){
                free(kernel);
                VERBOSE_OUT(flag);
                return flag;
            }
            kernels[i] = kernel;

            // Expand the kernels array appending the new one
            cl_kernel *backup = master_kernels;
            num_master_kernels++;
            master_kernels = (cl_kernel*)malloc(
                num_master_kernels * sizeof(cl_kernel));
            memcpy(master_kernels,
                   backup,
                   (num_master_kernels - 1) * sizeof(cl_kernel));
            free(backup);
            master_kernels[num_master_kernels - 1] = kernel;
        }
    }
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clCreateKernelsInProgram);

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainKernel(cl_kernel     kernel) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    // return oclandRetainKernel(kernel->ptr);
    kernel->rcount++;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clRetainKernel);

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseKernel(cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    // Ensure that the object can be destroyed
    kernel->rcount--;
    if(kernel->rcount){
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }
    // Reference count has reached 0, so the object should be destroyed
    cl_uint i,j;
    cl_int flag = oclandReleaseKernel(kernel);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    if(kernel->args){
        for(i = 0; i < kernel->num_args; i++){
            free(kernel->args[i]->type_name);
            kernel->args[i]->type_name = NULL;
            free(kernel->args[i]->name);
            kernel->args[i]->name = NULL;
            free(kernel->args[i]);
            kernel->args[i] = NULL;
        }
        free(kernel->args);
        kernel->args = NULL;
    }
    free(kernel->func_name);
    kernel->func_name = NULL;
    free(kernel);
    for(i = 0; i < num_master_kernels; i++){
        if(master_kernels[i] == kernel){
            // Create a new array removing the selected one
            cl_kernel *backup = master_kernels;
            master_kernels = NULL;
            if(num_master_kernels-1)
                master_kernels = (cl_kernel*)malloc(
                    (num_master_kernels - 1) * sizeof(cl_kernel));
            memcpy(master_kernels, backup, i * sizeof(cl_kernel));
            memcpy(master_kernels + i,
                   backup + i + 1,
                   (num_master_kernels - 1 - i) * sizeof(cl_kernel));
            free(backup);
            break;
        }
    }
    num_master_kernels--;
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clReleaseKernel);

CL_API_ENTRY cl_int CL_API_CALL
icd_clSetKernelArg(cl_kernel     kernel ,
                   cl_uint       arg_index ,
                   size_t        arg_size ,
                   const void *  arg_value) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    if(arg_index >= kernel->num_args){
        VERBOSE_OUT(CL_INVALID_ARG_INDEX);
        return CL_INVALID_ARG_INDEX;
    }
    if(!arg_size){
        VERBOSE_OUT(CL_INVALID_ARG_SIZE);
        return CL_INVALID_ARG_SIZE;
    }
    // Test if the passed argument is the same already set
    cl_kernel_arg arg = kernel->args[arg_index];
    if(arg_size == arg->bytes){
        if(!arg_value && !arg->value){
            // Local memory
            VERBOSE_OUT(CL_SUCCESS);
            return CL_SUCCESS;
        }
        if(!memcmp(arg_value, arg->value, arg_size)){
            VERBOSE_OUT(CL_SUCCESS);
            return CL_SUCCESS;
        }
    }
    // We are not heuristically checking if the argument is either a cl_mem or a
    // cl_sampler anymore, but we are using the arguments recovered data.
    // Therefore OpenCL 1.2 non-supported platforms will fail, however it is not
    // safe to try to do it in a different way.
    cl_kernel_arg_address_qualifier address;
    cl_int flag = clGetKernelArgInfo(kernel,
                                     arg_index,
                                     CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                                     sizeof(cl_kernel_arg_address_qualifier),
                                     &address,
                                     NULL);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    void *val = (void*)arg_value;
    if(   ((arg_size == sizeof(cl_mem)) || (arg_size == sizeof(cl_sampler)))
       && (address == CL_KERNEL_ARG_ADDRESS_GLOBAL)){
        // It is not a common object, so we should look for the object server
        // reference
        cl_mem mem_obj = *(cl_mem*)(arg_value);
        cl_sampler sampler = *(cl_sampler*)(arg_value);
        if(isMemObject(mem_obj)){
            val = (void*)(&(mem_obj->ptr));
        }
        else if(isSampler(sampler)){
            val = (void*)(&(sampler->ptr));
        }
    }
    flag = oclandSetKernelArg(kernel,
                              arg_index,
                              arg_size,
                              val);
    if(flag != CL_SUCCESS){
        VERBOSE_OUT(flag);
        return flag;
    }
    arg->bytes = arg_size;
    if(!arg_value){
        // Local memory
        arg->value = arg_value;
        VERBOSE_OUT(flag);
        return flag;
    }
    arg->value = malloc(arg_size);
    memcpy(arg->value, arg_value, arg_size);

    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clSetKernelArg);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetKernelInfo(cl_kernel        kernel ,
                    cl_kernel_info   param_name ,
                    size_t           param_value_size ,
                    void *           param_value ,
                    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isKernel(kernel)){
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
    else{
        // What are you asking for?
        // Anyway, let's see if the server knows it (>1.2 compatibility)
        cl_int flag = oclandGetKernelInfo(kernel,
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
SYMB(clGetKernelInfo);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetKernelWorkGroupInfo(cl_kernel                   kernel ,
                             cl_device_id                device ,
                             cl_kernel_work_group_info   param_name ,
                             size_t                      param_value_size ,
                             void *                      param_value ,
                             size_t *                    param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isKernel(kernel)){
        VERBOSE_OUT(CL_INVALID_KERNEL);
        return CL_INVALID_KERNEL;
    }
    if(!isDevice(device)){
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
    cl_int flag = oclandGetKernelWorkGroupInfo(kernel,
                                               device,
                                               param_name,
                                               param_value_size,
                                               param_value,
                                               param_value_size_ret);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clGetKernelWorkGroupInfo);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetKernelArgInfo(cl_kernel        kernel ,
                       cl_uint          arg_indx ,
                       cl_kernel_arg_info   param_name ,
                       size_t           param_value_size ,
                       void *           param_value ,
                       size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    if(!isKernel(kernel)){
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
        if(!arg->address){
            VERBOSE_OUT(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }
        size_ret = sizeof(cl_kernel_arg_address_qualifier);
        value = &(arg->address);
    }
    else if(param_name == CL_KERNEL_ARG_ACCESS_QUALIFIER){
        if(!arg->access){
            VERBOSE_OUT(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }
        size_ret = sizeof(cl_kernel_arg_access_qualifier);
        value = &(arg->access);
    }
    else if(param_name == CL_KERNEL_ARG_TYPE_NAME){
        if(!arg->type_name){
            VERBOSE_OUT(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }
        size_ret = sizeof(char)*(strlen(arg->type_name) + 1);
        value = arg->type_name;
    }
    else if(param_name == CL_KERNEL_ARG_TYPE_QUALIFIER){
        if(!arg->type){
            VERBOSE_OUT(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }
        size_ret = sizeof(cl_kernel_arg_type_qualifier);
        value = &(arg->type);
    }
    else if(param_name == CL_KERNEL_ARG_NAME){
        if(!arg->name){
            VERBOSE_OUT(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }
        size_ret = sizeof(char) * (strlen(arg->name) + 1);
        value = arg->name;
    }
    else{
        // What are you asking for?
        // Anyway, let's see if the server knows it (>1.2 compatibility)
        cl_int flag = oclandGetKernelArgInfo(kernel,
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
SYMB(clGetKernelArgInfo);

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
SYMB(clWaitForEvents);

CL_API_ENTRY cl_int CL_API_CALL
icd_clGetEventInfo(cl_event          event ,
                   cl_event_info     param_name ,
                   size_t            param_value_size ,
                   void *            param_value ,
                   size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    cl_uint i;
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
SYMB(clGetEventInfo);

CL_API_ENTRY cl_int CL_API_CALL
icd_clRetainEvent(cl_event  event) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isEvent(event)){
        VERBOSE_OUT(CL_INVALID_EVENT);
        return CL_INVALID_EVENT;
    }
    // Simply increase the number of references to this object
    event->rcount++;
    VERBOSE_OUT(CL_SUCCESS);
    return CL_SUCCESS;
}
SYMB(clRetainEvent);

CL_API_ENTRY cl_int CL_API_CALL
icd_clReleaseEvent(cl_event  event) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isEvent(event)){
        VERBOSE_OUT(CL_INVALID_EVENT);
        return CL_INVALID_EVENT;
    }
    // Decrease the number of references to this object
    event->rcount--;
    if(event->rcount){
        // There are some active references to the object, so we must retain it
        VERBOSE_OUT(CL_SUCCESS);
        return CL_SUCCESS;
    }

    cl_uint i,j;
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
SYMB(clReleaseEvent);

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
SYMB(clGetEventProfilingInfo);

CL_API_ENTRY cl_event CL_API_CALL
icd_clCreateUserEvent(cl_context     context ,
                      cl_int *       errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    VERBOSE_IN();
    if(!isContext(context)){
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
    event->socket = context->socket;
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
SYMB(clCreateUserEvent);

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
SYMB(clSetUserEventStatus);

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
SYMB(clSetEventCallback);

// --------------------------------------------------------------
// Enqueues
// --------------------------------------------------------------

CL_API_ENTRY cl_int CL_API_CALL
icd_clFlush(cl_command_queue  command_queue) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    cl_int flag = oclandFlush(command_queue);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clFlush);

CL_API_ENTRY cl_int CL_API_CALL
icd_clFinish(cl_command_queue  command_queue) CL_API_SUFFIX__VERSION_1_0
{
    VERBOSE_IN();
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    cl_int flag = oclandFinish(command_queue);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clFinish);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(buffer)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueReadBuffer);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(buffer)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueWriteBuffer);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(src_buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!isMemObject(dst_buffer)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueCopyBuffer);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(image)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueReadImage);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(image)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueWriteImage);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(src_image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!isMemObject(dst_image)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueCopyImage);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(src_image)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!isMemObject(dst_buffer)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueCopyImageToBuffer);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(src_buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!isMemObject(dst_image)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueCopyBufferToImage);

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
    if(!isCommandQueue(command_queue)){
        if(errcode_ret) *errcode_ret = CL_INVALID_COMMAND_QUEUE;
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return NULL;
    }
    if(!isMemObject(buffer)){
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
                    VERBOSE_OUT(flag);
                    return flag;
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

    cl_map mapobj;
    mapobj.map_flags = map_flags;
    mapobj.blocking = blocking_map;
    mapobj.type = CL_MEM_OBJECT_BUFFER;
    mapobj.mapped_ptr = host_ptr;
    mapobj.offset = offset;
    mapobj.cb = cb;
    mapobj.origin[0] = 0; mapobj.origin[1] = 0; mapobj.origin[2] = 0;
    mapobj.region[0] = 0; mapobj.region[1] = 0; mapobj.region[2] = 0;
    mapobj.row_pitch = 0;
    mapobj.slice_pitch = 0;

    cl_map *backup = buffer->maps;
    buffer->map_count++;
    buffer->maps = (cl_map*)malloc(buffer->map_count*sizeof(cl_map));
    memcpy(buffer->maps, backup, (buffer->map_count-1)*sizeof(cl_map));
    free(backup);
    buffer->maps[buffer->map_count-1] = mapobj;

    VERBOSE_OUT(CL_SUCCESS);
    return mapobj.mapped_ptr;
}
SYMB(clEnqueueMapBuffer);

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
    if(!isCommandQueue(command_queue)){
        if(errcode_ret) *errcode_ret = CL_INVALID_COMMAND_QUEUE;
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return NULL;
    }
    if(!isMemObject(image)){
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
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
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
                    VERBOSE_OUT(flag);
                    return flag;
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

    cl_map mapobj;
    mapobj.map_flags = map_flags;
    mapobj.blocking = blocking_map;
    mapobj.type = CL_MEM_OBJECT_IMAGE3D;
    mapobj.mapped_ptr = host_ptr;
    mapobj.offset = 0;
    mapobj.cb = 0;
    memcpy(mapobj.origin,origin,3*sizeof(size_t));
    memcpy(mapobj.region,region,3*sizeof(size_t));
    mapobj.row_pitch = row_pitch;
    mapobj.slice_pitch = slice_pitch;

    cl_map *backup = image->maps;
    image->map_count++;
    image->maps = (cl_map*)malloc(image->map_count*sizeof(cl_map));
    memcpy(image->maps, backup, (image->map_count-1)*sizeof(cl_map));
    free(backup);
    image->maps[image->map_count-1] = mapobj;

    VERBOSE_OUT(CL_SUCCESS);
    return mapobj.mapped_ptr;
}
SYMB(clEnqueueMapImage);

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueUnmapMemObject(cl_command_queue  command_queue ,
                            cl_mem            memobj ,
                            void *            mapped_ptr ,
                            cl_uint           num_events_in_wait_list ,
                            const cl_event *   event_wait_list ,
                            cl_event *         event) CL_API_SUFFIX__VERSION_1_0
{
    cl_uint i;
    cl_map *mapobj = NULL;
    VERBOSE_IN();
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(memobj)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(command_queue->context != memobj->context){
        VERBOSE_OUT(CL_INVALID_CONTEXT);
        return CL_INVALID_CONTEXT;
    }
    for(i=0;i<memobj->map_count;i++){
        if(mapped_ptr == memobj->maps[i].mapped_ptr){
            mapobj = &(memobj->maps[i]);
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
        if(mapped_ptr == memobj->maps[i].mapped_ptr){
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
SYMB(clEnqueueUnmapMemObject);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isKernel(kernel)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueNDRangeKernel);

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
SYMB(clEnqueueTask);

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
SYMB(clEnqueueNativeKernel);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(buffer)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueReadBufferRect);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(buffer)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueWriteBufferRect);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(src_buffer)){
        VERBOSE_OUT(CL_INVALID_MEM_OBJECT);
        return CL_INVALID_MEM_OBJECT;
    }
    if(!isMemObject(dst_buffer)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueCopyBufferRect);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(buffer)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueFillBuffer);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    if(!isMemObject(image)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueFillImage);

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
    if(!isCommandQueue(command_queue)){
        VERBOSE_OUT(CL_INVALID_COMMAND_QUEUE);
        return CL_INVALID_COMMAND_QUEUE;
    }
    for(i=0;i<num_mem_objects;i++){
        if(!isMemObject(mem_objects[i])){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueMigrateMemObjects);

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueMarkerWithWaitList(cl_command_queue  command_queue ,
                                cl_uint            num_events_in_wait_list ,
                                const cl_event *   event_wait_list ,
                                cl_event *         event) CL_API_SUFFIX__VERSION_1_2
{
    cl_uint i;
    VERBOSE_IN();
    if(!isCommandQueue(command_queue)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueMarkerWithWaitList);

CL_API_ENTRY cl_int CL_API_CALL
icd_clEnqueueBarrierWithWaitList(cl_command_queue  command_queue ,
                                 cl_uint            num_events_in_wait_list ,
                                 const cl_event *   event_wait_list ,
                                 cl_event *         event) CL_API_SUFFIX__VERSION_1_2
{
    cl_uint i;
    VERBOSE_IN();
    if(!isCommandQueue(command_queue)){
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
        e->socket = command_queue->socket;
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
SYMB(clEnqueueBarrierWithWaitList);

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
icd_clEnqueueMarker(cl_command_queue    command_queue ,
                    cl_event *          event) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    VERBOSE_IN();
    cl_int flag = icd_clEnqueueMarkerWithWaitList(command_queue, 0, NULL, event);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clEnqueueMarker);

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
SYMB(clEnqueueWaitForEvents);

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
icd_clEnqueueBarrier(cl_command_queue command_queue ) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    VERBOSE_IN();
    cl_int flag = icd_clEnqueueBarrierWithWaitList(command_queue, 0, NULL, NULL);
    VERBOSE_OUT(flag);
    return flag;
}
SYMB(clEnqueueBarrier);

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
SYMB(clCreateFromGLBuffer);

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
SYMB(clCreateFromGLTexture);

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
SYMB(clCreateFromGLRenderbuffer);

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
SYMB(clGetGLObjectInfo);

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
SYMB(clGetGLTextureInfo);

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
SYMB(clEnqueueAcquireGLObjects);

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
SYMB(clEnqueueReleaseGLObjects);

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
SYMB(clCreateFromGLTexture2D);

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
SYMB(clCreateFromGLTexture3D);

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
SYMB(clGetGLContextInfoKHR);

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
SYMB(clGetExtensionFunctionAddress);

CL_API_ENTRY void * CL_API_CALL
icd_clGetExtensionFunctionAddressForPlatform(cl_platform_id platform,
                                             const char *   func_name) CL_API_SUFFIX__VERSION_1_2
{
    VERBOSE_IN();
    return icd_clGetExtensionFunctionAddress(func_name);
}
SYMB(clGetExtensionFunctionAddressForPlatform);

#pragma GCC visibility pop

/// Dummy function to parse non-implemented methods
void dummyFunc(void){}

struct _cl_icd_dispatch master_dispatch = {
  (void(*)(void))& icd_clGetPlatformIDs,
  (void(*)(void))& icd_clGetPlatformInfo,
  (void(*)(void))& icd_clGetDeviceIDs,
  (void(*)(void))& icd_clGetDeviceInfo,
  (void(*)(void))& icd_clCreateContext,
  (void(*)(void))& icd_clCreateContextFromType,
  (void(*)(void))& icd_clRetainContext,
  (void(*)(void))& icd_clReleaseContext,
  (void(*)(void))& icd_clGetContextInfo,
  (void(*)(void))& icd_clCreateCommandQueue,
  (void(*)(void))& icd_clRetainCommandQueue,
  (void(*)(void))& icd_clReleaseCommandQueue,
  (void(*)(void))& icd_clGetCommandQueueInfo,
  (void(*)(void))& icd_clSetCommandQueueProperty,  // DEPRECATED
  (void(*)(void))& icd_clCreateBuffer,
  (void(*)(void))& icd_clCreateImage2D,
  (void(*)(void))& icd_clCreateImage3D,
  (void(*)(void))& icd_clRetainMemObject,
  (void(*)(void))& icd_clReleaseMemObject,
  (void(*)(void))& icd_clGetSupportedImageFormats,
  (void(*)(void))& icd_clGetMemObjectInfo,
  (void(*)(void))& icd_clGetImageInfo,
  (void(*)(void))& icd_clCreateSampler,
  (void(*)(void))& icd_clRetainSampler,
  (void(*)(void))& icd_clReleaseSampler,
  (void(*)(void))& icd_clGetSamplerInfo,
  (void(*)(void))& icd_clCreateProgramWithSource,
  (void(*)(void))& icd_clCreateProgramWithBinary,
  (void(*)(void))& icd_clRetainProgram,
  (void(*)(void))& icd_clReleaseProgram,
  (void(*)(void))& icd_clBuildProgram,
  (void(*)(void))& icd_clUnloadCompiler,  // DEPRECATED
  (void(*)(void))& icd_clGetProgramInfo,
  (void(*)(void))& icd_clGetProgramBuildInfo,
  (void(*)(void))& icd_clCreateKernel,
  (void(*)(void))& icd_clCreateKernelsInProgram,
  (void(*)(void))& icd_clRetainKernel,
  (void(*)(void))& icd_clReleaseKernel,
  (void(*)(void))& icd_clSetKernelArg,
  (void(*)(void))& icd_clGetKernelInfo,
  (void(*)(void))& icd_clGetKernelWorkGroupInfo,
  (void(*)(void))& icd_clWaitForEvents,
  (void(*)(void))& icd_clGetEventInfo,
  (void(*)(void))& icd_clRetainEvent,
  (void(*)(void))& icd_clReleaseEvent,
  (void(*)(void))& icd_clGetEventProfilingInfo,
  (void(*)(void))& icd_clFlush,
  (void(*)(void))& icd_clFinish,
  (void(*)(void))& icd_clEnqueueReadBuffer,
  (void(*)(void))& icd_clEnqueueWriteBuffer,
  (void(*)(void))& icd_clEnqueueCopyBuffer,
  (void(*)(void))& icd_clEnqueueReadImage,
  (void(*)(void))& icd_clEnqueueWriteImage,
  (void(*)(void))& icd_clEnqueueCopyImage,
  (void(*)(void))& icd_clEnqueueCopyImageToBuffer,
  (void(*)(void))& icd_clEnqueueCopyBufferToImage,
  (void(*)(void))& icd_clEnqueueMapBuffer,
  (void(*)(void))& icd_clEnqueueMapImage,
  (void(*)(void))& icd_clEnqueueUnmapMemObject,
  (void(*)(void))& icd_clEnqueueNDRangeKernel,
  (void(*)(void))& icd_clEnqueueTask,
  (void(*)(void))& icd_clEnqueueNativeKernel,
  (void(*)(void))& icd_clEnqueueMarker,
  (void(*)(void))& icd_clEnqueueWaitForEvents,
  (void(*)(void))& icd_clEnqueueBarrier,
  (void(*)(void))& dummyFunc,    // icd_clGetExtensionFunctionAddress,  // KEEP HIDDEN
  (void(*)(void))& icd_clCreateFromGLBuffer,
  (void(*)(void))& icd_clCreateFromGLTexture2D,
  (void(*)(void))& icd_clCreateFromGLTexture3D,
  (void(*)(void))& icd_clCreateFromGLRenderbuffer,
  (void(*)(void))& icd_clGetGLObjectInfo,
  (void(*)(void))& icd_clGetGLTextureInfo,
  (void(*)(void))& icd_clEnqueueAcquireGLObjects,
  (void(*)(void))& icd_clEnqueueReleaseGLObjects,
  (void(*)(void))& dummyFunc,    // clGetGLContextInfoKHR
  (void(*)(void))& dummyFunc,    // clUnknown75
  (void(*)(void))& dummyFunc,    // clUnknown76
  (void(*)(void))& dummyFunc,    // clUnknown77
  (void(*)(void))& dummyFunc,    // clUnknown78
  (void(*)(void))& dummyFunc,    // clUnknown79
  (void(*)(void))& dummyFunc,    // clUnknown80
  (void(*)(void))& icd_clSetEventCallback,
  (void(*)(void))& icd_clCreateSubBuffer,
  (void(*)(void))& icd_clSetMemObjectDestructorCallback,
  (void(*)(void))& icd_clCreateUserEvent,
  (void(*)(void))& icd_clSetUserEventStatus,
  (void(*)(void))& icd_clEnqueueReadBufferRect,
  (void(*)(void))& icd_clEnqueueWriteBufferRect,
  (void(*)(void))& icd_clEnqueueCopyBufferRect,
  (void(*)(void))& dummyFunc,    // clCreateSubDevicesEXT
  (void(*)(void))& dummyFunc,    // clRetainDeviceEXT
  (void(*)(void))& dummyFunc,    // clReleaseDeviceEXT
  (void(*)(void))& dummyFunc,    // clCreateEventFromGLsyncKHR
  (void(*)(void))& icd_clCreateSubDevices,
  (void(*)(void))& icd_clRetainDevice,
  (void(*)(void))& icd_clReleaseDevice,
  (void(*)(void))& icd_clCreateImage,
  (void(*)(void))& icd_clCreateProgramWithBuiltInKernels,
  (void(*)(void))& icd_clCompileProgram,
  (void(*)(void))& icd_clLinkProgram,
  (void(*)(void))& icd_clUnloadPlatformCompiler,
  (void(*)(void))& icd_clGetKernelArgInfo,
  (void(*)(void))& icd_clEnqueueFillBuffer,
  (void(*)(void))& icd_clEnqueueFillImage,
  (void(*)(void))& icd_clEnqueueMigrateMemObjects,
  (void(*)(void))& icd_clEnqueueMarkerWithWaitList,
  (void(*)(void))& icd_clEnqueueBarrierWithWaitList,
  (void(*)(void))& dummyFunc,    // clGetExtensionFunctionAddressForPlatform  // KEEP HIDDEN
  (void(*)(void))& icd_clCreateFromGLTexture,
  (void(*)(void))& dummyFunc,    // clUnknown109
  (void(*)(void))& dummyFunc,    // clUnknown110
  (void(*)(void))& dummyFunc,    // clUnknown111
  (void(*)(void))& dummyFunc,    // clUnknown112
  (void(*)(void))& dummyFunc,    // clUnknown113
  (void(*)(void))& dummyFunc,    // clUnknown114
  (void(*)(void))& dummyFunc,    // clUnknown115
  (void(*)(void))& dummyFunc,    // clUnknown116
  (void(*)(void))& dummyFunc,    // clUnknown117
  (void(*)(void))& dummyFunc,    // clUnknown118
  (void(*)(void))& dummyFunc,    // clUnknown119
  (void(*)(void))& dummyFunc,    // clUnknown120
  (void(*)(void))& dummyFunc,    // clUnknown121
  (void(*)(void))& dummyFunc,    // clUnknown122
  (void(*)(void))& dummyFunc,    // clUnknown123
  (void(*)(void))& dummyFunc,    // clUnknown124
};

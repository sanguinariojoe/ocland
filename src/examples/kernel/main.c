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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <CL/opencl.h>

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

const char* program_src = "__kernel void test(__global float* x, \n\
                                              __global float* y, \n\
                                              __global float* z, \n\
                                              unsigned int i0,   \n\
                                              unsigned int N)    \n\
{                                                                \n\
    unsigned int i = get_global_id(0);                           \n\
    if(i >= N) return;                                           \n\
    i+=i0;                                                       \n\
    z[i]=x[i]*y[i];                                              \n\
}                                                                \n\
";

int main(int argc, char *argv[])
{
    unsigned int i,j,k;
    char buffer[1025]; strcpy(buffer, "");
    cl_uint num_entries = 0, num_platforms = 0;
    cl_platform_id *platforms = NULL;
    cl_int flag;
    // Get number of platforms
    flag = clGetPlatformIDs(0, NULL, &num_platforms);
    if(flag != CL_SUCCESS){
        printf("Error getting number of platforms\n");
        if(flag == CL_INVALID_VALUE)
            printf("\tCL_INVALID_VALUE\n");
        if(flag == CL_OUT_OF_HOST_MEMORY)
            printf("\tCL_OUT_OF_HOST_MEMORY\n");
        return EXIT_FAILURE;
    }
    if(!num_platforms){
        printf("No OpenCL platforms found...\n");
        return EXIT_FAILURE;
    }
    printf("%u platforms found...\n", num_platforms);
    // Build platforms array
    num_entries = num_platforms;
    platforms   = (cl_platform_id*)malloc(num_entries*sizeof(cl_platform_id));
    // Get platforms array
    flag = clGetPlatformIDs(num_entries, platforms, &num_platforms);
    if(flag != CL_SUCCESS){
        printf("Error getting platforms\n");
        if(flag == CL_INVALID_VALUE)
            printf("\tCL_INVALID_VALUE\n");
        if(flag == CL_OUT_OF_HOST_MEMORY)
            printf("\tCL_OUT_OF_HOST_MEMORY\n");
        return EXIT_FAILURE;
    }
    // Create the devices
    for(i = 0; i < num_platforms; i++){
        printf("Platform %u...\n", i);
        size_t platform_name_size = 0;
        flag = clGetPlatformInfo(platforms[i],
                                 CL_PLATFORM_NAME,
                                 0,
                                 NULL,
                                 &platform_name_size);
        if(flag == CL_SUCCESS){
            char *platform_name = (char*)malloc(platform_name_size);
            if(platform_name){
                flag = clGetPlatformInfo(platforms[i],
                                         CL_PLATFORM_NAME,
                                         platform_name_size,
                                         platform_name,
                                         NULL);
                printf("\t%s\n", platform_name);
            }
        }
        // Get number of devices
        num_entries = 0;
        cl_uint num_devices = 0;
        cl_device_id *devices = NULL;
        cl_event *events = NULL;
        flag = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_entries, devices, &num_devices);
        if( (flag != CL_SUCCESS) && (flag != CL_DEVICE_NOT_FOUND) ) {
            printf("Error getting number of devices\n");
            if(flag == CL_INVALID_PLATFORM)
                printf("\tCL_INVALID_PLATFORM\n");
            if(flag == CL_INVALID_DEVICE_TYPE)
                printf("\tCL_INVALID_DEVICE_TYPE\n");
            if(flag == CL_INVALID_VALUE)
                printf("\tCL_INVALID_VALUE\n");
            if(flag == CL_OUT_OF_RESOURCES)
                printf("\tCL_OUT_OF_RESOURCES\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        if( (!num_devices) || (flag == CL_DEVICE_NOT_FOUND) ){
            printf("\tWithout devices.\n");
            continue;
        }
        // Build devices array
        num_entries = num_devices;
        devices     = (cl_device_id*)malloc(num_entries*sizeof(cl_device_id));
        events      = (cl_event*)malloc(num_entries*sizeof(cl_event));
        // Get devices array
        flag = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_entries, devices, &num_devices);
        if( (flag != CL_SUCCESS) && (flag != CL_DEVICE_NOT_FOUND) ) {
            printf("Error getting number of devices\n");
            if(flag == CL_INVALID_PLATFORM)
                printf("\tCL_INVALID_PLATFORM\n");
            if(flag == CL_INVALID_DEVICE_TYPE)
                printf("\tCL_INVALID_DEVICE_TYPE\n");
            if(flag == CL_INVALID_VALUE)
                printf("\tCL_INVALID_VALUE\n");
            if(flag == CL_OUT_OF_RESOURCES)
                printf("\tCL_OUT_OF_RESOURCES\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        // Create a context
        cl_context_properties contextProperties[3] = {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)platforms[i],
            0
        };
        cl_context context = clCreateContext(contextProperties, num_devices, devices, NULL, NULL, &flag);
        if(flag != CL_SUCCESS) {
            printf("Error building context\n");
            if(flag == CL_INVALID_PLATFORM)
                printf("\tCL_INVALID_PLATFORM\n");
            if(flag == CL_INVALID_VALUE)
                printf("\tCL_INVALID_VALUE\n");
            if(flag == CL_INVALID_DEVICE)
                printf("\tCL_INVALID_DEVICE\n");
            if(flag == CL_DEVICE_NOT_AVAILABLE)
                printf("\tCL_DEVICE_NOT_AVAILABLE\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        printf("\tBuilt context with %u devices!\n", num_devices);

        // Create the command queues for each device
        cl_command_queue *queues = (cl_command_queue*)malloc(
            num_devices * sizeof(cl_command_queue));
        for(j = 0; j < num_devices; j++){
            queues[j] = clCreateCommandQueue(context, devices[j], 0, &flag);
            if(flag != CL_SUCCESS) {
                printf("Error building command queue\n");
                if(flag == CL_INVALID_CONTEXT)
                    printf("\tCL_INVALID_CONTEXT\n");
                if(flag == CL_INVALID_DEVICE)
                    printf("\tCL_INVALID_DEVICE\n");
                if(flag == CL_INVALID_VALUE)
                    printf("\tCL_INVALID_VALUE\n");
                if(flag == CL_INVALID_QUEUE_PROPERTIES)
                    printf("\tCL_INVALID_QUEUE_PROPERTIES\n");
                if(flag == CL_OUT_OF_HOST_MEMORY)
                    printf("\tCL_OUT_OF_HOST_MEMORY\n");
                return EXIT_FAILURE;
            }
            printf("\tBuilt command queue (device %u / %u)!\n", j, num_devices-1);
        }

        // Create the program
        size_t program_src_length = strlen(program_src) * sizeof(char);
        cl_program program = clCreateProgramWithSource(context,
                                                       1,
                                                       (const char **)&program_src,
                                                       &program_src_length,
                                                       &flag);
        if(flag != CL_SUCCESS){
            printf("Error creating program\n");
            if(flag == CL_INVALID_CONTEXT)
                printf("\tCL_INVALID_CONTEXT\n");
            if(flag == CL_INVALID_VALUE)
                printf("\tCL_INVALID_VALUE\n");
            if(flag == CL_OUT_OF_RESOURCES)
                printf("\tCL_OUT_OF_RESOURCES\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        printf("\tCreated program!\n");
        char C_FLAGS[256];
        strcpy(C_FLAGS, "-cl-mad-enable -cl-no-signed-zeros -cl-finite-math-only -cl-fast-relaxed-math");
        flag = clBuildProgram(program, num_devices, devices, C_FLAGS, NULL, NULL);
        if(flag != CL_SUCCESS){
            printf("Error building program\n");
            if(flag == CL_INVALID_PROGRAM)
                printf("\tCL_INVALID_PROGRAM\n");
            if(flag == CL_INVALID_VALUE)
                printf("\tCL_INVALID_VALUE\n");
            if(flag == CL_INVALID_DEVICE)
                printf("\tCL_INVALID_DEVICE\n");
            if(flag == CL_INVALID_BINARY)
                printf("\tCL_INVALID_BINARY\n");
            if(flag == CL_INVALID_BUILD_OPTIONS)
                printf("\tCL_INVALID_BUILD_OPTIONS\n");
            if(flag == CL_INVALID_OPERATION)
                printf("\tCL_INVALID_OPERATION\n");
            if(flag == CL_COMPILER_NOT_AVAILABLE)
                printf("\tCL_COMPILER_NOT_AVAILABLE\n");
            if(flag == CL_BUILD_PROGRAM_FAILURE)
                printf("\tCL_BUILD_PROGRAM_FAILURE\n");
            if(flag == CL_OUT_OF_RESOURCES)
                printf("\tCL_OUT_OF_RESOURCES\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
        }
        for(j = 0; j < num_devices; j++){
            printf("\t\tDevice %u:\n", j);
            cl_build_status build_status;
            clGetProgramBuildInfo(program,
                                  devices[j],
                                  CL_PROGRAM_BUILD_STATUS,
                                  sizeof(cl_build_status),
                                  &build_status,
                                  NULL);
            printf("\t\t\tCL_PROGRAM_BUILD_STATUS: ");
            if(build_status == CL_BUILD_SUCCESS){
                printf("CL_BUILD_NONE\n");
            }
            else if(build_status == CL_BUILD_ERROR){
                printf("CL_BUILD_ERROR\n");
            }
            else if(build_status == CL_BUILD_SUCCESS){
                printf("CL_BUILD_SUCCESS\n");
            }
            else if(build_status == CL_BUILD_IN_PROGRESS){
                printf("CL_BUILD_IN_PROGRESS\n");
            }
            else{
                printf("%u (UNKNOW)\n", build_status);
            }
            size_t build_log_size;
            clGetProgramBuildInfo(program,
                                  devices[j],
                                  CL_PROGRAM_BUILD_LOG,
                                  0,
                                  NULL,
                                  &build_log_size);
            char build_log[build_log_size];
            clGetProgramBuildInfo(program,
                                  devices[j],
                                  CL_PROGRAM_BUILD_LOG,
                                  build_log_size,
                                  build_log,
                                  NULL);
            printf("\t\t\tCL_PROGRAM_BUILD_LOG: \"%s\"\n", build_log);
        }

        // Create the kernel
        cl_kernel kernel = clCreateKernel(program, "test", &flag);
        if(flag != CL_SUCCESS){
            printf("Error creating kernel\n");
            if(flag == CL_INVALID_PROGRAM)
                printf("\tCL_INVALID_PROGRAM\n");
            if(flag == CL_INVALID_PROGRAM_EXECUTABLE)
                printf("\tCL_INVALID_PROGRAM_EXECUTABLE\n");
            if(flag == CL_INVALID_KERNEL_NAME)
                printf("\tCL_INVALID_KERNEL_NAME\n");
            if(flag == CL_INVALID_KERNEL_DEFINITION)
                printf("\tCL_INVALID_KERNEL_DEFINITION\n");
            if(flag == CL_INVALID_VALUE)
                printf("\tCL_INVALID_VALUE\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        printf("\tCreated kernel!\n");

        // Print kernel info
        size_t function_name_size = 0;
        clGetKernelInfo(kernel,
                        CL_KERNEL_FUNCTION_NAME,
                        0,
                        NULL,
                        &function_name_size);
        char function_name[function_name_size];
        clGetKernelInfo(kernel,
                        CL_KERNEL_FUNCTION_NAME,
                        function_name_size,
                        function_name,
                        NULL);
        printf("\t\tCL_KERNEL_FUNCTION_NAME: ");
        if(!strncmp("test", function_name, function_name_size)){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }
        cl_uint num_args = 0;
        clGetKernelInfo(kernel,
                        CL_KERNEL_NUM_ARGS,
                        sizeof(cl_uint),
                        &num_args,
                        NULL);
        printf("\t\tCL_KERNEL_NUM_ARGS: ");
        if(num_args == 5){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }
        cl_context ret_context = NULL;
        clGetKernelInfo(kernel,
                        CL_KERNEL_CONTEXT,
                        sizeof(cl_context),
                        &ret_context,
                        NULL);
        printf("\t\tCL_KERNEL_CONTEXT: ");
        if(ret_context == context){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n", ret_context, context);
        }
        cl_program ret_program = NULL;
        clGetKernelInfo(kernel,
                        CL_KERNEL_PROGRAM,
                        sizeof(cl_program),
                        &ret_program,
                        NULL);
        printf("\t\tCL_KERNEL_PROGRAM: ");
        if(ret_program == program){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }
        size_t attribute_size = 0;
        clGetKernelInfo(kernel,
                        CL_KERNEL_ATTRIBUTES,
                        0,
                        NULL,
                        &attribute_size);
        char attribute[attribute_size];
        clGetKernelInfo(kernel,
                        CL_KERNEL_ATTRIBUTES,
                        attribute_size,
                        attribute,
                        NULL);
        printf("\t\tCL_KERNEL_ATTRIBUTES: ");
        if(!strncmp("", attribute, attribute_size)){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }
        // Print kernel work group info
        for(j = 0; j < num_devices; j++){
            printf("\t\tDevice %u:\n", j);
            size_t global_work_size[3];
            clGetKernelWorkGroupInfo(kernel,
                                     devices[j],
                                     CL_KERNEL_GLOBAL_WORK_SIZE,
                                     3 * sizeof(size_t),
                                     global_work_size,
                                     NULL);
            printf("\t\t\tCL_KERNEL_GLOBAL_WORK_SIZE: (%lu, %lu, %lu)\n",
                   global_work_size[0],
                   global_work_size[1],
                   global_work_size[2]);
            size_t work_group_size = 0;
            clGetKernelWorkGroupInfo(kernel,
                                     devices[j],
                                     CL_KERNEL_WORK_GROUP_SIZE,
                                     sizeof(size_t),
                                     &work_group_size,
                                     NULL);
            printf("\t\t\tCL_KERNEL_WORK_GROUP_SIZE: %lu\n", work_group_size);
            size_t compile_work_group_size[3];
            clGetKernelWorkGroupInfo(kernel,
                                     devices[j],
                                     CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                                     3 * sizeof(size_t),
                                     compile_work_group_size,
                                     NULL);
            printf("\t\t\tCL_KERNEL_COMPILE_WORK_GROUP_SIZE: (%lu, %lu, %lu)\n",
                   compile_work_group_size[0],
                   compile_work_group_size[1],
                   compile_work_group_size[2]);
            cl_ulong local_mem_size = 0;
            clGetKernelWorkGroupInfo(kernel,
                                     devices[j],
                                     CL_KERNEL_LOCAL_MEM_SIZE,
                                     sizeof(cl_ulong),
                                     &local_mem_size,
                                     NULL);
            printf("\t\t\tCL_KERNEL_LOCAL_MEM_SIZE: %lu\n", local_mem_size);
            cl_ulong work_group_size_multiple = 0;
            clGetKernelWorkGroupInfo(kernel,
                                     devices[j],
                                     CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                     sizeof(size_t),
                                     &work_group_size_multiple,
                                     NULL);
            printf("\t\t\tCL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: %lu\n",
                   work_group_size_multiple);
            cl_ulong private_mem_size = 0;
            clGetKernelWorkGroupInfo(kernel,
                                     devices[j],
                                     CL_KERNEL_PRIVATE_MEM_SIZE,
                                     sizeof(cl_ulong),
                                     &private_mem_size,
                                     NULL);
            printf("\t\t\tCL_KERNEL_PRIVATE_MEM_SIZE: %lu\n", private_mem_size);
        }
        // Print kernel args info
        for(j = 0; j < num_args; j++){
            printf("\t\tArg %u:\n", j);
            cl_kernel_arg_address_qualifier address_qualifier;
            clGetKernelArgInfo(kernel,
                               j,
                               CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                               sizeof(cl_kernel_arg_address_qualifier),
                               &address_qualifier,
                               NULL);
            printf("\t\t\tCL_KERNEL_ARG_ADDRESS_QUALIFIER: ");
            if(address_qualifier == CL_KERNEL_ARG_ADDRESS_GLOBAL){
                printf("CL_KERNEL_ARG_ADDRESS_GLOBAL\n");
            }
            else if(address_qualifier == CL_KERNEL_ARG_ADDRESS_LOCAL){
                printf("CL_KERNEL_ARG_ADDRESS_LOCAL\n");
            }
            else if(address_qualifier == CL_KERNEL_ARG_ADDRESS_CONSTANT){
                printf("CL_KERNEL_ARG_ADDRESS_CONSTANT\n");
            }
            else if(address_qualifier == CL_KERNEL_ARG_ADDRESS_PRIVATE){
                printf("CL_KERNEL_ARG_ADDRESS_PRIVATE\n");
            }
            else{
                printf("%u (UNKNOWN)\n", address_qualifier);
            }
            cl_kernel_arg_access_qualifier access_qualifier;
            clGetKernelArgInfo(kernel,
                               j,
                               CL_KERNEL_ARG_ACCESS_QUALIFIER,
                               sizeof(cl_kernel_arg_access_qualifier),
                               &access_qualifier,
                               NULL);
            printf("\t\t\tCL_KERNEL_ARG_ACCESS_QUALIFIER: ");
            if(access_qualifier == CL_KERNEL_ARG_ACCESS_READ_ONLY){
                printf("CL_KERNEL_ARG_ACCESS_READ_ONLY\n");
            }
            else if(access_qualifier == CL_KERNEL_ARG_ACCESS_WRITE_ONLY){
                printf("CL_KERNEL_ARG_ACCESS_WRITE_ONLY\n");
            }
            else if(access_qualifier == CL_KERNEL_ARG_ACCESS_READ_WRITE){
                printf("CL_KERNEL_ARG_ACCESS_READ_WRITE\n");
            }
            else if(access_qualifier == CL_KERNEL_ARG_ACCESS_NONE){
                printf("CL_KERNEL_ARG_ACCESS_NONE\n");
            }
            else{
                printf("%u (UNKNOWN)\n", access_qualifier);
            }
            size_t type_name_size = 0;
            clGetKernelArgInfo(kernel,
                               j,
                               CL_KERNEL_ARG_TYPE_NAME,
                               0,
                               NULL,
                               &type_name_size);
            char type_name[type_name_size];
            clGetKernelArgInfo(kernel,
                               j,
                               CL_KERNEL_ARG_TYPE_NAME,
                               type_name_size,
                               type_name,
                               NULL);
            printf("\t\t\tCL_KERNEL_ARG_TYPE_NAME: \"%s\"\n", type_name);
            cl_kernel_arg_type_qualifier type_qualifier;
            clGetKernelArgInfo(kernel,
                               j,
                               CL_KERNEL_ARG_TYPE_QUALIFIER,
                               sizeof(cl_kernel_arg_type_qualifier),
                               &type_qualifier,
                               NULL);
            printf("\t\t\tCL_KERNEL_ARG_TYPE_QUALIFIER: ");
            if(type_qualifier == CL_KERNEL_ARG_TYPE_CONST){
                printf("CL_KERNEL_ARG_TYPE_CONST\n");
            }
            else if(type_qualifier == CL_KERNEL_ARG_TYPE_RESTRICT){
                printf("CL_KERNEL_ARG_TYPE_RESTRICT\n");
            }
            else if(type_qualifier == CL_KERNEL_ARG_TYPE_VOLATILE){
                printf("CL_KERNEL_ARG_TYPE_VOLATILE\n");
            }
            else if(type_qualifier == CL_KERNEL_ARG_TYPE_NONE){
                printf("CL_KERNEL_ARG_TYPE_NONE\n");
            }
            else{
                printf("%u (UNKNOWN)\n", type_qualifier);
            }
            size_t arg_name_size = 0;
            clGetKernelArgInfo(kernel,
                               j,
                               CL_KERNEL_ARG_NAME,
                               0,
                               NULL,
                               &arg_name_size);
            char arg_name[arg_name_size];
            clGetKernelArgInfo(kernel,
                               j,
                               CL_KERNEL_ARG_NAME,
                               arg_name_size,
                               arg_name,
                               NULL);
            printf("\t\t\tCL_KERNEL_ARG_NAME: \"%s\"\n", arg_name);
        }
        
        flag = clReleaseKernel(kernel);
        if(flag != CL_SUCCESS){
            printf("Error releasing kernel\n");
            if(flag == CL_INVALID_KERNEL)
                printf("\tCL_INVALID_KERNEL\n");
            if(flag == CL_OUT_OF_RESOURCES)
                printf("\tCL_OUT_OF_RESOURCES\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        printf("\tRemoved kernel.\n");

        if(program) clReleaseProgram(program); program=NULL;
        printf("\tRemoved program.\n");

        for(j = 0; j < num_devices; j++){
            flag = clReleaseCommandQueue(queues[j]);
            if(flag != CL_SUCCESS) {
                printf("Error releasing command queue\n");
                if(flag == CL_INVALID_COMMAND_QUEUE)
                    printf("\tCL_INVALID_COMMAND_QUEUE\n");
                return EXIT_FAILURE;
            }
            printf("\tRemoved command queue (device %u / %u).\n", j, num_devices-1);
        }
        if(queues) free(queues); queues=NULL;

        flag = clReleaseContext(context);
        if(flag != CL_SUCCESS) {
            printf("Error releasing context\n");
            if(flag == CL_INVALID_CONTEXT)
                printf("\tCL_INVALID_CONTEXT\n");
            if(flag == CL_OUT_OF_RESOURCES)
                printf("\tCL_OUT_OF_RESOURCES\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        printf("\tRemoved context.\n");
        if(devices) free(devices); devices=NULL;
    }
    if(platforms) free(platforms); platforms=NULL;
    return EXIT_SUCCESS;
}

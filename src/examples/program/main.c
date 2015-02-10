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

    // Get the platforms
    flag = clGetPlatformIDs(0, NULL, &num_platforms);
    if(flag != CL_SUCCESS){
        printf("Error getting number of platforms\n");
        printf("\t%s\n", OpenCLError(flag));
        return EXIT_FAILURE;
    }
    if(!num_platforms){
        printf("No OpenCL platforms found...\n");
        return EXIT_FAILURE;
    }
    printf("%u platforms found...\n", num_platforms);

    num_entries = num_platforms;
    platforms   = (cl_platform_id*)malloc(num_entries*sizeof(cl_platform_id));
    if(!platforms){
        printf("Failure allocating memory for the platforms\n");
    }

    flag = clGetPlatformIDs(num_entries, platforms, &num_platforms);
    if(flag != CL_SUCCESS){
        printf("Error getting platforms\n");
        printf("\t%s\n", OpenCLError(flag));
        return EXIT_FAILURE;
    }

    // Work on each platform separately
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
        
        // Create the devices
        num_entries = 0;
        cl_uint num_devices = 0;
        cl_device_id *devices = NULL;
        flag = clGetDeviceIDs(platforms[i],
                              CL_DEVICE_TYPE_ALL,
                              num_entries,
                              devices,
                              &num_devices);
        if(flag != CL_SUCCESS){
            printf("Failure getting number of devices\n");
            printf("\t%s\n", OpenCLError(flag));
            return EXIT_FAILURE;
        }
        if(!num_devices){
            printf("\tWithout devices.\n");
            continue;
        }

        num_entries = num_devices;
        devices = (cl_device_id*)malloc(num_entries * sizeof(cl_device_id));
        if(!devices){
            printf("Failure allocating memory for the devices\n");
            return EXIT_FAILURE;
        }
        flag = clGetDeviceIDs(platforms[i],
                              CL_DEVICE_TYPE_ALL,
                              num_entries,
                              devices,
                              &num_devices);
        if(flag != CL_SUCCESS){
            printf("Failure getting the devices\n");
            printf("\t%s\n", OpenCLError(flag));
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
            printf("\t%s\n", OpenCLError(flag));
            return EXIT_FAILURE;
        }
        printf("\tBuilt context with %u devices!\n", num_devices);

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
            if(build_status == CL_BUILD_NONE){
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
            size_t build_options_size;
            clGetProgramBuildInfo(program,
                                  devices[j],
                                  CL_PROGRAM_BUILD_OPTIONS,
                                  0,
                                  NULL,
                                  &build_options_size);
            char build_options[build_options_size];
            clGetProgramBuildInfo(program,
                                  devices[j],
                                  CL_PROGRAM_BUILD_OPTIONS,
                                  build_options_size,
                                  build_options,
                                  NULL);
            printf("\t\t\tCL_PROGRAM_BUILD_OPTIONS: \"%s\"\n", build_options);
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
            cl_program_binary_type binary_type;
            clGetProgramBuildInfo(program,
                                  devices[j],
                                  CL_PROGRAM_BINARY_TYPE,
                                  sizeof(cl_program_binary_type),
                                  &binary_type,
                                  NULL);
            printf("\t\t\tCL_PROGRAM_BINARY_TYPE: ");
            if(binary_type == CL_PROGRAM_BINARY_TYPE_NONE){
                printf("CL_PROGRAM_BINARY_TYPE_NONE\n");
            }
            else if(binary_type == CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT){
                printf("CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT\n");
            }
            else if(binary_type == CL_PROGRAM_BINARY_TYPE_LIBRARY){
                printf("CL_PROGRAM_BINARY_TYPE_LIBRARY\n");
            }
            else if(binary_type == CL_PROGRAM_BINARY_TYPE_EXECUTABLE){
                printf("CL_PROGRAM_BINARY_TYPE_EXECUTABLE\n");
            }
            else{
                printf("%u (UNKNOW)\n", binary_type);
            }
        }

        // Print the program data
        cl_context ret_context = NULL;
        clGetProgramInfo(program,
                         CL_PROGRAM_CONTEXT,
                         sizeof(cl_context),
                         &ret_context,
                         NULL);
        printf("\t\tCL_PROGRAM_CONTEXT: ");
        if(ret_context == context){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }
        cl_uint ret_num_devices = 0;
        clGetProgramInfo(program,
                         CL_PROGRAM_NUM_DEVICES,
                         sizeof(cl_uint),
                         &ret_num_devices,
                         NULL);
        printf("\t\tCL_PROGRAM_NUM_DEVICES: ");
        if(ret_num_devices == num_devices){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }
        cl_device_id ret_devices[ret_num_devices];
        clGetProgramInfo(program,
                         CL_PROGRAM_DEVICES,
                         ret_num_devices * sizeof(cl_device_id),
                         &ret_devices,
                         NULL);
        printf("\t\tCL_PROGRAM_DEVICES: ");
        flag = CL_TRUE;
        for(j = 0; j < ret_num_devices; j++){
            if(ret_devices[j] != devices[j]){
                flag = CL_FALSE;
                break;
            }
        }
        if(flag == CL_TRUE){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }
        size_t ret_source_size = 0;
        clGetProgramInfo(program,
                         CL_PROGRAM_SOURCE,
                         0,
                         NULL,
                         &ret_source_size);
        char ret_source[ret_source_size];
        clGetProgramInfo(program,
                         CL_PROGRAM_SOURCE,
                         ret_source_size,
                         ret_source,
                         NULL);        
        printf("\t\tCL_PROGRAM_SOURCE: ");
        if(!strncmp(program_src, ret_source, ret_source_size)){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }
        size_t ret_binary_sizes[ret_num_devices];
        unsigned char* ret_binaries[ret_num_devices];
        clGetProgramInfo(program,
                         CL_PROGRAM_BINARY_SIZES,
                         ret_num_devices * sizeof(size_t),
                         ret_binary_sizes,
                         NULL);        
        printf("\t\tCL_PROGRAM_BINARY_SIZES: ");
        for(j = 0; j < ret_num_devices; j++){
            printf("%lu, ", ret_binary_sizes[j]);
            ret_binaries[j] = NULL;
            if(ret_binary_sizes[j])
                ret_binaries[j] = (unsigned char*)malloc(ret_binary_sizes[j]);
        }
        printf("\n");

        size_t ret_num_kernels;
        clGetProgramInfo(program,
                         CL_PROGRAM_NUM_KERNELS,
                         sizeof(size_t),
                         &ret_num_kernels,
                         NULL);        
        printf("\t\tCL_PROGRAM_NUM_KERNELS: ");
        if(ret_num_kernels == 1){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }

        size_t ret_kernels_size = 0;
        clGetProgramInfo(program,
                         CL_PROGRAM_KERNEL_NAMES,
                         0,
                         NULL,
                         &ret_kernels_size);
        char ret_kernels[ret_kernels_size];
        clGetProgramInfo(program,
                         CL_PROGRAM_KERNEL_NAMES,
                         ret_kernels_size,
                         ret_kernels,
                         NULL);        
        printf("\t\tCL_PROGRAM_KERNEL_NAMES: ");
        if(!strncmp("test", ret_kernels, ret_kernels_size)){
            printf("(OK)\n");
        }
        else{
            printf("(FAIL)\n");
        }

        if(program) clReleaseProgram(program); program=NULL;
        printf("\tRemoved program.\n");

        flag = clReleaseContext(context);
        if(flag != CL_SUCCESS) {
            printf("Error releasing context\n");
            printf("\t%s\n", OpenCLError(flag));
        }
        printf("\tRemoved context.\n");
        if(devices) free(devices); devices=NULL;
    }
    if(platforms) free(platforms); platforms=NULL;
    return EXIT_SUCCESS;
}

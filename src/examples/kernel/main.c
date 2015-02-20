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
    unsigned int i, j;
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
            printf("\t%s\n", OpenCLError(flag));
            return EXIT_FAILURE;
        }
        printf("\tCreated program!\n");
        char C_FLAGS[256];
        strcpy(C_FLAGS, "-cl-mad-enable -cl-no-signed-zeros -cl-finite-math-only -cl-fast-relaxed-math");
        flag = clBuildProgram(program, num_devices, devices, C_FLAGS, NULL, NULL);
        if(flag != CL_SUCCESS){
            printf("Error building program\n");
            printf("\t%s\n", OpenCLError(flag));
        }
        for(j = 0; j < num_devices; j++){
            printf("\t\tDevice %u: ", j);
            cl_build_status build_status;
            flag = clGetProgramBuildInfo(program,
                                         devices[j],
                                         CL_PROGRAM_BUILD_STATUS,
                                         sizeof(cl_build_status),
                                         &build_status,
                                         NULL);
            if((flag != CL_SUCCESS) || (build_status != CL_BUILD_SUCCESS)){
                printf("FAIL (NO CL_BUILD_SUCCESS REPORTED)\n");
                return EXIT_FAILURE;
            }
            else{
                printf("OK\n");
            }
        }

        // Create the kernel
        cl_kernel kernel = clCreateKernel(program, "test", &flag);
        if(flag != CL_SUCCESS){
            printf("Error creating kernel\n");
            printf("\t%s\n", OpenCLError(flag));
            return EXIT_FAILURE;
        }
        printf("\tCreated kernel!\n");

        // Print kernel info
        printf("\t\tCL_KERNEL_FUNCTION_NAME: ");
        size_t function_name_size = 0;
        flag = clGetKernelInfo(kernel,
                               CL_KERNEL_FUNCTION_NAME,
                               0,
                               NULL,
                               &function_name_size);
        if(flag != CL_SUCCESS){
            printf("FAIL (%s)\n", OpenCLError(flag));
        }
        else{
            char *function_name = (char*)malloc(function_name_size * sizeof(char));
            if(!function_name){
                printf("FAIL (MEMORY ALLOCATION FAILURE)\n");
            }
            else{
                flag = clGetKernelInfo(kernel,
                                    CL_KERNEL_FUNCTION_NAME,
                                    function_name_size,
                                    function_name,
                                    NULL);
                if(flag != CL_SUCCESS){
                    printf("FAIL (%s)\n", OpenCLError(flag));
                }
                else if(!strncmp("test", function_name, function_name_size)){
                    printf("OK\n");
                }
                else{
                    printf("FAIL\n");
                }
                free(function_name); function_name=NULL;
            }
        }
        printf("\t\tCL_KERNEL_NUM_ARGS: ");
        cl_uint num_args = 0;
        flag = clGetKernelInfo(kernel,
                               CL_KERNEL_NUM_ARGS,
                               sizeof(cl_uint),
                               &num_args,
                               NULL);
        if(flag != CL_SUCCESS){
            printf("FAIL (%s)\n", OpenCLError(flag));
        }
        else if(num_args == 5){
            printf("OK\n");
        }
        else{
            printf("FAIL\n");
        }
        printf("\t\tCL_KERNEL_CONTEXT: ");
        cl_context ret_context = NULL;
        flag = clGetKernelInfo(kernel,
                               CL_KERNEL_CONTEXT,
                               sizeof(cl_context),
                               &ret_context,
                               NULL);
        if(flag != CL_SUCCESS){
            printf("FAIL (%s)\n", OpenCLError(flag));
        }
        else if(ret_context == context){
            printf("OK\n");
        }
        else{
            printf("FAIL\n");
        }
        printf("\t\tCL_KERNEL_PROGRAM: ");
        cl_program ret_program = NULL;
        flag = clGetKernelInfo(kernel,
                               CL_KERNEL_PROGRAM,
                               sizeof(cl_program),
                               &ret_program,
                               NULL);
        if(flag != CL_SUCCESS){
            printf("FAIL (%s)\n", OpenCLError(flag));
        }
        else if(ret_program == program){
            printf("OK\n");
        }
        else{
            printf("FAIL\n");
        }
        printf("\t\tCL_KERNEL_ATTRIBUTES: ");
        size_t attribute_size = 0;
        flag = clGetKernelInfo(kernel,
                               CL_KERNEL_ATTRIBUTES,
                               0,
                               NULL,
                               &attribute_size);
        if(flag != CL_SUCCESS){
            printf("FAIL (%s)\n", OpenCLError(flag));
        }
        else{
            char *attribute = (char*)malloc(attribute_size * sizeof(char));
            if(!attribute){
                printf("FAIL (MEMORY ALLOCATION FAILURE)\n");
            }
            else{
                flag = clGetKernelInfo(kernel,
                                    CL_KERNEL_ATTRIBUTES,
                                    attribute_size,
                                    attribute,
                                    NULL);
                if(flag != CL_SUCCESS){
                    printf("FAIL (%s)\n", OpenCLError(flag));
                }
                else if(!strncmp("", attribute, attribute_size)){
                    printf("OK\n");
                }
                else{
                    printf("FAIL\n");
                }
                free(attribute); attribute=NULL;
            }
        }
        // Print kernel work group info
        for(j = 0; j < num_devices; j++){
            printf("\t\tDevice %u:\n", j);
            // Removed because it only works if it is a custom device
            /*
            printf("\t\t\tCL_KERNEL_GLOBAL_WORK_SIZE: ");
            size_t global_work_size[3];
            flag = clGetKernelWorkGroupInfo(kernel,
                                            devices[j],
                                            CL_KERNEL_GLOBAL_WORK_SIZE,
                                            3 * sizeof(size_t),
                                            global_work_size,
                                            NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else{
                printf("(%lu, %lu, %lu)\n",
                       global_work_size[0],
                       global_work_size[1],
                       global_work_size[2]);
            }
            */
            printf("\t\t\tCL_KERNEL_WORK_GROUP_SIZE: ");
            size_t work_group_size = 0;
            flag = clGetKernelWorkGroupInfo(kernel,
                                            devices[j],
                                            CL_KERNEL_WORK_GROUP_SIZE,
                                            sizeof(size_t),
                                            &work_group_size,
                                            NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else{
                printf("%lu\n", work_group_size);
            }
            printf("\t\t\tCL_KERNEL_COMPILE_WORK_GROUP_SIZE: ");
            size_t compile_work_group_size[3];
            flag = clGetKernelWorkGroupInfo(kernel,
                                            devices[j],
                                            CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                                            3 * sizeof(size_t),
                                            compile_work_group_size,
                                            NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else{
                printf("(%lu, %lu, %lu)\n",
                       compile_work_group_size[0],
                       compile_work_group_size[1],
                       compile_work_group_size[2]);
            }
            printf("\t\t\tCL_KERNEL_LOCAL_MEM_SIZE: ");
            cl_ulong local_mem_size = 0;
            flag = clGetKernelWorkGroupInfo(kernel,
                                            devices[j],
                                            CL_KERNEL_LOCAL_MEM_SIZE,
                                            sizeof(cl_ulong),
                                            &local_mem_size,
                                            NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else{
                printf("%lu\n", local_mem_size);
            }
            printf("\t\t\tCL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: ");
            cl_ulong work_group_size_multiple = 0;
            flag = clGetKernelWorkGroupInfo(kernel,
                                            devices[j],
                                            CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                            sizeof(size_t),
                                            &work_group_size_multiple,
                                            NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else{
                printf("%lu\n", work_group_size_multiple);
            }
            printf("\t\t\tCL_KERNEL_PRIVATE_MEM_SIZE: ");
            cl_ulong private_mem_size = 0;
            flag = clGetKernelWorkGroupInfo(kernel,
                                            devices[j],
                                            CL_KERNEL_PRIVATE_MEM_SIZE,
                                            sizeof(cl_ulong),
                                            &private_mem_size,
                                            NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else{
                printf("%lu\n", private_mem_size);
            }
        }
        // Print kernel args info
        for(j = 0; j < num_args; j++){
            printf("\t\tArg %u:\n", j);
            printf("\t\t\tCL_KERNEL_ARG_ADDRESS_QUALIFIER: ");
            cl_kernel_arg_address_qualifier address_qualifier;
            flag = clGetKernelArgInfo(kernel,
                                      j,
                                      CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                                      sizeof(cl_kernel_arg_address_qualifier),
                                      &address_qualifier,
                                      NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else if(address_qualifier == CL_KERNEL_ARG_ADDRESS_GLOBAL){
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
            printf("\t\t\tCL_KERNEL_ARG_ACCESS_QUALIFIER: ");
            cl_kernel_arg_access_qualifier access_qualifier;
            flag = clGetKernelArgInfo(kernel,
                                      j,
                                      CL_KERNEL_ARG_ACCESS_QUALIFIER,
                                      sizeof(cl_kernel_arg_access_qualifier),
                                      &access_qualifier,
                                      NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else if(access_qualifier == CL_KERNEL_ARG_ACCESS_READ_ONLY){
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
            printf("\t\t\tCL_KERNEL_ARG_TYPE_NAME: ");
            size_t type_name_size = 0;
            flag = clGetKernelArgInfo(kernel,
                                      j,
                                      CL_KERNEL_ARG_TYPE_NAME,
                                      0,
                                      NULL,
                                      &type_name_size);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else{
                char *type_name = (char*)malloc(type_name_size * sizeof(char));
                if(!type_name){
                    printf("FAIL (MEMORY ALLOCATION FAILURE)\n");
                }
                else{
                    flag = clGetKernelArgInfo(kernel,
                                              j,
                                              CL_KERNEL_ARG_TYPE_NAME,
                                              type_name_size,
                                              type_name,
                                              NULL);
                    if(flag != CL_SUCCESS){
                        printf("FAIL (%s)\n", OpenCLError(flag));
                    }
                    else{
                        printf("\"%s\"\n", type_name);
                    }
                    free(type_name); type_name=NULL;
                }
            }
            printf("\t\t\tCL_KERNEL_ARG_TYPE_QUALIFIER: ");
            cl_kernel_arg_type_qualifier type_qualifier;
            flag = clGetKernelArgInfo(kernel,
                                      j,
                                      CL_KERNEL_ARG_TYPE_QUALIFIER,
                                      sizeof(cl_kernel_arg_type_qualifier),
                                      &type_qualifier,
                                      NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else if(type_qualifier == CL_KERNEL_ARG_TYPE_CONST){
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
                printf("%lu (UNKNOWN)\n", type_qualifier);
            }
            printf("\t\t\tCL_KERNEL_ARG_NAME: ");
            size_t arg_name_size = 0;
            flag = clGetKernelArgInfo(kernel,
                                      j,
                                      CL_KERNEL_ARG_NAME,
                                      0,
                                      NULL,
                                      &arg_name_size);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
            }
            else{
                char *arg_name = (char*)malloc(arg_name_size * sizeof(char));
                if(!arg_name){
                    printf("FAIL (MEMORY ALLOCATION FAILURE)\n");
                }
                else{
                    flag = clGetKernelArgInfo(kernel,
                                            j,
                                            CL_KERNEL_ARG_NAME,
                                            arg_name_size,
                                            arg_name,
                                            NULL);
                    if(flag != CL_SUCCESS){
                        printf("FAIL (%s)\n", OpenCLError(flag));
                    }
                    else{
                        printf("\"%s\"\n", arg_name);
                    }
                    free(arg_name); arg_name=NULL;
                }
            }
        }
        
        flag = clReleaseKernel(kernel);
        if(flag != CL_SUCCESS){
            printf("Error releasing kernel\n");
            printf("\t%s\n", OpenCLError(flag));
        }
        else{
            printf("\tRemoved kernel.\n");
        }

        flag = clReleaseProgram(program); program=NULL;
        if(flag != CL_SUCCESS) {
            printf("Error releasing program\n");
            printf("\t%s\n", OpenCLError(flag));
        }
        else{
            printf("\tRemoved program.\n");
        }

        flag = clReleaseContext(context);
        if(flag != CL_SUCCESS) {
            printf("Error releasing context\n");
            printf("\t%s\n", OpenCLError(flag));
        }
        else{
            printf("\tRemoved context.\n");
        }
        if(devices) free(devices); devices=NULL;
    }
    if(platforms) free(platforms); platforms=NULL;
    return EXIT_SUCCESS;
}

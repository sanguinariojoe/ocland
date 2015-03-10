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

int main(int argc, char *argv[])
{
    unsigned int i, j;
    cl_uint num_entries = 0, num_platforms = 0;
    cl_platform_id *platforms = NULL;
    cl_int flag;
    cl_bool test_failed = CL_FALSE;

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
        return EXIT_FAILURE;
    }

    flag = clGetPlatformIDs(num_entries, platforms, &num_platforms);
    if(flag != CL_SUCCESS){
        printf("Error getting platforms\n");
        printf("\t%s\n", OpenCLError(flag));
        free(platforms); platforms = NULL;
        return EXIT_FAILURE;
    }

    // Work on each platform separately
    for(i = 0; i < num_platforms; i++){
        size_t platform_name_size = 0;
        char *platform_name = NULL;

        printf("Platform %u...\n", i);
        flag = clGetPlatformInfo(platforms[i],
                                 CL_PLATFORM_NAME,
                                 0,
                                 NULL,
                                 &platform_name_size);
        if(flag != CL_SUCCESS){
            printf("Failure getting platform name size\n");
            printf("\t%s\n", OpenCLError(flag));
            test_failed = CL_TRUE;
            continue;
        }

        platform_name = (char*)malloc(platform_name_size);
        if(!platform_name){
            printf("Failure allocating memory for the platform name\n");
            test_failed = CL_TRUE;
            continue;
        }
        flag = clGetPlatformInfo(platforms[i],
                                 CL_PLATFORM_NAME,
                                 platform_name_size,
                                 platform_name,
                                 NULL);
        if(flag != CL_SUCCESS){
            printf("Failure getting platform name\n");
            printf("\t%s\n", OpenCLError(flag));
            test_failed = CL_TRUE;
            free(platform_name); platform_name = NULL;
            continue;
        }
        printf("\t%s\n", platform_name);
        free(platform_name); platform_name = NULL;

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
            test_failed = CL_TRUE;
            continue;
        }
        if(!num_devices){
            printf("\tWithout devices.\n");
            // this is not a fail actually, just move to next platform
            continue;
        }

        num_entries = num_devices;
        devices = (cl_device_id*)malloc(num_entries * sizeof(cl_device_id));
        if(!devices){
            printf("Failure allocating memory for the devices\n");
            test_failed = CL_TRUE;
            continue;
        }
        flag = clGetDeviceIDs(platforms[i],
                              CL_DEVICE_TYPE_ALL,
                              num_entries,
                              devices,
                              &num_devices);
        if(flag != CL_SUCCESS){
            printf("Failure getting the devices\n");
            printf("\t%s\n", OpenCLError(flag));
            if(devices) free(devices); devices = NULL;
            test_failed = CL_TRUE;
            continue;
        }

        // Print devices data
        for(j = 0; j < num_devices; j++){
            printf("\tDevice %u...\n", j);
            printf("\t\tNAME: ");
            size_t device_name_size = 0;
            flag = clGetDeviceInfo(devices[j],
                                   CL_DEVICE_NAME,
                                   0,
                                   NULL,
                                   &device_name_size);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                test_failed = CL_TRUE;
                continue;
            }
            char *device_name = (char*)malloc(device_name_size);
            if(!device_name){
                printf("FAIL (Memory allocation failure)\n");
                test_failed = CL_TRUE;
                continue;
            }
            flag = clGetDeviceInfo(devices[j],
                                   CL_DEVICE_NAME,
                                   device_name_size,
                                   device_name,
                                   NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                free(device_name); device_name = NULL;
                test_failed = CL_TRUE;
                continue;
            }
            printf("\"%s\"\n", device_name);
            free(device_name); device_name = NULL;
            printf("\t\tTYPE: ");
            cl_device_type dtype;
            flag = clGetDeviceInfo(devices[j],
                                   CL_DEVICE_TYPE,
                                   sizeof(cl_device_type),
                                   &dtype,
                                   NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                test_failed = CL_TRUE;
                continue;
            }
            switch(dtype){
                case CL_DEVICE_TYPE_CPU:
                    printf("CL_DEVICE_TYPE_CPU\n");
                    break;
                case CL_DEVICE_TYPE_GPU:
                    printf("CL_DEVICE_TYPE_GPU\n");
                    break;
                case CL_DEVICE_TYPE_ACCELERATOR:
                    printf("CL_DEVICE_TYPE_ACCELERATOR\n");
                    break;
                case CL_DEVICE_TYPE_DEFAULT:
                    printf("CL_DEVICE_TYPE_DEFAULT\n");
                    break;
                default:
                    printf("Unknow\n");
            }
            printf("\t\tCL_DEVICE_ADDRESS_BITS: ");
            cl_uint bits;
            flag = clGetDeviceInfo(devices[j],
                                   CL_DEVICE_ADDRESS_BITS,
                                   sizeof(cl_uint),
                                   &bits,
                                   NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                test_failed = CL_TRUE;
                continue;
            }
            printf("%u\n", bits);
            printf("\t\tCL_DEVICE_AVAILABLE: ");
            cl_bool available = CL_FALSE;
            flag = clGetDeviceInfo(devices[j],
                                   CL_DEVICE_AVAILABLE,
                                   sizeof(cl_bool),
                                   &available,
                                   NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                test_failed = CL_TRUE;
                continue;
            }
            switch(available){
                case CL_TRUE:
                    printf("YES\n");
                    break;
                case CL_FALSE:
                    printf("NO\n");
                    break;
                default:
                    printf("Wrong value\n");
                    test_failed = CL_TRUE;
            }
            printf("\t\tCL_DEVICE_EXTENSIONS: ");
            size_t extensions_size = 0;
            flag = clGetDeviceInfo(devices[j],
                                   CL_DEVICE_EXTENSIONS,
                                   0,
                                   NULL,
                                   &extensions_size);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                test_failed = CL_TRUE;
                continue;
            }
            char *extensions = (char*)malloc(extensions_size);
            if(!extensions){
                printf("FAIL (Memory allocation failure)\n");
                test_failed = CL_TRUE;
                continue;
            }
            flag = clGetDeviceInfo(devices[j],
                                   CL_DEVICE_EXTENSIONS,
                                   extensions_size,
                                   extensions,
                                   NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                free(extensions); extensions = NULL;
                test_failed = CL_TRUE;
                continue;
            }
            printf("\"%s\"\n", extensions);
            free(extensions); extensions = NULL;
            printf("\t\tCL_DEVICE_VERSION: ");
            size_t opencl_version_size = 0;
            flag = clGetDeviceInfo(devices[j],
                                   CL_DEVICE_VERSION,
                                   0,
                                   NULL,
                                   &opencl_version_size);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                test_failed = CL_TRUE;
                continue;
            }
            char *opencl_version = (char*)malloc(opencl_version_size);
            if(!opencl_version){
                printf("FAIL (Memory allocation failure)\n");
                test_failed = CL_TRUE;
                continue;
            }
            flag = clGetDeviceInfo(devices[j],
                                   CL_DEVICE_VERSION,
                                   opencl_version_size,
                                   opencl_version,
                                   NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                free(opencl_version); opencl_version = NULL;
                test_failed = CL_TRUE;
                continue;
            }
            printf("\"%s\"\n", opencl_version);
            free(opencl_version); opencl_version = NULL;
            printf("\t\tCL_DRIVER_VERSION: ");
            size_t driver_version_size = 0;
            flag = clGetDeviceInfo(devices[j],
                                   CL_DRIVER_VERSION,
                                   0,
                                   NULL,
                                   &driver_version_size);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                test_failed = CL_TRUE;
                continue;
            }
            char *driver_version = (char*)malloc(driver_version_size);
            if(!driver_version){
                printf("FAIL (Memory allocation failure)\n");
                test_failed = CL_TRUE;
                continue;
            }
            flag = clGetDeviceInfo(devices[j],
                                   CL_DRIVER_VERSION,
                                   driver_version_size,
                                   driver_version,
                                   NULL);
            if(flag != CL_SUCCESS){
                printf("FAIL (%s)\n", OpenCLError(flag));
                free(driver_version); driver_version = NULL;
                test_failed = CL_TRUE;
                continue;
            }
            printf("\"%s\"\n", driver_version);
            free(driver_version); driver_version = NULL;
        }
        if(devices) free(devices); devices=NULL;
    }
    if(platforms) free(platforms); platforms=NULL;
    if(test_failed) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

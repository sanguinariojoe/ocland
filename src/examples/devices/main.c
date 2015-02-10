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

int main(int argc, char *argv[])
{
    unsigned int i,j;
    cl_uint num_entries = 0, num_platforms = 0;
    cl_platform_id *platforms = NULL;
    cl_int flag;
    // Get number of platforms
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
    // Build platforms array
    num_entries = num_platforms;
    platforms   = (cl_platform_id*)malloc(num_entries*sizeof(cl_platform_id));
    // Get platforms array
    flag = clGetPlatformIDs(num_entries, platforms, &num_platforms);
    if(flag != CL_SUCCESS){
        printf("Error getting platforms\n");
        printf("\t%s\n", OpenCLError(flag));
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
        flag = clGetDeviceIDs(platforms[i],
                              CL_DEVICE_TYPE_ALL,
                              num_entries,
                              devices,
                              &num_devices);
        if( (flag != CL_SUCCESS) && (flag != CL_DEVICE_NOT_FOUND) ) {
            printf("Error getting number of devices\n");
            printf("\t%s\n", OpenCLError(flag));
            return EXIT_FAILURE;
        }
        if( (!num_devices) || (flag == CL_DEVICE_NOT_FOUND) ){
            printf("\tWithout devices.\n");
            continue;
        }
        // Build devices array
        num_entries = num_devices;
        devices = (cl_device_id*)malloc(num_entries * sizeof(cl_device_id));
        // Get devices array
        flag = clGetDeviceIDs(platforms[i],
                              CL_DEVICE_TYPE_ALL,
                              num_entries,
                              devices,
                              &num_devices);
        if(flag != CL_SUCCESS){
            printf("Error getting number of devices\n");
            printf("\t%s\n", OpenCLError(flag));
            return EXIT_FAILURE;
        }
        // Print devices data
        for(j=0;j<num_devices;j++){
            printf("\tDevice %u...\n", j);
            char buffer[1025];
            clGetDeviceInfo(devices[j],CL_DEVICE_NAME,1025*sizeof(char),buffer, NULL);
            printf("\t\tNAME: %s\n", buffer);
            cl_device_type dtype;
            clGetDeviceInfo(devices[j],CL_DEVICE_TYPE,sizeof(cl_device_type),&dtype, NULL);
            printf("\t\tTYPE: ");
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
            cl_uint bits;
            clGetDeviceInfo(devices[j],
                            CL_DEVICE_ADDRESS_BITS,
                            sizeof(cl_uint),
                            &bits,
                            NULL);
            printf("\t\tCL_DEVICE_ADDRESS_BITS: %u\n", bits);
            cl_bool available = CL_FALSE;
            clGetDeviceInfo(devices[j],
                            CL_DEVICE_AVAILABLE,
                            sizeof(cl_bool),
                            &available,
                            NULL);
            printf("\t\tCL_DEVICE_AVAILABLE: ");
            switch(available){
                case CL_TRUE:
                    printf("YES\n");
                    break;
                case CL_FALSE:
                    printf("NO\n");
                    break;
                default:
                    printf("?\n");
            }
            size_t extensions_size = 0;
            clGetDeviceInfo(devices[j],
                            CL_DEVICE_BUILT_IN_KERNELS,
                            0,
                            NULL,
                            &extensions_size);
            if(extensions_size){
                char extensions[extensions_size];
                strcpy(extensions, "");
                clGetDeviceInfo(devices[j],
                                CL_DEVICE_EXTENSIONS,
                                extensions_size,
                                &extensions,
                                NULL);
                printf("\t\tCL_DEVICE_EXTENSIONS: \"%s\"\n", extensions);
            }
            size_t opencl_version_size = 0;
            clGetDeviceInfo(devices[j],
                            CL_DEVICE_BUILT_IN_KERNELS,
                            0,
                            NULL,
                            &opencl_version_size);
            if(opencl_version_size){
                char opencl_version[opencl_version_size];
                strcpy(opencl_version, "");
                clGetDeviceInfo(devices[j],
                                CL_DEVICE_VERSION,
                                opencl_version_size,
                                &opencl_version,
                                NULL);
                printf("\t\tCL_DEVICE_VERSION: \"%s\"\n", opencl_version);
            }
            size_t driver_version_size = 0;
            clGetDeviceInfo(devices[j],
                            CL_DRIVER_VERSION,
                            0,
                            NULL,
                            &driver_version_size);
            if(driver_version_size){
                char driver_version[driver_version_size];
                strcpy(driver_version, "");
                clGetDeviceInfo(devices[j],
                                CL_DRIVER_VERSION,
                                driver_version_size,
                                &driver_version,
                                NULL);
                printf("\t\tCL_DRIVER_VERSION: \"%s\"\n", driver_version);
            }
            
            
        }
        if(devices) free(devices); devices=NULL;
    }
    if(platforms) free(platforms); platforms=NULL;
    return EXIT_SUCCESS;
}

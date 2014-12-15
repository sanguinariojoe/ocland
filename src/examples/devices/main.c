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

int main(int argc, char *argv[])
{
    unsigned int i,j;
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
    for(i=0;i<num_platforms;i++){
        printf("Platform %u...\n", i);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1025*sizeof(char),buffer, NULL);
        printf("\t%s\n", buffer);
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
        // Print devices data
        for(j=0;j<num_devices;j++){
            printf("\tDevice %u...\n", j);
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

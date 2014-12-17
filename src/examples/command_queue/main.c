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

        // Print command queue data
        for(j = 0; j < num_devices; j++){
            printf("\tCommand queue %u / %u!\n", j, num_devices-1);
            cl_context context_helper = NULL;
            clGetCommandQueueInfo(queues[j],
                                  CL_QUEUE_CONTEXT,
                                  sizeof(cl_context),
                                  &context_helper,
                                  NULL);
            printf("\t\tCL_QUEUE_CONTEXT: ");
            if(context_helper == context){
                printf("OK\n");
            }
            else{
                printf("FAIL\n");
            }
            cl_device_id device_helper = NULL;
            clGetCommandQueueInfo(queues[j],
                                  CL_QUEUE_DEVICE,
                                  sizeof(cl_device_id),
                                  &device_helper,
                                  NULL);
            printf("\t\tCL_QUEUE_DEVICE: ");
            if(device_helper == devices[j]){
                printf("OK\n");
            }
            else{
                printf("FAIL\n");
            }
            cl_uint ref_count = 0;
            clGetCommandQueueInfo(queues[j],
                                  CL_QUEUE_REFERENCE_COUNT,
                                  sizeof(cl_uint),
                                  &ref_count,
                                  NULL);
            printf("\t\tCL_QUEUE_REFERENCE_COUNT: %u\n", ref_count);
            cl_command_queue_properties properties_helper = 0;
            clGetCommandQueueInfo(queues[j],
                                  CL_QUEUE_PROPERTIES,
                                  sizeof(cl_command_queue_properties),
                                  &properties_helper,
                                  NULL);
            printf("\t\tCL_QUEUE_PROPERTIES: %lu ", properties_helper);
            if(properties_helper == 0){
                printf("(OK)\n");
            }
            else{
                printf("(FAIL)\n");
            }
        }
        
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

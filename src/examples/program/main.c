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
}";

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

        // Create the program
        size_t program_src_length = (strlen(program_src) + 1) * sizeof(char);
        cl_program program = clCreateProgramWithSource(context, 1, (const char **)&program_src,
                                                       &program_src_length, &flag);
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
        printf("\tRemoved the program.\n");

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

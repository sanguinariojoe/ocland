/*
 *  This file is part of ocland, a free CFD program based on SPH.
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

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <CL/opencl.h>

/// OpenCL program source code
const char* program_src = "__kernel void test(__global float* x, __global float* y,__global float* z,unsigned int i0, unsigned int N){unsigned int i = get_global_id(0);if(i >= N)return;i+=i0;z[i]=x[i]*y[i];}";

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
        printf("%d vs. %d\n", flag, CL_SUCCESS);
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
    // Print platforms data
    for(i=0;i<num_platforms;i++){
        printf("Platform %u...\n", i);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_PROFILE,1025*sizeof(char),buffer, NULL);
        printf("\tPROFILE: %s\n", buffer);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_VERSION,1025*sizeof(char),buffer, NULL);
        printf("\tVERSION: %s\n", buffer);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1025*sizeof(char),buffer, NULL);
        printf("\tNAME: %s\n", buffer);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_VENDOR,1025*sizeof(char),buffer, NULL);
        printf("\tVENDOR: %s\n", buffer);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_ICD_SUFFIX_KHR,1025*sizeof(char),buffer, NULL);
        printf("\tCL_PLATFORM_ICD_SUFFIX_KHR: %s\n", buffer);
        printf("\t---\n");
        // Get number of devices
        num_entries = 0;
        cl_uint num_devices = 0;
        cl_device_id *devices = NULL;
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
            printf("\tWithout devices.");
            continue;
        }
        // Build devices array
        num_entries = num_devices;
        devices     = (cl_device_id*)malloc(num_entries*sizeof(cl_device_id));
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
        // Create buffers
        unsigned int n = 1000000;
        cl_float *hx = (cl_float*)malloc(n*sizeof(cl_float));
        cl_float *hy = (cl_float*)malloc(n*sizeof(cl_float));
        cl_float *hz = (cl_float*)malloc(n*sizeof(cl_float));
        for(j=0;j<n;j++){
            hx[j] = j;
            hy[j] = 1.f / (j*j);
        }
        cl_mem x, y, z;
        x = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           n*sizeof(cl_float), hx, &flag);
        if(flag != CL_SUCCESS) {
            printf("Error creating x buffer\n");
            if(flag == CL_INVALID_CONTEXT)
                printf("\tCL_INVALID_CONTEXT\n");
            if(flag == CL_INVALID_VALUE)
                printf("\tCL_INVALID_VALUE\n");
            if(flag == CL_INVALID_BUFFER_SIZE)
                printf("\tCL_INVALID_BUFFER_SIZE\n");
            if(flag == CL_INVALID_HOST_PTR)
                printf("\tCL_INVALID_HOST_PTR\n");
            if(flag == CL_MEM_OBJECT_ALLOCATION_FAILURE)
                printf("\tCL_MEM_OBJECT_ALLOCATION_FAILURE\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        y = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           n*sizeof(cl_float), hy, &flag);
        if(flag != CL_SUCCESS) {
            printf("Error creating y buffer\n");
            if(flag == CL_INVALID_CONTEXT)
                printf("\tCL_INVALID_CONTEXT\n");
            if(flag == CL_INVALID_VALUE)
                printf("\tCL_INVALID_VALUE\n");
            if(flag == CL_INVALID_BUFFER_SIZE)
                printf("\tCL_INVALID_BUFFER_SIZE\n");
            if(flag == CL_INVALID_HOST_PTR)
                printf("\tCL_INVALID_HOST_PTR\n");
            if(flag == CL_MEM_OBJECT_ALLOCATION_FAILURE)
                printf("\tCL_MEM_OBJECT_ALLOCATION_FAILURE\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        z = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                           n*sizeof(cl_float), NULL, &flag);
        if(flag != CL_SUCCESS) {
            printf("Error creating z buffer\n");
            if(flag == CL_INVALID_CONTEXT)
                printf("\tCL_INVALID_CONTEXT\n");
            if(flag == CL_INVALID_VALUE)
                printf("\tCL_INVALID_VALUE\n");
            if(flag == CL_INVALID_BUFFER_SIZE)
                printf("\tCL_INVALID_BUFFER_SIZE\n");
            if(flag == CL_INVALID_HOST_PTR)
                printf("\tCL_INVALID_HOST_PTR\n");
            if(flag == CL_MEM_OBJECT_ALLOCATION_FAILURE)
                printf("\tCL_MEM_OBJECT_ALLOCATION_FAILURE\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        // Create command queues by each device
        cl_command_queue *queues = (cl_command_queue*)malloc(num_devices*sizeof(cl_command_queue));
        for(j=0;j<num_devices;j++){
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
        // Load and compile the program
        size_t program_src_length = (strlen(program_src) + 1)*sizeof(char);
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
        char program_log[16384];
        for(j=0;j<num_devices;j++){
            flag = clGetProgramBuildInfo(program, devices[j], CL_PROGRAM_BUILD_LOG, 16384*sizeof(char), program_log, NULL );
            if(flag != CL_SUCCESS){
                printf("Error getting program build log\n");
                if(flag == CL_INVALID_DEVICE)
                    printf("\tCL_INVALID_DEVICE\n");
                if(flag == CL_INVALID_VALUE)
                    printf("\tCL_INVALID_VALUE\n");
                if(flag == CL_INVALID_PROGRAM)
                    printf("\tCL_INVALID_PROGRAM\n");
                if(flag == CL_OUT_OF_RESOURCES)
                    printf("\tCL_OUT_OF_RESOURCES\n");
                if(flag == CL_OUT_OF_HOST_MEMORY)
                    printf("\tCL_OUT_OF_HOST_MEMORY\n");
                return EXIT_FAILURE;
            }
            printf("--- Build log (device %u / %u) -------------\n", j, num_devices-1);
            printf("%s\n", program_log);
            printf("------------- Build log (device %u / %u) ---\n", j, num_devices-1);
        }
        printf("\tBuilt program!\n");
        // Create the kernel
        cl_kernel kernel = clCreateKernel(program,"test",&flag);
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
        // Compute the amount of data computed by each device
        unsigned int n_per_device = n / num_devices;
        unsigned int i0[num_devices], N[num_devices];
        i0[0] = 0;
        N[0]  = n_per_device;
        for(i=1;i<num_devices;i++){
            i0[i] = i0[i-1] + N[i-1];
            N[i] = n_per_device;
        }
        N[num_devices-1] += n % num_devices;
        printf("\t%u points computed per device (%u computed by the last one).\n",n_per_device,N[num_devices-1]);
        // Send common to all devices kernel arguments
        flag |= clSetKernelArg(kernel,0,sizeof(cl_mem),&x);
        flag |= clSetKernelArg(kernel,1,sizeof(cl_mem),&y);
        flag |= clSetKernelArg(kernel,2,sizeof(cl_mem),&z);
        if(flag != CL_SUCCESS){
            printf("Error setting common kernel arguments\n");
            if(flag & CL_INVALID_KERNEL)
                printf("\tCL_INVALID_KERNEL\n");
            if(flag & CL_INVALID_ARG_INDEX)
                printf("\tCL_INVALID_ARG_INDEX\n");
            if(flag & CL_INVALID_ARG_VALUE)
                printf("\tCL_INVALID_ARG_VALUE\n");
            if(flag & CL_INVALID_MEM_OBJECT)
                printf("\tCL_INVALID_MEM_OBJECT\n");
            if(flag & CL_INVALID_SAMPLER)
                printf("\tCL_INVALID_SAMPLER\n");
            if(flag & CL_INVALID_ARG_SIZE)
                printf("\tCL_INVALID_ARG_SIZE\n");
            if(flag & CL_OUT_OF_RESOURCES)
                printf("\tCL_OUT_OF_RESOURCES\n");
            if(flag & CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        // Launch the execution at each device
        for(i=0;i<num_devices;i++){
            printf("\tDevice %u...\n",i);
            // Set device specific kernel arguments
            flag |= clSetKernelArg(kernel,3,sizeof(unsigned int),&(i0[i]));
            flag |= clSetKernelArg(kernel,4,sizeof(unsigned int),&(N[i]));
            if(flag != CL_SUCCESS){
                printf("Error setting %u device' kernel arguments\n", i);
                if(flag & CL_INVALID_KERNEL)
                    printf("\tCL_INVALID_KERNEL\n");
                if(flag & CL_INVALID_ARG_INDEX)
                    printf("\tCL_INVALID_ARG_INDEX\n");
                if(flag & CL_INVALID_ARG_VALUE)
                    printf("\tCL_INVALID_ARG_VALUE\n");
                if(flag & CL_INVALID_MEM_OBJECT)
                    printf("\tCL_INVALID_MEM_OBJECT\n");
                if(flag & CL_INVALID_SAMPLER)
                    printf("\tCL_INVALID_SAMPLER\n");
                if(flag & CL_INVALID_ARG_SIZE)
                    printf("\tCL_INVALID_ARG_SIZE\n");
                if(flag & CL_OUT_OF_RESOURCES)
                    printf("\tCL_OUT_OF_RESOURCES\n");
                if(flag & CL_OUT_OF_HOST_MEMORY)
                    printf("\tCL_OUT_OF_HOST_MEMORY\n");
                return EXIT_FAILURE;
            }
            printf("\t\tArguments sent!\n");
            // Launch the kernel
            printf("\t\tKernel computed!\n");
            // Recover the data
            flag = clEnqueueReadBuffer(queues[i],z,CL_TRUE,i0[i]*sizeof(float),N[i]*sizeof(float),hz + i0[i]*sizeof(float),0,NULL,NULL);
            if(flag != CL_SUCCESS){
                printf("Error getting result\n");
                if(flag & CL_INVALID_COMMAND_QUEUE)
                    printf("\tCL_INVALID_COMMAND_QUEUE\n");
                if(flag & CL_INVALID_CONTEXT)
                    printf("\tCL_INVALID_CONTEXT\n");
                if(flag & CL_INVALID_MEM_OBJECT)
                    printf("\tCL_INVALID_MEM_OBJECT\n");
                if(flag & CL_INVALID_VALUE)
                    printf("\tCL_INVALID_VALUE\n");
                if(flag & CL_INVALID_EVENT_WAIT_LIST)
                    printf("\tCL_INVALID_EVENT_WAIT_LIST\n");
                if(flag & CL_MEM_OBJECT_ALLOCATION_FAILURE)
                    printf("\tCL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                if(flag & CL_OUT_OF_HOST_MEMORY)
                    printf("\tCL_OUT_OF_HOST_MEMORY\n");
                return EXIT_FAILURE;
            }
            printf("\t\tResult get!\n");
        }
        // Clean up
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
        flag = clReleaseProgram(program);
        if(flag != CL_SUCCESS){
            printf("Error releasing program\n");
            if(flag == CL_INVALID_PROGRAM)
                printf("\tCL_INVALID_PROGRAM\n");
            if(flag == CL_OUT_OF_RESOURCES)
                printf("\tCL_OUT_OF_RESOURCES\n");
            if(flag == CL_OUT_OF_HOST_MEMORY)
                printf("\tCL_OUT_OF_HOST_MEMORY\n");
            return EXIT_FAILURE;
        }
        printf("\tRemoved program.\n");
        if(x) clReleaseMemObject(x); x=NULL;
        if(y) clReleaseMemObject(y); y=NULL;
        if(z) clReleaseMemObject(z); z=NULL;
        if(hx) free(hx); hx=NULL;
        if(hy) free(hy); hy=NULL;
        if(hz) free(hz); hz=NULL;
        printf("\tCleaned memory.\n");
        for(j=0;j<num_devices;j++){
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

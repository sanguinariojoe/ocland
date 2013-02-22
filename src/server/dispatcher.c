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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <ocland/common/dataExchange.h>
#include <ocland/server/dispatcher.h>
#include <ocland/server/ocland_cl.h>

#ifndef BUFF_SIZE
    #define BUFF_SIZE 1025u
#endif

void *client_thread(void *socket)
{
    char buffer[BUFF_SIZE];
    int *clientfd = (int*)socket;
    validator v = NULL;
    initValidator(&v);
    // Work until client still connected
    while(*clientfd >= 0){
        dispatch(clientfd, buffer, v);
    }
    closeValidator(&v);
    pthread_exit(NULL);
    return NULL;
}

int dispatch(int* clientfd, char* buffer, validator v)
{
    unsigned int commDim=0;
    // Read in order to find command declaration.
    // Command declaration is the number of characters of the
    // command name, bassically because is a sort ammount of
    // data that can be received asynchronously.
    int msgSize = recv(*clientfd,&commDim,sizeof(unsigned int),MSG_DONTWAIT);
    if(msgSize < 0){
        return 0;
    }
    if(!msgSize){
        // Peer called to close connection
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getsockname(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        printf("%s disconnected, goodbye ;-)\n", inet_ntoa(adr_inet.sin_addr)); fflush(stdout);
        close(*clientfd);
        *clientfd = -1;
        return 1;
    }
    if(commDim > BUFF_SIZE){
        // Buffer overflow, disconnect client for protection
        struct sockaddr_in adr_inet;
        socklen_t len_inet;
        len_inet = sizeof(adr_inet);
        getsockname(*clientfd, (struct sockaddr*)&adr_inet, &len_inet);
        printf("%s request potential overflowing transfer", inet_ntoa(adr_inet.sin_addr));
        printf(", disconnected for protection...\n"); fflush(stdout);
        close(*clientfd);
        *clientfd = -1;
        return 0;
    }
    // Read command to process
    Recv(clientfd,buffer,commDim*sizeof(char),MSG_WAITALL);
    // Look for the command called
    if(!strcmp(buffer,"clGetPlatformIDs")){
        return ocland_clGetPlatformIDs(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetPlatformInfo")){
        return ocland_clGetPlatformInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetDeviceIDs")){
        return ocland_clGetDeviceIDs(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetDeviceInfo")){
        return ocland_clGetDeviceInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateContext")){
        return ocland_clCreateContext(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateContextFromType")){
        return ocland_clCreateContextFromType(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clRetainContext")){
        return ocland_clRetainContext(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clReleaseContext")){
        return ocland_clReleaseContext(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetContextInfo")){
        return ocland_clGetContextInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateCommandQueue")){
        return ocland_clCreateCommandQueue(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clRetainCommandQueue")){
        return ocland_clRetainCommandQueue(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clReleaseCommandQueue")){
        return ocland_clReleaseCommandQueue(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetCommandQueueInfo")){
        return ocland_clGetCommandQueueInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateBuffer")){
        return ocland_clCreateBuffer(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clRetainMemObject")){
        return ocland_clRetainMemObject(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clReleaseMemObject")){
        return ocland_clReleaseMemObject(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetSupportedImageFormats")){
        return ocland_clGetSupportedImageFormats(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetMemObjectInfo")){
        return ocland_clGetMemObjectInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetImageInfo")){
        return ocland_clGetImageInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateSampler")){
        return ocland_clCreateSampler(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clRetainSampler")){
        return ocland_clRetainSampler(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clReleaseSampler")){
        return ocland_clReleaseSampler(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetSamplerInfo")){
        return ocland_clGetSamplerInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateProgramWithSource")){
        return ocland_clCreateProgramWithSource(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateProgramWithBinary")){
        return ocland_clCreateProgramWithBinary(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clRetainProgram")){
        return ocland_clRetainProgram(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clReleaseProgram")){
        return ocland_clReleaseProgram(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clBuildProgram")){
        return ocland_clBuildProgram(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetProgramBuildInfo")){
        return ocland_clGetProgramBuildInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateKernel")){
        return ocland_clCreateKernel(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateKernelsInProgram")){
        return ocland_clCreateKernelsInProgram(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clRetainKernel")){
        return ocland_clRetainKernel(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clReleaseKernel")){
        return ocland_clReleaseKernel(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clSetKernelArg")){
        return ocland_clSetKernelArg(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clSetKernelNullArg")){
        return ocland_clSetKernelNullArg(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetKernelInfo")){
        return ocland_clGetKernelInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetKernelWorkGroupInfo")){
        return ocland_clGetKernelWorkGroupInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clWaitForEvents")){
        return ocland_clWaitForEvents(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetEventInfo")){
        return ocland_clGetEventInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clRetainEvent")){
        return ocland_clRetainEvent(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clReleaseEvent")){
        return ocland_clReleaseEvent(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetEventProfilingInfo")){
        return ocland_clGetEventProfilingInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clFlush")){
        return ocland_clFlush(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clFinish")){
        return ocland_clFinish(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueReadBuffer")){
        return ocland_clEnqueueReadBuffer(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueWriteBuffer")){
        return ocland_clEnqueueWriteBuffer(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueCopyBuffer")){
        return ocland_clEnqueueCopyBuffer(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueCopyImage")){
        return ocland_clEnqueueCopyImage(clientfd, buffer, v);
    }
    #ifdef CL_API_SUFFIX__VERSION_1_1
    else if(!strcmp(buffer,"clCreateSubBuffer")){
        return ocland_clCreateSubBuffer(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateUserEvent")){
        return ocland_clCreateUserEvent(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clSetUserEventStatus")){
        return ocland_clSetUserEventStatus(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueReadBufferRect")){
        return ocland_clEnqueueReadBufferRect(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueWriteBufferRect")){
        return ocland_clEnqueueWriteBufferRect(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueCopyBufferRect")){
        return ocland_clEnqueueCopyBufferRect(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueReadImage")){
        return ocland_clEnqueueReadImage(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueWriteImage")){
        return ocland_clEnqueueWriteImage(clientfd, buffer, v);
    }
    #endif
    #ifdef CL_API_SUFFIX__VERSION_1_2
    else if(!strcmp(buffer,"clCreateSubDevices")){
        return ocland_clCreateSubDevices(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clRetainDevice")){
        return ocland_clRetainDevice(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clReleaseDevice")){
        return ocland_clReleaseDevice(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateImage")){
        return ocland_clCreateImage(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCreateProgramWithBuiltInKernels")){
        return ocland_clCreateProgramWithBuiltInKernels(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clCompileProgram")){
        return ocland_clCompileProgram(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clLinkProgram")){
        return ocland_clLinkProgram(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clUnloadPlatformCompiler")){
        return ocland_clUnloadPlatformCompiler(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetProgramInfo")){
        return ocland_clGetProgramInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clGetKernelArgInfo")){
        return ocland_clGetKernelArgInfo(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueFillBuffer")){
        return ocland_clEnqueueFillBuffer(clientfd, buffer, v);
    }
    else if(!strcmp(buffer,"clEnqueueFillImage")){
        return ocland_clEnqueueFillImage(clientfd, buffer, v);
    }
    #endif
    return 0;
}

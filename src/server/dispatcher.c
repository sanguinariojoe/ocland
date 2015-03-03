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
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
    #include <winsock2.h>
    typedef int socklen_t;
    #define MSG_DONTWAIT 0 //TODO: non-blocking mode for socket?
    #ifndef MSG_WAITALL
        //is undefined on MINGW system headers
        #define MSG_WAITALL 0x8
    #endif
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include <pthread.h>

#include <ocland/common/dataExchange.h>
#include <ocland/server/dispatcher.h>
#include <ocland/server/ocland_cl.h>

#ifndef BUFF_SIZE
    #define BUFF_SIZE 1025u
#endif

typedef int(*func)(int* clientfd, char* buffer, validator v);

/// List of functions to dispatch request from client
static func dispatchFunctions[75] =
{
    &ocland_clGetPlatformIDs,
    &ocland_clGetPlatformInfo,
    &ocland_clGetDeviceIDs,
    &ocland_clGetDeviceInfo,
    &ocland_clCreateContext,
    &ocland_clCreateContextFromType,
    &ocland_clRetainContext,
    &ocland_clReleaseContext,
    &ocland_clGetContextInfo,
    &ocland_clCreateCommandQueue,
    &ocland_clRetainCommandQueue,
    &ocland_clReleaseCommandQueue,
    &ocland_clGetCommandQueueInfo,
    &ocland_clCreateBuffer,
    &ocland_clRetainMemObject,
    &ocland_clReleaseMemObject,
    &ocland_clGetSupportedImageFormats,
    &ocland_clGetMemObjectInfo,
    &ocland_clGetImageInfo,
    &ocland_clCreateSampler,
    &ocland_clRetainSampler,
    &ocland_clReleaseSampler,
    &ocland_clGetSamplerInfo,
    &ocland_clCreateProgramWithSource,
    &ocland_clCreateProgramWithBinary,
    &ocland_clRetainProgram,
    &ocland_clReleaseProgram,
    &ocland_clBuildProgram,
    &ocland_clGetProgramBuildInfo,
    &ocland_clCreateKernel,
    &ocland_clCreateKernelsInProgram,
    &ocland_clRetainKernel,
    &ocland_clReleaseKernel,
    &ocland_clSetKernelArg,
    &ocland_clGetKernelInfo,
    &ocland_clGetKernelWorkGroupInfo,
    &ocland_clWaitForEvents,
    &ocland_clGetEventInfo,
    &ocland_clRetainEvent,
    &ocland_clReleaseEvent,
    &ocland_clGetEventProfilingInfo,
    &ocland_clFlush,
    &ocland_clFinish,
    &ocland_clEnqueueReadBuffer,
    &ocland_clEnqueueWriteBuffer,
    &ocland_clEnqueueCopyBuffer,
    &ocland_clEnqueueCopyImage,
    &ocland_clEnqueueCopyImageToBuffer,
    &ocland_clEnqueueCopyBufferToImage,
    &ocland_clEnqueueNDRangeKernel,
    &ocland_clCreateSubBuffer,
    &ocland_clCreateUserEvent,
    &ocland_clSetUserEventStatus,
    &ocland_clEnqueueReadBufferRect,
    NULL, // &ocland_clEnqueueWriteBufferRect,
    NULL, // &ocland_clEnqueueCopyBufferRect,
    &ocland_clEnqueueReadImage,
    &ocland_clEnqueueWriteImage,
    NULL, // &ocland_clCreateSubDevices,
    NULL, // &ocland_clRetainDevice,
    NULL, // &ocland_clReleaseDevice,
    &ocland_clCreateImage,
    NULL, // &ocland_clCreateProgramWithBuiltInKernels,
    NULL, // &ocland_clCompileProgram,
    NULL, // &ocland_clLinkProgram,
    NULL, // &ocland_clUnloadPlatformCompiler,
    &ocland_clGetProgramInfo,
    &ocland_clGetKernelArgInfo,
    NULL, // &ocland_clEnqueueFillBuffer,
    NULL, // &ocland_clEnqueueFillImage,
    NULL, // &ocland_clEnqueueMigrateMemObjects,
    NULL, // &ocland_clEnqueueMarkerWithWaitList,
    NULL, // &ocland_clEnqueueBarrierWithWaitList
    &ocland_clCreateImage2D,
    &ocland_clCreateImage3D,
};

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
    // Test for received command
    unsigned int comm;
    // Don't use Recv here!!!
    int flag = recv(*clientfd,
                    &comm,
                    sizeof(unsigned int),
                    MSG_DONTWAIT | MSG_PEEK);
    if(flag < 0){
        return 0;
    }
    if(!flag){
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
    // Read and execute the command
    flag = Recv(clientfd,
                &comm,
                sizeof(unsigned int),
                MSG_WAITALL);
    if(flag){
        shutdown(*clientfd, 2);
        *clientfd = -1;
        return 0;
    }
    dispatchFunctions[comm] (clientfd, buffer, v);
    return 1;
}

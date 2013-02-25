/*
Copyright (c) 2012, Brice Videau <brice.videau@imag.fr>
Copyright (c) 2012, Vincent Danjean <Vincent.Danjean@ens-lyon.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ocl_icd.h>
#include <ocland/client/ocland.h>

struct _cl_icd_dispatch master_dispatch = {
  clGetPlatformIDs,
  clGetPlatformInfo,
  clGetDeviceIDs,
  clGetDeviceInfo,
  clCreateContext,
  clCreateContextFromType,
  clRetainContext,
  clReleaseContext,
  clGetContextInfo,
  clCreateCommandQueue,
  clRetainCommandQueue,
  clReleaseCommandQueue,
  clGetCommandQueueInfo,
  clSetCommandQueueProperty,
  clCreateBuffer,
  clCreateImage2D,
  clCreateImage3D,
  clRetainMemObject,
  clReleaseMemObject,
  clGetSupportedImageFormats,
  clGetMemObjectInfo,
  clGetImageInfo,
  clCreateSampler,
  clRetainSampler,
  clReleaseSampler,
  clGetSamplerInfo,
  clCreateProgramWithSource,
  clCreateProgramWithBinary,
  clRetainProgram,
  clReleaseProgram,
  clBuildProgram,
  clUnloadCompiler,
  clGetProgramInfo,
  clGetProgramBuildInfo,
  clCreateKernel,
  clCreateKernelsInProgram,
  clRetainKernel,
  clReleaseKernel,
  clSetKernelArg,
  clGetKernelInfo,
  clGetKernelWorkGroupInfo,
  clWaitForEvents,
  clGetEventInfo,
  clRetainEvent,
  clReleaseEvent,
  clGetEventProfilingInfo,
  clFlush,
  clFinish,
  clEnqueueReadBuffer,
  clEnqueueWriteBuffer,
  clEnqueueCopyBuffer,
  clEnqueueReadImage,
  clEnqueueWriteImage,
  clEnqueueCopyImage,
  clEnqueueCopyImageToBuffer,
  clEnqueueCopyBufferToImage,
  clEnqueueMapBuffer,
  clEnqueueMapImage,
  clEnqueueUnmapMemObject,
  clEnqueueNDRangeKernel,
  clEnqueueTask,
  clEnqueueNativeKernel,
  clEnqueueMarker,
  clEnqueueWaitForEvents,
  clEnqueueBarrier,
  clGetExtensionFunctionAddress,
  clCreateFromGLBuffer,
  clCreateFromGLTexture2D,
  clCreateFromGLTexture3D,
  clCreateFromGLRenderbuffer,
  clGetGLObjectInfo,
  clGetGLTextureInfo,
  clEnqueueAcquireGLObjects,
  clEnqueueReleaseGLObjects,
  clGetGLContextInfoKHR,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  clSetEventCallback,
  clCreateSubBuffer,
  clSetMemObjectDestructorCallback,
  clCreateUserEvent,
  clSetUserEventStatus,
  clEnqueueReadBufferRect,
  clEnqueueWriteBufferRect,
  clEnqueueCopyBufferRect,
  clCreateSubDevicesEXT,
  clRetainDeviceEXT,
  clReleaseDeviceEXT,
  (void *) NULL,
  clCreateSubDevices,
  clRetainDevice,
  clReleaseDevice,
  clCreateImage,
  clCreateProgramWithBuiltInKernels,
  clCompileProgram,
  clLinkProgram,
  clUnloadPlatformCompiler,
  clGetKernelArgInfo,
  clEnqueueFillBuffer,
  clEnqueueFillImage,
  clEnqueueMigrateMemObjects,
  clEnqueueMarkerWithWaitList,
  clEnqueueBarrierWithWaitList,
  clGetExtensionFunctionAddressForPlatform,
  clCreateFromGLTexture,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL
};

CL_API_ENTRY cl_int CL_API_CALL clIcdGetPlatformIDsKHR(
             cl_uint num_entries,
             cl_platform_id *platforms,
             cl_uint *num_platforms) {
    if( platforms == NULL && num_platforms == NULL )
        return CL_INVALID_VALUE;
    if( num_entries == 0 && platforms != NULL )
        return CL_INVALID_VALUE;
    cl_uint ocland_num_platforms = 0;
    cl_int flag = oclandGetPlatformIDs(0,NULL,&ocland_num_platforms);
    if( (flag != CL_SUCCESS) || (!ocland_num_platforms)){
        if(num_platforms)
            *num_platforms = 0;
        return CL_PLATFORM_NOT_FOUND_KHR;
    }
    if(num_platforms)
        *num_platforms = ocland_num_platforms;
    if(!platforms)
        return CL_SUCCESS;
    flag = oclandGetPlatformIDs(num_entries,platforms,NULL);
    if(flag != CL_SUCCESS){
        if(num_platforms)
            *num_platforms = 0;
        return CL_PLATFORM_NOT_FOUND_KHR;
    }
    return CL_SUCCESS;
}

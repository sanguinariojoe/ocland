/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2015  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with ocland.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMANDS_ENUM_H_INCLUDED
#define COMMANDS_ENUM_H_INCLUDED

/** @brief Identifiers for the commands.
 *
 * This identificators are used to can say the server which command should
 * launch.
 */
enum {
    ocland_clGetPlatformIDs,
    ocland_clGetPlatformInfo,
    ocland_clGetDeviceIDs,
    ocland_clGetDeviceInfo,
    ocland_clCreateContext,
    ocland_clCreateContextFromType,
    ocland_clRetainContext,
    ocland_clReleaseContext,
    ocland_clGetContextInfo,
    ocland_clCreateCommandQueue,
    ocland_clRetainCommandQueue,
    ocland_clReleaseCommandQueue,
    ocland_clGetCommandQueueInfo,
    ocland_clCreateBuffer,
    ocland_clRetainMemObject,
    ocland_clReleaseMemObject,
    ocland_clGetSupportedImageFormats,
    ocland_clGetMemObjectInfo,
    ocland_clGetImageInfo,
    ocland_clCreateSampler,
    ocland_clRetainSampler,
    ocland_clReleaseSampler,
    ocland_clGetSamplerInfo,
    ocland_clCreateProgramWithSource,
    ocland_clCreateProgramWithBinary,
    ocland_clRetainProgram,
    ocland_clReleaseProgram,
    ocland_clBuildProgram,
    ocland_clGetProgramBuildInfo,
    ocland_clCreateKernel,
    ocland_clCreateKernelsInProgram,
    ocland_clRetainKernel,
    ocland_clReleaseKernel,
    ocland_clSetKernelArg,
    ocland_clGetKernelInfo,
    ocland_clGetKernelWorkGroupInfo,
    ocland_clWaitForEvents,
    ocland_clGetEventInfo,
    ocland_clRetainEvent,
    ocland_clReleaseEvent,
    ocland_clGetEventProfilingInfo,
    ocland_clFlush,
    ocland_clFinish,
    ocland_clEnqueueReadBuffer,
    ocland_clEnqueueWriteBuffer,
    ocland_clEnqueueCopyBuffer,
    ocland_clEnqueueCopyImage,
    ocland_clEnqueueCopyImageToBuffer,
    ocland_clEnqueueCopyBufferToImage,
    ocland_clEnqueueNDRangeKernel,
    ocland_clCreateSubBuffer,
    ocland_clCreateUserEvent,
    ocland_clSetUserEventStatus,
    ocland_clEnqueueReadBufferRect,
    ocland_clEnqueueWriteBufferRect,
    ocland_clEnqueueCopyBufferRect,
    ocland_clEnqueueReadImage,
    ocland_clEnqueueWriteImage,
    ocland_clCreateSubDevices,
    ocland_clRetainDevice,
    ocland_clReleaseDevice,
    ocland_clCreateImage,
    ocland_clCreateProgramWithBuiltInKernels,
    ocland_clCompileProgram,
    ocland_clLinkProgram,
    ocland_clUnloadPlatformCompiler,
    ocland_clGetProgramInfo,
    ocland_clGetKernelArgInfo,
    ocland_clEnqueueFillBuffer,
    ocland_clEnqueueFillImage,
    ocland_clEnqueueMigrateMemObjects,
    ocland_clEnqueueMarkerWithWaitList,
    ocland_clEnqueueBarrierWithWaitList,
    ocland_clCreateImage2D,
    ocland_clCreateImage3D
};

#endif // COMMANDS_ENUM_H_INCLUDED

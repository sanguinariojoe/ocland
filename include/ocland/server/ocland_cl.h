/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2015  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ocland.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ocland/server/validator.h>
#include <ocland/server/ocland_event.h>
#include <ocland/server/ocland_mem.h>
#include <ocland/server/ocland_version.h>

#ifndef OCLAND_CL_H_INCLUDED
#define OCLAND_CL_H_INCLUDED

/** @brief clGetPlatformIDs ocland abstraction. In ocland server
 * only num_entries will be requested, and assumed that if
 * num_entries > 0 platforms array is requested, so must
 * be the client who analyze arguments looking for mistakes.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if errors detected.
 */
int ocland_clGetPlatformIDs(int* clientfd, validator v);

/** @brief clGetDeviceIDs ocland abstraction. In ocland server
 * platform_id, device_type and num_entries will be requested. \n
 * If num_entries > 0 server will assume that devices array
 * is requested, so must be the client who analyze arguments
 * looking for mistakes.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetPlatformInfo(int* clientfd, validator v);

/** @brief clGetDeviceIDs ocland abstraction. In ocland server
 * platform_id, device_type and num_entries will be requested. \n
 * If num_entries > 0 server will assume that devices array
 * is requested, so must be the client who analyze arguments
 * looking for mistakes.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetDeviceIDs(int* clientfd, validator v);

/** @brief clGetDeviceInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetDeviceInfo(int* clientfd, validator v);

/** @brief clCreateContext ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateContext(int* clientfd, validator v);

/** @brief clCreateContextFromType ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateContextFromType(int* clientfd, validator v);

/** @brief clRetainContext ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainContext(int* clientfd, validator v);

/** @brief clReleaseContext ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseContext(int* clientfd, validator v);

/** @brief clGetContextInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetContextInfo(int* clientfd, validator v);

/** @brief clCreateCommandQueue ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateCommandQueue(int* clientfd, validator v);

/** @brief clRetainCommandQueue ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainCommandQueue(int* clientfd, validator v);

/** @brief clReleaseCommandQueue ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseCommandQueue(int* clientfd, validator v);

/** @brief clGetCommandQueueInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetCommandQueueInfo(int* clientfd, validator v);

/** @brief clCreateBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateBuffer(int* clientfd, validator v);

/** @brief clRetainMemObject ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainMemObject(int* clientfd, validator v);

/** @brief clReleaseMemObject ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseMemObject(int* clientfd, validator v);

/** @brief clGetSupportedImageFormats ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetSupportedImageFormats(int* clientfd, validator v);

/** @brief clGetMemObjectInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetMemObjectInfo(int* clientfd, validator v);

/** @brief clGetImageInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetImageInfo(int* clientfd, validator v);

/** @brief clCreateSampler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateSampler(int* clientfd, validator v);

/** @brief clRetainSampler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainSampler(int* clientfd, validator v);

/** @brief clReleaseSampler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseSampler(int* clientfd, validator v);

/** @brief clGetSamplerInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetSamplerInfo(int* clientfd, validator v);

/** @brief clCreateProgramWithSource ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateProgramWithSource(int* clientfd, validator v);

/** @brief clCreateProgramWithBinary ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateProgramWithBinary(int* clientfd, validator v);

/** @brief clRetainProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainProgram(int* clientfd, validator v);

/** @brief clReleaseProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseProgram(int* clientfd, validator v);

/** @brief clBuildProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clBuildProgram(int* clientfd, validator v);

/** @brief clGetProgramInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetProgramInfo(int* clientfd, validator v);

/** @brief clGetProgramBuildInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetProgramBuildInfo(int* clientfd, validator v);

/** @brief clCreateKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateKernel(int* clientfd, validator v);

/** @brief clCreateKernelsInProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateKernelsInProgram(int* clientfd, validator v);

/** @brief clRetainKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainKernel(int* clientfd, validator v);

/** @brief clReleaseKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseKernel(int* clientfd, validator v);

/** @brief clSetKernelArg ocland abstraction. It is the most dangerous
 * method at server side because we can't warranty that, in case
 * of cl_mem or cl_sampler arguments, passed pointers are right,
 * due to we can't test the parameter type (not in OpenCL < 1.2
 * at least). So Segmentation Faults can be expected from bad clients.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clSetKernelArg(int* clientfd, validator v);

/** @brief clGetKernelInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetKernelInfo(int* clientfd, validator v);

/** @brief clGetKernelWorkGroupInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetKernelWorkGroupInfo(int* clientfd, validator v);

/** @brief clWaitForEvents ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clWaitForEvents(int* clientfd, validator v);

/** @brief clGetEventInfo ocland abstraction.
 *
 * This is a little bit dangerous
 * method due to if info is requested before event has been generated,
 * i.e.- ocland is still performing work before calling OpenCL method,
 * CL_INVALID_EVENT will be returned.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetEventInfo(int* clientfd, validator v);

/** @brief clRetainEvent ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainEvent(int* clientfd, validator v);

/** @brief clReleaseEvent ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseEvent(int* clientfd, validator v);

/** @brief clGetEventProfilingInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetEventProfilingInfo(int* clientfd, validator v);

/** @brief clFlush ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clFlush(int* clientfd, validator v);

/** @brief clFinish ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clFinish(int* clientfd, validator v);

/** @brief clEnqueueReadBuffer ocland abstraction. Since this method
 * implies huge memory transfer, and can be done asynchronously,
 * the transfer of memory must be done in a parallel thread, in
 * order to don't block the execution, and also memory transfer
 * must be done in a new socket on different port in order to
 * avoid interferences with following commands transfered by
 * network.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueReadBuffer(int* clientfd, validator v);

/** @brief clEnqueueWriteBuffer ocland abstraction. Since this method
 * implies huge memory transfer, and can be done asynchronously,
 * the transfer of memory must be done in a parallel thread, in
 * order to don't block the execution, and also memory transfer
 * must be done in a new socket on different port in order to
 * avoid interferences with following commands transfered by
 * network.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueWriteBuffer(int* clientfd, validator v);

/** @brief clEnqueueCopyBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyBuffer(int* clientfd, validator v);

/** @brief clEnqueueCopyImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyImage(int* clientfd, validator v);

/** @brief clEnqueueCopyImageToBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyImageToBuffer(int* clientfd, validator v);

/** @brief clEnqueueCopyBufferToImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyBufferToImage(int* clientfd, validator v);

/** @brief clEnqueueNDRangeKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueNDRangeKernel(int* clientfd, validator v);

/** @brief clEnqueueReadImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueReadImage(int* clientfd, validator v);

/** @brief clEnqueueWriteImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueWriteImage(int* clientfd, validator v);

/** @brief clCreateImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateImage2D(int* clientfd, validator v);

/** @brief clCreateImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateImage3D(int* clientfd, validator v);

// ----------------------------------
// OpenCL 1.1
// ----------------------------------
/** @brief clCreateSubBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateSubBuffer(int* clientfd, validator v);

/** @brief clCreateUserEvent ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateUserEvent(int* clientfd, validator v);

/** @brief clSetUserEventStatus ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clSetUserEventStatus(int* clientfd, validator v);

/** @brief clEnqueueReadBufferRect ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueReadBufferRect(int* clientfd, validator v);

/** @brief clEnqueueWriteBufferRect ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueWriteBufferRect(int* clientfd, validator v);

/** @brief clEnqueueCopyBufferRect ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyBufferRect(int* clientfd, validator v);

// ----------------------------------
// OpenCL 1.2
// ----------------------------------
/** @brief clCreateSubDevices ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateSubDevices(int* clientfd, validator v);

/** @brief clRetainDevice ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainDevice(int* clientfd, validator v);

/** @brief clReleaseDevice ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseDevice(int* clientfd, validator v);

/** @brief clCreateImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateImage(int* clientfd, validator v);

/** @brief clCreateProgramWithBuiltInKernels ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateProgramWithBuiltInKernels(int* clientfd, validator v);

/** @brief clCompileProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCompileProgram(int* clientfd, validator v);

/** @brief clLinkProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clLinkProgram(int* clientfd, validator v);

/** @brief clUnloadPlatformCompiler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clUnloadPlatformCompiler(int* clientfd, validator v);

/** @brief clGetKernelArgInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetKernelArgInfo(int* clientfd, validator v);

/** @brief clEnqueueFillBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueFillBuffer(int* clientfd, validator v);

/** @brief clEnqueueFillImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueFillImage(int* clientfd, validator v);

/** @brief clEnqueueMigrateMemObjects ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueMigrateMemObjects(int* clientfd, validator v);

/** @brief clEnqueueMarkerWithWaitList ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueMarkerWithWaitList(int* clientfd, validator v);

/** @brief clEnqueueBarrierWithWaitList ocland abstraction.
 * @param clientfd Client connection socket.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueBarrierWithWaitList(int* clientfd, validator v);

#endif // OCLAND_CL_H_INCLUDED

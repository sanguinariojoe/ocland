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

#include <ocland/server/validator.h>
#include <ocland/server/ocland_event.h>
#include <ocland/server/ocland_mem.h>
#include <ocland/server/ocland_version.h>

#ifndef OCLAND_CL_H_INCLUDED
#define OCLAND_CL_H_INCLUDED

/** clGetPlatformIDs ocland abstraction. In ocland server
 * only num_entries will be requested, and assumed that if
 * num_entries > 0 platforms array is requested, so must
 * be the client who analyze arguments looking for mistakes.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if errors detected.
 */
int ocland_clGetPlatformIDs(int* clientfd, char* buffer, validator v, void* data);

/** clGetDeviceIDs ocland abstraction. In ocland server
 * platform_id, device_type and num_entries will be requested. \n
 * If num_entries > 0 server will assume that devices array
 * is requested, so must be the client who analyze arguments
 * looking for mistakes.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetPlatformInfo(int* clientfd, char* buffer, validator v, void* data);

/** clGetDeviceIDs ocland abstraction. In ocland server
 * platform_id, device_type and num_entries will be requested. \n
 * If num_entries > 0 server will assume that devices array
 * is requested, so must be the client who analyze arguments
 * looking for mistakes.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetDeviceIDs(int* clientfd, char* buffer, validator v, void* data);

/** clGetDeviceInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetDeviceInfo(int* clientfd, char* buffer, validator v, void* data);

/** clCreateContext ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateContext(int* clientfd, char* buffer, validator v, void* data);

/** clCreateContextFromType ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateContextFromType(int* clientfd, char* buffer, validator v, void* data);

/** clRetainContext ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainContext(int* clientfd, char* buffer, validator v, void* data);

/** clReleaseContext ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseContext(int* clientfd, char* buffer, validator v, void* data);

/** clGetContextInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetContextInfo(int* clientfd, char* buffer, validator v, void* data);

/** clCreateCommandQueue ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateCommandQueue(int* clientfd, char* buffer, validator v, void* data);

/** clRetainCommandQueue ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainCommandQueue(int* clientfd, char* buffer, validator v, void* data);

/** clReleaseCommandQueue ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseCommandQueue(int* clientfd, char* buffer, validator v, void* data);

/** clGetCommandQueueInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetCommandQueueInfo(int* clientfd, char* buffer, validator v, void* data);

/** clCreateBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateBuffer(int* clientfd, char* buffer, validator v, void* data);

/** clRetainMemObject ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainMemObject(int* clientfd, char* buffer, validator v, void* data);

/** clReleaseMemObject ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseMemObject(int* clientfd, char* buffer, validator v, void* data);

/** clGetSupportedImageFormats ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetSupportedImageFormats(int* clientfd, char* buffer, validator v, void* data);

/** clGetMemObjectInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetMemObjectInfo(int* clientfd, char* buffer, validator v, void* data);

/** clGetImageInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetImageInfo(int* clientfd, char* buffer, validator v, void* data);

/** clCreateSampler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateSampler(int* clientfd, char* buffer, validator v, void* data);

/** clRetainSampler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainSampler(int* clientfd, char* buffer, validator v, void* data);

/** clReleaseSampler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseSampler(int* clientfd, char* buffer, validator v, void* data);

/** clGetSamplerInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetSamplerInfo(int* clientfd, char* buffer, validator v, void* data);

/** clCreateProgramWithSource ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateProgramWithSource(int* clientfd, char* buffer, validator v, void* data);

/** clCreateProgramWithBinary ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateProgramWithBinary(int* clientfd, char* buffer, validator v, void* data);

/** clRetainProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainProgram(int* clientfd, char* buffer, validator v, void* data);

/** clReleaseProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseProgram(int* clientfd, char* buffer, validator v, void* data);

/** clBuildProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clBuildProgram(int* clientfd, char* buffer, validator v, void* data);

/** clGetProgramInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetProgramInfo(int* clientfd, char* buffer, validator v, void* data);

/** clGetProgramBuildInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetProgramBuildInfo(int* clientfd, char* buffer, validator v, void* data);

/** clCreateKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateKernel(int* clientfd, char* buffer, validator v, void* data);

/** clCreateKernelsInProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateKernelsInProgram(int* clientfd, char* buffer, validator v, void* data);

/** clRetainKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainKernel(int* clientfd, char* buffer, validator v, void* data);

/** clReleaseKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseKernel(int* clientfd, char* buffer, validator v, void* data);

/** clSetKernelArg ocland abstraction. It is the most dangerous
 * method at server side because we can't warranty that, in case
 * of cl_mem or cl_sampler arguments, passed pointers are right,
 * due to we can't test the parameter type (not in OpenCL < 1.2
 * at least). So Segmentation Faults can be expected from bad clients.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clSetKernelArg(int* clientfd, char* buffer, validator v, void* data);

/** clGetKernelInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetKernelInfo(int* clientfd, char* buffer, validator v, void* data);

/** clGetKernelWorkGroupInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetKernelWorkGroupInfo(int* clientfd, char* buffer, validator v, void* data);

/** clWaitForEvents ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clWaitForEvents(int* clientfd, char* buffer, validator v, void* data);

/** clGetEventInfo ocland abstraction. This is a little bit dangerous
 * method due to if info is requested before event has been generated,
 * i.e.- ocland is still performing work before calling OpenCL method,
 * CL_INVALID_EVENT will be returned.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetEventInfo(int* clientfd, char* buffer, validator v, void* data);

/** clRetainEvent ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainEvent(int* clientfd, char* buffer, validator v, void* data);

/** clReleaseEvent ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseEvent(int* clientfd, char* buffer, validator v, void* data);

/** clGetEventProfilingInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetEventProfilingInfo(int* clientfd, char* buffer, validator v, void* data);

/** clFlush ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clFlush(int* clientfd, char* buffer, validator v, void* data);

/** clFinish ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clFinish(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueReadBuffer ocland abstraction. Since this method
 * implies huge memory transfer, and can be done asynchronously,
 * the transfer of memory must be done in a parallel thread, in
 * order to don't block the execution, and also memory transfer
 * must be done in a new socket on different port in order to
 * avoid interferences with following commands transfered by
 * network.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueReadBuffer(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueWriteBuffer ocland abstraction. Since this method
 * implies huge memory transfer, and can be done asynchronously,
 * the transfer of memory must be done in a parallel thread, in
 * order to don't block the execution, and also memory transfer
 * must be done in a new socket on different port in order to
 * avoid interferences with following commands transfered by
 * network.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param data Data received by the client.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueWriteBuffer(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueCopyBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyBuffer(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueCopyImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyImage(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueCopyImageToBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyImageToBuffer(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueCopyBufferToImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyBufferToImage(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueNDRangeKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueNDRangeKernel(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueReadImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueReadImage(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueWriteImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueWriteImage(int* clientfd, char* buffer, validator v, void* data);

// ----------------------------------
// OpenCL 1.1
// ----------------------------------
/** clCreateSubBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateSubBuffer(int* clientfd, char* buffer, validator v, void* data);

/** clCreateUserEvent ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateUserEvent(int* clientfd, char* buffer, validator v);

/** clSetUserEventStatus ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clSetUserEventStatus(int* clientfd, char* buffer, validator v);

/** clEnqueueReadBufferRect ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueReadBufferRect(int* clientfd, char* buffer, validator v);

/** clEnqueueWriteBufferRect ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueWriteBufferRect(int* clientfd, char* buffer, validator v);

/** clEnqueueCopyBufferRect ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueCopyBufferRect(int* clientfd, char* buffer, validator v);

// ----------------------------------
// OpenCL 1.2
// ----------------------------------
/** clCreateSubDevices ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateSubDevices(int* clientfd, char* buffer, validator v);

/** clRetainDevice ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainDevice(int* clientfd, char* buffer, validator v);

/** clReleaseDevice ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseDevice(int* clientfd, char* buffer, validator v);

/** clCreateImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateImage(int* clientfd, char* buffer, validator v);

/** clCreateProgramWithBuiltInKernels ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateProgramWithBuiltInKernels(int* clientfd, char* buffer, validator v);

/** clCompileProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCompileProgram(int* clientfd, char* buffer, validator v);

/** clLinkProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clLinkProgram(int* clientfd, char* buffer, validator v);

/** clUnloadPlatformCompiler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clUnloadPlatformCompiler(int* clientfd, char* buffer, validator v);

/** clGetKernelArgInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @param data Data received by the client.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetKernelArgInfo(int* clientfd, char* buffer, validator v, void* data);

/** clEnqueueFillBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueFillBuffer(int* clientfd, char* buffer, validator v);

/** clEnqueueFillImage ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueFillImage(int* clientfd, char* buffer, validator v);

/** clEnqueueMigrateMemObjects ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueMigrateMemObjects(int* clientfd, char* buffer, validator v);

/** clEnqueueMarkerWithWaitList ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueMarkerWithWaitList(int* clientfd, char* buffer, validator v);

/** clEnqueueBarrierWithWaitList ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clEnqueueBarrierWithWaitList(int* clientfd, char* buffer, validator v);

#endif // OCLAND_CL_H_INCLUDED

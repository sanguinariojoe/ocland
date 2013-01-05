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

#include <ocland/server/validator.h>

#ifndef OCLAND_CL_H_INCLUDED
#define OCLAND_CL_H_INCLUDED

/** clGetPlatformIDs ocland abstraction. In ocland server
 * only num_entries will be requested, and assumed that if
 * num_entries > 0 platforms array is requested, so must
 * be the client who analyze arguments looking for mistakes.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if errors detected.
 */
int ocland_clGetPlatformIDs(int* clientfd, char* buffer, validator v);

/** clGetDeviceIDs ocland abstraction. In ocland server
 * platform_id, device_type and num_entries will be requested. \n
 * If num_entries > 0 server will assume that devices array
 * is requested, so must be the client who analyze arguments
 * looking for mistakes.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetPlatformInfo(int* clientfd, char* buffer, validator v);

/** clGetDeviceIDs ocland abstraction. In ocland server
 * platform_id, device_type and num_entries will be requested. \n
 * If num_entries > 0 server will assume that devices array
 * is requested, so must be the client who analyze arguments
 * looking for mistakes.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetDeviceIDs(int* clientfd, char* buffer, validator v);

/** clGetDeviceInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetDeviceInfo(int* clientfd, char* buffer, validator v);

/** clCreateContext ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateContext(int* clientfd, char* buffer, validator v);

/** clCreateContextFromType ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateContextFromType(int* clientfd, char* buffer, validator v);

/** clRetainContext ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainContext(int* clientfd, char* buffer, validator v);

/** clReleaseContext ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseContext(int* clientfd, char* buffer, validator v);

/** clGetContextInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetContextInfo(int* clientfd, char* buffer, validator v);

/** clCreateCommandQueue ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateCommandQueue(int* clientfd, char* buffer, validator v);

/** clRetainCommandQueue ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainCommandQueue(int* clientfd, char* buffer, validator v);

/** clReleaseCommandQueue ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseCommandQueue(int* clientfd, char* buffer, validator v);

/** clGetCommandQueueInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetCommandQueueInfo(int* clientfd, char* buffer, validator v);

/** clCreateBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateBuffer(int* clientfd, char* buffer, validator v);

/** clRetainMemObject ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainMemObject(int* clientfd, char* buffer, validator v);

/** clReleaseMemObject ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseMemObject(int* clientfd, char* buffer, validator v);

/** clGetSupportedImageFormats ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetSupportedImageFormats(int* clientfd, char* buffer, validator v);

/** clGetMemObjectInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetMemObjectInfo(int* clientfd, char* buffer, validator v);

/** clGetImageInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetImageInfo(int* clientfd, char* buffer, validator v);

/** clCreateSampler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateSampler(int* clientfd, char* buffer, validator v);

/** clRetainSampler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainSampler(int* clientfd, char* buffer, validator v);

/** clReleaseSampler ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseSampler(int* clientfd, char* buffer, validator v);

/** clGetSamplerInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetSamplerInfo(int* clientfd, char* buffer, validator v);

/** clCreateProgramWithSource ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateProgramWithSource(int* clientfd, char* buffer, validator v);

/** clCreateProgramWithBinary ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateProgramWithBinary(int* clientfd, char* buffer, validator v);

/** clRetainProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainProgram(int* clientfd, char* buffer, validator v);

/** clReleaseProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseProgram(int* clientfd, char* buffer, validator v);

/** clBuildProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clBuildProgram(int* clientfd, char* buffer, validator v);

/** clGetProgramInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetProgramInfo(int* clientfd, char* buffer, validator v);

/** clGetProgramBuildInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetProgramBuildInfo(int* clientfd, char* buffer, validator v);

/** clCreateKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateKernel(int* clientfd, char* buffer, validator v);

/** clCreateKernelsInProgram ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateKernelsInProgram(int* clientfd, char* buffer, validator v);

/** clRetainKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainKernel(int* clientfd, char* buffer, validator v);

/** clReleaseKernel ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseKernel(int* clientfd, char* buffer, validator v);

/** clSetKernelArg ocland abstraction. It is the most dangerous
 * method at server side because we can't warranty that, in case
 * of cl_mem or cl_sampler arguments, passed pointers are right,
 * due to we can't test the parameter type (not in OpenCL < 1.2
 * at least). So Segmentation Faults can be expected from bad clients.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clSetKernelArg(int* clientfd, char* buffer, validator v);

/** clSetKernelArg ocland abstraction, used when arg_value is Null.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clSetKernelNullArg(int* clientfd, char* buffer, validator v);

/** clGetKernelInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetKernelInfo(int* clientfd, char* buffer, validator v);

/** clGetKernelWorkGroupInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetKernelWorkGroupInfo(int* clientfd, char* buffer, validator v);

/** clWaitForEvents ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clWaitForEvents(int* clientfd, char* buffer, validator v);

/** clGetEventInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetEventInfo(int* clientfd, char* buffer, validator v);

/** clRetainEvent ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clRetainEvent(int* clientfd, char* buffer, validator v);

/** clReleaseEvent ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clReleaseEvent(int* clientfd, char* buffer, validator v);

/** clGetEventProfilingInfo ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetEventProfilingInfo(int* clientfd, char* buffer, validator v);

/** clFlush ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clFlush(int* clientfd, char* buffer, validator v);

/** clFinish ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clFinish(int* clientfd, char* buffer, validator v);

#ifdef CL_API_SUFFIX__VERSION_1_1
/** clCreateSubBuffer ocland abstraction.
 * @param clientfd Client connection socket.
 * @param buffer Buffer to exchange data.
 * @param v Validator.
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clCreateSubBuffer(int* clientfd, char* buffer, validator v);

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
#endif

#ifdef CL_API_SUFFIX__VERSION_1_2
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
 * @return 0 if message can't be dispatched, 1 otherwise.
 */
int ocland_clGetKernelArgInfo(int* clientfd, char* buffer, validator v);
#endif

#endif // OCLAND_CL_H_INCLUDED

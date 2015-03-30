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

/** @file
 * @brief ICD cl_program implementation
 *
 * cl_program is a typedef definition:
 *
 *     typedef struct _cl_program* cl_program
 *
 * In this file such data structure, and all the associated methods, are
 * declared.
 * @see program.c
 */

#ifndef PROGRAM_H_INCLUDED
#define PROGRAM_H_INCLUDED

#include <ocl_icd.h>
#include <ocland/client/platform_id.h>
#include <ocland/client/context.h>

/** @brief ICD program identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_program
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_program ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server where this object is allocated
    oclandServer server;
    /// Associated context
    cl_context context;
    /// Number of devices
    cl_uint num_devices;
    /// Devices
    cl_device_id *devices;
    /// Source code
    char* source;
    /// Binary sizes
    size_t *binary_lengths;
    /// Binaries
    unsigned char** binaries;
    /// Number of available kernels
    size_t num_kernels;
    /// Names of the kernels
    char *kernels;
};

/** @brief Check for program validity
 * @param program Program to check
 * @return 1 if the program is a known one, 0 otherwise.
 */
int hasProgram(cl_program program);

/** @brief Get a program from the server instance pointer.
 * @param srv_program Server program instance
 * @return ICD program instance, NULL if \a srv_program cannot be found.
 */
cl_program programFromServer(cl_program srv_program);

/** @brief clGetSamplerInfo ocland abstraction method.
 */
cl_program createProgramWithSource(cl_context         context ,
                                   cl_uint            count ,
                                   const char **      strings ,
                                   const size_t *     lengths ,
                                   cl_int *           errcode_ret);

/** @brief clCreateProgramWithBinary ocland abstraction method.
 */
cl_program createProgramWithBinary(cl_context                      context ,
                                   cl_uint                         num_devices ,
                                   const cl_device_id *            device_list ,
                                   const size_t *                  lengths ,
                                   const unsigned char **          binaries ,
                                   cl_int *                        binary_status ,
                                   cl_int *                        errcode_ret);

/** @brief clCreateProgramWithBuiltInKernels ocland abstraction method.
 */
cl_program createProgramWithBuiltInKernels(cl_context             context ,
                                           cl_uint                num_devices ,
                                           const cl_device_id *   device_list ,
                                           const char *           kernel_names ,
                                           cl_int *               errcode_ret);

/** @brief clRetainProgram ocland abstraction method.
 */
cl_int retainProgram(cl_program  program);

/** @brief clReleaseProgram ocland abstraction method.
 */
cl_int releaseProgram(cl_program  program);

/** @brief clBuildProgram ocland abstraction method.
 */
cl_int buildProgram(cl_program            program ,
                    cl_uint               num_devices ,
                    const cl_device_id *  device_list ,
                    const char *          options ,
                    void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                    void *                user_data);

/** @brief clGetProgramInfo ocland abstraction method.
 */
cl_int getProgramInfo(cl_program          program ,
                      cl_program_info     param_name ,
                      size_t              param_value_size ,
                      void *              param_value ,
                      size_t *            param_value_size_ret);

/** @brief clGetProgramBuildInfo ocland abstraction method.
 */
cl_int getProgramBuildInfo(cl_program             program ,
                           cl_device_id           device ,
                           cl_program_build_info  param_name ,
                           size_t                 param_value_size ,
                           void *                 param_value ,
                           size_t *               param_value_size_ret);

/** @brief clCompileProgram ocland abstraction method.
 */
cl_int compileProgram(cl_program            program ,
                      cl_uint               num_devices ,
                      const cl_device_id *  device_list ,
                      const char *          options ,
                      cl_uint               num_input_headers ,
                      const cl_program *    input_headers,
                      const char **         header_include_names ,
                      void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                      void *                user_data);

/** @brief clLinkProgram ocland abstraction method.
 */
cl_program linkProgram(cl_context            context ,
                       cl_uint               num_devices ,
                       const cl_device_id *  device_list ,
                       const char *          options ,
                       cl_uint               num_input_programs ,
                       const cl_program *    input_programs ,
                       void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                       void *                user_data ,
                       cl_int *              errcode_ret);

#endif // PROGRAM_H_INCLUDED

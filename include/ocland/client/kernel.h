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

/** @file
 * @brief ICD cl_kernel implementation
 *
 * cl_kernel is a typedef definition:
 *
 *     typedef struct _cl_kernel* cl_kernel
 *
 * In this file such data structure, and all the associated methods, are
 * declared.
 * @see kernel.c
 */

#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

#include <ocl_icd.h>
#include <ocland/client/platform_id.h>
#include <ocland/client/program.h>

/** @brief Helper object for the kernel arguments.
 * @note It is not an OpenCL standard.
 */
struct _cl_kernel_arg
{
    /// Argument index
    cl_uint index;
    /// Argument address
    cl_kernel_arg_address_qualifier address;
    /// Argument access qualifier
    cl_kernel_arg_access_qualifier access;
    /// Argument access qualifier availability
    cl_bool access_available;
    /// Argument type name
    char* type_name;
    /// Argument type name availability
    cl_bool type_name_available;
    /// Argument type qualifier
    cl_kernel_arg_type_qualifier type;
    /// Argument type qualifier availability
    cl_bool type_available;
    /// Argument name
    char* name;
    /// Argument name availability
    cl_bool name_available;
    /// Last set argument size (0 if it has not been set yet)
    size_t bytes;
    /// Last argument value (NULL if it has not been set yet)
    void* value;
    /// Has the argument already been set??
    cl_bool is_set;
};
typedef struct _cl_kernel_arg *cl_kernel_arg;

/** @brief ICD kernel identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_kernel
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_kernel ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server where this object is allocated
    oclandServer server;
    /// Associated context
    cl_context context;
    /// Associated program
    cl_program program;
    /// Function name
    char* func_name;
    /// Number of input arguments
    cl_uint num_args;
    /// Arguments
    cl_kernel_arg *args;
    /// Function name
    char* attributes;
};

/** @brief Check for kernel validity
 * @param kernel Kernel to check
 * @return 1 if the kernel is a known one, 0 otherwise.
 */
int hasKernel(cl_kernel kernel);

/** @brief Get a kernel from the server instance pointer.
 * @param srv_kernel Server kernel instance
 * @return ICD kernel instance, NULL if \a srv_kernel cannot be found.
 */
cl_kernel kernelFromServer(cl_kernel srv_kernel);

/** @brief clCreateKernel ocland abstraction method.
 */
cl_kernel createKernel(cl_program       program ,
                       const char *     kernel_name ,
                       cl_int *         errcode_ret);

/** @brief clCreateKernelsInProgram ocland abstraction method.
 */
cl_int createKernelsInProgram(cl_program      program ,
                              cl_uint         num_kernels ,
                              cl_kernel *     kernels ,
                              cl_uint *       num_kernels_ret);

/** @brief clRetainKernel ocland abstraction method.
 */
cl_int retainKernel(cl_kernel     kernel);

/** @brief clReleaseKernel ocland abstraction method.
 */
cl_int releaseKernel(cl_kernel    kernel);

/** @brief clSetKernelArg ocland abstraction method.
 */
cl_int setKernelArg(cl_kernel     kernel ,
                    cl_uint       arg_index ,
                    size_t        arg_size ,
                    const void *  arg_value);

/** @brief clGetKernelInfo ocland abstraction method.
 */
cl_int getKernelInfo(cl_kernel        kernel ,
                     cl_kernel_info   param_name ,
                     size_t           param_value_size ,
                     void *           param_value ,
                     size_t *         param_value_size_ret);

/** @brief clGetKernelWorkGroupInfo ocland abstraction method.
 */
cl_int getKernelWorkGroupInfo(cl_kernel                   kernel ,
                              cl_device_id                device ,
                              cl_kernel_work_group_info   param_name ,
                              size_t                      param_value_size ,
                              void *                      param_value ,
                              size_t *                    param_value_size_ret);

/** @brief clUnloadPlatformCompiler ocland abstraction method.
 */
cl_int getKernelArgInfo(cl_kernel        kernel ,
                        cl_uint          arg_index ,
                        cl_kernel_arg_info   param_name ,
                        size_t           param_value_size ,
                        void *           param_value ,
                        size_t *         param_value_size_ret);

#endif // KERNEL_H_INCLUDED

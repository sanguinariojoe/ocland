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
 * @brief ICD cl_kernel implementation
 * @see kernel.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/client/commands_enum.h>
#include <ocland/common/verbose.h>
#include <ocland/client/kernel.h>
#include <ocland/common/dataPack.h>
#include <ocland/common/dataExchange.h>

/// Number of known kernels
cl_uint num_global_kernels = 0;
/// List of known kernels
cl_kernel *global_kernels = NULL;

int hasKernel(cl_kernel kernel){
    cl_uint i;
    for(i = 0; i < num_global_kernels; i++){
        if(kernel == global_kernels[i])
            return 1;
    }
    return 0;
}

/** @brief Add a set of kernels to the global list global_kernels.
 *
 * This method is not checking if the objects are already present in the list,
 * however, accidentally adding the same object several times will only imply
 * performance penalties.
 * @param num_kernels Number of objects to append.
 * @param kernels Objects to append.
 * @return CL_SUCCESS if the objects are already generated, an error code
 * otherwise.
 */
cl_int addKernels(cl_uint     num_kernels,
                  cl_kernel*  kernels)
{
    if(!num_kernels)
        return CL_SUCCESS;

    // Add the kernels to the global list
    cl_kernel *backup_kernels = global_kernels;

    global_kernels = (cl_kernel*)malloc(
        (num_global_kernels + num_kernels) * sizeof(cl_kernel));
    if(!global_kernels){
        VERBOSE("Failure allocating memory for %u kernels!\n",
                num_global_kernels + num_kernels);
        free(backup_kernels); backup_kernels = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_kernels){
        memcpy(global_kernels,
               backup_kernels,
               num_global_kernels * sizeof(cl_kernel));
        free(backup_kernels); backup_kernels = NULL;
    }

    memcpy(&(global_kernels[num_global_kernels]),
           kernels,
           num_kernels * sizeof(cl_kernel));
    num_global_kernels += num_kernels;

    return CL_SUCCESS;
}

/** @brief Get the kernel index in the global list
 * @param kernel Object to look for
 * @return Index of the object, num_global_kernels if it is not found.
 */
cl_uint kernelIndex(cl_kernel kernel)
{
    cl_uint i;
    for(i = 0; i < num_global_kernels; i++){
        if(kernel == global_kernels[i])
            break;
    }
    return i;
}

cl_kernel kernelFromServer(ptr_wrapper_t srv_kernel)
{
    cl_uint i;
    for(i = 0; i < num_global_kernels; i++){
        if(equal_ptr_wrappers(srv_kernel, global_kernels[i]->ptr_on_peer))
            return global_kernels[i];
    }
    return NULL;
}

/** @brief Remove a kernel from the global list.
 *
 * For instance when clReleaseKernel() is called.
 * @param kernel Object to be removed.
 * @return CL_SUCCESS if the object has been already discarded or
 * CL_INVALID_VALUE if the object does not exist.
 */
cl_int discardKernel(cl_kernel kernel)
{
    if(!hasKernel(kernel)){
        return CL_INVALID_VALUE;
    }
    cl_uint i, index;

    // Remove the kernel stuff
    free(kernel);

    assert(num_global_kernels > 0);
    // Remove the kernel from the global list
    index = kernelIndex(kernel);
    for(i = index; i < num_global_kernels - 1; i++){
        global_kernels[i] = global_kernels[i + 1];
    }
    num_global_kernels--;
    global_kernels[num_global_kernels] = NULL;

    return CL_SUCCESS;
}

/** @brief Retrieve from the server the kernel argument data.
 * @param kernel Kernel from which the argument should be asked.
 * @param arg Argument to be built up.
 * @return CL_SUCCESS if the argument has been successfully set up,
 * CL_OUT_OF_HOST_MEMORY if the library failed allocating required resources,
 * CL_OUT_OF_RESOURCES either if the data cannot be accessed or remote server
 * is not OpenCL >= 1.2 compliance.
 */
cl_int setupKernelArg(cl_kernel kernel, cl_kernel_arg arg)
{
    cl_int flag;
    // Set initial data
    arg->address = 0;
    arg->access = 0;
    arg->access_available = CL_TRUE;
    arg->type_name = NULL;
    arg->type_name_available = CL_TRUE;
    arg->type = 0;
    arg->type_available = CL_TRUE;
    arg->name = NULL;
    arg->name_available = CL_TRUE;
    arg->bytes = 0;
    arg->value = NULL;
    arg->is_set = CL_FALSE;
    // Get the available data
    size_t ret_size;
    flag = getKernelArgInfo(kernel,
                            arg->index,
                            CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                            sizeof(cl_uint),
                            &(arg->address),
                            NULL);
    if(flag != CL_SUCCESS){
        // This field is mandatory, so CL_KERNEL_ARG_INFO_NOT_AVAILABLE cannot
        // be accepted
        return CL_OUT_OF_RESOURCES;
    }
    flag = getKernelArgInfo(kernel,
                            arg->index,
                            CL_KERNEL_ARG_ACCESS_QUALIFIER,
                            sizeof(cl_uint),
                            &(arg->access),
                            NULL);
    if(flag == CL_KERNEL_ARG_INFO_NOT_AVAILABLE){
        arg->access_available = CL_FALSE;
    }
    else if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    flag = getKernelArgInfo(kernel,
                            arg->index,
                            CL_KERNEL_ARG_TYPE_NAME,
                            0,
                            NULL,
                            &ret_size);
    if(flag == CL_KERNEL_ARG_INFO_NOT_AVAILABLE){
        arg->type_name_available = CL_FALSE;
    }
    else if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    arg->type_name = (char*)malloc(ret_size);
    if(!arg->type_name){
        return CL_OUT_OF_HOST_MEMORY;
    }
    flag = getKernelArgInfo(kernel,
                            arg->index,
                            CL_KERNEL_ARG_TYPE_NAME,
                            ret_size,
                            arg->type_name,
                            NULL);
    if(flag == CL_KERNEL_ARG_INFO_NOT_AVAILABLE){
        arg->type_name_available = CL_FALSE;
    }
    else if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    flag = getKernelArgInfo(kernel,
                            arg->index,
                            CL_KERNEL_ARG_TYPE_QUALIFIER,
                            sizeof(cl_kernel_arg_type_qualifier),
                            &(arg->type),
                            NULL);
    if(flag == CL_KERNEL_ARG_INFO_NOT_AVAILABLE){
        arg->type_available = CL_FALSE;
    }
    else if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    flag = getKernelArgInfo(kernel,
                            arg->index,
                            CL_KERNEL_ARG_NAME,
                            0,
                            NULL,
                            &ret_size);
    if(flag == CL_KERNEL_ARG_INFO_NOT_AVAILABLE){
        arg->name_available = CL_FALSE;
    }
    else if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    arg->name = (char*)malloc(ret_size);
    if(!arg->name){
        return CL_OUT_OF_HOST_MEMORY;
    }
    flag = getKernelArgInfo(kernel,
                            arg->index,
                            CL_KERNEL_ARG_NAME,
                            ret_size,
                            arg->name,
                            NULL);
    if(flag == CL_KERNEL_ARG_INFO_NOT_AVAILABLE){
        arg->name_available = CL_FALSE;
    }
    else if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    return CL_SUCCESS;
}

/** @brief Setup the kernel data, including its arguments.
 * @param kernel Already built kernel to fill up.
 * @return CL_SUCCESS if the kernel is successfully setup, CL_OUT_OF_HOST_MEMORY
 * if the library failed allocating required resources, CL_OUT_OF_RESOURCES
 * either if the data cannot be accessed or remote server is not OpenCL >= 1.2
 * compliance.
 * @see setup
 */
cl_int setupKernel(cl_kernel kernel)
{
    cl_uint i,j;
    cl_int flag;
    // Set the kernel as unbuilt
    kernel->func_name = (char*)malloc(sizeof(char));
    if(!kernel->func_name){
        return CL_OUT_OF_HOST_MEMORY;
    }
    strcpy(kernel->func_name, "");
    kernel->num_args = 0;
    kernel->args = NULL;
    kernel->attributes = NULL;

    size_t ret_size;
    flag = getKernelInfo(kernel,
                         CL_KERNEL_FUNCTION_NAME,
                         0,
                         NULL,
                         &ret_size);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    free(kernel->func_name);
    kernel->func_name = (char*)malloc(ret_size);
    if(!kernel->func_name){
        return CL_OUT_OF_HOST_MEMORY;
    }
    flag = getKernelInfo(kernel,
                         CL_KERNEL_FUNCTION_NAME,
                         ret_size,
                         kernel->func_name,
                         NULL);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }

    flag = getKernelInfo(kernel,
                         CL_KERNEL_NUM_ARGS,
                         sizeof(cl_uint),
                         &(kernel->num_args),
                         NULL);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_HOST_MEMORY;
    }
    VERBOSE("\tfunction: %s, arguments: %u\n",
            kernel->func_name,
            kernel->num_args);

    if(kernel->num_args){
        kernel->args = (cl_kernel_arg*)malloc(
            kernel->num_args * sizeof(cl_kernel_arg));
        if(!kernel->args){
            return CL_OUT_OF_HOST_MEMORY;
        }
    }
    for(i = 0; i < kernel->num_args; i++){
        kernel->args[i] = (cl_kernel_arg)malloc(sizeof(struct _cl_kernel_arg));
        if(!kernel->args[i]){
            for(j = 0; j < i; j++){
                free(kernel->args[j]);
                kernel->args[j] = NULL;
            }
            free(kernel->args); kernel->args = NULL;
            return CL_OUT_OF_HOST_MEMORY;
        }
        kernel->args[i]->index = i;
        flag = setupKernelArg(kernel, kernel->args[i]);
        if(flag != CL_SUCCESS){
            for(j = 0; j < i; j++){
                free(kernel->args[j]);
                kernel->args[j] = NULL;
            }
            free(kernel->args); kernel->args = NULL;
            return CL_OUT_OF_RESOURCES;
        }
    }

    flag = getKernelInfo(kernel,
                         CL_KERNEL_ATTRIBUTES,
                         0,
                         NULL,
                         &ret_size);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    kernel->attributes = (char*)malloc(ret_size);
    if(!kernel->attributes){
        return CL_OUT_OF_HOST_MEMORY;
    }
    flag = getKernelInfo(kernel,
                         CL_KERNEL_ATTRIBUTES,
                         ret_size,
                         kernel->attributes,
                         NULL);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }

    return CL_SUCCESS;
}

cl_kernel createKernel(cl_program       program ,
                       const char *     kernel_name ,
                       cl_int *         errcode_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateKernel;
    size_t kernel_name_size = (strlen(kernel_name)+1)*sizeof(char);
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = program->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    // Try to build up a new instance
    cl_kernel kernel=NULL;
    ptr_wrapper_t srv_kernel;
    kernel = (cl_kernel)malloc(sizeof(struct _cl_kernel));
    if(!kernel){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    kernel->dispatch = program->dispatch;
    memset(&(kernel->ptr_on_peer), 0, sizeof(ptr_wrapper_t));
    kernel->rcount = 1;
    kernel->server = program->server;
    kernel->context = program->context;
    kernel->program = program;

    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_PROGRAM, program->ptr_on_peer, MSG_MORE);
    socket_flag |= Send_size_t(sockfd, kernel_name_size, MSG_MORE);
    socket_flag |= Send(sockfd, kernel_name, kernel_name_size, 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(kernel); kernel = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        free(kernel); kernel = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv_pointer_wrapper(sockfd, PTR_TYPE_KERNEL, &srv_kernel);
    if(socket_flag){
        free(kernel); kernel = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    kernel->ptr_on_peer = srv_kernel;

    // Add the object to the global list
    flag = addKernels(1, &kernel);
    if(flag != CL_SUCCESS){
        free(kernel); kernel = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    // Ask data from the object
    flag = setupKernel(kernel);
    if(flag != CL_SUCCESS){
        discardKernel(kernel);
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return kernel;
}

cl_int createKernelsInProgram(cl_program      program ,
                              cl_uint         num_kernels ,
                              cl_kernel *     kernels ,
                              cl_uint *       num_kernels_ret)
{
    unsigned int i, j;
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    cl_uint n;
    unsigned int comm = ocland_clCreateKernelsInProgram;
    if(num_kernels_ret) *num_kernels_ret=0;
    int *sockfd = program->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_PROGRAM, program->ptr_on_peer, MSG_MORE);
    socket_flag |= Send(sockfd, &num_kernels, sizeof(cl_uint), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv(sockfd, &n, sizeof(cl_uint), MSG_WAITALL);
    if(num_kernels_ret) *num_kernels_ret=n;
    if(kernels){
        if(num_kernels < n) {
            n = num_kernels;
        }
        for(i = 0; i < n; i++){
            ptr_wrapper_t srv_kernel;
            socket_flag |= Recv_pointer_wrapper(sockfd, PTR_TYPE_KERNEL, &srv_kernel);
            if(socket_flag){
                return CL_OUT_OF_RESOURCES;
            }
            // Build up the new instance
            cl_kernel kernel=NULL;
            kernel = (cl_kernel)calloc(1, sizeof(struct _cl_kernel));
            if(!kernel){
                for(j = 0; j < i; j++){
                    free(kernels[j]);
                }
                return CL_OUT_OF_HOST_MEMORY;
            }
            kernel->dispatch = program->dispatch;
            kernel->ptr_on_peer = srv_kernel;
            kernel->rcount = 1;
            kernel->server = program->server;
            kernel->context = program->context;
            kernel->program = program;

            // Ask data from the object
            flag = setupKernel(kernel);
            if(flag != CL_SUCCESS){
                for(j = 0; j < i; j++){
                    free(kernels[j]);
                }
                free(kernel);
                return CL_OUT_OF_RESOURCES;
            }

            // Swap the object with the new one
            kernels[i] = kernel;
        }
        // Add the objects to the global list
        flag = addKernels(n, kernels);
        if(flag != CL_SUCCESS){
            for(i = 0; i < n; i++){
                free(kernels[i]);
            }
            return CL_OUT_OF_RESOURCES;
        }
    }
    return CL_SUCCESS;
}

cl_int retainKernel(cl_kernel     kernel)
{
    kernel->rcount++;
    return CL_SUCCESS;
}

cl_int releaseKernel(cl_kernel    kernel)
{
    kernel->rcount--;
    if(kernel->rcount){
        return CL_SUCCESS;
    }

    // Call the server to clear the instance
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clReleaseKernel;
    int *sockfd = kernel->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_KERNEL, kernel->ptr_on_peer, 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }

    // Free the memory
    flag = discardKernel(kernel);

    return CL_SUCCESS;
}

cl_int setKernelArg(cl_kernel     kernel ,
                    cl_uint       arg_index ,
                    size_t        arg_size ,
                    const void *  arg_value)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clSetKernelArg;
    int *sockfd = kernel->server->socket;
    if(!sockfd){
        return CL_INVALID_KERNEL;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_KERNEL, kernel->ptr_on_peer, MSG_MORE);
    socket_flag |= Send(sockfd, &arg_index, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send_size_t(sockfd, arg_size, MSG_MORE);
    if(arg_value){
        // send arg_size twice, server receives it as arg_value_size
        socket_flag |= Send_size_t(sockfd, arg_size, MSG_MORE);
        socket_flag |= Send(sockfd, arg_value, arg_size, 0);
    }
    else{  // local memory
        size_t null_size=0;
        socket_flag |= Send_size_t(sockfd, null_size, 0);
    }
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    return flag;
}

cl_int getKernelInfo(cl_kernel        kernel ,
                     cl_kernel_info   param_name ,
                     size_t           param_value_size ,
                     void *           param_value ,
                     size_t *         param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetKernelInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = kernel->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_KERNEL, kernel->ptr_on_peer, MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_kernel_info), MSG_MORE);
    socket_flag |= Send_size_t(sockfd, param_value_size, 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv_size_t(sockfd, &size_ret);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        socket_flag |= Recv(sockfd, param_value, size_ret, MSG_WAITALL);
        if(socket_flag){
            return CL_OUT_OF_RESOURCES;
        }
    }
    return CL_SUCCESS;
}

cl_int getKernelWorkGroupInfo(cl_kernel                   kernel ,
                              cl_device_id                device ,
                              cl_kernel_work_group_info   param_name ,
                              size_t                      param_value_size ,
                              void *                      param_value ,
                              size_t *                    param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetKernelWorkGroupInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = kernel->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    if (param_value) {
        size_t minSize = param_value_size;
        switch (param_name) {
            case CL_KERNEL_GLOBAL_WORK_SIZE:
            case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
                minSize = 3 * sizeof(size_t);
                break;
            case CL_KERNEL_WORK_GROUP_SIZE:
            case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
                minSize = sizeof(size_t);
                break;
            default:
                break;
        }
        if (param_value_size < minSize) {
            return CL_INVALID_VALUE;
        }
    }
    cl_bool value_is_size = CL_FALSE;
    if (   param_name == CL_KERNEL_GLOBAL_WORK_SIZE
        || param_name == CL_KERNEL_COMPILE_WORK_GROUP_SIZE
        || param_name == CL_KERNEL_WORK_GROUP_SIZE
        || param_name == CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE) {
        value_is_size = CL_TRUE;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_KERNEL, kernel->ptr_on_peer, MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_DEVICE, device->ptr_on_peer, MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_kernel_work_group_info), MSG_MORE);
    if (value_is_size) {
        socket_flag |= Send_size_t(sockfd, param_value_size / sizeof(size_t), 0);
    } else {
        socket_flag |= Send_size_t(sockfd, param_value_size, 0);
    }
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv_size_t(sockfd, &size_ret);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if (value_is_size)
    {
        // size_ret is in size_t values
        if(param_value_size_ret) *param_value_size_ret = size_ret * sizeof(size_t);
        if(param_value){
            socket_flag |= Recv_size_t_array(sockfd, param_value, size_ret);
            if(socket_flag){
                return CL_OUT_OF_RESOURCES;
            }
        }
    } else {
        if(param_value_size_ret) *param_value_size_ret = size_ret;
        if(param_value){
            socket_flag |= Recv(sockfd, param_value, size_ret, MSG_WAITALL);
            if(socket_flag){
                return CL_OUT_OF_RESOURCES;
            }
        }
    }
    return CL_SUCCESS;
}

cl_int getKernelArgInfo(cl_kernel            kernel ,
                        cl_uint              arg_index ,
                        cl_kernel_arg_info   param_name ,
                        size_t               param_value_size ,
                        void *               param_value ,
                        size_t *             param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetKernelArgInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = kernel->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send_pointer_wrapper(sockfd, PTR_TYPE_KERNEL, kernel->ptr_on_peer, MSG_MORE);
    socket_flag |= Send(sockfd, &arg_index, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_kernel_arg_info), MSG_MORE);
    socket_flag |= Send_size_t(sockfd, param_value_size, 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv_size_t(sockfd, &size_ret);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(param_value){
        socket_flag |= Recv(sockfd, param_value, size_ret, MSG_WAITALL);
        if(socket_flag){
            return CL_OUT_OF_RESOURCES;
        }
    }
    return CL_SUCCESS;
}

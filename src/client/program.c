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
 * @see program.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/client/commands_enum.h>
#include <ocland/common/verbose.h>
#include <ocland/client/program.h>
#include <ocland/common/dataPack.h>
#include <ocland/common/dataExchange.h>

/// Number of known programs
cl_uint num_global_programs = 0;
/// List of known programs
cl_program *global_programs = NULL;

int hasProgram(cl_program program){
    cl_uint i;
    for(i = 0; i < num_global_programs; i++){
        if(program == global_programs[i])
            return 1;
    }
    return 0;
}

/** @brief Add a set of programs to the global list global_programs.
 *
 * This method is not checking if the objects are already present in the list,
 * however, accidentally adding the same object several times will only imply
 * performance penalties.
 * @param num_programs Number of objects to append.
 * @param programs Objects to append.
 * @return CL_SUCCESS if the objects are already generated, an error code
 * otherwise.
 */
cl_int addPrograms(cl_uint      num_programs,
                   cl_program*  programs)
{
    if(!num_programs)
        return CL_SUCCESS;

    // Add the programs to the global list
    cl_program *backup_programs = global_programs;

    global_programs = (cl_program*)malloc(
        (num_global_programs + num_programs) * sizeof(cl_program));
    if(!global_programs){
        VERBOSE("Failure allocating memory for %u programs!\n",
                num_global_programs + num_programs);
        free(backup_programs); backup_programs = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_programs){
        memcpy(global_programs,
               backup_programs,
               num_global_programs * sizeof(cl_program));
        free(backup_programs); backup_programs = NULL;
    }

    memcpy(&(global_programs[num_global_programs]),
           programs,
           num_programs * sizeof(cl_program));
    num_global_programs += num_programs;

    return CL_SUCCESS;
}

/** @brief Get the program index in the global list
 * @param program Object to look for
 * @return Index of the object, num_global_programs if it is not found.
 */
cl_uint programIndex(cl_program program)
{
    cl_uint i;
    for(i = 0; i < num_global_programs; i++){
        if(program == global_programs[i])
            break;
    }
    return i;
}

cl_program programFromServer(cl_program srv_program)
{
    cl_uint i;
    for(i = 0; i < num_global_programs; i++){
        if(srv_program == global_programs[i]->ptr)
            return global_programs[i];
    }
    return NULL;
}

/** @brief Remove a program from the global list.
 *
 * For instance when clReleaseProgram() is called.
 * @param program Object to be removed.
 * @return CL_SUCCESS if the object has been already discarded or
 * CL_INVALID_VALUE if the object does not exist.
 */
cl_int discardProgram(cl_program program)
{
    if(!hasProgram(program)){
        return CL_INVALID_VALUE;
    }
    cl_uint i, index;

    // Remove the program stuff
    if(program->devices) free(program->devices);
    if(program->source) free(program->source);
    if(program->binary_lengths) free(program->binary_lengths);
    for(i=0;i<program->num_devices;i++){
        if(program->binaries[i]) free(program->binaries[i]);
    }
    if(program->binaries) free(program->binaries);
    if(program->kernels) free(program->kernels);
    free(program);

    assert(num_global_programs > 0);
    // Remove the program from the global list
    index = programIndex(program);
    for(i = index; i < num_global_programs - 1; i++){
        global_programs[i] = global_programs[i + 1];
    }
    num_global_programs--;
    global_programs[num_global_programs] = NULL;

    return CL_SUCCESS;
}

/** @brief Get the program data from the server.
 * @param program Program from which the data is required.
 * @return One of the following values
 *     - CL_SUCCESS if the data has been successfully downloaded
 *     - CL_OUT_OF_HOST_MEMORY if memory for the program data cannot be
 *       allocated
 *     - CL_OUT_OF_RESOURCES if the data cannot be downloaded
 */
cl_int setupProgram(cl_program program)
{
    cl_uint i;
    cl_int flag;
    size_t ret_size;

    // Clear the program previously stored data
    if(program->source) free(program->source);
    if(program->binary_lengths) free(program->binary_lengths);
    if(program->binaries){
        for(i = 0; i < program->num_devices; i++){
            if(program->binaries[i]) free(program->binaries[i]);
            program->binaries[i]=NULL;
        }
        free(program->binaries);
    }
    if(program->kernels) free(program->kernels);
    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->kernels = NULL;
    program->num_kernels = 0;

    // Allocate the arrays for the binary data
    program->binary_lengths = (size_t*)malloc(
        program->num_devices * sizeof(size_t));
    program->binaries = (unsigned char**)malloc(
        program->num_devices * sizeof(unsigned char*));
    if((!program->binary_lengths) ||
       (!program->binaries) ){
        return CL_OUT_OF_HOST_MEMORY;
    }
    for(i = 0; i < program->num_devices; i++){
        program->binary_lengths[i] = 0;
        program->binaries[i] = NULL;
    }

    // Get the source code
    flag = getProgramInfo(program,
                          CL_PROGRAM_SOURCE,
                          0,
                          NULL,
                          &ret_size);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    if(ret_size){
        program->source = (char*)malloc(ret_size);
        if(!program->source){
            return CL_OUT_OF_HOST_MEMORY;
        }
        flag = getProgramInfo(program,
                              CL_PROGRAM_SOURCE,
                              ret_size,
                              program->source,
                              NULL);
        if(flag != CL_SUCCESS){
            return CL_OUT_OF_RESOURCES;
        }
    }

    // Get the compiled binaries (if they are available)
    flag = getProgramInfo(program,
                          CL_PROGRAM_BINARY_SIZES,
                          program->num_devices * sizeof(size_t),
                          program->binary_lengths,
                          NULL);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }

    for(i = 0; i < program->num_devices; i++){
        if(!program->binary_lengths[i]){
            continue;
        }
        program->binaries[i] = (unsigned char*)malloc(
            program->binary_lengths[i]);
        if(!program->binaries[i]){
            return CL_OUT_OF_HOST_MEMORY;
        }
    }
    flag = getProgramInfo(program,
                          CL_PROGRAM_BINARIES,
                          program->num_devices * sizeof(unsigned char*),
                          program->binaries,
                          NULL);

    // Get the list of available kernels
    flag = getProgramInfo(program,
                          CL_PROGRAM_KERNEL_NAMES,
                          0,
                          NULL,
                          &ret_size);
    if(flag == CL_INVALID_PROGRAM_EXECUTABLE){
        // That's right, the client has not asked to build the program yet,
        // or it has failed. Just let the kernel names array as NULL to let know
        // it to the ICD
        return CL_SUCCESS;
    }
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_RESOURCES;
    }
    if(ret_size){
        program->kernels = (char*)malloc(ret_size);
        if(!program->kernels){
            return CL_OUT_OF_HOST_MEMORY;
        }
        flag = getProgramInfo(program,
                              CL_PROGRAM_KERNEL_NAMES,
                              ret_size,
                              program->kernels,
                              NULL);
        if(flag != CL_SUCCESS){
            return CL_OUT_OF_RESOURCES;
        }
        // Count the number of kernels
        cl_uint n_chars = ret_size / sizeof(char);
        program->num_kernels = 1;
        for(i = 0; i < n_chars; i++){
            if(program->kernels[i] == ';')
                program->num_kernels++;
        }
        if(program->kernels[n_chars - 1] == ';')
            program->num_kernels--;
    }

    return CL_SUCCESS;
}

#ifdef _MSC_VER // VS does not support C99
    #define snprintf _snprintf
#endif

/** @brief Modify compilation options
* Add "-cl-kernel-arg-info" argument for kernel arguments information to work
* @param orig_options original program options string
* @param new_options options with kernel arg info parameter added or original options if this parameter has been set already
* @note memory pointer returned by function must be freed if it is not same as original options
* @return One of the following values
*     - CL_SUCCESS if options were successfully set
*     - CL_OUT_OF_HOST_MEMORY if memory could not be allocated
*/
cl_int addArgInfoOption(const char *orig_options, char ** new_options)
{
    size_t opt_size = 0;

    const char* arg_info_opt = "-cl-kernel-arg-info";
    if (strstr(orig_options, arg_info_opt)) {
        // arg info options is already set
        *new_options = (char*)orig_options;
        return CL_SUCCESS;
    }
    opt_size += strlen(arg_info_opt);
    if (orig_options) {
        opt_size++; // space
        opt_size += strlen(orig_options);
    }
    opt_size++; // terminating zero

    *new_options = malloc(opt_size);
    if (!*new_options) {
        *new_options = (char*)orig_options;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if (orig_options) {
        snprintf(*new_options, opt_size, "%s %s", arg_info_opt, orig_options);
    }
    else {
        snprintf(*new_options, opt_size, "%s", arg_info_opt);
    }
    return CL_SUCCESS;
}

cl_program createProgramWithSource(cl_context         context ,
                                   cl_uint            count ,
                                   const char **      strings ,
                                   const size_t *     lengths ,
                                   cl_int *           errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateProgramWithSource;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = context->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }
    size64* non_zero_lengths = calloc(count, sizeof(size64));
    if(!non_zero_lengths){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    // Two cases handled:
    // 1) lengths is NULL - all strings are null-terminated
    // 2) some lengths are zero - those strings are null-terminated
    for(i = 0; i < count; i++){
        if (lengths) {
            non_zero_lengths[i] = lengths[i];
        }
        if (0 == non_zero_lengths[i]){
            non_zero_lengths[i] = strlen(strings[i]);
        }
    }

    // Try to build up a new instance (we'll need to pass it as identifier
    // to the server)
    cl_program program=NULL, program_srv=NULL;
    program = (cl_program)malloc(sizeof(struct _cl_program));
    if(!program){
        free(non_zero_lengths);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    program->dispatch = context->dispatch;
    program->ptr = NULL;
    program->rcount = 1;
    program->server = context->server;
    program->context = context;
    program->num_devices = context->num_devices;
    program->devices = (cl_device_id*)malloc(
        context->num_devices * sizeof(cl_device_id));
    if(!program->devices){
        free(non_zero_lengths);
        free(program);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    memcpy(program->devices,
           context->devices,
           context->num_devices * sizeof(cl_device_id));
    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->num_kernels = 0;
    program->kernels = NULL;

    // Call the server to generate the object
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &context->ptr_on_peer, sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &count, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send(sockfd, non_zero_lengths, count * sizeof(size64), 0);
    for(i = 0; i < count; i++){
        socket_flag |= Send(sockfd, strings[i], non_zero_lengths[i], 0);
    }
    free(non_zero_lengths);
    non_zero_lengths = NULL;
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &program_srv, sizeof(cl_program), MSG_WAITALL);
    if(socket_flag){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    program->ptr = program_srv;

    // Add the object to the global list
    flag = addPrograms(1, &program);
    if(flag != CL_SUCCESS){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    // Ask data from the object
    flag = setupProgram(program);
    if(flag != CL_SUCCESS){
        discardProgram(program);
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return program;
}

cl_program createProgramWithBinary(cl_context                      context ,
                                   cl_uint                         num_devices ,
                                   const cl_device_id *            device_list ,
                                   const size_t *                  lengths ,
                                   const unsigned char **          binaries ,
                                   cl_int *                        binary_status ,
                                   cl_int *                        errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    cl_int* status = NULL;
    unsigned int comm = ocland_clCreateProgramWithBinary;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = context->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }

    // Substitute the local references to the remote ones
    pointer *devices = calloc(num_devices, sizeof(pointer));
    if ((num_devices > 0) && (NULL == devices)){
        if (errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    for(i = 0; i < num_devices; i++){
        devices[i] = device_list[i]->ptr_on_peer;
    }

    // Try to build up a new instance (we'll need to pass it as identifier
    // to the server)
    cl_program program=NULL, program_srv=NULL;
    program = (cl_program)malloc(sizeof(struct _cl_program));
    if(!program){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    program->dispatch = context->dispatch;
    program->ptr = NULL;
    program->rcount = 1;
    program->server = context->server;
    program->context = context;
    program->num_devices = num_devices;
    program->devices = (cl_device_id*)malloc(
        num_devices * sizeof(cl_device_id));
    if(!program->devices){
        free(devices);
        free(program);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    memcpy(program->devices,
           devices,
           num_devices * sizeof(cl_device_id));
    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->num_kernels = 0;
    program->kernels = NULL;

    // Call the server to generate the object
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr_on_peer), sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send(sockfd, devices, num_devices*sizeof(pointer), MSG_MORE);

    if (sizeof(size_t) == sizeof(size64)) {
        // usual case, 64 bit client
        socket_flag |= Send(sockfd, lengths, num_devices*sizeof(size64), MSG_MORE);
    }
    else {
        // 32 bit client - send lengths as 64 integers
        for (i = 0; i < num_devices; i++) {
            size64 current_length = lengths[i];
            socket_flag |= Send(sockfd, &current_length, sizeof(size64), MSG_MORE);
        }
    }

    for(i=0;i<num_devices;i++){
        int flag = (i == num_devices - 1) ? 0 : MSG_MORE;
        socket_flag |= Send(sockfd, binaries[i], lengths[i], flag);
    }
    free(devices); devices = NULL;
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &program_srv, sizeof(cl_program), MSG_WAITALL);
    if(socket_flag){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    program->ptr = program_srv;
    status = (cl_int*)malloc(num_devices * sizeof(cl_int));
    if(!status){
        free(status); status = NULL;
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    socket_flag |= Recv(sockfd, status, num_devices * sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(status); status = NULL;
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(binary_status)
        memcpy((void*)binary_status, (void*)status, num_devices * sizeof(cl_int));
    free(status); status = NULL;

    // Add the object to the global list
    flag = addPrograms(1, &program);
    if(flag != CL_SUCCESS){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    // Ask data from the object
    flag = setupProgram(program);
    if(flag != CL_SUCCESS){
        discardProgram(program);
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return program;
}

cl_program createProgramWithBuiltInKernels(cl_context             context ,
                                           cl_uint                num_devices ,
                                           const cl_device_id *   device_list ,
                                           const char *           kernel_names ,
                                           cl_int *               errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCreateProgramWithBuiltInKernels;
    size64 kernel_names_size = (strlen(kernel_names) + 1)*sizeof(char);
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    int *sockfd = context->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return NULL;
    }

    // Substitute the local references to the remote ones
    pointer *devices = calloc(num_devices, sizeof(pointer));
    if ((num_devices > 0) && (NULL == devices)){
        if (errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    for(i=0;i<num_devices;i++){
        devices[i] = device_list[i]->ptr_on_peer;
    }

    // Try to build up a new instance (we'll need to pass it as identifier
    // to the server)
    cl_program program=NULL, program_srv=NULL;
    program = (cl_program)malloc(sizeof(struct _cl_program));
    if(!program){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    program->dispatch = context->dispatch;
    program->ptr = NULL;
    program->rcount = 1;
    program->server = context->server;
    program->context = context;
    program->num_devices = num_devices;
    program->devices = (cl_device_id*)malloc(
        num_devices * sizeof(cl_device_id));
    if(!program->devices){
        free(devices);
        free(program);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    memcpy(program->devices,
           devices,
           num_devices * sizeof(cl_device_id));
    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->num_kernels = 0;
    program->kernels = NULL;

    // Call the server to generate the object
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr_on_peer), sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send(sockfd, devices, num_devices*sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &kernel_names_size, sizeof(size64), MSG_MORE);
    socket_flag |= Send(sockfd, kernel_names, kernel_names_size, 0);
    free(devices); devices = NULL;
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &program_srv, sizeof(cl_program), MSG_WAITALL);
    if(socket_flag){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    program->ptr = program_srv;

    // Add the object to the global list
    flag = addPrograms(1, &program);
    if(flag != CL_SUCCESS){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    // Ask data from the object
    flag = setupProgram(program);
    if(flag != CL_SUCCESS){
        discardProgram(program);
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return program;
}

cl_int retainProgram(cl_program  program)
{
    program->rcount++;
    return CL_SUCCESS;
}

cl_int releaseProgram(cl_program  program)
{
    program->rcount--;
    if(program->rcount){
        return CL_SUCCESS;
    }

    // Call the server to clear the instance
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clReleaseProgram;
    int *sockfd = program->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(program->ptr), sizeof(cl_program), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }

    // Free the memory
    flag = discardProgram(program);

    return CL_SUCCESS;
}


cl_int buildProgram(cl_program            program ,
                    cl_uint               num_devices ,
                    const cl_device_id *  device_list ,
                    const char *          options ,
                    void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                    void *                user_data)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    cl_uint i;
    unsigned int comm = ocland_clBuildProgram;
    char * mod_options = (char*)options;
    size64 options_size = 0;
    int *sockfd = program->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Substitute the local references to the remote ones
    pointer *devices = calloc(num_devices, sizeof(pointer));
    if ((num_devices > 0) && (NULL == devices)){
        return CL_OUT_OF_HOST_MEMORY;
    }
    addArgInfoOption(options, &mod_options);
    options_size = (strlen(mod_options) + 1) * sizeof(char);
    for(i = 0; i < num_devices; i++){
        devices[i] = device_list[i]->ptr_on_peer;
    }
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    socket_flag |= Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send(sockfd, devices, num_devices*sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &options_size, sizeof(size64), MSG_MORE);
    socket_flag |= Send(sockfd, mod_options, options_size, 0);
    free(devices); devices = NULL;
    if (mod_options != options) {
        free(mod_options); mod_options = NULL;
    }
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }

    // Ask new data
    flag = setupProgram(program);
    return flag;
}

cl_int compileProgram(cl_program            program ,
                      cl_uint               num_devices ,
                      const cl_device_id *  device_list ,
                      const char *          options ,
                      cl_uint               num_input_headers ,
                      const cl_program *    input_headers,
                      const char **         header_include_names ,
                      void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                      void *                user_data)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clCompileProgram;
    char * mod_options = (char*)options;
    size64 options_size = 0;
    size64 str_size = 0;
    int *sockfd = program->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    // Substitute the local references to the remote ones
    pointer *devices = calloc(num_devices, sizeof(pointer));
    if ((num_devices > 0) && (NULL == devices)){
        return CL_OUT_OF_HOST_MEMORY;
    }
    for(i = 0; i < num_devices; i++){
        devices[i] = device_list[i]->ptr_on_peer;
    }
    cl_program *headers = calloc(num_input_headers, sizeof(cl_program));
    if ((num_devices > 0) && (NULL == devices)){
        return CL_OUT_OF_HOST_MEMORY;
    }
    for(i = 0; i < num_input_headers; i++){
        headers[i] = input_headers[i]->ptr;
    }
    addArgInfoOption(options, &mod_options);
    options_size = (strlen(mod_options) + 1) * sizeof(char);

    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    socket_flag |= Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send(sockfd, devices, num_devices * sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &options_size, sizeof(size64), MSG_MORE);
    socket_flag |= Send(sockfd, mod_options, options_size, MSG_MORE);
    if(num_input_headers){
        socket_flag |= Send(sockfd, &num_input_headers, sizeof(cl_uint), MSG_MORE);
        socket_flag |= Send(sockfd, headers, num_input_headers * sizeof(cl_program), MSG_MORE);
        for(i = 0; i < num_input_headers - 1; i++){
            str_size = (strlen(header_include_names[i]) + 1) * sizeof(char);
            socket_flag |= Send(sockfd, &str_size, sizeof(size64), MSG_MORE);
            socket_flag |= Send(sockfd, header_include_names[i], str_size, MSG_MORE);
        }
        str_size = (strlen(header_include_names[i]) + 1) * sizeof(char);
        socket_flag |= Send(sockfd, &str_size, sizeof(size64), MSG_MORE);
        socket_flag |= Send(sockfd, header_include_names[i], str_size, 0);
    }
    else{
        socket_flag |= Send(sockfd, &num_input_headers, sizeof(cl_uint), 0);
    }
    free(devices); devices = NULL;
    free(headers); headers = NULL;
    if (mod_options != options) {
        free(mod_options); mod_options = NULL;
    }
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }

    // Ask data from the object
    flag = setupProgram(program);
    return flag;
}

cl_program linkProgram(cl_context            context ,
                       cl_uint               num_devices ,
                       const cl_device_id *  device_list ,
                       const char *          options ,
                       cl_uint               num_input_programs ,
                       const cl_program *    input_programs ,
                       void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                       void *                user_data ,
                       cl_int *              errcode_ret)
{
    unsigned int i;
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clLinkProgram;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    size64 str_size = (strlen(options) + 1)*sizeof(char);
    int *sockfd = context->server->socket;
    if(!sockfd){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }

    // Substitute the local references to the remote ones
    pointer *devices = calloc(num_devices, sizeof(pointer));
    if ((num_devices > 0) && (NULL == devices)) {
        if (errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    for(i = 0; i < num_devices; i++){
        devices[i] = device_list[i]->ptr_on_peer;
    }
    cl_program *programs = calloc(num_input_programs, sizeof(cl_program));
    if ((num_input_programs > 0) && (NULL == programs)){
        if (errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    for(i = 0; i < num_input_programs; i++){
        programs[i] = input_programs[i]->ptr;
    }

    // Try to build up a new instance (we'll need to pass it as identifier
    // to the server)
    cl_program program=NULL, program_srv=NULL;
    program = (cl_program)malloc(sizeof(struct _cl_program));
    if(!program){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    program->dispatch = context->dispatch;
    program->ptr = NULL;
    program->rcount = 1;
    program->server = context->server;
    program->context = context;
    if(!num_devices)
         program->num_devices = context->num_devices;
    else
        program->num_devices = num_devices;
    program->devices = (cl_device_id*)malloc(
        num_devices * sizeof(cl_device_id));
    if(!program->devices){
        free(devices);
        free(program);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    if(!num_devices){
        memcpy(program->devices,
               context->devices,
               program->num_devices * sizeof(cl_device_id));
    }
    else{
        memcpy(program->devices,
               devices,
               program->num_devices * sizeof(cl_device_id));
    }
    program->source = NULL;
    program->binary_lengths = NULL;
    program->binaries = NULL;
    program->num_kernels = 0;
    program->kernels = NULL;

    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(context->ptr_on_peer), sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &num_devices, sizeof(cl_uint), MSG_MORE);
    socket_flag |= Send(sockfd, devices, num_devices * sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &str_size, sizeof(size64), MSG_MORE);
    socket_flag |= Send(sockfd, options, str_size, MSG_MORE);
    if(num_input_programs){
        for(i=0;i<num_input_programs;i++){
            programs[i] = input_programs[i]->ptr;
        }
        socket_flag |= Send(sockfd, &num_input_programs, sizeof(cl_uint), MSG_MORE);
        socket_flag |= Send(sockfd, programs, num_input_programs * sizeof(cl_program), 0);
    }
    else{
        Send(sockfd, &num_input_programs, sizeof(cl_uint), 0);
    }
    free(devices); devices = NULL;
    free(programs); programs = NULL;
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(flag != CL_SUCCESS){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }
    socket_flag |= Recv(sockfd, &program_srv, sizeof(cl_program), MSG_WAITALL);
    if(socket_flag){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    program->ptr = program_srv;

    // Add the object to the global list
    flag = addPrograms(1, &program);
    if(flag != CL_SUCCESS){
        free(program); program = NULL;
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    // Ask data from the object
    flag = setupProgram(program);
    if(flag != CL_SUCCESS){
        discardProgram(program);
        if(errcode_ret) *errcode_ret = flag;
        return NULL;
    }

    return program;
}

cl_int getProgramInfo(cl_program          program ,
                      cl_program_info     param_name ,
                      size_t              param_value_size ,
                      void *              param_value ,
                      size_t *            param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    cl_uint i;
    size_t size_ret=0;
    unsigned int comm = ocland_clGetProgramInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = program->server->socket;
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }

    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_program_info), MSG_MORE);
    socket_flag |= Send(sockfd, &param_value_size, sizeof(size_t), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv(sockfd, &size_ret, sizeof(size_t), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(param_value_size_ret) *param_value_size_ret = size_ret;
    if(!param_value_size){
        return CL_SUCCESS;
    }
    if(param_name != CL_PROGRAM_BINARIES){
        socket_flag |= Recv(sockfd, param_value, size_ret, MSG_WAITALL);
        if(socket_flag){
            return CL_OUT_OF_RESOURCES;
        }
        return CL_SUCCESS;
    }
    for(i = 0; i < program->num_devices; i++){
        if(!program->binary_lengths[i])
            continue;
        socket_flag |= Recv(sockfd,
                            program->binaries[i],
                            program->binary_lengths[i],
                            MSG_WAITALL);
        memcpy(((char**)param_value)[i],
               program->binaries[i],
               program->binary_lengths[i]);
    }
    return CL_SUCCESS;
}

cl_int getProgramBuildInfo(cl_program             program ,
                           cl_device_id           device ,
                           cl_program_build_info  param_name ,
                           size_t                 param_value_size ,
                           void *                 param_value ,
                           size_t *               param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size64 size_ret=0;
    unsigned int comm = ocland_clGetProgramBuildInfo;
    if(param_value_size_ret) *param_value_size_ret=0;
    int *sockfd = program->server->socket;
    if(!sockfd){
        return CL_INVALID_PROGRAM;
    }

    size64 param_value_size_64 = param_value_size;
    // Call the server
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(program->ptr), sizeof(cl_program), MSG_MORE);
    socket_flag |= Send(sockfd, &(device->ptr_on_peer), sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_program_build_info), MSG_MORE);
    socket_flag |= Send(sockfd, &param_value_size_64, sizeof(size64), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv(sockfd, &size_ret, sizeof(size64), MSG_WAITALL);
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

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

#include <CL/cl.h>
#include <CL/cl_ext.h>

#include <ocland/server/ocland_context.h>
#include <ocland/server/ocland_event.h>

#ifndef VALIDATOR_H_INCLUDED
#define VALIDATOR_H_INCLUDED

/// Abstraction of validator_st structure
typedef struct validator_st* validator;

/** @brief Stores the data referenced by a client.
 *
 * It includes:
 *     - Connection sockets opened with the client
 *     - OpenCL generated instances
 *
 * It is therefore an initial barrier for segmentation faults.
 *
 * @note In Ocland the client ICD is storing all the instances generated here,
 * in the server side, using them to ask us for OpenCL methods.
 * However the server should try to protect itself from invalid pointers just
 * because otherwise they may cause the server crash.
 */
struct validator_st{
    /** @brief Main data stream socket.
     *
     * The main data stream is where the commands are received, and the answers
     * provided.
     */
    int *socket;
    /** @brief Callbacks data stream socket.
     *
     * Callbacks are an asynchronous stuff which is called "randomly", and
     * therefore they cannot be managed by the main data stream.
     *
     * Therefore a new socket is opened in order to can ask the affected clients
     * to parse a callback function when we want.
     */
    int *callbacks_socket;

    /// Number of devices registered
    cl_uint num_devices;
    /// Recognized platforms array
    cl_device_id *devices;
    /// Number of context created
    cl_uint num_contexts;
    /// Generated contexts
    ocland_context *contexts;
    /// Number of queues created
    cl_uint num_queues;
    /// Generated queues
    cl_command_queue *queues;
    /// Number of memory objects created
    cl_uint num_buffers;
    /// Generated memory objects
    cl_mem *buffers;
    /// Number of samplers created
    cl_uint num_samplers;
    /// Generated samplers
    cl_sampler *samplers;
    /// Number of programs created
    cl_uint num_programs;
    /// Generated programs
    cl_program *programs;
    /// Number of kernels created
    cl_uint num_kernels;
    /// Generated kernels
    cl_kernel *kernels;
    /// Number of events created
    cl_uint num_events;
    /// Generated events
    ocland_event *events;
};

/** Initialize validator. Call ever this method before strat working.
 * @param v Uninitialized validator.
 */
void initValidator(validator v);

/** Destroy validator.
 * @param v Validator.
 */
void closeValidator(validator v);

/** Validate if a platform is present on the server.
 * @param v Active validator.
 * @param platform OpenCL platform.
 * @return CL_SUCCESS if platform is found, CL_INVALID_PLATFORM otherwise.
 * @note Platforms can't be stored, or following clients may fail creating
 * contexts, so only real time validation can be performed.
 */
cl_int isPlatform(validator v, cl_platform_id platform);

/** Validate if a device is present on the server.
 * @param v Active validator.
 * @param device OpenCL device.
 * @return CL_SUCCESS if device is found, CL_INVALID_DEVICE otherwise.
 */
cl_int isDevice(validator v, cl_device_id device);

/** Register devices into the valid list. If repeated devices are detected
 * will be ignored.
 * @param v Active validator.
 * @param num_devices Number of devices passed.
 * @param devices Platforms array.
 * @return number of devices stored.
 */
cl_uint registerDevices(validator v, cl_uint num_devices, cl_device_id *devices);

/** Removes devices from the valid list.
 * @param v Active validator.
 * @param num_devices Number of devices passed.
 * @param devices Platforms array.
 * @return number of devices stored.
 */
cl_uint unregisterDevices(validator v, cl_uint num_devices, cl_device_id *devices);

/** Validate if a context has been generated on this server.
 * @param v Active validator.
 * @param context OpenCL context.
 * @return CL_SUCCESS if context is found, CL_INVALID_CONTEXT otherwise.
 */
cl_int isContext(validator v, ocland_context context);

/** Register contexts into the valid list. If repeated contexts are detected
 * will be ignored.
 * @param v Active validator.
 * @param context OpenCL context.
 * @return number of contexts stored.
 */
cl_uint registerContext(validator v, ocland_context context);

/** Removes the context from the valid list.
 * @param v Active validator.
 * @param context OpenCL context.
 * @return number of contexts stored.
 */
cl_uint unregisterContext(validator v, ocland_context context);

/** Validate if a command queue has been generated on this server.
 * @param v Active validator.
 * @param queue OpenCL queue.
 * @return CL_SUCCESS if queue is found, CL_INVALID_CONTEXT otherwise.
 */
cl_int isQueue(validator v, cl_command_queue queue);

/** Register a command queue into the valid list. If repeated queues are detected
 * will be ignored.
 * @param v Active validator.
 * @param queue OpenCL queue.
 * @return number of queues stored.
 */
cl_uint registerQueue(validator v, cl_command_queue queue);

/** Removes the queue from the valid list.
 * @param v Active validator.
 * @param queue OpenCL queue.
 * @return number of queues stored.
 */
cl_uint unregisterQueue(validator v, cl_command_queue queue);

/** Validate if a memory object has been generated on this server.
 * @param v Active validator.
 * @param buffer OpenCL memory object.
 * @return CL_SUCCESS if memory object is found, CL_INVALID_CONTEXT otherwise.
 */
cl_int isBuffer(validator v, cl_mem buffer);

/** Register a memory object into the valid list. If repeated memory object are detected
 * will be ignored.
 * @param v Active validator.
 * @param buffer OpenCL memory object.
 * @return number of memory objects stored.
 */
cl_uint registerBuffer(validator v, cl_mem buffer);

/** Removes the memory object from the valid list.
 * @param v Active validator.
 * @param buffer OpenCL memory object.
 * @return number of memory objects stored.
 */
cl_uint unregisterBuffer(validator v, cl_mem buffer);

/** Validate if a sampler has been generated on this server.
 * @param v Active validator.
 * @param sampler OpenCL sampler.
 * @return CL_SUCCESS if sampler is found, CL_INVALID_CONTEXT otherwise.
 */
cl_int isSampler(validator v, cl_sampler sampler);

/** Register a sampler into the valid list. If repeated sampler are detected
 * will be ignored.
 * @param v Active validator.
 * @param sampler OpenCL sampler.
 * @return number of samplers stored.
 */
cl_uint registerSampler(validator v, cl_sampler sampler);

/** Removes the sampler from the valid list.
 * @param v Active validator.
 * @param sampler OpenCL sampler.
 * @return number of samplers stored.
 */
cl_uint unregisterSampler(validator v, cl_sampler sampler);

/** Validate if a program has been generated on this server.
 * @param v Active validator.
 * @param program OpenCL program.
 * @return CL_SUCCESS if program is found, CL_INVALID_CONTEXT otherwise.
 */
cl_int isProgram(validator v, cl_program program);

/** Register a program into the valid list. If repeated program are detected
 * will be ignored.
 * @param v Active validator.
 * @param program OpenCL program.
 * @return number of programs stored.
 */
cl_uint registerProgram(validator v, cl_program program);

/** Removes the program from the valid list.
 * @param v Active validator.
 * @param program OpenCL program.
 * @return number of programs stored.
 */
cl_uint unregisterProgram(validator v, cl_program program);

/** Validate if a kernel has been generated on this server.
 * @param v Active validator.
 * @param kernel OpenCL kernel.
 * @return CL_SUCCESS if kernel is found, CL_INVALID_CONTEXT otherwise.
 */
cl_int isKernel(validator v, cl_kernel kernel);

/** Register a kernel into the valid list. If repeated kernel are detected
 * will be ignored.
 * @param v Active validator.
 * @param kernel OpenCL kernel.
 * @return number of kernels stored.
 */
cl_uint registerKernel(validator v, cl_kernel kernel);

/** Removes the kernel from the valid list.
 * @param v Active validator.
 * @param kernel OpenCL kernel.
 * @return number of kernels stored.
 */
cl_uint unregisterKernel(validator v, cl_kernel kernel);

/** Validate if a event has been generated on this server.
 * @param v Active validator.
 * @param event OpenCL event.
 * @return CL_SUCCESS if event is found, CL_INVALID_CONTEXT otherwise.
 */
cl_int isEvent(validator v, ocland_event event);

/** Register a event into the valid list. If repeated event are detected
 * will be ignored.
 * @param v Active validator.
 * @param event OpenCL event.
 * @return number of kernels stored.
 */
cl_uint registerEvent(validator v, ocland_event event);

/** Removes the event from the valid list.
 * @param v Active validator.
 * @param event OpenCL event.
 * @return number of kernels stored.
 */
cl_uint unregisterEvent(validator v, ocland_event event);

#endif // VALIDATOR_H_INCLUDED

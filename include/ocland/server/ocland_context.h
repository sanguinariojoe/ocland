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
 * @brief cl_context management
 *
 * ocland_context is an intermediate data structure to store some useful data
 * related with the cl_context instance.
 * @see ocland_context.c
 */

#ifndef OCLAND_CONTEXT_H_INCLUDED
#define OCLAND_CONTEXT_H_INCLUDED

#include <CL/cl.h>
#include <CL/cl_ext.h>

#include <ocland/common/dataExchange.h>

/** @brief _ocland_context abstraction.
 */
typedef struct _ocland_context* ocland_context;


/** @brief Intermediate data structure to manage the cl_context instance.
 *
 * Contexts have the ability of registering callback functions, however such
 * callback functions should not be executed in this side, but in the client
 * one.
 *
 * In ocland it is solved storing a shared identifier (usually the client
 * object pointer), registering the callback function, and passing such
 * identifier to it.
 *
 * The callback function will communicate with the client by a parallel
 * connection.
 */
struct _ocland_context
{
    /// Internally OpenCL managed event
    cl_context context;
    /// References count
    cl_uint rcount;
    /// Callbacks socket to call the client
    int *sockcb;
    /// Shared identifier with the client
    /// It is client address space context pointer now
    ptr_wrapper_t identifier;
};

/** @brief clCreateContext extension in order to register and manage the
 * callback function.
 * @param properties List of context properties.
 * @param num_devices Number of devices into \a devices.
 * @param devices List of devices.
 * @param identifier Shared identifier for the callback function. NULL if the
 * callback should not be registered.
 * @param socket_cb Callbacks communication channel socket.
 * @param errcode_ret CL_SUCCESS upon a successfully context creation, error
 * code otherwise. If NULL is provided, it will be ignored.
 * @return The generated context, NULL if errors are detected.
 */
ocland_context oclandCreateContext(cl_context_properties *properties,
                                   cl_uint num_devices,
                                   cl_device_id* devices,
                                   ptr_wrapper_t identifier,
                                   int *socket_cb,
                                   cl_int *errcode_ret);

/** @brief clCreateContextFromType extension in order to register and manage the
 * callback function.
 * @param properties List of context properties.
 * @param device_type Type of device.
 * @param identifier Shared identifier for the callback function. NULL if the
 * callback should not be registered.
 * @param socket_cb Callbacks communication channel socket.
 * @param errcode_ret CL_SUCCESS upon a successfully context creation, error
 * code otherwise. If NULL is provided, it will be ignored.
 * @return The generated context, NULL if errors are detected.
 */
ocland_context oclandCreateContextFromType(cl_context_properties *properties,
                                           cl_device_type device_type,
                                           ptr_wrapper_t identifier,
                                           int *socket_cb,
                                           cl_int *errcode_ret);

/** @brief clRetainContext extension.
 * @param context Context object.
 * @return Either CL_SUCCESS or an error code.
 */
cl_int oclandRetainContext(ocland_context context);

/** @brief clReleaseContext extension.
 * @param context Context object.
 * @return Either CL_SUCCESS or an error code.
 */
cl_int oclandReleaseContext(ocland_context context);

/** @brief clReleaseContext extension.
 * @param context Context object.
 * @return Either CL_SUCCESS or an error code.
 */
cl_int oclandGetContextInfo(ocland_context   context,
                            cl_context_info  param_name,
  	                        size_t           param_value_size,
  	                        void            *param_value,
  	                        size_t          *param_value_size_ret);

#endif // OCLAND_CONTEXT_H_INCLUDED

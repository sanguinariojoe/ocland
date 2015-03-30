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
 * @brief ICD cl_context implementation
 *
 * cl_context is a typedef definition:
 *
 *     typedef struct _cl_context* cl_context
 *
 * In this file such data structure, and all the associated methods, are
 * declared.
 * @see context.c
 */

#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include <ocl_icd.h>
#include <ocland/client/platform_id.h>
#include <ocland/client/device_id.h>

/** @brief ICD context identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_context
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_context ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server where this object is allocated
    oclandServer server;
    /// Associated platform
    cl_platform_id platform;
    /// Associated number of devices
    cl_uint num_devices;
    /// Associated devices
    cl_device_id *devices;
    /// Number of context properties (including the trailing zero)
    cl_uint num_properties;
    /// Context properties (including the trailing zero)
    cl_context_properties *properties;
    /// Callback function
    void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *);
    /// User data for the callback function
    void *user_data;
    /// Task to manage the callback function
    task task_notify;
    /// Task to manage the callback stream errors
    task task_error;
};

/** @brief Check for context validity
 * @param context Context to check
 * @return 1 if the context is a known one, 0 otherwise.
 */
int hasContext(cl_context context);

/** @brief Get a context from the server instance pointer.
 * @param srv_context Server context instance
 * @return ICD context instance, NULL if \a srv_context cannot be found.
 */
cl_context contextFromServer(cl_context srv_context);

/** clCreateContext ocland abstraction method.
 * @param num_properties Number of properties into properties array
 */
cl_context createContext(cl_platform_id                platform,
                         const cl_context_properties * properties,
                         cl_uint                       num_properties,
                         cl_uint                       num_devices ,
                         const cl_device_id *          devices,
                         void (CL_CALLBACK *           pfn_notify)(const char *,
                                                                   const void *,
                                                                   size_t,
                                                                   void *),
                         void *                        user_data,
                         cl_int *                      errcode_ret);

/** clCreateContextFromType ocland abstraction method.
 * @param num_properties Number of properties into properties array
 */
cl_context createContextFromType(cl_platform_id                platform,
                                 const cl_context_properties * properties,
                                 cl_uint                       num_properties,
                                 cl_device_type                device_type,
                                 void (CL_CALLBACK *           pfn_notify)(const char *,
                                                                           const void *,
                                                                           size_t,
                                                                           void *),
                                 void *                        user_data,
                                 cl_int *                      errcode_ret);

/** clRetainContext ocland abstraction method.
 */
cl_int retainContext(cl_context context);

/** clRetainContext ocland abstraction method.
 */
cl_int releaseContext(cl_context context);

/** clGetContextInfo ocland abstraction method.
 */
cl_int getContextInfo(cl_context         context,
                      cl_context_info    param_name,
                      size_t             param_value_size,
                      void *             param_value,
                      size_t *           param_value_size_ret);

#endif // CONTEXT_H_INCLUDED

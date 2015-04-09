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
 * @brief ICD cl_device_id implementation
 *
 * cl_device_id is a typedef definition:
 *
 *     typedef struct _cl_device_id* cl_device_id
 *
 * In this file such data structure, and all the associated methods, are
 * declared.
 * @see device_id.c
 */

#ifndef DEVICE_ID_H_INCLUDED
#define DEVICE_ID_H_INCLUDED

#include <ocl_icd.h>
#include <ocland/client/platform_id.h>
#include <ocland/common/dataExchange.h>

/** @brief ICD device identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_device_id
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server allocated instance
    ptr_wrapper_t ptr_on_peer;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server where this device is allocated
    oclandServer server;
    /// Associated platform
    cl_platform_id platform;
    /// Parent device, NULL for devices from clGetDeviceIDs (root-level)
    cl_device_id parent_device;
    /// Device type
    cl_device_type device_type;
};

/** @brief Check for devices validity
 * @param device Device to check
 * @return 1 if the device is a known device, 0 otherwise.
 */
int hasDevice(cl_device_id device);

/** @brief Get a device from the server instance pointer.
 * @param srv_device Server device instance
 * @return ICD device instance, NULL if \a srv_device cannot be found.
 */
cl_device_id deviceFromServer(ptr_wrapper_t srv_device);

/** @brief clGetDeviceIDs() ocland abstraction method.
 *
 * In ocland CL_DEVICE_TYPE_DEFAULT is considered equal to CL_DEVICE_TYPE_ALL
 */
cl_int getDeviceIDs(cl_platform_id   platform,
                    cl_device_type   device_type,
                    cl_uint          num_entries,
                    cl_device_id *   devices,
                    cl_uint *        num_devices);

/** @brief clGetDeviceInfo() ocland abstraction method.
 */
cl_int getDeviceInfo(cl_device_id    device,
                     cl_device_info  param_name,
                     size_t          param_value_size,
                     void *          param_value,
                     size_t *        param_value_size_ret);

/** clCreateSubDevices() ocland abstraction method.
 * @param num_properties Number of properties into properties array.
 */
cl_int createSubDevices(cl_device_id                         in_device,
                        const cl_device_partition_property * properties,
                        cl_uint                              num_properties,
                        cl_uint                              num_entries,
                        cl_device_id                       * out_devices,
                        cl_uint                            * num_devices);

/** clRetainDevice() ocland abstraction method.
 */
cl_int retainDevice(cl_device_id device);

/** clReleaseDevice() ocland abstraction method.
 */
cl_int releaseDevice(cl_device_id device);

#endif // DEVICE_ID_H_INCLUDED

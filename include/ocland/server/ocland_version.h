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

#ifndef OCLAND_VERSION_H_INCLUDED
#define OCLAND_VERSION_H_INCLUDED

/** @struct _cl_version
 * Describes the OpenCL platform version.
 */
struct _cl_version
{
    /// Major version number
    cl_uint major;
    /// Minor version number
    cl_uint minor;
};

/** Compute the OpenCL version.
 * @param platform The platform ID.
 * @return OpenCL platform version. If the version returned is 0.0,
 * errors during the computation can be expected.
 */
struct _cl_version clGetPlatformVersion(cl_platform_id platform);

/** Compute the OpenCL version.
 * @param device The device ID.
 * @return OpenCL platform version. If the version returned is 0.0,
 * errors during the computation can be expected.
 */
struct _cl_version clGetDeviceVersion(cl_device_id device);

/** Compute the OpenCL version.
 * @param context The context ID.
 * @return OpenCL platform version. If the version returned is 0.0,
 * errors during the computation can be expected.
 */
struct _cl_version clGetContextVersion(cl_context context);

/** Compute the OpenCL version.
 * @param command_queue The command queue ID.
 * @return OpenCL platform version. If the version returned is 0.0,
 * errors during the computation can be expected.
 */
struct _cl_version clGetCommandQueueVersion(cl_command_queue command_queue);

/** Compute the OpenCL version.
 * @param memobj The memory object ID.
 * @return OpenCL platform version. If the version returned is 0.0,
 * errors during the computation can be expected.
 */
struct _cl_version clGetMemObjectVersion(cl_mem memobj);

/** Compute the OpenCL version.
 * @param sampler The sampler ID.
 * @return OpenCL platform version. If the version returned is 0.0,
 * errors during the computation can be expected.
 */
struct _cl_version clGetSamplerVersion(cl_sampler sampler);

/** Compute the OpenCL version.
 * @param program The program ID.
 * @return OpenCL platform version. If the version returned is 0.0,
 * errors during the computation can be expected.
 */
struct _cl_version clGetProgramVersion(cl_program program);

/** Compute the OpenCL version.
 * @param kernel The kernel ID.
 * @return OpenCL platform version. If the version returned is 0.0,
 * errors during the computation can be expected.
 */
struct _cl_version clGetKernelVersion(cl_kernel kernel);

/** Compute the OpenCL version.
 * @param event The event ID.
 * @return OpenCL platform version. If the version returned is 0.0,
 * errors during the computation can be expected.
 */
struct _cl_version clGetEventVersion(cl_event event);

#endif // OCLAND_VERSION_H_INCLUDED

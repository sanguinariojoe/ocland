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
 * @brief ICD cl_sampler implementation
 *
 * cl_sampler is a typedef definition:
 *
 *     typedef struct _cl_sampler* cl_sampler
 *
 * In this file such data structure, and all the associated methods, are
 * declared.
 * @see sampler.c
 */

#ifndef SAMPLER_H_INCLUDED
#define SAMPLER_H_INCLUDED

#include <ocl_icd.h>
#include <ocland/client/platform_id.h>
#include <ocland/client/context.h>

/** @brief ICD sampler identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_sampler
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_sampler ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server where this object is allocated
    oclandServer server;
    /// Associated context
    cl_context context;
    /// Normalized coordinates
    cl_bool normalized_coords;
    /// Addressing mode
 	cl_addressing_mode addressing_mode;
    /// Filter mode
    cl_filter_mode filter_mode;
};

/** @brief Check for sampler validity
 * @param sampler Sampler to check
 * @return 1 if the sampler is a known one, 0 otherwise.
 */
int hasSampler(cl_sampler sampler);

/** @brief Get a sampler from the server instance pointer.
 * @param srv_sampler Server sampler instance
 * @return ICD sampler instance, NULL if \a srv_sampler cannot be found.
 */
cl_sampler samplerFromServer(cl_sampler srv_sampler);

/** @brief clCreateSampler ocland abstraction method.
 */
cl_sampler createSampler(cl_context           context ,
                         cl_bool              normalized_coords ,
                         cl_addressing_mode   addressing_mode ,
                         cl_filter_mode       filter_mode ,
                         cl_int *             errcode_ret);


/** @brief clRetainSampler ocland abstraction method.
 */
cl_int retainSampler(cl_sampler sampler);

/** @brief clReleaseSampler ocland abstraction method.
 */
cl_int releaseSampler(cl_sampler sampler);

/** @brief clGetSamplerInfo ocland abstraction method.
 */
cl_int getSamplerInfo(cl_sampler          sampler ,
                      cl_sampler_info     param_name ,
                      size_t              param_value_size ,
                      void *              param_value ,
                      size_t *            param_value_size_ret);

#endif // SAMPLER_H_INCLUDED

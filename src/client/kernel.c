/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2012  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
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
cl_int addkernels(cl_uint      num_kernels,
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

cl_kernel kernelFromServer(cl_kernel srv_kernel)
{
    cl_uint i;
    for(i = 0; i < num_global_kernels; i++){
        if(srv_kernel == global_kernels[i]->ptr)
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


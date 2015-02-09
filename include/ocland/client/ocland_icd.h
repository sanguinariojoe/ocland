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

// #include <CL/opencl.h>
#include <ocl_icd.h>

/** ICD platform identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_platform_id {
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_platform_id ptr;
    /// Server which has generated it
    int socket;
};

/** ICD device identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_device_id
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_device_id ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server which has generated it
    int *socket;
    /// Associated platform
    cl_platform_id platform;
};

/** ICD context identifier.
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
    /// Server which has generated it
    int *socket;
    /// Associated platform
    cl_platform_id platform;
    /// Associated number of devices
    cl_uint num_devices;
    /// Associated devices
    cl_device_id *devices;
    /// Number of context properties
    cl_uint num_properties;
    /// Context properties
    cl_context_properties *properties;
};

/** ICD command queue identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_command_queue
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_command_queue ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server which has generated it
    int *socket;
    /// Associated context
    cl_context context;
    /// Associated device
    cl_device_id device;
    /// Command queue properties
    cl_command_queue_properties properties;
};

/** Auxiliar object for the mapping requests.
 * @note It is not an OpenCL standard.
 */
struct _cl_map
{
    /// Operation flags
    cl_map_flags map_flags;
    /// Blocking flag
    cl_bool blocking;
    /// Memory map type
    cl_mem_object_type type;
    /// host pointer associated
    void *mapped_ptr;
    /// Offset (buffer map)
    size_t offset;
    /// Size (buffer map)
    size_t cb;
    /// Origin (image)
    size_t origin[3];
    /// Region (image)
    size_t region[3];
    /// Row size
    size_t row_pitch;
    /// Slice pitch
    size_t slice_pitch;
};
typedef struct _cl_map cl_map;

/** ICD memory object identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_mem
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_mem ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server which has generated it
    int *socket;
    /// Associated context
    cl_context context;
    /// Memory object type
    cl_mem_object_type type;
    /// Buffer flags
    cl_mem_flags flags;
    /// Memory object size
    size_t size;
    /// Host memory pointer associated with this object
    void *host_ptr;
    /// Map references count
    cl_uint map_count;
    /// Map operations
    cl_map *maps;
    /// Image format
    cl_image_format *image_format;
    /// Size of each element of the image
    size_t element_size;
    /// Size in bytes of a row of elements
    size_t row_pitch;
    /// Size in bytes of a 2D slice of elements
    size_t slice_pitch;
    /// Width of the image
    size_t width;
    /// Height of the image
    size_t height;
    /// Depth of the image
    size_t depth;
    /// Memory object from which this one has been created
    cl_mem mem_associated;
    /// Memory offset
    size_t offset;
};

/** ICD sampler identifier.
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
    /// Server which has generated it
    int *socket;
    /// Associated context
    cl_context context;
    /// Normalized coordinates
    cl_bool normalized_coords;
    /// Addressing mode
 	cl_addressing_mode addressing_mode;
    /// Filter mode
    cl_filter_mode filter_mode;
};

/** ICD program identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_program
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_program ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server which has generated it
    int *socket;
    /// Associated context
    cl_context context;
    /// Number of devices
    cl_uint num_devices;
    /// Devices
    cl_device_id *devices;
    /// Source code
    char* source;
    /// Binary sizes
    size_t *binary_lengths;
    /// Binaries
    unsigned char** binaries;
    /// Number of available kernels
    size_t num_kernels;
    /// Names of the kernels
    char *kernels;
};

/** Auxiliar object for the kernel arguments.
 * @note It is not an OpenCL standard.
 */
struct _cl_kernel_arg
{
    /// Argument index
    cl_uint index;
    /// Argument address (0 if it can not be readed)
    cl_kernel_arg_address_qualifier address;
    /// Argument access qualifier (0 if it can not be readed)
    cl_kernel_arg_access_qualifier access;
    /// Argument type name  (NULL if it can not be readed)
    char* type_name;
    /// Argument type qualifier (0 if it can not be readed)
    cl_kernel_arg_type_qualifier type;
    /// Argument name (NULL if it can not be readed)
    char* name;
    /// Last set argument size (0 if it has not been set yet)
    size_t bytes;
    /// Last argument value (NULL if it has not been set yet)
    void* value;
};
typedef struct _cl_kernel_arg *cl_kernel_arg;

/** ICD kernel identifier.
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
    /// Server which has generated it
    int *socket;
    /// Associated context
    cl_context context;
    /// Associated program
    cl_context program;
    /// Function name
    char* func_name;
    /// Number of input arguments
    cl_uint num_args;
    /// Arguments
    cl_kernel_arg *args;
    /// Flag to report if the kernel data has been collected
    cl_bool built;
};

/** ICD event identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_event
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_event ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
    /// Server which has generated it
    int *socket;
    /// Associated command queue
    cl_command_queue command_queue;
    /// Associated context
    cl_context context;
    /// The command associated to the event
    cl_command_type command_type;
};

#pragma GCC visibility push(hidden)

struct _cl_icd_dispatch master_dispatch;

typeof(clGetPlatformInfo) icd_clGetPlatformInfo;
typeof(clGetPlatformIDs) icd_clGetPlatformIDs;

#pragma GCC visibility pop

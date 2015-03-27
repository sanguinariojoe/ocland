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
 * @brief ICD cl_mem implementation
 *
 * cl_mem is a typedef definition:
 *
 *     typedef struct _cl_mem* cl_mem
 *
 * In this file such data structure, and all the associated methods, are
 * declared.
 * @see mem.c
 */

#ifndef MEM_H_INCLUDED
#define MEM_H_INCLUDED

#include <ocl_icd.h>
#include <ocland/client/platform_id.h>
#include <ocland/client/context.h>

/** @brief Helper object for the mapping requests.
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
typedef struct _cl_map* cl_map;

/** @brief ICD memory object identifier.
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
    /// Server where this object is allocated
    oclandServer server;
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
    /// Image descriptor
    cl_image_desc *image_desc;
    /// Size of each element of the image
    size_t element_size;
    /// Memory object from which this one has been created
    cl_mem mem_associated;
    /// Sub-buffer offset (size is already stored)
    size_t offset;
    /// Number of destruction callback functions
    cl_uint num_pfn_notify;
    /// List of destruction callback functions
    void (CL_CALLBACK **pfn_notify)(cl_mem, void*);
    /// List of user data pointers
    void **user_data;
};

/** @brief Check for memory object validity
 * @param mem Memory object to check
 * @return 1 if the memory object is a known one, 0 otherwise.
 */
int hasMem(cl_mem mem);

/** @brief Get a memory object from the server instance pointer.
 * @param srv_mem Server memory object instance
 * @return ICD memory object instance, NULL if \a srv_mem cannot be found.
 */
cl_mem memFromServer(cl_mem srv_mem);

/** @brief clCreateBuffer ocland abstraction method.
 */
cl_mem createBuffer(cl_context    context ,
                    cl_mem_flags  flags ,
                    size_t        size ,
                    void *        host_ptr ,
                    cl_int *      errcode_ret);

/** @brief clCreateSubBuffer ocland abstraction method.
 */
cl_mem createSubBuffer(cl_mem                    buffer ,
                       cl_mem_flags              flags ,
                       cl_buffer_create_type     buffer_create_type ,
                       const void *              buffer_create_info ,
                       cl_int *                  errcode_ret);

/** @brief clCreateImage ocland abstraction method.
 * @param element_size Size of each element
 */
cl_mem createImage(cl_context              context,
                   cl_mem_flags            flags,
                   const cl_image_format * image_format,
                   const cl_image_desc *   image_desc,
                   size_t                  element_size,
                   void *                  host_ptr,
                   cl_int *                errcode_ret);

/** @brief clRetainMemObject ocland abstraction method.
 */
cl_int retainMemObject(cl_mem memobj);

/** @brief clReleaseMemObject ocland abstraction method.
 */
cl_int releaseMemObject(cl_mem memobj);

/** @brief clGetSupportedImageFormats ocland abstraction method.
 */
cl_int getSupportedImageFormats(cl_context           context,
                                cl_mem_flags         flags,
                                cl_mem_object_type   image_type ,
                                cl_uint              num_entries ,
                                cl_image_format *    image_formats ,
                                cl_uint *            num_image_formats);

/** @brief clGetMemObjectInfo ocland abstraction method.
 */
cl_int getMemObjectInfo(cl_mem            memobj ,
                        cl_mem_info       param_name ,
                        size_t            param_value_size ,
                        void *            param_value ,
                        size_t *          param_value_size_ret);

/** @brief clGetImageInfo ocland abstraction method.
 */
cl_int getImageInfo(cl_mem            image ,
                    cl_image_info     param_name ,
                    size_t            param_value_size ,
                    void *            param_value ,
                    size_t *          param_value_size_ret);

/** @brief clSetMemObjectDestructorCallback ocland abstraction method.
 */
cl_int setMemObjectDestructorCallback(cl_mem  memobj,
  	                                  void (CL_CALLBACK  *pfn_notify)(cl_mem memobj,
  	                                                                  void *user_data),
  	                                  void *user_data);
#endif // MEM_H_INCLUDED

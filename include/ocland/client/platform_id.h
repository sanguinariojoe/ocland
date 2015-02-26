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
 * @brief ICD cl_platform_id implementation
 *
 * cl_platform_id is a typedef definition:
 *
 *     typedef struct _cl_platform_id* cl_platform_id
 *
 * In this file such data structure, and all the associated methods, are
 * declared.
 * In ocland, the platforms are clearly associated with the servers, which main
 * connection is managed here.
 * @see platform_id.c
 */

#ifndef PLATFORM_ID_H_INCLUDED
#define PLATFORM_ID_H_INCLUDED

#include <ocl_icd.h>
#include <ocland/common/downloadStream.h>

/// Abstraction of oclandServer_st
typedef struct oclandServer_st* oclandServer;

/** @brief Store useful data about servers.
 */
struct oclandServer_st
{
    /// Address of servers
    char* address;
    /// Socket assigned to main data transfer
    int* socket;
    /** @brief Download stream socket.
     *
     * While download stream will be created on demand, the socket used is
     * connected at the same time than oclandServer_st::socket, and kept
     * alive during the entire application execution.
     * @see oclandServer_st::download_stream
     */
    int* callbacks_socket;
    /** @brief Download stream for callback functions
     *
     * Such stream will be started as NULL, becoming created on demand, and
     * destroyed when no objects refers to it.
     */
    download_stream callbacks_stream;
};

/** @brief Return the server address for an specific socket
 * @param socket Server socket.
 * @return Server addresses. NULL if the server does not exist.
 */
const char* oclandServerAddress(int socket);

/** @brief ICD platform identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_platform_id {
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server allocated instance
    cl_platform_id ptr;
    /// Server where this platform is allocated
    oclandServer server;
    /// Number of devices inside this platform
    cl_uint num_devices;
    /// List of devices inside this platform
    cl_device_id *devices;
};

/** @brief Check for platforms validity
 * @param platform Platform to check
 * @return 1 if the platform is a known platform, 0 otherwise.
 */
int hasPlatform(cl_platform_id platform);

/** @brief Get a platform from the server instance pointer.
 * @param srv_platform Server platform instance
 * @return ICD platform instance, NULL if \a srv_platform cannot be found.
 */
cl_platform_id platformFromServer(cl_platform_id srv_platform);

/** @brief Remove a platform from the master list.
 *
 * It may be required when a platform is not supporting OpenCL 1.2, or when the
 * server has crashed.
 * @param platform platform to be removed.
 * @return CL_SUCCESS if the platform has been already discarded or
 * CL_INVALID_VALUE if the platform does not exist.
 */
cl_int discardPlatform(cl_platform_id platform);

/** @brief clGetPlatformIDs() ocland abstraction method.
 * @param dispatch Dispatching table:
 * https://www.khronos.org/registry/cl/extensions/khr/cl_khr_icd.txt
 */
cl_int getPlatformIDs(cl_uint                   num_entries,
                      struct _cl_icd_dispatch*  dispatch,
                      cl_platform_id*           platforms,
                      cl_uint*                  num_platforms);

/** @brief clGetPlatformInfo() ocland abstraction method.
 */
cl_int getPlatformInfo(cl_platform_id    platform,
                       cl_platform_info  param_name,
                       size_t            param_value_size,
                       void *            param_value,
                       size_t *          param_value_size_ret);


#endif // PLATFORM_ID_H_INCLUDED

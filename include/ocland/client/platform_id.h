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
#include <ocland/common/dataExchange.h>

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

/** @brief Build up a callbacks download stream.
 *
 * If the stream is already enabled, this function will call to
 * retainCallbackStream() and return the already generated stream.
 * @param server Server where the stream should be enabled.
 * @return Download stream, NULL if errors happened.
 */
download_stream createCallbackStream(oclandServer server);

/** @brief Get the callbacks download stream.
 * @param server Server which is connected by the stream
 * @return Download stream, NULL if it has not been started yet.
 * @see createCallbackStream();
 */
download_stream getCallbackStream(oclandServer server);

/** @brief Retain the callbacks download stream.
 *
 * It will increase its references count.
 * @param server Server where the stream should be enabled.
 * @return CL_SUCCESS if the stream can be retained, CL_INVALID_VALUE otherwise.
 */
cl_int retainCallbackStream(oclandServer server);

/** @brief Release the callbacks download stream.
 *
 * It will decrease its references count, and when it reached 0, the stream will
 * be disabled (and oclandServer_st::callbacks_stream set to NULL again).
 *
 * After such case getCallbackStream() will return NULL again.
 * @param server Server where the stream should be enabled.
 * @return CL_SUCCESS if the stream can be retained, CL_INVALID_VALUE otherwise.
 */
cl_int releaseCallbackStream(oclandServer server);

/*
 * Platforms stuff
 * ===============
 */
/** @brief ICD platform identifier.
 * @note OpenCL 2.0 extensions specification, section 9.16
 */
struct _cl_platform_id {
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server allocated instance
    pointer ptr_on_peer;
    /// Server where this platform is allocated
    oclandServer server;
    /// Number of devices inside this platform
    cl_uint num_devices;
    /// List of devices inside this platform
    cl_device_id *devices;
    /// Number of contexts inside this platform
    cl_uint num_contexts;
    /// List of contexts inside this platform
    cl_context *contexts;
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
cl_platform_id platformFromServer(pointer srv_platform);

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

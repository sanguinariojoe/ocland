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
 * @brief Upload streamer.
 *
 * Upload streamer is the mirror of the upload streamer, designed to can send
 * data in an asynchronous way.
 *
 * @see uploadStream.c
 * @see uploadStream.h
 */

#ifndef UPLOADSTREAM_H_INCLUDED
#define UPLOADSTREAM_H_INCLUDED

#include <ocl_icd.h>
#include <pthread.h>

/// Abstraction of _upload_package
typedef struct _upload_package* upload_package;

/** @brief Upload package
 *
 * The upload package contains all the required info about the data to be sent
 * to the remote peer.
 */
struct _upload_package
{
    /** Link to the next package on the queue, NULL if there are no pending
     * packages after this one.
     */
    upload_package next_package;
    /// Shared identifier with the remote peer.
    void* identifier;
    /// Data to be sent. It is instantiated, not copied.
    void *data;
    /// Ammount of data to be sent.
    size_t cb;
};

/// Abstraction of _upload_stream
typedef struct _upload_stream* upload_stream;

/** @brief Upload stream
 *
 * A list of registered tasks.
 */
struct _upload_stream
{
    /// Parallel thread to be waiting for packages
    pthread_t thread;
    /// Connection socket with the remote peer.
    int* socket;
    /** Next package to being computed.
     */
    upload_package next_package;
    /** @brief Mutex to control the access to the packages queue.
     *
     * It should be asserted that the queue is not modified while it is
     * read from another thread.
     */
    pthread_mutex_t mutex;
    /** @brief References count
     *
     * The upload streamer requires a parallel thread, that must be created
     * and destroyed on demand in order to avoid the user application hang
     * when trying to quit.
     *
     * The object will be removed when the reference count reach 0.
     */
    cl_uint rcount;
};

/** @brief Create a upload streamer.
 *
 * This method will launch the parallel thread.
 * @param Already connected socket with the server.
 * @return upload stream. NULL is returned in case of error.
 */
upload_stream createUploadStream(int *socket);

/** @brief Add a package to be sent by the upload stream.
 * @param stream Upload stream to be used to send the package.
 * @param identifier Shared identifier between the server and the client.
 * @param host_ptr Pointer to the already allocated data to be sent.
 * @param cb Size of the data to be sent.
 * @return CL_SUCCESS if the data is correctly enqueued to be sent,
 * CL_OUT_OF_HOST_MEMORY if errors happened while memory required by the
 * implementation is allocated.
 */
cl_int enqueueUploadData(upload_stream stream,
                         void* identifier,
                         void* host_ptr,
                         size_t cb);

/** @brief Increments _upload_stream::rcount.
 * @param stream Upload stream.
 * @return CL_SUCCESS.
 */
cl_int retainUploadStream(upload_stream stream);

/** @brief Decrements _upload_stream::rcount.
 *
 * When such value reachs 0, the upload stream will be destroyed.
 * @note Once stream is destroyed, pointer to it must not be used any more
 * @param stream Upload stream.
 * @return CL_SUCCESS.
 */
cl_int releaseUploadStream(upload_stream stream);

#endif // UPLOADSTREAM_H_INCLUDED

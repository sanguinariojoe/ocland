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
 * @brief Some convenient functions to send/receive data to/from remote peers.
 */

#include <ocland/common/sockets.h>

#ifndef DATAEXCHANGE_H_INCLUDED
#define DATAEXCHANGE_H_INCLUDED

/** @brief recv method reimplementation.
 *
 * This reimplementation will expect that received data matches with requested
 * data on length, otherwise connection will be closed (and socket conveniently
 * set to -1).
 * @param socket Specifies the socket file descriptor.
 * @param buffer Points to a buffer where the message should be stored.
 * @param length Specifies the length in bytes of the buffer pointed to by the
 * buffer argument.
 * @param flags Specifies the type of message reception.
 * @return 0 upon successful completion.
 * 1 either if the peer has asked for the connection close or errors are
 * detected.
 * @note if OCLAND_VERBOSE is defined the function is printing the error message
 * in the stdout.
 */
int Recv(int *socket, void *buffer, size_t length, int flags);

/** @brief send method reimplementation.
 *
 * This reimplementation will expect that sent data matches with the requested
 * by length, otherwise connection will be closed (and socket conveniently
 * set to -1).
 * @param socket Specifies the socket file descriptor.
 * @param buffer Points to the buffer containing the message to send.
 * @param length Specifies the length of the message in bytes.
 * @param flags Specifies the type of message transmission.
 * @return 0 upon successful completion.
 * 1 if errors are detected.
 * @note if OCLAND_VERBOSE is defined the function is printing the error message
 * in the stdout.
 */
int Send(int *socket, const void *buffer, size_t length, int flags);

/** @brief Checks if data is ready for reading.
 *
 * Check if data is available for reading. This function is not blocking. If error occured,
 * socket is set to -1.
 * @param socket Specifies the socket file descriptor.
 * @return 1 if data is available for reading, 0 if connection was closed by peer,
 *         -1 if no data is available or error occured.
 * @note if OCLAND_VERBOSE is defined the function is printing the error message
 * in the stdout.
 */
int CheckDataAvailable(int *socket);

/** @brief Portably receive size_t value
 * @note size_t size can be different on server and client
 */
int Recv_size_t(int *socket, size_t *val);

/** @brief Portably receive several size_t valuee
 * @note size_t size can be different on server and client
 */
int Recv_size_t_array(int *socket, size_t *val, size_t count);

/** @brief Portably send size_t value
 * @note size_t size can be different on server and client
 */
int Send_size_t(int *socket, size_t val, int flags);

/** @brief Portably send several size_t values
 * @note size_t size can be different on server and client
 */
int Send_size_t_array(int *socket, const size_t *val, size_t count, int flags);


#ifdef _MSC_VER
    #pragma pack(push,1)
    #define PACKED
#else
    #define PACKED __attribute__ ((__packed__))
#endif
/** Portable way to transform pointers between x86 and x64 machines and back is storing
 * pointers in 64 bit and packing pointer architeture and type in additional two bytes for
 * error checking
 */
typedef struct {
    unsigned char object_ptr[8]; ///< actual pointer data, 32 or 64 bits are used
    unsigned char system_arch;   ///< architecture type, 32 of 64 bits little endian
    unsigned char object_type;   ///< OpenCL object type pointer is used for
} PACKED ptr_wrapper_t;

#ifdef _MSC_VER
    #pragma pack(pop)
#endif
#undef PACKED

/// Host architecture
typedef enum {
    PTR_ARCH_UNSET,
    PTR_ARCH_LE32, ///< little endian, 32 bit
    PTR_ARCH_LE64, ///< little endian, 64 bit
} ptr_arch_t;

/// Object type pointer is used for
typedef enum {
    PTR_TYPE_UNSET,
    PTR_TYPE_PLATFORM,
    PTR_TYPE_DEVICE,
    PTR_TYPE_CONTEXT,
    PTR_TYPE_COMMAND_QUEUE,
    PTR_TYPE_MEM,
    PTR_TYPE_PROGRAM,
    PTR_TYPE_KERNEL,
    PTR_TYPE_EVENT,
    PTR_TYPE_SAMPLER
} ptr_type_t;

/// Receive our memory space pointer from peer
int Recv_pointer(int *socket, ptr_type_t ptr_type, void **val);

/// Receive pointer wrapper object from peer
///
/// This could either our address space pointer wrapped either peer
/// addrfes space pointer wrapped
int Recv_pointer_wrapper(int *socket, ptr_type_t ptr_type, ptr_wrapper_t *val);

/// Send our memory space pointer to peer
int Send_pointer(int *socket, ptr_type_t ptr_type, const void *val, int flags);

/// Send pointer wrapper object to peer
///
/// This could either our address space pointer wrapped either peer
/// addrfes space pointer wrapped
int Send_pointer_wrapper(int *socket, ptr_type_t ptr_type, ptr_wrapper_t val, int flags);

/// Get current host architecture
/// @todo check if we are little endian
ptr_arch_t Get_current_arch();

/// Check if two pointer wrappers are equal
int equal_ptr_wrappers(ptr_wrapper_t a, ptr_wrapper_t b);

/// Check if two pointer wrapper is NULL
int is_null_ptr_wrapper(ptr_wrapper_t val);

/// Set pointer wrapper as NULL
void set_null_ptr_wrapper(ptr_wrapper_t *val);

#endif // DATAEXCHANGE_H_INCLUDED

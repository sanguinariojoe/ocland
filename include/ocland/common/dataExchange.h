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

#endif // DATAEXCHANGE_H_INCLUDED

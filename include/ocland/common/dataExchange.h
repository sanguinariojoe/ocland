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

#ifndef DATAEXCHANGE_H_INCLUDED
#define DATAEXCHANGE_H_INCLUDED

/** Returns the last socket error detected
 * @return Error detected.
 */
const char* SocketsError();

/** recv method reimplementation. This reimplementation will expect that received data matchs
 * with request data on length, if not (errors or mismatch) connection will closed (and socket
 * conviniently set to -1).
 * @param socket Specifies the socket file descriptor.
 * @param buffer Points to a buffer where the message should be stored.
 * @param length Specifies the length in bytes of the buffer pointed to by the buffer argument.
 * @param flags Specifies the type of message reception.
 * @return Upon successful completion, Recv() shall return the length of the message in bytes.
 * If no messages are available to be received and the peer has performed an orderly shutdown,
 * Recv() shall return 0. Otherwise, -1 shall be returned and errno set to indicate the error.
 */
int Recv(int *socket, void *buffer, size_t length, int flags);

/** send method reimplementation. This reimplementation will expect that sent data matchs
 * with request data on length, if not (errors or mismatch) connection will closed (and socket
 * conviniently set to -1).
 * @param socket Specifies the socket file descriptor.
 * @param buffer Points to the buffer containing the message to send.
 * @param length Specifies the length of the message in bytes.
 * @param flags Specifies the type of message transmission.
 * @return Upon successful completion, Send() shall return the number of bytes sent. Otherwise,
 * -1 shall be returned and errno set to indicate the error.
 */
int Send(int *socket, const void *buffer, size_t length, int flags);

#endif // DATAEXCHANGE_H_INCLUDED

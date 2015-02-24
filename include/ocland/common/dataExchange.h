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
 * @return Upon successful completion, the length of the message in bytes.
 * 0 if no messages are available to be received and the peer has asked for the
 * connection close.
 * -1 in case of errors.
 * @note if DATA_EXCHANGE_VERBOSE is defined the function is printing the error
 * message in the screen.
 */
ssize_t Recv(int *socket, void *buffer, size_t length, int flags, int verbose);

/** @brief send method reimplementation.
 *
 * This reimplementation will expect that sent data matches with the requested
 * by length, otherwise connection will be closed (and socket conveniently
 * set to -1).
 * @param socket Specifies the socket file descriptor.
 * @param buffer Points to the buffer containing the message to send.
 * @param length Specifies the length of the message in bytes.
 * @param flags Specifies the type of message transmission.
 * @return Upon successful completion, the length of the message in bytes.
 * -1 in case of errors.
 * @note if DATA_EXCHANGE_VERBOSE is defined the function is printing the error
 * message in the screen.
 */
ssize_t Send(int *socket, const void *buffer, size_t length, int flags);

#define ssend(socket, buffer, length, flags) {printf("[line %d]: %s...\n", __LINE__, __func__); fflush(stdout);}
    #define VERBOSE_OUT(flag) {printf("\t%s -> %s\n", __func__, OpenCLError(flag)); fflush(stdout);}
    #define VERBOSE(...) {printf(__VA_ARGS__); fflush(stdout);}
#else
    #define VERBOSE_IN()
    #define VERBOSE_OUT(flag)
    #define VERBOSE(...)
#endif

#endif // DATAEXCHANGE_H_INCLUDED

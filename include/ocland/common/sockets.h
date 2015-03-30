/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2015  Timur Magomedov <tim239@yandex.ru>
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
 * @brief Network sockets OS compatibility helper file.
 *
 * This file provides a set of includes and defines to provide uniform access
 * to system-specific sockets API headers.
 */

#ifndef SOCKETS_H_INCLUDED
#define SOCKETS_H_INCLUDED

#ifdef WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h> // inet_pton()

    #define MSG_MORE 0     // ignore on win32

    #ifndef MSG_WAITALL
        //is undefined on MINGW system headers
        #define MSG_WAITALL 0x8
    #endif

    #ifndef ECONNREFUSED
        // is undefined on MINGW system headers
        #define ECONNREFUSED    107
    #endif

    #ifndef close
        static int close(int fd) { return closesocket(fd); }
    #endif

    #ifndef inet_pton
        // For now, inet_pton function is missed in MinGW ws2tcpip.h header - see MinGW bug #1641
        INT WSAAPI inet_pton(INT  Family, CONST CHAR * pszAddrString, PVOID pAddrBuf);
    #endif

    typedef int ssize_t;
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#endif /* SOCKETS_H_INCLUDED */

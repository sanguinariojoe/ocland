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

#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <ocland/common/dataExchange.h>

const char* SocketsError()
{
    static char str[256];
    strcpy(str, "");

    switch(errno){
        case EPERM:
            strcpy(str, "Not owner");
            break;
        case ENOENT:
            strcpy(str, "No such file");
            break;
        case ESRCH:
            strcpy(str, "No such process");
            break;
        case EINTR:
            strcpy(str, "Interrupted system");
            break;
        case EIO:
            strcpy(str, "I/O error");
            break;
        case ENXIO:
            strcpy(str, "No such device");
            break;
        case E2BIG:
            strcpy(str, "Argument list too long");
            break;
        case ENOEXEC:
            strcpy(str, "Exec format error");
            break;
        case EBADF:
            strcpy(str, "Bad file number");
            break;
        case ECHILD:
            strcpy(str, "No children ");
            break;
        /*
        case EAGAIN:
            strcpy(str, "No more processes");
            break;
        case EWOULDBLOCK:
            strcpy(str, "Operation would block");
            break;
        */
        case EWOULDBLOCK:
            strcpy(str, "No more processes / Operation would block");
            break;
        case ENOMEM:
            strcpy(str, "Not enough core");
            break;
        case EACCES:
            strcpy(str, "Permission denied");
            break;
        case EFAULT:
            strcpy(str, "Bad address");
            break;
        case ENOTBLK:
            strcpy(str, "Block device required");
            break;
        case EBUSY:
            strcpy(str, "Mount device busy");
            break;
        case EEXIST:
            strcpy(str, "File exists");
            break;
        case EXDEV:
            strcpy(str, "Cross-device link");
            break;
        case ENODEV:
            strcpy(str, "No such device");
            break;
        case ENOTDIR:
            strcpy(str, "Not a directory");
            break;
        case EISDIR:
            strcpy(str, "Is a directory");
            break;
        case EINVAL:
            strcpy(str, "Invalid argument");
            break;
        case ENFILE:
            strcpy(str, "File table overflow");
            break;
        case EMFILE:
            strcpy(str, "Too many opened files");
            break;
        case ENOTTY:
            strcpy(str, "Not a typewriter");
            break;
        case ETXTBSY:
            strcpy(str, "Text file busy");
            break;
        case EFBIG:
            strcpy(str, "File too large");
            break;
        case ENOSPC:
            strcpy(str, "No space left on");
            break;
        case ESPIPE:
            strcpy(str, "Illegal seek");
            break;
        case EROFS:
            strcpy(str, "Read-only file system");
            break;
        case EMLINK:
            strcpy(str, "Too many links");
            break;
        case EPIPE:
            strcpy(str, "Broken pipe");
            break;
        case EINPROGRESS:
            strcpy(str, "Operation now in progress");
            break;
        case EALREADY:
            strcpy(str, "Operation already in progress");
            break;
        case ENOTSOCK:
            strcpy(str, "Socket operation on");
            break;
        case EDESTADDRREQ:
            strcpy(str, "Destination address required");
            break;
        case EMSGSIZE:
            strcpy(str, "Message too long");
            break;
        case EPROTOTYPE:
            strcpy(str, "Protocol wrong type");
            break;
        case ENOPROTOOPT:
            strcpy(str, "Protocol not available");
            break;
        case EPROTONOSUPPORT:
            strcpy(str, "Protocol not supported");
            break;
        case ESOCKTNOSUPPORT:
            strcpy(str, "Socket type not supported");
            break;
        case EOPNOTSUPP:
            strcpy(str, "Operation not supported");
            break;
        case EPFNOSUPPORT:
            strcpy(str, "Protocol family not supported");
            break;
        case EAFNOSUPPORT:
            strcpy(str, "Address family not supported");
            break;
        case EADDRINUSE:
            strcpy(str, "Address already in use");
            break;
        case EADDRNOTAVAIL:
            strcpy(str, "Can't assign requested address");
            break;
        case ENETDOWN:
            strcpy(str, "Network is down");
            break;
        case ENETUNREACH:
            strcpy(str, "Network is unreachable");
            break;
        case ENETRESET:
            strcpy(str, "Network dropped connection");
            break;
        case ECONNABORTED:
            strcpy(str, "Software caused connection");
            break;
        case ECONNRESET:
            strcpy(str, "Connection reset by peer");
            break;
        case ENOBUFS:
            strcpy(str, "No buffer space available");
            break;
        case EISCONN:
            strcpy(str, "Socket is already connected");
            break;
        case ENOTCONN:
            strcpy(str, "Socket is not connected");
            break;
        case ESHUTDOWN:
            strcpy(str, "Can't send after shutdown");
            break;
        case ETOOMANYREFS:
            strcpy(str, "Too many references");
            break;
        case ETIMEDOUT:
            strcpy(str, "Connection timed out");
            break;
        case ECONNREFUSED:
            strcpy(str, "Connection refused");
            break;
        case ELOOP:
            strcpy(str, "Too many levels of nesting");
            break;
        case ENAMETOOLONG:
            strcpy(str, "File name too long");
            break;
        case EHOSTDOWN:
            strcpy(str, "Host is down");
            break;
        case EHOSTUNREACH:
            strcpy(str, "No route to host");
            break;
        case ENOTEMPTY:
            strcpy(str, "Directory not empty");
            break;
        /*
        case EPROCLIM:
            strcpy(str, "Too many processes");
            break;
        */
        case EUSERS:
            strcpy(str, "Too many users");
            break;
        case EDQUOT:
            strcpy(str, "Disc quota exceeded");
            break;
        case ESTALE:
            strcpy(str, "Stale NFS file handle");
            break;
        case EREMOTE:
            strcpy(str, "Too many levels of remote in the path");
            break;
        case ENOSTR:
            strcpy(str, "Device is not a stream");
            break;
        case ETIME:
            strcpy(str, "Timer expired");
            break;
        case ENOSR:
            strcpy(str, "Out of streams resources");
            break;
        case ENOMSG:
            strcpy(str, "No message");
            break;
        case EBADMSG:
            strcpy(str, "Trying to read unreadable message");
            break;
        case EIDRM:
            strcpy(str, "Identifier removed");
            break;
        case EDEADLK:
            strcpy(str, "Deadlock condition");
            break;
        case ENOLCK:
            strcpy(str, "No record locks available");
            break;
        case ENONET:
            strcpy(str, "Machine is not on network");
            break;
        /*
        case ERREMOTE:
            strcpy(str, "Object is remote");
            break;
        */
        case ENOLINK:
            strcpy(str, "The link has been severed");
            break;
        case EADV:
            strcpy(str, "ADVERTISE error");
            break;
        case ESRMNT:
            strcpy(str, "SRMOUNT error");
            break;
        case ECOMM:
            strcpy(str, "Communication error");
            break;
        case EPROTO:
            strcpy(str, "Protocol error");
            break;
        case EMULTIHOP:
            strcpy(str, "Multihop attempted");
            break;
        case EDOTDOT:
            strcpy(str, "Cross mount point");
            break;
        case EREMCHG:
            strcpy(str, "Remote address change");
            break;
        /*
        case EIBMBADCALL:
            strcpy(str, "A bad socket-call constant was found in the IUCV header");
            break;
        case EIBMBADPARM:
            strcpy(str, "Other IUCV header error, bad length, etc.");
            break;
        case EIBMSOCKOUTOFRANGE:
            strcpy(str, "Socket number assigned by client interface code (for socket() and accept()) is out of range");
            break;
        case EIBMSOCKINUSE:
            strcpy(str, "Socket number assigned by client interface code is already in use");
            break;
        case EIBMIUCVERR:
            strcpy(str, "Request failed due to IUCV error");
            break;
        case EIBMCONFLICT:
            strcpy(str, "Conflicting call already outstanding on socket");
            break;
        case EIBMCANCELLED:
            strcpy(str, "Request cancelled via socket function CANCEL request");
            break;
        */
    }
    return str;
}

ssize_t Recv(int *socket, void *buffer, size_t length, int flags)
{
    if(*socket < 0)
        return 0;
    // Compare data with the buffer length in order to switch on/off
    // the Nagle's algorithm.
    int tcp_nodelay_flag = 1;
    int buffsize=0;
    getsockopt(*socket, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(int));
    if((int)length >= buffsize){
        tcp_nodelay_flag = 0;
    }
    setsockopt(*socket, SOL_SOCKET, TCP_NODELAY, &tcp_nodelay_flag, sizeof(int));
    // Receive the data
    ssize_t readed = recv(*socket, buffer, length, flags);
    /*
    if(readed != length){
        #ifdef OCLAND_LOG_VERBOSE
            struct sockaddr_in adr_inet;
            socklen_t len_inet;
            len_inet = sizeof(adr_inet);
            getsockname(*socket, (struct sockaddr*)&adr_inet, &len_inet);
            printf("Data transfered from %s mismatchs, ", inet_ntoa(adr_inet.sin_addr));
            printf("disconnected (flow broken).\n"); fflush(stdout);
            printf("* %lu bytes requested, %li readed\n", length, readed);
        #endif
        close(*socket);
        *socket = -1;
    }
    */
    return readed;
}

ssize_t Send(int *socket, const void *buffer, size_t length, int flags)
{
    if(*socket < 0)
        return 0;
    // Compare data with the buffer length in order to switch on/off
    // the Nagle's algorithm.
    int tcp_nodelay_flag = 1;
    int buffsize=0;
    getsockopt(*socket, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(int));
    if((int)length >= buffsize){
        tcp_nodelay_flag = 0;
    }
    setsockopt(*socket, SOL_SOCKET, TCP_NODELAY, &tcp_nodelay_flag, sizeof(int));
    // Send the data
    ssize_t sent = send(*socket, buffer, length, flags);
    /*
    if(sent != length){
        #ifdef OCLAND_LOG_VERBOSE
            struct sockaddr_in adr_inet;
            socklen_t len_inet;
            len_inet = sizeof(adr_inet);
            getsockname(*socket, (struct sockaddr*)&adr_inet, &len_inet);
            printf("Data transfered to %s mismatchs, ", inet_ntoa(adr_inet.sin_addr));
            printf("client disconnected (flow broken).\n"); fflush(stdout);
        #endif
        close(*socket);
        *socket = -1;
    }
    */
    return sent;
}

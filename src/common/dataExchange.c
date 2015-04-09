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
 * @see dataExchange.h
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/common/dataExchange.h>
#include <ocland/common/verbose.h>

int Recv(int *socket, void *buffer, size_t length, int flags)
{
    if(*socket < 0)
        return 0;
    // Compare data with the buffer length in order to switch on/off
    // the Nagle's algorithm.
    /*
    int switch_on = 1;
    int buffsize=0;
    getsockopt(*socket, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(int));
    if((int)length >= buffsize){
        switch_on = 0;
    }
    setsockopt(*socket, SOL_SOCKET, TCP_NODELAY, &switch_on, sizeof(int));
    */
    // Receive the data
    ssize_t readed = recv(*socket, buffer, length, flags);
    if(readed <= 0){
        #ifdef OCLAND_VERBOSE
            #ifdef WIN32
                int error_code = WSAGetLastError();
            #else
                int error_code = errno;
            #endif
            struct sockaddr_in adr_inet;
            socklen_t len_inet = sizeof(adr_inet);
            getpeername(*socket, (struct sockaddr*)&adr_inet, &len_inet);
            printf("Failure receiving data from %s:\n",
                   inet_ntoa(adr_inet.sin_addr));
            if (readed == 0) {
                printf("\tRemote peer asked to shutdown the connection\n");
            }
            else {
                #ifdef WIN32
                    printf("\t%d\n", error_code);
                #else
                    printf("\t%s\n", strerror(error_code));
                #endif
            }
            printf("Closing the connection...\n");
            if(shutdown(*socket, 2)){
                printf("Connection shutdown failed: %s\n", strerror(errno));
            }
            fflush(stdout);
        #else
            shutdown(*socket, 2);
        #endif
        *socket = -1;
        return 1;
    }
    return 0;
}

int Send(int *socket, const void *buffer, size_t length, int flags)
{
    if(*socket < 0)
        return 0;
    // Compare data with the buffer length in order to switch on/off
    // the Nagle's algorithm.
    /*
    int tcp_nodelay_flag = 1;
    int buffsize=0;
    getsockopt(*socket, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(int));
    if((int)length >= buffsize){
        tcp_nodelay_flag = 0;
    }
    setsockopt(*socket, SOL_SOCKET, TCP_NODELAY, &tcp_nodelay_flag, sizeof(int));
    */
    // Send the data
    ssize_t sent = send(*socket, buffer, length, flags);
    if(sent != length){
        #ifdef OCLAND_VERBOSE
            #ifdef WIN32
                int error_code = WSAGetLastError();
            #else
                int error_code = errno;
            #endif
            struct sockaddr_in adr_inet;
            socklen_t len_inet = sizeof(adr_inet);
            getpeername(*socket, (struct sockaddr*)&adr_inet, &len_inet);
            printf("Failure sending data to %s:\n",
                   inet_ntoa(adr_inet.sin_addr));
            #ifdef WIN32
                printf("\t%d\n", error_code);
            #else
                printf("\t%s\n", strerror(error_code));
            #endif
            printf("Closing the connection...\n");
            if(shutdown(*socket, 2)){
                printf("Connection shutdown failed: %s\n", strerror(errno));
            }
            fflush(stdout);
        #else
            shutdown(*socket, 2);
        #endif
        *socket = -1;
        return 1;
    }
    return 0;
}

int CheckDataAvailable(int *socket)
{
    if(*socket < 0)
        return -1;
#ifdef WIN32
    WSAPOLLFD fds = { 0 };
    fds.fd = *socket;
    fds.events = POLLIN;

    int flag = WSAPoll(&fds, 1, 0);
    if (fds.revents & POLLHUP) {
        // disconnected
        return 0;
    }
    if (flag == 0) {
        // no data ready, return -1 but do not shutdown socket
        return -1;
    }
    if (flag == SOCKET_ERROR) {
        // some error, shutdown socket
        #ifdef OCLAND_VERBOSE
            int error_code = WSAGetLastError();
            struct sockaddr_in adr_inet;
            socklen_t len_inet = sizeof(adr_inet);
            getpeername(*socket, (struct sockaddr*)&adr_inet, &len_inet);
            printf("Failure checking available data from %s:\n",
                inet_ntoa(adr_inet.sin_addr));
            printf("\t%d\n", error_code);

            printf("Closing the connection...\n");
            if (shutdown(*socket, 2)){
                printf("Connection shutdown failed: %s\n", WSAGetLastError());
            }
            fflush(stdout);
        #else
            shutdown(*socket, 2);
        #endif
        *socket = -1;
    }
    return flag;
#else
    char data;
    ssize_t flag = recv(*socket, &data, sizeof(data), MSG_DONTWAIT | MSG_PEEK);
    if (flag < 0) {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
            #ifdef OCLAND_VERBOSE
                int error_code = errno;
                struct sockaddr_in adr_inet;
                socklen_t len_inet = sizeof(adr_inet);
                getpeername(*socket, (struct sockaddr*)&adr_inet, &len_inet);
                printf("Failure checking available data from %s:\n",
                        inet_ntoa(adr_inet.sin_addr));
                printf("\t%s\n", strerror(error_code));
                printf("Closing the connection...\n");
                if(shutdown(*socket, 2)){
                    printf("Connection shutdown failed: %s\n", strerror(errno));
                }
                fflush(stdout);
            #else
                shutdown(*socket, 2);
            #endif
            *socket = -1;
        }
    }
    return flag;
#endif
}

pointer StorePtr(void* ptr)
{
    assert((sizeof(ptr) == 4) || (sizeof(ptr) == 8));
    assert(sizeof(pointer) == 8);

    pointer packed = 0;
    memcpy(&packed, &ptr, sizeof(ptr));
    return packed;
}

void * RestorePtr(pointer packed_ptr)
{
    void * ptr = NULL;
    memcpy(&ptr, &packed_ptr, sizeof(ptr));
    return ptr;
}

#ifdef _MSC_VER
    typedef unsigned __int64 size64;
#else
    typedef uint64_t size64;
#endif

int Recv_size_t(int *socket, size_t *val)
{
    size64 val64 = *val;
    int ret = Recv(socket, &val64, sizeof(size64), MSG_WAITALL);
    *val = (size_t)val64;
    return ret;
}

int Recv_size_t_array(int *socket, size_t *val, size_t count)
{
    unsigned int i;
    int ret = 0;
    if (sizeof(size64) == sizeof(size_t)) {
        // we are 64 bit platform - receive data unmodified
        return Recv(socket, val, count * sizeof(size_t), MSG_WAITALL);
    }
    // we are 32 bit platform with 32 bit size_t - receive
    // them as 64 bit integers and move to 32 bit integers
    size64 * val64 = calloc(count, sizeof(size64));
    if (val64) {
        ret = Recv(socket, val64, count * sizeof(size64), MSG_WAITALL);
        for (i = 0; i < count; i++) {
            val[i] = (size_t)val64[i];
        }
        free(val64);
        return ret;
    }
    else {
        // not enought memory - proceed anyway
        // receive values one by one
        for (i = 0; i < count; i++) {
            ret |= Recv_size_t(socket, val + i);
        }
        return ret;
    }
}

int Send_size_t(int *socket, size_t val, int flags)
{
    size64 val64 = val;
    return Send(socket, &val64, sizeof(size64), flags);
}

int Send_size_t_array(int *socket, const size_t *val, size_t count, int flags)
{
    unsigned int i;
    int ret = 0;
    assert((flags == MSG_MORE) || (flags == 0));
    if (sizeof(size64) == sizeof(size_t)) {
        // we are 64 bit platform - send data unmodified
        return Send(socket, val, count * sizeof(size_t), flags);
    }
    // we are 32 bit platform with 32 bit size_t - move
    // them to 64 bit integers and send
    size64 * val64 = malloc(count * sizeof(size64));
    if (val64) {
        for (i = 0; i < count; i++) {
            val64[i] = val[i];
        }
        ret = Send(socket, val64, count * sizeof(size64), flags);
        free(val64);
        return ret;
    }
    else {
        // not enought memory - proceed anyway
        // send values one by one
        for (i = 0; i < count; i++) {
            int current_flag = (i == count - 1) ? flags : MSG_MORE;
            ret |= Send_size_t(socket, val[i], current_flag);
        }
        return ret;
    }
}

/// Get current host architecture
/// @todo check if we are little endian
static ptr_arch_t Get_current_arch()
{
    // other variants can be added
    // for now, two arches are supported
    if (sizeof(void*) == 4) {
        return PTR_ARCH_LE32;
    } else if (sizeof(void*) == 8) {
        return PTR_ARCH_LE64;
    } else {
        assert(0);
        return PTR_ARCH_UNSET;
    }
}

int equal_ptr_wrappers(ptr_wrapper_t a, ptr_wrapper_t b)
{
    return !memcmp(&a, &b, sizeof(ptr_wrapper_t));
}

int Recv_pointer_wrapper(int *socket, ptr_type_t ptr_type, ptr_wrapper_t *val)
{
    ptr_wrapper_t ptr_wrapper;
    memset(&ptr_wrapper, 0, sizeof(ptr_wrapper_t));
    *val = ptr_wrapper;
    int ret = Recv(socket, &ptr_wrapper, sizeof(ptr_wrapper_t), MSG_WAITALL);
    if (ret != 0) {
        return ret;
    }
    // system arch of peer can be different - no problem
    // but pointer object type must be the one we expect - protocol is broken otherwise
    ptr_type_t obj_type = ptr_wrapper.object_type;
    if (ptr_type != obj_type) {
        return -1;
    }
    *val = ptr_wrapper;
    return 0;
}

int Recv_pointer(int *socket, ptr_type_t ptr_type, void **val)
{
    ptr_wrapper_t ptr_wrapper;
    memset(&ptr_wrapper, 0, sizeof(ptr_wrapper_t));
    *val = NULL;
    int ret = Recv(socket, &ptr_wrapper, sizeof(ptr_wrapper_t), MSG_WAITALL);
    if (ret != 0) {
        return ret;
    }
    // This should be pointer from our memory space - check arch
    ptr_arch_t arch_type = ptr_wrapper.system_arch;
    if ((arch_type != PTR_ARCH_UNSET) && (Get_current_arch() != arch_type)) {
        return -1;
    }
    // Pointer object type must be the one we expect - protocol is broken otherwise
    ptr_type_t obj_type = ptr_wrapper.object_type;
    if ((ptr_type != PTR_TYPE_UNSET) && (ptr_type != obj_type)) {
        return -1;
    }
    memcpy(val, ptr_wrapper.object_ptr, sizeof(void*));
    return 0;
}

int Send_pointer(int *socket, ptr_type_t ptr_type, void *val, int flags)
{
    ptr_wrapper_t ptr_wrapper;
    memset(&ptr_wrapper, 0, sizeof(ptr_wrapper_t));
    memcpy(ptr_wrapper.object_ptr, &val, sizeof(void*));
    ptr_wrapper.system_arch = Get_current_arch();
    ptr_wrapper.object_type = ptr_type;
    return Send(socket, &ptr_wrapper, sizeof(ptr_wrapper_t), flags);
}

int Send_pointer_wrapper(int *socket, ptr_type_t ptr_type, ptr_wrapper_t val, int flags)
{
    if (val.object_type != ptr_type) {
        return -1;
    }
    return Send(socket, &val, sizeof(ptr_wrapper_t), flags);
}

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

ssize_t Recv(int *socket, void *buffer, size_t length, int flags)
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
    if(readed != length){
        #ifdef DATA_EXCHANGE_VERBOSE
            struct sockaddr_in adr_inet;
            socklen_t len_inet = sizeof(adr_inet);
            getsockname(*socket, (struct sockaddr*)&adr_inet, &len_inet);
            printf("Failure receiving data from %s:\n",
                   inet_ntoa(adr_inet.sin_addr));
            if(readed = 0)
                printf("\tRemote peer asked to shutdown the connection\n");
            else
                printf("\t%s", strerror(errno));
            printf("Closing the connection...\n")
            if(shutdown(*socket, 2)){
                printf("Connection shutdown failed: %s\n", strerror(errno));
            }
            fflush(stdout);
        #else
            shutdown(*socket, 2);
        #endif
        *socket = -1;
    }
    return readed;
}

ssize_t Send(int *socket, const void *buffer, size_t length, int flags)
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
        #ifdef DATA_EXCHANGE_VERBOSE
            struct sockaddr_in adr_inet;
            socklen_t len_inet = sizeof(adr_inet);
            getsockname(*socket, (struct sockaddr*)&adr_inet, &len_inet);
            printf("Failure sending data to %s:\n",
                   inet_ntoa(adr_inet.sin_addr));
            printf("\t%s", strerror(errno));
            printf("Closing the connection...\n")
            if(shutdown(*socket, 2)){
                printf("Connection shutdown failed: %s\n", strerror(errno));
            }
            fflush(stdout);
        #else
            shutdown(*socket, 2);
        #endif
        *socket = -1;
    }
    return sent;
}

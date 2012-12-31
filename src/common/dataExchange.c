/*
 *  This file is part of ocland, a free CFD program based on SPH.
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
#include <netinet/in.h>
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
    ssize_t readed = recv(*socket, buffer, length, flags);
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
    return readed;
}

ssize_t Send(int *socket, const void *buffer, size_t length, int flags)
{
    if(*socket < 0)
        return 0;
    ssize_t sent = send(*socket, buffer, length, flags);
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
    return sent;
}

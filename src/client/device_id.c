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
 * @brief ICD cl_device_id implementation
 * @see device_id.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <ocland/common/sockets.h>
#include <ocland/client/commands_enum.h>
#include <ocland/common/verbose.h>
#include <ocland/client/device_id.h>
#include <ocland/common/dataExchange.h>

/// Number of known devices
cl_uint num_global_devices = 0;
/// List of known devices
cl_device_id *global_devices = NULL;

int hasDevice(cl_device_id device){
    cl_uint i;
    for(i = 0; i < num_global_devices; i++){
        if(device == global_devices[i])
            return 1;
    }
    return 0;
}

/** @brief Add a set of devices to the global devices list global_devices.
 *
 * The devices will be added to the platform list as well.
 *
 * This method is not checking if the devices are already present in the list,
 * however, accidentally adding the same device several times will only imply
 * performance penalties.
 * @param num_devices Number of devices to append.
 * @param devices Devices to append.
 * @return CL_SUCCESS if the devices are already generated, an error code
 * otherwise.
 * @warning It is guessed that all the devices of the list \a devices belongs to
 * the same platform.
 */
cl_int addDevices(cl_uint        num_devices,
                  cl_device_id*  devices)
{
    if(!num_devices)
        return CL_SUCCESS;

    // Add the devices to the global list
    cl_device_id *backup_devices = global_devices;

    global_devices = (cl_device_id*)malloc(
        (num_global_devices + num_devices) * sizeof(cl_device_id));
    if(!global_devices){
        VERBOSE("Failure allocating memory for %u devices!\n",
                num_global_devices + num_devices);
        free(backup_devices); backup_devices = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_devices){
        memcpy(global_devices,
               backup_devices,
               num_global_devices * sizeof(cl_device_id));
        free(backup_devices); backup_devices = NULL;
    }

    memcpy(&(global_devices[num_global_devices]),
           devices,
           num_devices * sizeof(cl_device_id));
    num_global_devices += num_devices;

    // Add the devices to the platform list
    cl_platform_id platform = devices[0]->platform;
    backup_devices = platform->devices;

    platform->devices = (cl_device_id*)malloc(
        (platform->num_devices + num_devices) * sizeof(cl_device_id));
    if(!platform->devices){
        VERBOSE("Failure allocating memory for %u devices!\n",
                platform->num_devices + num_devices);
        free(backup_devices); backup_devices = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    if(backup_devices){
        memcpy(platform->devices,
               backup_devices,
               platform->num_devices * sizeof(cl_device_id));
        free(backup_devices); backup_devices = NULL;
    }

    memcpy(&(platform->devices[platform->num_devices]),
           devices,
           num_devices * sizeof(cl_device_id));
    platform->num_devices += num_devices;

    return CL_SUCCESS;
}

/** @brief Get the device index in the global list
 * @param device Device to look for
 * @return Index of the device, num_global_devices if it is not found.
 */
cl_uint deviceIndex(cl_device_id device)
{
    cl_uint i;
    for(i = 0; i < num_global_devices; i++){
        if(device == global_devices[i])
            break;
    }
    return i;
}

cl_device_id deviceFromServer(pointer srv_device)
{
    cl_uint i;
    for(i = 0; i < num_global_devices; i++){
        if(srv_device == global_devices[i]->ptr_on_peer)
            return global_devices[i];
    }
    return NULL;
}

/** @brief Get the device index in the platform list
 * @param device Device to look for
 * @return Index of the device, _cl_platform_id::num_devices if it is not found.
 */
cl_uint deviceInPlatformIndex(cl_device_id device)
{
    cl_uint i;
    cl_platform_id platform = device->platform;
    for(i = 0; i < platform->num_devices; i++){
        if(device == platform->devices[i])
            break;
    }
    return i;
}

/** @brief Remove a device from the global list, and from the platform.
 *
 * For instance when clReleaseDevice() is called.
 * @param device device to be removed.
 * @return CL_SUCCESS if the device has been already discarded or
 * CL_INVALID_VALUE if the device does not exist.
 */
cl_int discardDevice(cl_device_id device)
{
    if(!hasDevice(device)){
        return CL_INVALID_VALUE;
    }
    cl_uint i, index;

    // Remove the device stuff
    index = deviceIndex(device);
    free(global_devices[index]);

    assert(num_global_devices > 0);
    // Remove the device from the global list
    for(i = index; i < num_global_devices - 1; i++){
        global_devices[i] = global_devices[i + 1];
    }
    num_global_devices--;
    global_devices[num_global_devices] = NULL;

    // Remove the device from the platform list
    index = deviceInPlatformIndex(device);
    cl_platform_id platform = device->platform;
    assert(platform->num_devices > 0);
    for(i = index; i < platform->num_devices - 1; i++){
        platform->devices[i] = platform->devices[i + 1];
    }
    platform->num_devices--;
    // free(platform->devices[platform->num_devices]);  // Already removed
    platform->devices[platform->num_devices] = NULL;

    return CL_SUCCESS;
}


/** @brief Get the devices from a platform.
 *
 * Also the devices are appended to the global devices list global_devices.
 * @param platform Platform from where the devices should be extracted.
 * @return CL_SUCCESS if the devices are already generated, an error code
 * otherwise.
 */
cl_int initDevices(cl_platform_id platform)
{
    unsigned int i, j;
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    cl_device_type device_type = CL_DEVICE_TYPE_ALL;
    cl_uint num_devices=0;
    cl_device_id *devices = NULL;
    pointer *devices_srv = NULL;
    unsigned int comm = ocland_clGetDeviceIDs;

    int *sockfd = platform->server->socket;
    if(!sockfd){
        return CL_INVALID_DEVICE;
    }

    // Get the number of devices
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(platform->ptr_on_peer), sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &device_type, sizeof(cl_device_type), MSG_MORE);
    socket_flag |= Send(sockfd, &num_devices, sizeof(cl_uint), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    else if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv(sockfd, &num_devices, sizeof(cl_uint), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }

    // Get the list of devices instances in the server
    devices_srv = malloc(num_devices * sizeof(pointer));
    if(!devices_srv){
        VERBOSE("Failure allocating memory for the server device instances!\n");
        return CL_OUT_OF_HOST_MEMORY;
    }
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(platform->ptr_on_peer), sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &device_type, sizeof(cl_device_type), MSG_MORE);
    socket_flag |= Send(sockfd, &num_devices, sizeof(cl_uint), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    else if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv(sockfd, &num_devices, sizeof(cl_uint), MSG_WAITALL);
    socket_flag |= Recv(sockfd, devices_srv, num_devices*sizeof(pointer), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }

    // Create the device objects
    devices = (cl_device_id*)malloc(num_devices * sizeof(cl_device_id));
    if(!devices){
        VERBOSE("Failure allocating memory for the devices list!\n");
        return CL_OUT_OF_HOST_MEMORY;
    }
    for(i = 0; i < num_devices; i++){
        devices[i] = (cl_device_id)malloc(sizeof(struct _cl_device_id));
        if(!devices[i]){
            VERBOSE("Failure allocating memory for the device %u!\n", i);
            for(j = 0; j < i; j++){
                free(devices[j]); devices[j] = NULL;
            }
            free(devices); devices = NULL;
            free(devices_srv); devices_srv = NULL;
            return CL_OUT_OF_HOST_MEMORY;
        }
        devices[i]->dispatch = platform->dispatch;
        devices[i]->ptr_on_peer = devices_srv[i];
        devices[i]->rcount = 1;
        devices[i]->server = platform->server;
        devices[i]->platform = platform;
        devices[i]->parent_device = NULL;
        flag = getDeviceInfo(devices[i],
                             CL_DEVICE_TYPE,
                             sizeof(cl_device_type),
                             &device_type,
                             NULL);
        if(flag != CL_SUCCESS){
            return CL_OUT_OF_RESOURCES;
        }
        devices[i]->device_type = device_type;
    }
    free(devices_srv); devices_srv = NULL;

    // Append the list of devices to the platform, and the global devices
    flag = addDevices(num_devices, devices);
    if(flag != CL_SUCCESS){
        free(devices); devices = NULL;
        return CL_OUT_OF_HOST_MEMORY;
    }

    return CL_SUCCESS;
}


cl_int getDeviceIDs(cl_platform_id   platform,
                    cl_device_type   device_type,
                    cl_uint          num_entries,
                    cl_device_id *   devices,
                    cl_uint *        num_devices)
{
    cl_int flag;
    cl_uint i, n=0, devices_index=0;
    if(!platform->devices){
        flag = initDevices(platform);
        if(flag != CL_SUCCESS){
            return flag;
        }
    }

    // Count the devices
    for(i = 0; i < platform->num_devices; i++){
        if((device_type == CL_DEVICE_TYPE_ALL) ||
           (device_type == CL_DEVICE_TYPE_DEFAULT) ||
           (device_type == platform->devices[i]->device_type))
        {
            n++;
        }
    }
    if(num_devices){
        *num_devices = n;
    }
    if(!n){
        return CL_DEVICE_NOT_FOUND;
    }

    // Get the devices
    if(!num_entries){
        return CL_SUCCESS;
    }
    if(num_entries < n){
        n = num_entries;
    }
    i = 0;
    while(devices_index < n){
        if((device_type == CL_DEVICE_TYPE_ALL) ||
           (device_type == CL_DEVICE_TYPE_DEFAULT) ||
           (device_type == platform->devices[i]->device_type))
        {
            devices[devices_index] = platform->devices[i];
            devices_index++;
        }
        i++;
    }

    return CL_SUCCESS;
}

cl_int getDeviceInfo(cl_device_id    device,
                     cl_device_info  param_name,
                     size_t          param_value_size,
                     void *          param_value,
                     size_t *        param_value_size_ret)
{
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    size64 size_ret;
    unsigned int comm = ocland_clGetDeviceInfo;
    if(param_value_size_ret) *param_value_size_ret = 0;

    int *sockfd = device->server->socket;
    if(!sockfd){
        return CL_INVALID_DEVICE;
    }

    size64 param_value_size_64 = param_value_size;
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(device->ptr_on_peer), sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &param_name, sizeof(cl_device_info), MSG_MORE);
    socket_flag |= Send(sockfd, &param_value_size_64, sizeof(size64), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv(sockfd, &size_ret, sizeof(size64), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(param_value_size_ret) *param_value_size_ret = (size_t)size_ret;
    if(param_value){
        socket_flag |= Recv(sockfd, param_value, size_ret, MSG_WAITALL);
        if(socket_flag){
            return CL_OUT_OF_RESOURCES;
        }
    }
    return CL_SUCCESS;
}

cl_int createSubDevices(cl_device_id                         in_device,
                        const cl_device_partition_property * properties,
                        cl_uint                              num_properties,
                        cl_uint                              num_entries,
                        cl_device_id                       * devices,
                        cl_uint                            * num_devices)
{
    unsigned int i, j;
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    cl_uint n;
    unsigned int comm = ocland_clCreateSubDevices;
    if(num_devices) *num_devices = 0;
    int *sockfd = in_device->server->socket;
    if(!sockfd){
        return CL_INVALID_DEVICE;
    }

    // Call the server to generate the devices  (or just report how many are)
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(in_device->ptr_on_peer), sizeof(pointer), MSG_MORE);
    socket_flag |= Send(sockfd, &num_properties, sizeof(cl_uint), MSG_MORE);
    if(num_properties){
        // TODO: pack pointers here
        socket_flag |= Send(sockfd,
                            properties,
                            num_properties * sizeof(cl_device_partition_property),
                            MSG_MORE);
    }
    socket_flag |= Send(sockfd, &num_entries, sizeof(cl_uint), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }
    socket_flag |= Recv(sockfd, &n, sizeof(cl_uint), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(num_devices) *num_devices = n;
    if(!devices){
        return CL_SUCCESS;
    }
    if(num_entries < n){
        return CL_INVALID_VALUE;
    }
    pointer* devices_srv = malloc(n * sizeof(pointer));
    if(!devices_srv){
        VERBOSE("Failure allocating memory for the server device instances!\n");
        return CL_OUT_OF_HOST_MEMORY;
    }
    socket_flag |= Recv(sockfd, devices_srv, n * sizeof(pointer), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }

    // Create the device objects
    for(i = 0; i < n; i++){
        devices[i] = (cl_device_id)malloc(sizeof(struct _cl_device_id));
        if(!devices[i]){
            VERBOSE("Failure allocating memory for the device %u!\n", i);
            for(j = 0; j < i; j++){
                free(devices[j]); devices[j] = NULL;
            }
            free(devices_srv); devices_srv = NULL;
            return CL_OUT_OF_HOST_MEMORY;
        }
        devices[i]->dispatch = in_device->dispatch;
        devices[i]->ptr_on_peer = devices_srv[i];
        devices[i]->rcount = 1;
        devices[i]->server = in_device->server;
        devices[i]->platform = in_device->platform;
        devices[i]->parent_device = in_device;
        devices[i]->device_type = in_device->device_type;
    }
    free(devices_srv); devices_srv = NULL;

    // Append the list of devices to the platform, and the global devices
    flag = addDevices(n, devices);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_HOST_MEMORY;
    }

    return CL_SUCCESS;
}

cl_int retainDevice(cl_device_id device)
{
    if(!device->parent_device){
        // It is not a subdevice, but a root-level one
        return CL_INVALID_DEVICE;
    }
    device->rcount++;
    return CL_SUCCESS;
}

cl_int releaseDevice(cl_device_id device)
{
    if(!device->parent_device){
        // It is not a subdevice, but a root-level one
        return CL_INVALID_DEVICE;
    }
    device->rcount--;
    if(device->rcount){
        return CL_SUCCESS;
    }

    // Call the server to clear the device
    cl_int flag = CL_OUT_OF_RESOURCES;
    int socket_flag = 0;
    unsigned int comm = ocland_clReleaseDevice;
    int *sockfd = device->server->socket;
    if(!sockfd){
        return CL_OUT_OF_RESOURCES;
    }
    socket_flag |= Send(sockfd, &comm, sizeof(unsigned int), MSG_MORE);
    socket_flag |= Send(sockfd, &(device->ptr_on_peer), sizeof(pointer), 0);
    socket_flag |= Recv(sockfd, &flag, sizeof(cl_int), MSG_WAITALL);
    if(socket_flag){
        return CL_OUT_OF_RESOURCES;
    }
    if(flag != CL_SUCCESS){
        return flag;
    }

    // Remove the object instance
    flag = discardDevice(device);
    if(flag != CL_SUCCESS){
        return CL_OUT_OF_HOST_MEMORY;
    }

    return CL_SUCCESS;
}

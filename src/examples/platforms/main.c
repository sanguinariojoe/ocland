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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <CL/opencl.h>

int main(int argc, char *argv[])
{
    unsigned int i;
    char buffer[1025]; strcpy(buffer, "");
    cl_uint num_entries = 0, num_platforms = 0;
    cl_platform_id *platforms = NULL;
    cl_int flag;
    // Get number of platforms
    flag = clGetPlatformIDs(0, NULL, &num_platforms);
    if(flag != CL_SUCCESS){
        printf("Error getting number of platforms\n");
        if(flag == CL_INVALID_VALUE)
            printf("\tCL_INVALID_VALUE\n");
        if(flag == CL_OUT_OF_HOST_MEMORY)
            printf("\tCL_OUT_OF_HOST_MEMORY\n");
        return EXIT_FAILURE;
    }
    if(!num_platforms){
        printf("No OpenCL platforms found...\n");
        return EXIT_FAILURE;
    }
    printf("%u platforms found...\n", num_platforms);
    // Build platforms array
    num_entries = num_platforms;
    platforms   = (cl_platform_id*)malloc(num_entries*sizeof(cl_platform_id));
    // Get platforms array
    flag = clGetPlatformIDs(num_entries, platforms, &num_platforms);
    if(flag != CL_SUCCESS){
        printf("Error getting platforms\n");
        if(flag == CL_INVALID_VALUE)
            printf("\tCL_INVALID_VALUE\n");
        if(flag == CL_OUT_OF_HOST_MEMORY)
            printf("\tCL_OUT_OF_HOST_MEMORY\n");
        return EXIT_FAILURE;
    }
    // Print platforms data
    for(i=0;i<num_platforms;i++){
        printf("Platform %u...\n", i);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_PROFILE,1025*sizeof(char),buffer, NULL);
        printf("\tPROFILE: %s\n", buffer);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_VERSION,1025*sizeof(char),buffer, NULL);
        printf("\tVERSION: %s\n", buffer);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1025*sizeof(char),buffer, NULL);
        printf("\tNAME: %s\n", buffer);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_VENDOR,1025*sizeof(char),buffer, NULL);
        printf("\tVENDOR: %s\n", buffer);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_EXTENSIONS,1025*sizeof(char),buffer, NULL);
        printf("\tCL_PLATFORM_EXTENSIONS: %s\n", buffer);
        clGetPlatformInfo(platforms[i],CL_PLATFORM_ICD_SUFFIX_KHR,1025*sizeof(char),buffer, NULL);
        printf("\tCL_PLATFORM_ICD_SUFFIX_KHR: %s\n", buffer);
        printf("\t---\n");
    }
    if(platforms) free(platforms); platforms=NULL;
    return EXIT_SUCCESS;
}

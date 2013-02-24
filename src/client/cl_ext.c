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

#include <string.h>

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include "CL/cl.h"
#include "CL/cl_ext.h"

struct driverStubextFunc_st
{
    const char *name;
    void *func;
};

#define EXT_FUNC(name) { #name, (void*)(name) }

static struct driverStubextFunc_st clExtensions[] =
{
    EXT_FUNC(clIcdGetPlatformIDsKHR),
};

static const unsigned int clExtensionCount = sizeof(clExtensions) / sizeof(clExtensions[0]);

CL_API_ENTRY void * CL_API_CALL
clGetExtensionFunctionAddress(const char *name)
{
    unsigned int i;

    for(i=0;i<clExtensionCount;i++) {
        if (!strcmp(name, clExtensions[i].name)) {
            return clExtensions[i].func;
        }
    }

    return NULL;
}

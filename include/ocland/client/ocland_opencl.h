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

#ifndef OCLAND_OPENCL_H_INCLUDED
#define OCLAND_OPENCL_H_INCLUDED

#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <CL/opencl.h>
#include <ocland/client/ocland.h>
#include <ocland/client/ocland_icd.h>
#ifndef CL_EXT_PREFIX__VERSION_1_1_DEPRECATED
    #define CL_EXT_PREFIX__VERSION_1_1_DEPRECATED
#endif
#ifndef CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
    #define CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
#endif

#endif // OCLAND_OPENCL_H_INCLUDED

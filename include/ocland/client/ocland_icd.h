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

#ifndef OCLAND_ICD_H_INCLUDED
#define OCLAND_ICD_H_INCLUDED

// #include <CL/opencl.h>
#include <ocl_icd.h>
#include <pthread.h>

#include <ocland/client/platform_id.h>
#include <ocland/client/device_id.h>
#include <ocland/client/context.h>
#include <ocland/client/command_queue.h>
#include <ocland/client/mem.h>
#include <ocland/client/sampler.h>
#include <ocland/client/program.h>
#include <ocland/client/kernel.h>
#include <ocland/client/event.h>

#pragma GCC visibility push(hidden)

struct _cl_icd_dispatch master_dispatch;

#pragma GCC visibility pop

#endif // OCLAND_ICD_H_INCLUDED

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

#include <CL/opencl.h>

struct _cl_icd_dispatch;
struct _cl_platform_id {
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_platform_id ptr;
};
struct _cl_device_id
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_device_id ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
};
struct _cl_context
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_context ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
};
struct _cl_command_queue
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_command_queue ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
};
struct _cl_mem
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_mem ptr;
    /// Memory object size
    size_t size;
    /// Element size (only for images)
    size_t element_size;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
};
struct _cl_sampler
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_sampler ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
};
struct _cl_program
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_program ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
};
struct _cl_kernel
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_kernel ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
};
struct _cl_event
{
    /// Dispatch table
    struct _cl_icd_dispatch *dispatch;
    /// Pointer of server instance
    cl_event ptr;
    /// Reference count to control when the object must be destroyed
    cl_uint rcount;
};

struct _cl_icd_dispatch {
  void(*func0)(void);
  void(*func1)(void);
  void(*func2)(void);
  void(*func3)(void);
  void(*func4)(void);
  void(*func5)(void);
  void(*func6)(void);
  void(*func7)(void);
  void(*func8)(void);
  void(*func9)(void);
  void(*func10)(void);
  void(*func11)(void);
  void(*func12)(void);
  void(*func13)(void);
  void(*func14)(void);
  void(*func15)(void);
  void(*func16)(void);
  void(*func17)(void);
  void(*func18)(void);
  void(*func19)(void);
  void(*func20)(void);
  void(*func21)(void);
  void(*func22)(void);
  void(*func23)(void);
  void(*func24)(void);
  void(*func25)(void);
  void(*func26)(void);
  void(*func27)(void);
  void(*func28)(void);
  void(*func29)(void);
  void(*func30)(void);
  void(*func31)(void);
  void(*func32)(void);
  void(*func33)(void);
  void(*func34)(void);
  void(*func35)(void);
  void(*func36)(void);
  void(*func37)(void);
  void(*func38)(void);
  void(*func39)(void);
  void(*func40)(void);
  void(*func41)(void);
  void(*func42)(void);
  void(*func43)(void);
  void(*func44)(void);
  void(*func45)(void);
  void(*func46)(void);
  void(*func47)(void);
  void(*func48)(void);
  void(*func49)(void);
  void(*func50)(void);
  void(*func51)(void);
  void(*func52)(void);
  void(*func53)(void);
  void(*func54)(void);
  void(*func55)(void);
  void(*func56)(void);
  void(*func57)(void);
  void(*func58)(void);
  void(*func59)(void);
  void(*func60)(void);
  void(*func61)(void);
  void(*func62)(void);
  void(*func63)(void);
  void(*func64)(void);
  void(*func65)(void);
  void(*func66)(void);
  void(*func67)(void);
  void(*func68)(void);
  void(*func69)(void);
  void(*func70)(void);
  void(*func71)(void);
  void(*func72)(void);
  void(*func73)(void);
  void(*func74)(void);
  void(*func75)(void);
  void(*func76)(void);
  void(*func77)(void);
  void(*func78)(void);
  void(*func79)(void);
  void(*func80)(void);
  void(*func81)(void);
  void(*func82)(void);
  void(*func83)(void);
  void(*func84)(void);
  void(*func85)(void);
  void(*func86)(void);
  void(*func87)(void);
  void(*func88)(void);
  void(*func89)(void);
  void(*func90)(void);
  void(*func91)(void);
  void(*func92)(void);
  void(*func93)(void);
  void(*func94)(void);
  void(*func95)(void);
  void(*func96)(void);
  void(*func97)(void);
  void(*func98)(void);
  void(*func99)(void);
  void(*func100)(void);
  void(*func101)(void);
  void(*func102)(void);
  void(*func103)(void);
  void(*func104)(void);
  void(*func105)(void);
  void(*func106)(void);
  void(*func107)(void);
  void(*func108)(void);
  void(*func109)(void);
  void(*func110)(void);
  void(*func111)(void);
  void(*func112)(void);
  void(*func113)(void);
  void(*func114)(void);
  void(*func115)(void);
  void(*func116)(void);
  void(*func117)(void);
  void(*func118)(void);
  void(*func119)(void);
  void(*func120)(void);
  void(*func121)(void);
  void(*func122)(void);
  void(*func123)(void);
  void(*func124)(void);
};

#pragma GCC visibility push(hidden)

struct _cl_icd_dispatch master_dispatch;

typeof(clGetPlatformInfo) icd_clGetPlatformInfo;
typeof(clGetPlatformIDs) icd_clGetPlatformIDs;
#pragma GCC visibility pop

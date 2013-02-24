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

#include <CL/cl.h>
#include <CL/cl_gl.h>

#ifndef CL_EXT_PREFIX__VERSION_1_1_DEPRECATED
#define CL_EXT_PREFIX__VERSION_1_1_DEPRECATED
#endif

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLBuffer(cl_context     context ,
                     cl_mem_flags   flags ,
                     cl_GLuint      bufobj ,
                     int *          errcode_ret ) CL_API_SUFFIX__VERSION_1_0
{
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    return NULL;
}

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLTexture(cl_context      context ,
                      cl_mem_flags    flags ,
                      cl_GLenum       target ,
                      cl_GLint        miplevel ,
                      cl_GLuint       texture ,
                      cl_int *        errcode_ret ) CL_API_SUFFIX__VERSION_1_2
{
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    return NULL;
}

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLRenderbuffer(cl_context   context ,
                           cl_mem_flags flags ,
                           cl_GLuint    renderbuffer ,
                           cl_int *     errcode_ret ) CL_API_SUFFIX__VERSION_1_0
{
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    return NULL;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clGetGLObjectInfo(cl_mem                memobj ,
                  cl_gl_object_type *   gl_object_type ,
                  cl_GLuint *           gl_object_name ) CL_API_SUFFIX__VERSION_1_0
{
    /** If have not been possible to generate GL memory
     * objects, is impossible that the memory object is
     * associated to a GL object.
     */
    return CL_INVALID_GL_OBJECT;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clGetGLTextureInfo(cl_mem               memobj ,
                   cl_gl_texture_info   param_name ,
                   size_t               param_value_size ,
                   void *               param_value ,
                   size_t *             param_value_size_ret ) CL_API_SUFFIX__VERSION_1_0
{
    /** If have not been possible to generate GL memory
     * objects, is impossible that the memory object is
     * associated to a GL object.
     */
    return CL_INVALID_GL_OBJECT;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAcquireGLObjects(cl_command_queue      command_queue ,
                          cl_uint               num_objects ,
                          const cl_mem *        mem_objects ,
                          cl_uint               num_events_in_wait_list ,
                          const cl_event *      event_wait_list ,
                          cl_event *            event ) CL_API_SUFFIX__VERSION_1_0
{
    /** If have not been possible to generate GL memory
     * objects, is impossible that the memory object is
     * associated to a GL object.
     */
    return CL_INVALID_GL_OBJECT;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReleaseGLObjects(cl_command_queue      command_queue ,
                          cl_uint               num_objects ,
                          const cl_mem *        mem_objects ,
                          cl_uint               num_events_in_wait_list ,
                          const cl_event *      event_wait_list ,
                          cl_event *            event ) CL_API_SUFFIX__VERSION_1_0
{
    /** If have not been possible to generate GL memory
     * objects, is impossible that the memory object is
     * associated to a GL object.
     */
    return CL_INVALID_GL_OBJECT;
}


// Deprecated OpenCL 1.1 APIs
extern CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
clCreateFromGLTexture2D(cl_context      context ,
                        cl_mem_flags    flags ,
                        cl_GLenum       target ,
                        cl_GLint        miplevel ,
                        cl_GLuint       texture ,
                        cl_int *        errcode_ret ) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    return NULL;
}

extern CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
clCreateFromGLTexture3D(cl_context      context ,
                        cl_mem_flags    flags ,
                        cl_GLenum       target ,
                        cl_GLint        miplevel ,
                        cl_GLuint       texture ,
                        cl_int *        errcode_ret ) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    /** GL objects generated in host are not valid for
     * the server, so this operation can't be executed.
     */
    *errcode_ret = CL_INVALID_GL_OBJECT;
    return NULL;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clGetGLContextInfoKHR(const cl_context_properties * properties ,
                      cl_gl_context_info            param_name ,
                      size_t                        param_value_size ,
                      void *                        param_value ,
                      size_t *                      param_value_size_ret ) CL_API_SUFFIX__VERSION_1_0
{
    return CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR;
}

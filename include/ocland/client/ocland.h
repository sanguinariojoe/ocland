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
#include <stdlib.h>
#include <string.h>

#include <CL/cl.h>
#include <CL/cl_ext.h>

#ifndef OCLAND_H_INCLUDED
#define OCLAND_H_INCLUDED

/** clGetSamplerInfo ocland abstraction method.
 */
cl_program oclandCreateProgramWithSource(cl_context         context ,
                                         cl_uint            count ,
                                         const char **      strings ,
                                         const size_t *     lengths ,
                                         cl_int *           errcode_ret);

/** clCreateProgramWithBinary ocland abstraction method.
 */
cl_program oclandCreateProgramWithBinary(cl_context                      context ,
                                         cl_uint                         num_devices ,
                                         const cl_device_id *            device_list ,
                                         const size_t *                  lengths ,
                                         const unsigned char **          binaries ,
                                         cl_int *                        binary_status ,
                                         cl_int *                        errcode_ret);

/** clRetainProgram ocland abstraction method.
 */
cl_int oclandRetainProgram(cl_program  program);

/** clReleaseProgram ocland abstraction method.
 */
cl_int oclandReleaseProgram(cl_program  program);

/** clBuildProgram ocland abstraction method.
 */
cl_int oclandBuildProgram(cl_program            program ,
                          cl_uint               num_devices ,
                          const cl_device_id *  device_list ,
                          const char *          options ,
                          void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                          void *                user_data);

/** clGetProgramInfo ocland abstraction method.
 */
cl_int oclandGetProgramInfo(cl_program          program ,
                            cl_program_info     param_name ,
                            size_t              param_value_size ,
                            void *              param_value ,
                            size_t *            param_value_size_ret);

/** clGetProgramBuildInfo ocland abstraction method.
 */
cl_int oclandGetProgramBuildInfo(cl_program             program ,
                                 cl_device_id           device ,
                                 cl_program_build_info  param_name ,
                                 size_t                 param_value_size ,
                                 void *                 param_value ,
                                 size_t *               param_value_size_ret);

/** clCreateKernel ocland abstraction method.
 */
cl_kernel oclandCreateKernel(cl_program       program ,
                             const char *     kernel_name ,
                             cl_int *         errcode_ret);

/** clCreateKernelsInProgram ocland abstraction method.
 */
cl_int oclandCreateKernelsInProgram(cl_program      program ,
                                    cl_uint         num_kernels ,
                                    cl_kernel *     kernels ,
                                    cl_uint *       num_kernels_ret);

/** clRetainKernel ocland abstraction method.
 */
cl_int oclandRetainKernel(cl_kernel     kernel);

/** clReleaseKernel ocland abstraction method.
 */
cl_int oclandReleaseKernel(cl_kernel    kernel);

/** clSetKernelArg ocland abstraction method.
 */
cl_int oclandSetKernelArg(cl_kernel     kernel ,
                          cl_uint       arg_index ,
                          size_t        arg_size ,
                          const void *  arg_value);

/** clGetKernelInfo ocland abstraction method.
 */
cl_int oclandGetKernelInfo(cl_kernel        kernel ,
                           cl_kernel_info   param_name ,
                           size_t           param_value_size ,
                           void *           param_value ,
                           size_t *         param_value_size_ret);

/** clGetKernelWorkGroupInfo ocland abstraction method.
 */
cl_int oclandGetKernelWorkGroupInfo(cl_kernel                   kernel ,
                                    cl_device_id                device ,
                                    cl_kernel_work_group_info   param_name ,
                                    size_t                      param_value_size ,
                                    void *                      param_value ,
                                    size_t *                    param_value_size_ret);

/** clWaitForEvents ocland abstraction method.
 */
cl_int oclandWaitForEvents(cl_uint              num_events,
                           const cl_event *     event_list);

/** clGetEventInfo ocland abstraction method. This is a little bit dangerous
 * method due to if info is requested before event has been generated,
 * i.e.- ocland is still performing work before calling OpenCL method,
 * CL_INVALID_EVENT will be returned.
 */
cl_int oclandGetEventInfo(cl_event          event ,
                          cl_event_info     param_name ,
                          size_t            param_value_size ,
                          void *            param_value ,
                          size_t *          param_value_size_ret);

/** clReleaseEvent ocland abstraction method.
 */
cl_int oclandReleaseEvent(cl_event  event);

/** clGetEventProfilingInfo ocland abstraction method.
 */
cl_int oclandGetEventProfilingInfo(cl_event             event ,
                                   cl_profiling_info    param_name ,
                                   size_t               param_value_size ,
                                   void *               param_value ,
                                   size_t *             param_value_size_ret);

/** clFlush ocland abstraction method.
 */
cl_int oclandFlush(cl_command_queue  command_queue);

/** clFinish ocland abstraction method.
 */
cl_int oclandFinish(cl_command_queue  command_queue);

/** clEnqueueReadBuffer ocland abstraction method.
 */
cl_int oclandEnqueueReadBuffer(cl_command_queue     command_queue ,
                               cl_mem               buffer ,
                               cl_bool              blocking_read ,
                               size_t               offset ,
                               size_t               cb ,
                               void *               ptr ,
                               cl_uint              num_events_in_wait_list ,
                               const cl_event *     event_wait_list ,
                               cl_event *           event);

/** clEnqueueWriteBuffer ocland abstraction method.
 */
cl_int oclandEnqueueWriteBuffer(cl_command_queue    command_queue ,
                                cl_mem              buffer ,
                                cl_bool             blocking_write ,
                                size_t              offset ,
                                size_t              cb ,
                                const void *        ptr ,
                                cl_uint             num_events_in_wait_list ,
                                const cl_event *    event_wait_list ,
                                cl_event *          event);

/** clEnqueueCopyBuffer ocland abstraction method.
 */
cl_int oclandEnqueueCopyBuffer(cl_command_queue     command_queue ,
                               cl_mem               src_buffer ,
                               cl_mem               dst_buffer ,
                               size_t               src_offset ,
                               size_t               dst_offset ,
                               size_t               cb ,
                               cl_uint              num_events_in_wait_list ,
                               const cl_event *     event_wait_list ,
                               cl_event *           event);

/** clEnqueueReadImage ocland abstraction method.
 * @param element_size Size of each element.
 */
cl_int oclandEnqueueReadImage(cl_command_queue      command_queue ,
                              cl_mem                image ,
                              cl_bool               blocking_read ,
                              const size_t *        origin ,
                              const size_t *        region ,
                              size_t                row_pitch ,
                              size_t                slice_pitch ,
                              size_t                element_size ,
                              void *                ptr ,
                              cl_uint               num_events_in_wait_list ,
                              const cl_event *      event_wait_list ,
                              cl_event *            event);

/** clEnqueueWriteImage ocland abstraction method.
 * @param element_size Size of each element.
 */
cl_int oclandEnqueueWriteImage(cl_command_queue     command_queue ,
                               cl_mem               image ,
                               cl_bool              blocking_write ,
                               const size_t *       origin ,
                               const size_t *       region ,
                               size_t               row_pitch ,
                               size_t               slice_pitch ,
                               size_t               element_size ,
                               const void *         ptr ,
                               cl_uint              num_events_in_wait_list ,
                               const cl_event *     event_wait_list ,
                               cl_event *           event);

/** clEnqueueCopyImage ocland abstraction method.
 */
cl_int oclandEnqueueCopyImage(cl_command_queue      command_queue ,
                              cl_mem                src_image ,
                              cl_mem                dst_image ,
                              const size_t *        src_origin ,
                              const size_t *        dst_origin ,
                              const size_t *        region ,
                              cl_uint               num_events_in_wait_list ,
                              const cl_event *      event_wait_list ,
                              cl_event *            event);

/** clEnqueueCopyImageToBuffer ocland abstraction method.
 */
cl_int oclandEnqueueCopyImageToBuffer(cl_command_queue  command_queue ,
                                      cl_mem            src_image ,
                                      cl_mem            dst_buffer ,
                                      const size_t *    src_origin ,
                                      const size_t *    region ,
                                      size_t            dst_offset ,
                                      cl_uint           num_events_in_wait_list ,
                                      const cl_event *  event_wait_list ,
                                      cl_event *        event);

/** clEnqueueCopyBufferToImage ocland abstraction method.
 */
cl_int oclandEnqueueCopyBufferToImage(cl_command_queue  command_queue ,
                                      cl_mem            src_buffer ,
                                      cl_mem            dst_image ,
                                      size_t            src_offset ,
                                      const size_t *    dst_origin ,
                                      const size_t *    region ,
                                      cl_uint           num_events_in_wait_list ,
                                      const cl_event *  event_wait_list ,
                                      cl_event *        event);

/** clEnqueueNDRangeKernel ocland abstraction method.
 */
cl_int oclandEnqueueNDRangeKernel(cl_command_queue  command_queue ,
                                  cl_kernel         kernel ,
                                  cl_uint           work_dim ,
                                  const size_t *    global_work_offset ,
                                  const size_t *    global_work_size ,
                                  const size_t *    local_work_size ,
                                  cl_uint           num_events_in_wait_list ,
                                  const cl_event *  event_wait_list ,
                                  cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

// -------------------------------------------- //
//                                              //
// OpenCL 1.1 methods                           //
//                                              //
// -------------------------------------------- //

/** clCreateUserEvent ocland abstraction method.
 */
cl_event oclandCreateUserEvent(cl_context     context ,
                               cl_int *       errcode_ret);

/** clSetUserEventStatus ocland abstraction method.
 */
cl_int oclandSetUserEventStatus(cl_event    event ,
                                cl_int      execution_status);

/** clEnqueueReadBufferRect ocland abstraction method.
 */
cl_int oclandEnqueueReadBufferRect(cl_command_queue     command_queue ,
                                   cl_mem               buffer ,
                                   cl_bool              blocking_read ,
                                   const size_t *       buffer_origin ,
                                   const size_t *       host_origin ,
                                   const size_t *       region ,
                                   size_t               buffer_row_pitch ,
                                   size_t               buffer_slice_pitch ,
                                   size_t               host_row_pitch ,
                                   size_t               host_slice_pitch ,
                                   void *               ptr ,
                                   cl_uint              num_events_in_wait_list ,
                                   const cl_event *     event_wait_list ,
                                   cl_event *           event);

/** clEnqueueWriteBufferRect ocland abstraction method.
 */
cl_int oclandEnqueueWriteBufferRect(cl_command_queue     command_queue ,
                                    cl_mem               buffer ,
                                    cl_bool              blocking_write ,
                                    const size_t *       buffer_origin ,
                                    const size_t *       host_origin ,
                                    const size_t *       region ,
                                    size_t               buffer_row_pitch ,
                                    size_t               buffer_slice_pitch ,
                                    size_t               host_row_pitch ,
                                    size_t               host_slice_pitch ,
                                    const void *         ptr ,
                                    cl_uint              num_events_in_wait_list ,
                                    const cl_event *     event_wait_list ,
                                    cl_event *           event);

/** clEnqueueCopyBufferRect ocland abstraction method.
 */
cl_int oclandEnqueueCopyBufferRect(cl_command_queue     command_queue ,
                                   cl_mem               src_buffer ,
                                   cl_mem               dst_buffer ,
                                   const size_t *       src_origin ,
                                   const size_t *       dst_origin ,
                                   const size_t *       region ,
                                   size_t               src_row_pitch ,
                                   size_t               src_slice_pitch ,
                                   size_t               dst_row_pitch ,
                                   size_t               dst_slice_pitch ,
                                   cl_uint              num_events_in_wait_list ,
                                   const cl_event *     event_wait_list ,
                                   cl_event *           event);

// -------------------------------------------- //
//                                              //
// OpenCL 1.2 methods                           //
//                                              //
// -------------------------------------------- //

/** clCreateProgramWithBuiltInKernels ocland abstraction method.
 */
cl_program oclandCreateProgramWithBuiltInKernels(cl_context             context ,
                                                 cl_uint                num_devices ,
                                                 const cl_device_id *   device_list ,
                                                 const char *           kernel_names ,
                                                 cl_int *               errcode_ret);

/** clCompileProgram ocland abstraction method.
 */
cl_int oclandCompileProgram(cl_program            program ,
                            cl_uint               num_devices ,
                            const cl_device_id *  device_list ,
                            const char *          options ,
                            cl_uint               num_input_headers ,
                            const cl_program *    input_headers,
                            const char **         header_include_names ,
                            void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                            void *                user_data);

/** clLinkProgram ocland abstraction method.
 */
cl_program oclandLinkProgram(cl_context            context ,
                             cl_uint               num_devices ,
                             const cl_device_id *  device_list ,
                             const char *          options ,
                             cl_uint               num_input_programs ,
                             const cl_program *    input_programs ,
                             void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                             void *                user_data ,
                             cl_int *              errcode_ret);

/** clUnloadPlatformCompiler ocland abstraction method.
 */
cl_int oclandUnloadPlatformCompiler(cl_platform_id  platform);

/** clUnloadPlatformCompiler ocland abstraction method.
 */
cl_int oclandGetKernelArgInfo(cl_kernel        kernel ,
                              cl_uint          arg_index ,
                              cl_kernel_arg_info   param_name ,
                              size_t           param_value_size ,
                              void *           param_value ,
                              size_t *         param_value_size_ret);

/** clEnqueueFillBuffer ocland abstraction method.
 */
cl_int oclandEnqueueFillBuffer(cl_command_queue    command_queue ,
                               cl_mem              buffer ,
                               const void *        pattern ,
                               size_t              pattern_size ,
                               size_t              offset ,
                               size_t              cb ,
                               cl_uint             num_events_in_wait_list ,
                               const cl_event *    event_wait_list ,
                               cl_event *          event);

/** clEnqueueFillImage ocland abstraction method.
 * @param fill_color_size Size of fill_color following the
 * rules described in clEnqueueFillImage specification.
 */
cl_int oclandEnqueueFillImage(cl_command_queue    command_queue ,
                              cl_mem              image ,
                              size_t              fill_color_size ,
                              const void *        fill_color ,
                              const size_t *      origin ,
                              const size_t *      region ,
                              cl_uint             num_events_in_wait_list ,
                              const cl_event *    event_wait_list ,
                              cl_event *          event);

/** clEnqueueMigrateMemObjects ocland abstraction method.
 */
cl_int oclandEnqueueMigrateMemObjects(cl_command_queue        command_queue ,
                                      cl_uint                 num_mem_objects ,
                                      const cl_mem *          mem_objects ,
                                      cl_mem_migration_flags  flags ,
                                      cl_uint                 num_events_in_wait_list ,
                                      const cl_event *        event_wait_list ,
                                      cl_event *              event);

/** clEnqueueMarkerWithWaitList ocland abstraction method.
 */
cl_int oclandEnqueueMarkerWithWaitList(cl_command_queue  command_queue ,
                                       cl_uint            num_events_in_wait_list ,
                                       const cl_event *   event_wait_list ,
                                       cl_event *         event);

/** clEnqueueBarrierWithWaitList ocland abstraction method.
 */
cl_int oclandEnqueueBarrierWithWaitList(cl_command_queue  command_queue ,
                                        cl_uint            num_events_in_wait_list ,
                                        const cl_event *   event_wait_list ,
                                        cl_event *         event);

#endif // OCLAND_H_INCLUDED

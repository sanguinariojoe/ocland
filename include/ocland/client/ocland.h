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

typedef struct oclandServers_st oclandServers;

/** @struct oclandServers_st
 * Store useful data about servers.
 */
struct oclandServers_st
{
    /// Number of servers located
    unsigned int num_servers;
    /// Address of servers
    char** address;
    /// Sockets asigned to each server
    int* sockets;
};

/** clGetPlatformIDs ocland abstraction method.
 */
cl_int oclandGetPlatformIDs(cl_uint         num_entries,
                            cl_platform_id* platforms,
                            int*            sockets,
                            cl_uint*        num_platforms);

/** clGetPlatformInfo ocland abstraction method.
 */
cl_int oclandGetPlatformInfo(cl_platform_id    platform,
                             cl_platform_info  param_name,
                             size_t            param_value_size,
                             void *            param_value,
                             size_t *          param_value_size_ret);

/** clGetDeviceIDs ocland abstraction method.
 */
cl_int oclandGetDeviceIDs(cl_platform_id   platform,
                          cl_device_type   device_type,
                          cl_uint          num_entries,
                          cl_device_id *   devices,
                          cl_uint *        num_devices);

/** clGetDeviceInfo ocland abstraction method.
 */
cl_int oclandGetDeviceInfo(cl_device_id    device,
                           cl_device_info  param_name,
                           size_t          param_value_size,
                           void *          param_value,
                           size_t *        param_value_size_ret);


/** clCreateContext ocland abstraction method.
 * @param num_properties Number of properties into properties array
 */
cl_context oclandCreateContext(cl_platform_id                platform,
                               const cl_context_properties * properties,
                               cl_uint                       num_properties,
                               cl_uint                       num_devices ,
                               const cl_device_id *          devices,
                               void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *),
                               void *                        user_data,
                               cl_int *                      errcode_ret);

/** clCreateContextFromType ocland abstraction method.
 * @param num_properties Number of properties into properties array
 */
cl_context oclandCreateContextFromType(cl_platform_id                platform,
                                       const cl_context_properties * properties,
                                       cl_uint                       num_properties,
                                       cl_device_type                device_type,
                                       void (CL_CALLBACK *     pfn_notify)(const char *, const void *, size_t, void *),
                                       void *                        user_data,
                                       cl_int *                      errcode_ret);

/** clRetainContext ocland abstraction method.
 */
cl_int oclandRetainContext(cl_context context);

/** clRetainContext ocland abstraction method.
 */
cl_int oclandReleaseContext(cl_context context);

/** clGetContextInfo ocland abstraction method.
 */
cl_int oclandGetContextInfo(cl_context         context,
                            cl_context_info    param_name,
                            size_t             param_value_size,
                            void *             param_value,
                            size_t *           param_value_size_ret);

/** clCreateCommandQueue ocland abstraction method.
 */
cl_command_queue oclandCreateCommandQueue(cl_context                     context,
                                          cl_device_id                   device,
                                          cl_command_queue_properties    properties,
                                          cl_int *                       errcode_ret);

/** clRetainCommandQueue ocland abstraction method.
 */
cl_int oclandRetainCommandQueue(cl_command_queue command_queue);

/** clReleaseCommandQueue ocland abstraction method.
 */
cl_int oclandReleaseCommandQueue(cl_command_queue command_queue);

/** clGetCommandQueueInfo ocland abstraction method.
 */
cl_int oclandGetCommandQueueInfo(cl_command_queue      command_queue,
                                 cl_command_queue_info param_name,
                                 size_t                param_value_size,
                                 void *                param_value,
                                 size_t *              param_value_size_ret);

/** clCreateBuffer ocland abstraction method.
 */
cl_mem oclandCreateBuffer(cl_context    context ,
                          cl_mem_flags  flags ,
                          size_t        size ,
                          void *        host_ptr ,
                          cl_int *      errcode_ret);

/** clRetainMemObject ocland abstraction method.
 */
cl_int oclandRetainMemObject(cl_mem memobj);

/** clReleaseMemObject ocland abstraction method.
 */
cl_int oclandReleaseMemObject(cl_mem memobj);

/** clGetSupportedImageFormats ocland abstraction method.
 */
cl_int oclandGetSupportedImageFormats(cl_context           context,
                                      cl_mem_flags         flags,
                                      cl_mem_object_type   image_type ,
                                      cl_uint              num_entries ,
                                      cl_image_format *    image_formats ,
                                      cl_uint *            num_image_formats);

/** clGetMemObjectInfo ocland abstraction method.
 */
cl_int oclandGetMemObjectInfo(cl_mem            memobj ,
                              cl_mem_info       param_name ,
                              size_t            param_value_size ,
                              void *            param_value ,
                              size_t *          param_value_size_ret);

/** clGetImageInfo ocland abstraction method.
 */
cl_int oclandGetImageInfo(cl_mem            image ,
                          cl_image_info     param_name ,
                          size_t            param_value_size ,
                          void *            param_value ,
                          size_t *          param_value_size_ret);

/** clCreateSampler ocland abstraction method.
 */
cl_sampler oclandCreateSampler(cl_context           context ,
                               cl_bool              normalized_coords ,
                               cl_addressing_mode   addressing_mode ,
                               cl_filter_mode       filter_mode ,
                               cl_int *             errcode_ret);


/** clRetainSampler ocland abstraction method.
 */
cl_int oclandRetainSampler(cl_sampler sampler);

/** clReleaseSampler ocland abstraction method.
 */
cl_int oclandReleaseSampler(cl_sampler sampler);

/** clGetSamplerInfo ocland abstraction method.
 */
cl_int oclandGetSamplerInfo(cl_sampler          sampler ,
                            cl_sampler_info     param_name ,
                            size_t              param_value_size ,
                            void *              param_value ,
                            size_t *            param_value_size_ret);

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

/** clCreateImage2D ocland abstraction method.
 * @param element_size Size of each element
 */
cl_mem oclandCreateImage2D(cl_context context,
                           cl_mem_flags flags,
                           const cl_image_format *image_format,
                           size_t image_width,
                           size_t image_height,
                           size_t image_row_pitch,
                           size_t element_size,
                           void *host_ptr,
                           cl_int *errcode_ret);

/** clCreateImage3D ocland abstraction method.
 * @param element_size Size of each element
 */
cl_mem oclandCreateImage3D(cl_context context,
                           cl_mem_flags flags,
                           const cl_image_format *image_format,
                           size_t image_width,
                           size_t image_height,
                           size_t image_depth,
                           size_t image_row_pitch,
                           size_t image_slice_pitch,
                           size_t element_size,
                           void *host_ptr,
                           cl_int *errcode_ret);

// -------------------------------------------- //
//                                              //
// OpenCL 1.1 methods                           //
//                                              //
// -------------------------------------------- //

/** clCreateSubBuffer ocland abstraction method.
 */
cl_mem oclandCreateSubBuffer(cl_mem                    buffer ,
                             cl_mem_flags              flags ,
                             cl_buffer_create_type     buffer_create_type ,
                             const void *              buffer_create_info ,
                             cl_int *                  errcode_ret);

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

/** clCreateSubDevices ocland abstraction method.
 * @param num_properties Number of properties into properties array
 */
cl_int oclandCreateSubDevices(cl_device_id                         in_device,
                              const cl_device_partition_property * properties,
                              cl_uint                              num_properties,
                              cl_uint                              num_entries,
                              cl_device_id                       * out_devices,
                              cl_uint                            * num_devices);

/** clRetainDevice ocland abstraction method.
 */
cl_int oclandRetainDevice(cl_device_id device);

/** clReleaseDevice ocland abstraction method.
 */
cl_int oclandReleaseDevice(cl_device_id device);

/** clCreateImage ocland abstraction method.
 * @param element_size Size of each element
 */
cl_mem oclandCreateImage(cl_context              context,
                         cl_mem_flags            flags,
                         const cl_image_format * image_format,
                         const cl_image_desc *   image_desc,
                         size_t                  element_size,
                         void *                  host_ptr,
                         cl_int *                errcode_ret);

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

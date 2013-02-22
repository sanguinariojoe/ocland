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

#include <ocland/client/ocland.h>

CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformIDs(cl_uint           num_entries ,
                 cl_platform_id *  platforms ,
                 cl_uint *         num_platforms) CL_API_SUFFIX__VERSION_1_0
{
    // validate the arguments
    if( (!num_entries && platforms) || (!platforms && !num_platforms) ){
        return CL_INVALID_VALUE;
    }
    // Connect to servers to look for platforms
    return oclandGetPlatformIDs(num_entries, platforms, num_platforms);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformInfo(cl_platform_id    platform,
                  cl_platform_info  param_name,
                  size_t            param_value_size,
                  void *            param_value,
                  size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    // validate the arguments
    if (param_value_size == 0 && param_value != NULL) {
        return CL_INVALID_VALUE;
    }
    // Connect to servers to get info
    return oclandGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id   platform,
               cl_device_type   device_type,
               cl_uint          num_entries,
               cl_device_id *   devices,
               cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0
{
    // validate the arguments
    if( (!num_entries && devices) || (!devices && !num_devices) ){
        return CL_INVALID_VALUE;
    }
    // Connect to servers to get devices
    return oclandGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id    device,
                cl_device_info  param_name,
                size_t          param_value_size,
                void *          param_value,
                size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(const cl_context_properties * properties,
                cl_uint                       num_devices ,
                const cl_device_id *          devices,
                void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *),
                void *                        user_data,
                cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    /** callbacks can't be implemented trought network, so
     * if you request a callback CL_OUT_OF_RESOURCES will
     * be reported.
     */
    if(pfn_notify || user_data){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    // validate the arguments
    if( !num_devices || !devices || (!pfn_notify && user_data) ){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return NULL;
    }
    // Count the number of properties
    cl_uint num_properties = 0;
    if(properties){
        while(properties[num_properties] != 0)
            num_properties++;
        num_properties++;   // Final zero must be counted
    }
    /// pfn_notify can't be implmented trought network, so will simply disabled.
    return oclandCreateContext(properties, num_properties, num_devices, devices, NULL, NULL, errcode_ret);
}

CL_API_ENTRY cl_context CL_API_CALL
clCreateContextFromType(const cl_context_properties * properties,
                        cl_device_type                device_type,
                        void (CL_CALLBACK *     pfn_notify)(const char *, const void *, size_t, void *),
                        void *                        user_data,
                        cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    /** callbacks can't be implemented trought network, so
     * if you request a callback CL_OUT_OF_RESOURCES will
     * be reported.
     */
    if(pfn_notify || user_data){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if(!pfn_notify && user_data){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return NULL;
    }
    // Count the number of properties
    cl_uint num_properties = 0;
    if(properties){
        while(properties[num_properties] != 0)
            num_properties++;
        num_properties++;   // Final zero must be counted
    }
    return oclandCreateContextFromType(properties, num_properties, device_type, NULL, NULL, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    return oclandRetainContext(context);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    return oclandReleaseContext(context);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetContextInfo(cl_context         context,
                 cl_context_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(cl_context                     context,
                     cl_device_id                   device,
                     cl_command_queue_properties    properties,
                     cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandCreateCommandQueue(context,device,properties,errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return oclandRetainCommandQueue(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return oclandReleaseCommandQueue(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetCommandQueueInfo(cl_command_queue      command_queue,
                      cl_command_queue_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetCommandQueueInfo(command_queue,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context    context ,
               cl_mem_flags  flags ,
               size_t        size ,
               void *        host_ptr ,
               cl_int *      errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    /** CL_MEM_USE_HOST_PTR and CL_MEM_ALLOC_HOST_PTR are unusable
     * along network, so if detected CL_INVALID_VALUE will
     * returned. In future developments a walking around method can
     * be drafted.
     */
    if( (flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_ALLOC_HOST_PTR) ){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        return NULL;
    }
    // OpenCL specified errors
    if(host_ptr && ( !(flags & CL_MEM_COPY_HOST_PTR) && !(flags & CL_MEM_USE_HOST_PTR) )){
        if(errcode_ret) *errcode_ret=CL_INVALID_HOST_PTR;
        return NULL;
    }
    else if(!host_ptr && ( (flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_COPY_HOST_PTR) )){
        if(errcode_ret) *errcode_ret=CL_INVALID_HOST_PTR;
        return NULL;
    }

    return oclandCreateBuffer(context, flags, size, host_ptr, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    return oclandRetainMemObject(memobj);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    return oclandReleaseMemObject(memobj);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSupportedImageFormats(cl_context           context,
                           cl_mem_flags         flags,
                           cl_mem_object_type   image_type ,
                           cl_uint              num_entries ,
                           cl_image_format *    image_formats ,
                           cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0
{
    if(!num_entries && image_formats){
        return CL_INVALID_VALUE;
    }
    return oclandGetSupportedImageFormats(context,flags,image_type,num_entries,image_formats,num_image_formats);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectInfo(cl_mem            memobj ,
                   cl_mem_info       param_name ,
                   size_t            param_value_size ,
                   void *            param_value ,
                   size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetMemObjectInfo(memobj,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetImageInfo(cl_mem            image ,
               cl_image_info     param_name ,
               size_t            param_value_size ,
               void *            param_value ,
               size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetImageInfo(image,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSampler(cl_context           context ,
                cl_bool              normalized_coords ,
                cl_addressing_mode   addressing_mode ,
                cl_filter_mode       filter_mode ,
                cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandCreateSampler(context,normalized_coords,addressing_mode,filter_mode,errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainSampler(cl_sampler  sampler) CL_API_SUFFIX__VERSION_1_0
{
    return oclandRetainSampler(sampler);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseSampler(cl_sampler  sampler) CL_API_SUFFIX__VERSION_1_0
{
    return oclandReleaseSampler(sampler);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSamplerInfo(cl_sampler          sampler ,
                 cl_sampler_info     param_name ,
                 size_t              param_value_size ,
                 void *              param_value ,
                 size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetSamplerInfo(sampler,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(cl_context         context ,
                          cl_uint            count ,
                          const char **      strings ,
                          const size_t *     lengths ,
                          cl_int *           errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    if((!count) || (!strings)){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        return NULL;
    }
    unsigned int i;
    for(i=0;i<count;i++){
        if(!strings[i]){
            if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
            return NULL;
        }
    }
    return oclandCreateProgramWithSource(context,count,strings,lengths,errcode_ret);
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(cl_context                      context ,
                          cl_uint                         num_devices ,
                          const cl_device_id *            device_list ,
                          const size_t *                  lengths ,
                          const unsigned char **          binaries ,
                          cl_int *                        binary_status ,
                          cl_int *                        errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    if(!num_devices || !device_list || !lengths || !binaries){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        return NULL;
    }
    unsigned int i;
    for(i=0;i<num_devices;i++){
        if(!lengths[i] || !binaries[i]){
            if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
            return NULL;
        }
    }
    return oclandCreateProgramWithBinary(context,num_devices,device_list,lengths,binaries,binary_status,errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(cl_program  program) CL_API_SUFFIX__VERSION_1_0
{
    return clRetainProgram(program);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program  program) CL_API_SUFFIX__VERSION_1_0
{
    return oclandReleaseProgram(program);
}

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program            program ,
               cl_uint               num_devices ,
               const cl_device_id *  device_list ,
               const char *          options ,
               void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
               void *                user_data) CL_API_SUFFIX__VERSION_1_0
{
    /** callbacks can't be implemented trought network, so
     * if you request a callback CL_OUT_OF_RESOURCES will
     * be reported.
     */
    if(pfn_notify || user_data){
        return CL_OUT_OF_RESOURCES;
    }
    if((!pfn_notify  &&  user_data  ) ||
       ( num_devices && !device_list) ||
       (!num_devices &&  device_list) ){
        return CL_INVALID_VALUE;
    }
    return oclandBuildProgram(program,num_devices,device_list,options,NULL,NULL);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramInfo(cl_program          program ,
                 cl_program_info     param_name ,
                 size_t              param_value_size ,
                 void *              param_value ,
                 size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetProgramInfo(program,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(cl_program             program ,
                      cl_device_id           device ,
                      cl_program_build_info  param_name ,
                      size_t                 param_value_size ,
                      void *                 param_value ,
                      size_t *               param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetProgramBuildInfo(program,device,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program       program ,
               const char *     kernel_name ,
               cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandCreateKernel(program,kernel_name,errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateKernelsInProgram(cl_program      program ,
                         cl_uint         num_kernels ,
                         cl_kernel *     kernels ,
                         cl_uint *       num_kernels_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandCreateKernelsInProgram(program,num_kernels,kernels,num_kernels_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(cl_kernel     kernel) CL_API_SUFFIX__VERSION_1_0
{
    return oclandRetainKernel(kernel);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0
{
    return oclandReleaseKernel(kernel);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel     kernel ,
               cl_uint       arg_index ,
               size_t        arg_size ,
               const void *  arg_value) CL_API_SUFFIX__VERSION_1_0
{
    return oclandSetKernelArg(kernel,arg_index,arg_size,arg_value);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelInfo(cl_kernel        kernel ,
                cl_kernel_info   param_name ,
                size_t           param_value_size ,
                void *           param_value ,
                size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetKernelInfo(kernel,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelWorkGroupInfo(cl_kernel                   kernel ,
                         cl_device_id                device ,
                         cl_kernel_work_group_info   param_name ,
                         size_t                      param_value_size ,
                         void *                      param_value ,
                         size_t *                    param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetKernelWorkGroupInfo(kernel,device,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clWaitForEvents(cl_uint              num_events ,
                const cl_event *     event_list) CL_API_SUFFIX__VERSION_1_0
{
    if(!num_events || !event_list)
        return CL_INVALID_VALUE;
    return oclandWaitForEvents(num_events,event_list);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetEventInfo(cl_event          event ,
               cl_event_info     param_name ,
               size_t            param_value_size ,
               void *            param_value ,
               size_t *          param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetEventInfo(event,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainEvent(cl_event  event) CL_API_SUFFIX__VERSION_1_0
{
    return oclandRetainEvent(event);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseEvent(cl_event  event) CL_API_SUFFIX__VERSION_1_0
{
    return oclandReleaseEvent(event);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetEventProfilingInfo(cl_event             event ,
                        cl_profiling_info    param_name ,
                        size_t               param_value_size ,
                        void *               param_value ,
                        size_t *             param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return oclandGetEventProfilingInfo(event,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clFlush(cl_command_queue  command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return oclandFlush(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clFinish(cl_command_queue  command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return oclandFinish(command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue     command_queue ,
                    cl_mem               buffer ,
                    cl_bool              blocking_read ,
                    size_t               offset ,
                    size_t               cb ,
                    void *               ptr ,
                    cl_uint              num_events_in_wait_list ,
                    const cl_event *     event_wait_list ,
                    cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    if(!ptr)
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueReadBuffer(command_queue,buffer,blocking_read,offset,cb,ptr,num_events_in_wait_list,event_wait_list,event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue    command_queue ,
                     cl_mem              buffer ,
                     cl_bool             blocking_write ,
                     size_t              offset ,
                     size_t              cb ,
                     const void *        ptr ,
                     cl_uint             num_events_in_wait_list ,
                     const cl_event *    event_wait_list ,
                     cl_event *          event) CL_API_SUFFIX__VERSION_1_0
{
    if(!ptr)
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueWriteBuffer(command_queue,buffer,blocking_write,offset,cb,ptr,num_events_in_wait_list,event_wait_list,event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue     command_queue ,
                    cl_mem               src_buffer ,
                    cl_mem               dst_buffer ,
                    size_t               src_offset ,
                    size_t               dst_offset ,
                    size_t               cb ,
                    cl_uint              num_events_in_wait_list ,
                    const cl_event *     event_wait_list ,
                    cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    if(    ( num_events_in_wait_list && !event_wait_list)
    || (!num_events_in_wait_list &&  event_wait_list))
    return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueCopyBuffer(command_queue,src_buffer,dst_buffer,src_offset,dst_offset,cb,num_events_in_wait_list,event_wait_list,event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadImage(cl_command_queue      command_queue ,
                   cl_mem                image ,
                   cl_bool               blocking_read ,
                   const size_t *        origin ,
                   const size_t *        region ,
                   size_t                row_pitch ,
                   size_t                slice_pitch ,
                   void *                ptr ,
                   cl_uint               num_events_in_wait_list ,
                   const cl_event *      event_wait_list ,
                   cl_event *            event) CL_API_SUFFIX__VERSION_1_0
{
    // Test minimum data properties
    if(   (!ptr)
       || (!origin)
       || (!region))
        return CL_INVALID_VALUE;
    // Correct some values if not provided
    if(!row_pitch)
        row_pitch   = region[0];
    if(!slice_pitch)
        slice_pitch = region[1]*row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (row_pitch   < region[0])
       || (slice_pitch < region[1]*row_pitch)
       || (slice_pitch % row_pitch))
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueReadImage(command_queue,image,blocking_read,
                                  origin,region,
                                  row_pitch,slice_pitch,ptr,
                                  num_events_in_wait_list,event_wait_list,
                                  event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteImage(cl_command_queue     command_queue ,
                    cl_mem               image ,
                    cl_bool              blocking_write ,
                    const size_t *       origin ,
                    const size_t *       region ,
                    size_t               row_pitch ,
                    size_t               slice_pitch ,
                    const void *         ptr ,
                    cl_uint              num_events_in_wait_list ,
                    const cl_event *     event_wait_list ,
                    cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    // Test minimum data properties
    if(   (!ptr)
       || (!origin)
       || (!region))
        return CL_INVALID_VALUE;
    // Correct some values if not provided
    if(!row_pitch)
        row_pitch   = region[0];
    if(!slice_pitch)
        slice_pitch = region[1]*row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (row_pitch   < region[0])
       || (slice_pitch < region[1]*row_pitch)
       || (slice_pitch % row_pitch))
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueWriteImage(command_queue,image,blocking_write,
                                   origin,region,
                                   row_pitch,slice_pitch,ptr,
                                   num_events_in_wait_list,event_wait_list,
                                   event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImage(cl_command_queue      command_queue ,
                   cl_mem                src_image ,
                   cl_mem                dst_image ,
                   const size_t *        src_origin ,
                   const size_t *        dst_origin ,
                   const size_t *        region ,
                   cl_uint               num_events_in_wait_list ,
                   const cl_event *      event_wait_list ,
                   cl_event *            event) CL_API_SUFFIX__VERSION_1_0
{
    // Test minimum data properties
    if(   (!src_origin)
       || (!dst_origin)
       || (!region))
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueCopyImage(command_queue,src_image,dst_image,
                                  src_origin,dst_origin,region,
                                  num_events_in_wait_list,event_wait_list,
                                  event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImageToBuffer(cl_command_queue  command_queue ,
                           cl_mem            src_image ,
                           cl_mem            dst_buffer ,
                           const size_t *    src_origin ,
                           const size_t *    region ,
                           size_t            dst_offset ,
                           cl_uint           num_events_in_wait_list ,
                           const cl_event *  event_wait_list ,
                           cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    // Test minimum data properties
    if(   (!src_origin)
       || (!region))
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueCopyImageToBuffer(command_queue,src_image,dst_buffer,
                                          src_origin,region,dst_offset,
                                          num_events_in_wait_list,event_wait_list,
                                          event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue  command_queue ,
                           cl_mem            src_buffer ,
                           cl_mem            dst_image ,
                           size_t            src_offset ,
                           const size_t *    dst_origin ,
                           const size_t *    region ,
                           cl_uint           num_events_in_wait_list ,
                           const cl_event *  event_wait_list ,
                           cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    // Test minimum data properties
    if(   (!dst_origin)
       || (!region))
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueCopyBufferToImage(command_queue,src_buffer,dst_image,
                                          src_offset,dst_origin,region,
                                          num_events_in_wait_list,event_wait_list,
                                          event);
}

#ifdef CL_API_SUFFIX__VERSION_1_1
CL_API_ENTRY cl_mem CL_API_CALL
clCreateSubBuffer(cl_mem                    buffer ,
                  cl_mem_flags              flags ,
                  cl_buffer_create_type     buffer_create_type ,
                  const void *              buffer_create_info ,
                  cl_int *                  errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    /** CL_MEM_USE_HOST_PTR and CL_MEM_ALLOC_HOST_PTR are unusable
     * along network, so if detected CL_INVALID_VALUE will
     * returned. In future developments a walking around method can
     * be drafted.
     */
    if( (flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_ALLOC_HOST_PTR) ){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        return NULL;
    }

    return oclandCreateSubBuffer(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetMemObjectDestructorCallback(cl_mem  memobj ,
                                 void (CL_CALLBACK * pfn_notify)(cl_mem  memobj , void* user_data),
                                 void * user_data)             CL_API_SUFFIX__VERSION_1_1
{
    /** Callbacks can't be registered in ocland due
     * to the implicit network interface, so this
     * operation may fail ever.
     */
    return CL_INVALID_MEM_OBJECT;
}

CL_API_ENTRY cl_event CL_API_CALL
clCreateUserEvent(cl_context     context ,
                  cl_int *       errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
    return oclandCreateUserEvent(context,errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetUserEventStatus(cl_event    event ,
                     cl_int      execution_status) CL_API_SUFFIX__VERSION_1_1
{
    return oclandSetUserEventStatus(event,execution_status);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetEventCallback(cl_event     event ,
                   cl_int       command_exec_callback_type ,
                   void (CL_CALLBACK *  pfn_notify)(cl_event, cl_int, void *),
                   void *       user_data) CL_API_SUFFIX__VERSION_1_1
{
    if(!pfn_notify || (command_exec_callback_type != CL_COMPLETE))
        return CL_INVALID_VALUE;
    /** Callbacks can't be registered in ocland due
     * to the implicit network interface, so this
     * operation may fail ever.
     */
    return CL_INVALID_EVENT;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBufferRect(cl_command_queue     command_queue ,
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
                        cl_event *           event) CL_API_SUFFIX__VERSION_1_1
{
    // Test minimum data properties
    if(   (!ptr)
       || (!buffer_origin)
       || (!host_origin)
       || (!region))
        return CL_INVALID_VALUE;
    // Correct some values if not provided
    if(!buffer_row_pitch)
        buffer_row_pitch   = region[0];
    if(!host_row_pitch)
        host_row_pitch     = region[0];
    if(!buffer_slice_pitch)
        buffer_slice_pitch = region[1]*buffer_row_pitch;
    if(!host_slice_pitch)
        host_slice_pitch   = region[1]*host_row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (buffer_row_pitch   < region[0])
       || (host_row_pitch     < region[0])
       || (buffer_slice_pitch < region[1]*buffer_row_pitch)
       || (host_slice_pitch   < region[1]*host_row_pitch)
       || (buffer_slice_pitch % buffer_row_pitch)
       || (host_slice_pitch   % host_row_pitch))
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueReadBufferRect(command_queue,buffer,blocking_read,
                                       buffer_origin,host_origin,region,
                                       buffer_row_pitch,buffer_slice_pitch,
                                       host_row_pitch,host_slice_pitch,ptr,
                                       num_events_in_wait_list,event_wait_list,
                                       event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBufferRect(cl_command_queue     command_queue ,
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
                         cl_event *           event) CL_API_SUFFIX__VERSION_1_1
{
    // Test minimum data properties
    if(   (!ptr)
       || (!buffer_origin)
       || (!host_origin)
       || (!region))
        return CL_INVALID_VALUE;
    // Correct some values if not provided
    if(!buffer_row_pitch)
        buffer_row_pitch   = region[0];
    if(!host_row_pitch)
        host_row_pitch     = region[0];
    if(!buffer_slice_pitch)
        buffer_slice_pitch = region[1]*buffer_row_pitch;
    if(!host_slice_pitch)
        host_slice_pitch   = region[1]*host_row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (buffer_row_pitch   < region[0])
       || (host_row_pitch     < region[0])
       || (buffer_slice_pitch < region[1]*buffer_row_pitch)
       || (host_slice_pitch   < region[1]*host_row_pitch)
       || (buffer_slice_pitch % buffer_row_pitch)
       || (host_slice_pitch   % host_row_pitch))
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueWriteBufferRect(command_queue,buffer,blocking_write,
                                        buffer_origin,host_origin,region,
                                        buffer_row_pitch,buffer_slice_pitch,
                                        host_row_pitch,host_slice_pitch,ptr,
                                        num_events_in_wait_list,event_wait_list,
                                        event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferRect(cl_command_queue     command_queue ,
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
                        cl_event *           event) CL_API_SUFFIX__VERSION_1_1
{
    // Test minimum data properties
    if(   (!src_origin)
       || (!dst_origin)
       || (!region))
        return CL_INVALID_VALUE;
    // Correct some values if not provided
    if(!src_row_pitch)
        src_row_pitch   = region[0];
    if(!dst_row_pitch)
        dst_row_pitch   = region[0];
    if(!src_slice_pitch)
        src_slice_pitch = region[1]*src_row_pitch;
    if(!dst_slice_pitch)
        dst_slice_pitch = region[1]*dst_row_pitch;
    if(   (!region[0]) || (!region[1]) || (!region[2])
       || (src_row_pitch   < region[0])
       || (dst_row_pitch   < region[0])
       || (src_slice_pitch < region[1]*src_row_pitch)
       || (dst_slice_pitch < region[1]*dst_row_pitch)
       || (src_slice_pitch % src_row_pitch)
       || (dst_slice_pitch % dst_row_pitch))
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueCopyBufferRect(command_queue,src_buffer,dst_buffer,
                                       src_origin,dst_origin,region,
                                       src_row_pitch,src_slice_pitch,
                                       dst_row_pitch,dst_slice_pitch,
                                       num_events_in_wait_list,
                                       event_wait_list,event);
}
#endif

#ifdef CL_API_SUFFIX__VERSION_1_2
CL_API_ENTRY cl_int CL_API_CALL
clCreateSubDevices(cl_device_id                         in_device,
                   const cl_device_partition_property * properties,
                   cl_uint                              num_entries,
                   cl_device_id                       * out_devices,
                   cl_uint                            * num_devices) CL_API_SUFFIX__VERSION_1_2
{
    if(!out_devices)
        num_entries = 0;
    // Count the number of properties
    cl_uint num_properties = 0;
    if(properties){
        while(properties[num_properties] != 0)
            num_properties++;
        num_properties++;   // Final zero must be counted
    }
    return oclandCreateSubDevices(in_device, properties, num_properties, num_entries, out_devices, num_devices);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    return oclandRetainDevice(device);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    return oclandReleaseDevice(device);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage(cl_context              context,
              cl_mem_flags            flags,
              const cl_image_format * image_format,
              const cl_image_desc *   image_desc,
              void *                  host_ptr,
              cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    /** CL_MEM_USE_HOST_PTR and CL_MEM_ALLOC_HOST_PTR are unusable
     * along network, so if detected CL_INVALID_VALUE will
     * returned. In future developments a walking around method can
     * be drafted.
     */
    if( (flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_ALLOC_HOST_PTR) ){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        return NULL;
    }

    return oclandCreateImage(context, flags, image_format, image_desc, host_ptr, errcode_ret);
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBuiltInKernels(cl_context             context ,
                                  cl_uint                num_devices ,
                                  const cl_device_id *   device_list ,
                                  const char *           kernel_names ,
                                  cl_int *               errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    if(!num_devices || !device_list || !kernel_names){
        if(errcode_ret) *errcode_ret=CL_INVALID_VALUE;
        return NULL;
    }
    return oclandCreateProgramWithBuiltInKernels(context,num_devices,device_list,kernel_names,errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clCompileProgram(cl_program            program ,
                 cl_uint               num_devices ,
                 const cl_device_id *  device_list ,
                 const char *          options ,
                 cl_uint               num_input_headers ,
                 const cl_program *    input_headers,
                 const char **         header_include_names ,
                 void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
                 void *                user_data) CL_API_SUFFIX__VERSION_1_2
{
    /** callbacks can't be implemented trought network, so
     * if you request a callback CL_OUT_OF_RESOURCES will
     * be reported.
     */
    if(pfn_notify || user_data){
        return CL_OUT_OF_RESOURCES;
    }
    if((!pfn_notify  &&  user_data  ) ||
       ( num_devices && !device_list) ||
       (!num_devices &&  device_list) ||
       (!num_input_headers && ( input_headers ||  header_include_names) ) ||
       ( num_input_headers && (!input_headers || !header_include_names) ) ){
        return CL_INVALID_VALUE;
    }
    return oclandCompileProgram(program,num_devices,device_list,options,num_input_headers,input_headers,header_include_names,NULL,NULL);
}

CL_API_ENTRY cl_program CL_API_CALL
clLinkProgram(cl_context            context ,
              cl_uint               num_devices ,
              const cl_device_id *  device_list ,
              const char *          options ,
              cl_uint               num_input_programs ,
              const cl_program *    input_programs ,
              void (CL_CALLBACK *   pfn_notify)(cl_program  program , void *  user_data),
              void *                user_data ,
              cl_int *              errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    /** callbacks can't be implemented trought network, so
     * if you request a callback CL_OUT_OF_RESOURCES will
     * be reported.
     */
    if(pfn_notify || user_data){
        if(errcode_ret) *errcode_ret=CL_OUT_OF_RESOURCES;
        return NULL;
    }
    if((!pfn_notify  &&  user_data  ) ||
       ( num_devices && !device_list) ||
       (!num_devices &&  device_list) ||
       (!num_input_programs || !input_programs) ){
        if(errcode_ret) *errcode_ret=CL_OUT_OF_RESOURCES;
        return NULL;
    }
    return oclandLinkProgram(context,num_devices,device_list,options,num_input_programs,input_programs,NULL,NULL,errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clUnloadPlatformCompiler(cl_platform_id  platform) CL_API_SUFFIX__VERSION_1_2
{
    return clUnloadPlatformCompiler(platform);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelArgInfo(cl_kernel        kernel ,
                   cl_uint          arg_indx ,
                   cl_kernel_arg_info   param_name ,
                   size_t           param_value_size ,
                   void *           param_value ,
                   size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_2
{
    return oclandGetKernelArgInfo(kernel,arg_indx,param_name,param_value_size,param_value,param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillBuffer(cl_command_queue    command_queue ,
                    cl_mem              buffer ,
                    const void *        pattern ,
                    size_t              pattern_size ,
                    size_t              offset ,
                    size_t              cb ,
                    cl_uint             num_events_in_wait_list ,
                    const cl_event *    event_wait_list ,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_2
{
    if((!pattern) || (!pattern_size))
        return CL_INVALID_VALUE;
    if((offset % pattern_size) || (cb % pattern_size))
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueFillBuffer(command_queue,buffer,pattern,pattern_size,offset,cb,num_events_in_wait_list,event_wait_list,event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillImage(cl_command_queue    command_queue ,
                   cl_mem              image ,
                   const void *        fill_color ,
                   const size_t *      origin ,
                   const size_t *      region ,
                   cl_uint             num_events_in_wait_list ,
                   const cl_event *    event_wait_list ,
                   cl_event *          event) CL_API_SUFFIX__VERSION_1_2
{
    // To use this method before we may get the size of fill_color
    size_t fill_color_size = 4*sizeof(float);
    cl_image_format image_format;
    cl_int flag = clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &image_format, NULL);
    if(flag != CL_SUCCESS)
        return CL_INVALID_MEM_OBJECT;
    if(    image_format.image_channel_data_type == CL_SIGNED_INT8
        || image_format.image_channel_data_type == CL_SIGNED_INT16
        || image_format.image_channel_data_type == CL_SIGNED_INT32 ){
            fill_color_size = 4*sizeof(int);
    }
    if(    image_format.image_channel_data_type == CL_UNSIGNED_INT8
        || image_format.image_channel_data_type == CL_UNSIGNED_INT16
        || image_format.image_channel_data_type == CL_UNSIGNED_INT32 ){
            fill_color_size = 4*sizeof(unsigned int);
    }
    // Test minimum data properties
    if(   (!fill_color)
       || (!origin)
       || (!region))
        return CL_INVALID_VALUE;
    // Correct some values if not provided
    if(   (!region[0]) || (!region[1]) || (!region[2]) )
        return CL_INVALID_VALUE;
    if(    ( num_events_in_wait_list && !event_wait_list)
        || (!num_events_in_wait_list &&  event_wait_list))
        return CL_INVALID_EVENT_WAIT_LIST;
    return oclandEnqueueFillImage(command_queue,image,
                                  fill_color_size, fill_color,
                                  origin,region,
                                  num_events_in_wait_list,
                                  event_wait_list,event);
}
#endif

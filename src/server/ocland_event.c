/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2015  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with ocland.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ocland/common/usleep.h>
#include <ocland/server/ocland_event.h>

cl_int oclandWaitForEvents(cl_uint num_events, const ocland_event *event_list)
{
    unsigned int i;
    cl_int flag = CL_SUCCESS;
    cl_uint  cl_num_events=0;
    cl_event *cl_event_list = calloc(num_events, sizeof(cl_event));
    if (NULL == cl_event_list)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    // Wait until ocland ends the work, and set OpenCL events
    for(i=0;i<num_events;i++){
        while (event_list[i]->status != CL_COMPLETE){
            usleep(1000);
        }
        if(event_list[i]->event){
            cl_num_events++;
            cl_event_list[i] = event_list[i]->event;
        }
    }
    // Wait for OpenCL events
    if(cl_num_events)
        flag = clWaitForEvents(num_events, cl_event_list);
    free(cl_event_list); cl_event_list = NULL;
    return flag;
}

cl_int oclandGetEventInfo(ocland_event      event ,
                          cl_event_info     param_name ,
                          size_t            param_value_size ,
                          void *            param_value ,
                          size_t *          param_value_size_ret)
{
    cl_int flag = CL_SUCCESS;
    // If don't exist the OpenCL event we must set manually the values
    if(!event->event){
        if(param_name == CL_EVENT_COMMAND_QUEUE){
            if(param_value_size_ret) *param_value_size_ret=sizeof(cl_command_queue);
            if(param_value){
                if(param_value_size < sizeof(cl_command_queue)){
                    return CL_INVALID_VALUE;
                }
                ((cl_command_queue*)param_value)[0] = event->command_queue;
            }
        }
        else if(param_name == CL_EVENT_CONTEXT){
            if(param_value_size_ret) *param_value_size_ret=sizeof(cl_context);
            if(param_value){
                if(param_value_size < sizeof(cl_context)){
                    return CL_INVALID_VALUE;
                }
                ((cl_context*)param_value)[0] = event->context;
            }
        }
        else if(param_name == CL_EVENT_COMMAND_TYPE){
            if(param_value_size_ret) *param_value_size_ret=sizeof(cl_command_type);
            if(param_value){
                if(param_value_size < sizeof(cl_command_type)){
                    return CL_INVALID_VALUE;
                }
                ((cl_command_type*)param_value)[0] = event->command_type;
            }
        }
        else if(param_name == CL_EVENT_COMMAND_EXECUTION_STATUS){
            if(param_value_size_ret) *param_value_size_ret=sizeof(cl_int);
            if(param_value){
                if(param_value_size < sizeof(cl_int)){
                    return CL_INVALID_VALUE;
                }
                ((cl_int*)param_value)[0] = event->status;
            }
        }
        else if(param_name == CL_EVENT_REFERENCE_COUNT){
            if(param_value_size_ret) *param_value_size_ret=sizeof(cl_uint);
            if(param_value){
                if(param_value_size < sizeof(cl_uint)){
                    return CL_INVALID_VALUE;
                }
                ((cl_uint*)param_value)[0] = 1;
            }
        }
        else{
            return CL_INVALID_VALUE;
        }
        return CL_SUCCESS;
    }
    // Otherwise get the OpenCL data
    flag = clGetEventInfo(event->event,
                          param_name,
                          param_value_size,
                          param_value,
                          param_value_size_ret);
    if(flag != CL_SUCCESS){
        return flag;
    }
    // Fix CL_EVENT_COMMAND_EXECUTION_STATUS for asynchronous works,
    // when the OpenCL job can be completed but data transfer still
    // running
    if(param_value && (param_name == CL_EVENT_COMMAND_EXECUTION_STATUS)){
        if( (((cl_int*)param_value)[0] == CL_COMPLETE) && (event->status != CL_COMPLETE) ){
            ((cl_int*)param_value)[0] = CL_RUNNING;
        }
    }
    return CL_SUCCESS;
}

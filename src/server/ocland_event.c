/*clGetEventInfo
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <ocland/server/ocland_event.h>

cl_int oclandWaitForEvents(cl_uint num_events, const ocland_event *event_list)
{
    unsigned int i;
    cl_int flag = CL_SUCCESS;
    cl_event cl_event_list[num_events];
    // Wait until ocland ends the work, and set OpenCL events
    for(i=0;i<num_events;i++){
        while(event_list[i]->status != CL_COMPLETE);
        cl_event_list[i] = event_list[i]->event;
    }
    // Wait for OpenCL events
    flag = clWaitForEvents(num_events, cl_event_list);
    return flag;

}

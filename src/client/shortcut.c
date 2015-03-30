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

#include <ocland/client/shortcut.h>

static unsigned int num_shortcuts = 0;
static shortcut *shortcuts = NULL;

unsigned int addShortcut(void* ocl_ptr, int* socket)
{
    // Look if the shortcut already exist
    if(getShortcut(ocl_ptr)){
        return num_shortcuts;
    }
    if(!shortcuts){
        shortcuts = (shortcut*)malloc( (num_shortcuts + 1) * sizeof(shortcut));
        if(!shortcuts){
            num_shortcuts = 0;
            return 0;
        }
    }
    else{
        shortcuts = (shortcut*)realloc( shortcuts, (num_shortcuts + 1) * sizeof(shortcut));
        if(!shortcuts){
            num_shortcuts = 0;
            return 0;
        }
    }
    // Store new shortcut
    shortcuts[num_shortcuts].ocl_ptr = ocl_ptr;
    shortcuts[num_shortcuts].socket = socket;
    num_shortcuts++;
    return num_shortcuts;
}

unsigned int delShortcut(void* ocl_ptr)
{
    unsigned int i,id=0;
    // Look if the pointer don't exist
    if(!getShortcut(ocl_ptr)){
        return num_shortcuts;
    }
    if(num_shortcuts == 1){
        num_shortcuts = 0;
        if(shortcuts) free(shortcuts); shortcuts=NULL;
        return 0;
    }
    shortcut *backup = shortcuts;
    shortcuts = (shortcut*)malloc( (num_shortcuts - 1) * sizeof(shortcut));
    if(!shortcuts){
        if(backup) free(backup); backup=NULL;
        num_shortcuts = 0;
        return 0;
    }
    // Store queues not affected
    for(i=0;i<num_shortcuts;i++){
        if(ocl_ptr == backup[i].ocl_ptr){
            continue;
        }
        shortcuts[id] = backup[i];
        id++;
    }
    num_shortcuts--;
    if(backup) free(backup); backup=NULL;
    return num_shortcuts;
}

int* getShortcut(void* ocl_ptr)
{
    unsigned int i;
    for(i=0;i<num_shortcuts;i++){
        if(ocl_ptr == shortcuts[i].ocl_ptr)
            return shortcuts[i].socket;
    }
    return NULL;
}

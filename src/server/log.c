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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ocland/server/log.h>

/// Log file path
static char* log_path = NULL;

int setLogFile(const char* path)
{
    // Clean
    if(log_path) free(log_path); log_path=NULL;
    // Get log file path
    log_path = (char*)malloc((strlen(path)+1)*sizeof(char));
    strcpy(log_path, path);
    // Test if file can be touched
    FILE *test = fopen(log_path, "w");
    if(!test){
        if(log_path) free(log_path); log_path=NULL;
        return 0;
    }
    fclose(test);
    // Redirects standart outputs (checking the operation)
    FILE *f = NULL;
    f = freopen(log_path, "a+", stdout);
    if(!f)
        return 0;
    f = freopen(log_path, "a+", stderr);
    if(!f)
        return 0;
    return 1;
}

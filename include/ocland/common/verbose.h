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

/** @file
 * @brief Verbosity tools.
 *
 * This file provides a set of tools to verbose data without taking care about
 * the OCLAND_CLIENT_VERBOSE definition.
 */

#ifndef VERBOSE_H_INCLUDED
#define VERBOSE_H_INCLUDED

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/** @brief Get the literal translation from \a err_code to a string.
 * @param err_code OpenCL error code
 * @return Its name, the integer value if the errors is not known.
 */
const char* OpenCLError(cl_int err_code);

/// String with the file and line of a function
#define WHERESTR  "[file %s, line %d]: "
/// Arguments for WHERESTR
#define WHEREARG  __FILE__, __LINE__
/// Print an error
#define DEBUGPRINT2(...)       fprintf(stderr, __VA_ARGS__)
/** @brief Print an error, specifying the file and line where it is produced
 * @see DEBUGPRINT2
 */
#define DEBUGPRINT(_fmt, ...)  DEBUGPRINT2(WHERESTR _fmt, WHEREARG, __VA_ARGS__)

#ifndef __func__
    #define __func__ __FUNCTION__
#endif

#ifdef OCLAND_VERBOSE
    /** @brief Print the line and function name.
     *
     * It is not printing nothing if OCLAND_CLIENT_VERBOSE is not defined.
     */
    #define VERBOSE_IN() {printf("[line %d]: %s...\n", __LINE__, __func__); fflush(stdout);}
    /** @brief Print the function result (in OpenCL terms).
     *
     * It is not printing nothing if OCLAND_CLIENT_VERBOSE is not defined.
     * @see OpenCLError
     */
    #define VERBOSE_OUT(flag) {printf("\t%s -> %s\n", __func__, OpenCLError(flag)); fflush(stdout);}
    /** @brief Print a screen message.
     *
     * It is not printing nothing if OCLAND_CLIENT_VERBOSE is not defined.
     */
    #define VERBOSE(...) {printf(__VA_ARGS__); fflush(stdout);}
#else
    /** @brief Print the line and function name.
     *
     * It is not printing nothing if OCLAND_CLIENT_VERBOSE is not defined.
     */
    #define VERBOSE_IN()
    /** @brief Print the function result (in OpenCL terms).
     *
     * It is not printing nothing if OCLAND_CLIENT_VERBOSE is not defined.
     * @see OpenCLError()
     */
    #define VERBOSE_OUT(flag)
    /** @brief Print a screen message.
     *
     * It is not printing nothing if OCLAND_CLIENT_VERBOSE is not defined.
     */
    #define VERBOSE(...)
#endif


#endif // VERBOSE_H_INCLUDED

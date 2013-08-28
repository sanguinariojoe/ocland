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

#ifndef DATAPACK_H_INCLUDED
#define DATAPACK_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif
#ifdef HAVE_LZO
#include <lzo/lzo1x.h>
#endif
#ifdef HAVE_LZMA
#include <lzma.h>
#endif
#ifdef HAVE_BZIP2
#include <bzlib.h>
#endif
#ifdef HAVE_LZ4
#include <lz4.h>
#endif

/// Data packing structure
struct dataPackStruct
{
    /// Data size
    size_t size;
    /// Data array
    void *data;
};

/// Simple data call
typedef struct dataPackStruct dataPack;

/** Pack the data to accelerate the transmission.
 * @param in Data to pack.
 * @return Packed data.
 * @note The output packed data will be allocated internally in the method, so
 * free(out.data) must be called outside later.
 */
dataPack pack(dataPack in);

/** Unpack the data.
 * @param out Unpacked output data.
 * @param in Data to unpack.
 * @note Before calling this method, out.size should be provided and out.data
 * memory allocated.
 * @note out.size may change during the unpacking.
 */
void unpack(dataPack out, dataPack in);

#endif // DATAPACK_H_INCLUDED

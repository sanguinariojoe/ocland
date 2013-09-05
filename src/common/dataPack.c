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

#include <ocland/common/dataPack.h>

// ---------------------------------------------------------
// uncompressed data alternative
// The uncompressed data may take a lot of network time,
// with a heavy bottleneck blocking the commands
// transmission. It is not recommended.
// ---------------------------------------------------------
#ifdef HAVE_NO_COMPRESSION
dataPack pack(dataPack in)
{
    dataPack out;
    out.size = in.size;
    out.data = malloc(out.size);
    memcpy(out.data, in.data, out.size);
    return out;
}

void unpack(dataPack out, dataPack in)
{
    memcpy(out.data, in.data, out.size);
}
#endif

// ---------------------------------------------------------
// zlib alternative
// unsafe alternative, may fail several times, so it is
// strongly not recommended.
// ---------------------------------------------------------
#ifdef HAVE_ZLIB
dataPack pack(dataPack in)
{
    dataPack out;
    out.size = 0;
    out.data = NULL;
    // Get the output size bound
    uLong size = compressBound((uLong)in.size);
    Bytef *data = (Bytef*)malloc((size_t)size);
    // Compress the data
    int ret = compress(data, &size, in.data, (uLong)(in.size));
    if(ret != Z_OK){
        printf("Error compressing the data!\n\tzlib returned "); fflush(stdout);
        if(ret == Z_MEM_ERROR){
            printf("Z_MEM_ERROR\n"); fflush(stdout);
        }
        else if(ret == Z_BUF_ERROR){
            printf("Z_BUF_ERROR\n"); fflush(stdout);
        }
        else{
            printf("an unhandled exception (%d)\n", ret); fflush(stdout);
        }
        return out;
    }
    // Store the output data
    out.size = (size_t)size;
    out.data = malloc(out.size);
    memcpy(out.data, data, out.size);
    free(data); data=NULL;
    return out;
}

void unpack(dataPack out, dataPack in)
{
    uLong size = (uLong)out.size;
    int ret = uncompress(out.data, &size, in.data, (uLong)in.size);
    if(ret != Z_OK){
        printf("Error uncompressing data.\n\tzlib returned "); fflush(stdout);
        if(ret == Z_MEM_ERROR){
            printf("Z_MEM_ERROR\n"); fflush(stdout);
        }
        else if(ret == Z_BUF_ERROR){
            printf("Z_BUF_ERROR\n"); fflush(stdout);
        }
        else if(ret == Z_DATA_ERROR){
            printf("Z_DATA_ERROR\n"); fflush(stdout);
        }
        else{
            printf("an unhandled exception (%d)\n", ret); fflush(stdout);
        }
    }
}
#endif

// ---------------------------------------------------------
// LZO alternative
// Really robust and fast, not great compression rates can
// be expected.
// ---------------------------------------------------------
#ifdef HAVE_LZO
dataPack pack(dataPack in)
{
    dataPack out;
    out.size = 0;
    out.data = NULL;
    // Initialize the library. Only one time?
    if (lzo_init() != LZO_E_OK)
    {
        printf("Error initializing lzo!\n");
        return out;
    }
    // Get the output size bound
    lzo_uint inSize  = (lzo_uint)(in.size);
    lzo_uint outSize = inSize + inSize / 16 + 64 + 3;
    lzo_bytep data   = (lzo_bytep)malloc((size_t)outSize);
    // Compress the data
    lzo_voidp wrkmem = (lzo_voidp)malloc(LZO1X_1_MEM_COMPRESS);
    int ret = lzo1x_1_compress(in.data,inSize,data,&outSize,wrkmem);
    if(ret != LZO_E_OK){
        printf("Error compressing data.\n\tlzo returned %d\n", ret); fflush(stdout);
    }
    // Store the output data
    out.size = (size_t)outSize;
    out.data = malloc(out.size);
    memcpy(out.data, data, out.size);
    free(data); data=NULL;
    free(wrkmem); wrkmem=NULL;
    return out;
}

void unpack(dataPack out, dataPack in)
{
    lzo_uint inSize  = (lzo_uint)in.size;
    lzo_uint outSize = (lzo_uint)out.size;
    int ret = lzo1x_decompress(in.data,inSize,out.data,&outSize,NULL);
    if(ret != LZO_E_OK){
        printf("Error uncompressing data.\n\tlzo returned %d\n", ret); fflush(stdout);
    }
    out.size = (size_t)outSize;
}
#endif

// ---------------------------------------------------------
// LZMA alternative
// Too slow, good compression capabilities. Some fails have
// been detected, so not recommended either.
// ---------------------------------------------------------
#ifdef HAVE_LZMA

#define LZMA_COMPRESSION_LEVEL 1

dataPack pack(dataPack in)
{
    dataPack out;
    out.size = 0;
    out.data = NULL;
    // Get the output size bound
    size_t size   = in.size + (in.size >> 2) + 128;
    uint8_t *data = (uint8_t*)malloc(size);
    // Compress the data
    lzma_ret ret = lzma_easy_buffer_encode(
                       LZMA_COMPRESSION_LEVEL, LZMA_CHECK_CRC32, NULL,
                       in.data, in.size,
                       data, &(out.size), size);
    if(ret != LZMA_OK){
        printf("Error compressing data.\n\tlzo returned %d\n", ret); fflush(stdout);
    }
    // Store the output data
    out.data = malloc(out.size);
    memcpy(out.data, data, out.size);
    free(data); data=NULL;
    return out;
}

void unpack(dataPack out, dataPack in)
{
    static const size_t kMemLimit = 1 << 30;  // 1 GB.
    lzma_stream strm = LZMA_STREAM_INIT;
    size_t used=0, avail0=8192;
    lzma_ret ret = lzma_stream_decoder(&strm, kMemLimit, LZMA_CONCATENATED);
    if (ret != LZMA_OK){
        printf("Error creating lzma decoder.\n\tlzma returned %d\n", ret); fflush(stdout);
        return;
    }
    strm.next_in   = in.data;
    strm.avail_in  = in.size;
    strm.next_out  = out.data;
    strm.avail_out = avail0;
    while(1) {
        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
        if(ret == LZMA_STREAM_END) {
            used += avail0 - strm.avail_out;
            if (0 != strm.avail_in){  // Guaranteed by lzma_stream_decoder().
                printf("lzma reported end of stream, but some data still being available.\n"); fflush(stdout);
            }
            lzma_end(&strm);
            return;
        }
        if(ret != LZMA_OK){
            printf("Error uncompressing data.\n\tlzma returned %d\n", ret); fflush(stdout);
            return;
        }
        if(strm.avail_out == 0){
            used += avail0 - strm.avail_out;
            strm.next_out = (char*)(out.data) + used;
            if(used + avail0 > out.size)
                avail0 = out.size - used;
            strm.avail_out = avail0;
        }
    }
}
#endif

// ---------------------------------------------------------
// BZIP2 alternative
// Too slow, but robust and good compression ratio.
// ---------------------------------------------------------
#ifdef HAVE_BZIP2

#define BZIP2_COMPRESSION_LEVEL 1

dataPack pack(dataPack in)
{
    dataPack out;
    out.size = 0;
    out.data = NULL;
    // Get the output size bound
    size_t size = (size_t)(1.01*in.size) + 601;
    void *data  = malloc(size);
    // Compress the data
    int ret = BZ2_bzBuffToBuffCompress(data, &size, in.data, in.size,
                                       BZIP2_COMPRESSION_LEVEL, 0, 0);
    if(ret != BZ_OK){
        printf("Error compressing the data!\n\tbzip2 returned "); fflush(stdout);
        if(ret == BZ_CONFIG_ERROR){
            printf("BZ_CONFIG_ERROR\n"); fflush(stdout);
        }
        else if(ret == BZ_PARAM_ERROR){
            printf("BZ_PARAM_ERROR\n"); fflush(stdout);
        }
        else if(ret == BZ_MEM_ERROR){
            printf("BZ_MEM_ERROR\n"); fflush(stdout);
        }
        else if(ret == BZ_OUTBUFF_FULL){
            printf("BZ_OUTBUFF_FULL\n"); fflush(stdout);
        }
        else{
            printf("an unhandled exception (%d)\n", ret); fflush(stdout);
        }
        return out;
    }
    // Store the output data
    out.size = (size_t)size;
    out.data = malloc(out.size);
    memcpy(out.data, data, out.size);
    free(data); data=NULL;
    return out;
}

void unpack(dataPack out, dataPack in)
{
    int ret = BZ2_bzBuffToBuffDecompress(out.data,&(out.size),in.data,in.size,0,0);
    if(ret != BZ_OK){
        printf("Error uncompressing data.\n\tbzip2 returned "); fflush(stdout);
        if(ret == BZ_CONFIG_ERROR){
            printf("BZ_CONFIG_ERROR\n"); fflush(stdout);
        }
        else if(ret == BZ_PARAM_ERROR){
            printf("BZ_PARAM_ERROR\n"); fflush(stdout);
        }
        else if(ret == BZ_MEM_ERROR){
            printf("BZ_MEM_ERROR\n"); fflush(stdout);
        }
        else if(ret == BZ_OUTBUFF_FULL){
            printf("BZ_OUTBUFF_FULL\n"); fflush(stdout);
        }
        else if(ret == BZ_DATA_ERROR){
            printf("BZ_DATA_ERROR\n"); fflush(stdout);
        }
        else if(ret == BZ_DATA_ERROR_MAGIC){
            printf("BZ_DATA_ERROR_MAGIC\n"); fflush(stdout);
        }
        else if(ret == BZ_UNEXPECTED_EOF){
            printf("BZ_UNEXPECTED_EOF\n"); fflush(stdout);
        }
        else{
            printf("an unhandled exception (%d)\n", ret); fflush(stdout);
        }
    }
}
#endif

// ---------------------------------------------------------
// lz4 alternative
// extremely fast, but with low compression ratios.
// ---------------------------------------------------------
#ifdef HAVE_LZ4
dataPack pack(dataPack in)
{
    dataPack out;
    out.size = 0;
    out.data = NULL;
    // Get the output size bound
    int size   = (int)(in.size + ((in.size)/255) + 16);
    void *data = malloc((size_t)size);
    // Compress the data
    size = LZ4_compress(in.data, data, (int)(in.size));
    if(size == 0){
        printf("Error compressing the data!\n\tlz4 returned null size output.\n"); fflush(stdout);
        return out;
    }
    // Store the output data
    out.size = (size_t)size;
    out.data = malloc(out.size);
    memcpy(out.data, data, out.size);
    free(data); data=NULL;
    return out;
}

void unpack(dataPack out, dataPack in)
{
    int size = LZ4_decompress_safe(in.data, out.data, (int)(in.size), (int)(out.size));
    if(size <= 0){
        printf("Error uncompressing data with lz4.\n"); fflush(stdout);
    }
}
#endif

// ---------------------------------------------------------
// Snappy alternative
// extremely fast, and acceptable compression ratios.
// ---------------------------------------------------------
#ifdef HAVE_SNAPPY
dataPack pack(dataPack in)
{
    dataPack out;
    out.size = 0;
    out.data = NULL;
    // Get the output size bound
    size_t size = snappy_max_compressed_length(in.size);
    void *data = malloc(size);
    // Compress the data
    snappy_status status = snappy_compress(in.data, in.size, data, &size);
    if(status != SNAPPY_OK){
        printf("Error compressing the data!\n\tSnappy returned "); fflush(stdout);
        if(status == SNAPPY_INVALID_INPUT){
            printf("SNAPPY_INVALID_INPUT\n"); fflush(stdout);
        }
        else if(status == SNAPPY_BUFFER_TOO_SMALL){
            printf("SNAPPY_BUFFER_TOO_SMALL\n"); fflush(stdout);
        }
        else{
            printf("an unhandled exception.\n"); fflush(stdout);
        }
        return out;
    }
    // Store the output data
    out.size = size;
    out.data = malloc(out.size);
    memcpy(out.data, data, out.size);
    free(data); data=NULL;
    return out;
}

void unpack(dataPack out, dataPack in)
{
    snappy_status status = snappy_uncompress(in.data, in.size, out.data, &(out.size));
    if(status != SNAPPY_OK){
        printf("Error uncompressing the data!\n\tSnappy returned "); fflush(stdout);
        if(status == SNAPPY_INVALID_INPUT){
            printf("SNAPPY_INVALID_INPUT\n"); fflush(stdout);
        }
        else if(status == SNAPPY_BUFFER_TOO_SMALL){
            printf("SNAPPY_BUFFER_TOO_SMALL\n"); fflush(stdout);
        }
        else{
            printf("an unhandled exception.\n"); fflush(stdout);
        }
    }
}
#endif

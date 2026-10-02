/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkystorage.h"
#include "libkylog.h"
#include "../packages/libkypackages.h"
#include "magic.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h> //for io

#include <pwd.h>
#include <grp.h>

#include <dirent.h>

#include <zip.h>
#include <zlib.h>
#include <bzlib.h>
#include <lzma.h>
#include <lz4.h>
#include <lz4frame.h>
#include <zstd.h>
#include <archive.h>
#include <archive_entry.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

typedef enum _BrowserEnum
{
    None,
    Qax,
    FireFox,
    Chrome,
    Chromium
} BrowserEnum;

static inline void _create_directory(const char *path)
{
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

static inline char *_get_file_name(char *file)
{
    char *p = strrchr(file, '.');
    if (NULL != p)
    {
        *p = '\0';
    }
    return file;
}

/**************************************
 * zip格式
 **************************************/

static void add_directory_to_zip(zip_t *zip, const char *basic_path, const char *relative_path)
{
    struct stat st;
    if (stat(basic_path, &st) == -1)
    {
        klog_err("stat %s fail", basic_path);
        return;
    }

    // Add the directory to the ZIP (if it's a directory)
    if (S_ISDIR(st.st_mode))
    {
        zip_dir_add(zip, relative_path, ZIP_FL_ENC_GUESS);
    }
    else
    {
        zip_source_t *source = zip_source_file(zip, basic_path, 0, 0);
        zip_file_add(zip, relative_path, source, ZIP_FL_ENC_GUESS);
        zip_source_close(source);
        return;
    }

    DIR *d = opendir(basic_path);
    if (!d)
    {
        klog_err("opendir %s fail\n", basic_path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL)
    {
        if ((strcmp(entry->d_name, ".") == 0) && (strcmp(entry->d_name, "..") == 0))
            continue;

        char new_relative_path[1024], new_basic_path[1024];
        snprintf(new_basic_path, sizeof(new_basic_path), "%s/%s", basic_path, entry->d_name);
        snprintf(new_relative_path, sizeof(new_relative_path), "%s/%s", relative_path, entry->d_name);

        if (entry->d_type == DT_DIR)
        {
            // Recursively add the directory
            add_directory_to_zip(zip, new_basic_path, new_relative_path);
        }
        else
        {
            zip_source_t *source = zip_source_file(zip, new_basic_path, 0, 0);
            zip_file_add(zip, new_relative_path, source, ZIP_FL_ENC_GUESS);
            zip_source_close(source);
        }
    }
    closedir(d);
}

static int _zip_data_compress(char *iData, int iSize, char *oData, int *oSize)
{
    return -2;
}

static int _zip_data_decompress(char *iData, int iSize, char *oData, int *oSize)
{
    return -2;
}

static int _zip_file_compress(char *source_file, char *dest_file)
{
    char basic_path[4096], relative_path[4096];
    memset(basic_path, 0, 4096);
    memset(relative_path, 0, 4096);

    if (!realpath(source_file, basic_path))
        return -1;

    char *p = strrchr(basic_path, '/');
    if (NULL == p)
        strncpy(relative_path, basic_path, sizeof(relative_path) - 1);
    else
        strncpy(relative_path, p + 1, sizeof(relative_path) - 1);

    int err = 0;
    zip_t *zip = zip_open(dest_file, ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (zip == NULL)
    {
        char buf[4096];
        zip_error_to_str(buf, sizeof(buf), err, errno);
        klog_err("Failed to open zip: %s\n", buf);
        zip_close(zip);
        return -1;
    }

    add_directory_to_zip(zip, basic_path, relative_path);

    if (zip_close(zip) < 0)
    {
        klog_err("Failed to close zip: %s\n", zip_strerror(zip));
        return -1;
    }
    return 0;
}

static int _zip_file_decompress(char *source_file, char *dest_path)
{
    int err = 0;
    zip_t *zip = zip_open(source_file, 0, &err);

    if (!zip)
    {
        char errbuf[4096];
        zip_error_to_str(errbuf, sizeof(errbuf), err, errno);
        klog_err("Failed to open zip file: %s\n", errbuf);
        return -1;
    }

    zip_int64_t num_entries = zip_get_num_entries(zip, 0);
    for (zip_int64_t i = 0; i < num_entries; i++)
    {
        const char *file_name = zip_get_name(zip, i, 0);
        if (!file_name)
        {
            klog_err("Failed to get zip entry name\n");
            continue;
        }

        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", dest_path, file_name);

        // Check if the entry is a directory
        if (file_name[strnlen(file_name, STRNLEN_MAX) - 1] == '/')
        {
            _create_directory(full_path);
        }
        else
        {
            // Create directories if necessary
            char *p = strchr(full_path + 1, '/');
            while (p != NULL)
            {
                *p = '\0';
                _create_directory(full_path);
                *p = '/';
                p = strchr(p + 1, '/');
            }

            // Extract the file
            zip_file_t *z_file = zip_fopen(zip, file_name, 0);
            if (!z_file)
            {
                klog_err("Failed to open zip file entry: %s\n", file_name);
                continue;
            }

            FILE *out_file = fopen(full_path, "wb");
            if (!out_file)
            {
                klog_err("Failed to create file: %s\n", full_path);
                zip_fclose(z_file);
                continue;
            }

            // Read the data from the zip file and write to the output file
            char buffer[8192];
            zip_int64_t n;
            while ((n = zip_fread(z_file, buffer, sizeof(buffer))) > 0)
            {
                fwrite(buffer, 1, n, out_file);
            }

            fclose(out_file);
            zip_fclose(z_file);
        }
    }

    zip_close(zip);

    return 0;
}

/**************************************
 * gzip格式
 **************************************/

static int _gzip_data_compress(const char *iData, int iSize, char *oData, int *oSize)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                     15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        return -1;
    }

    strm.next_in = (Bytef *)iData;
    strm.avail_in = iSize;
    strm.next_out = (Bytef *)oData;
    strm.avail_out = *oSize;

    if (deflate(&strm, Z_FINISH) != Z_STREAM_END)
    {
        deflateEnd(&strm);
        return -1;
    }

    *oSize = strm.total_out;
    deflateEnd(&strm);
    return 0;
}

static int _gzip_data_decompress(const char *iData, int iSize, char *oData, int *oSize)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    if (inflateInit2(&strm, 15 + 16) != Z_OK)
    {
        return -1;
    }

    strm.next_in = (Bytef *)iData;
    strm.avail_in = iSize;
    strm.next_out = (Bytef *)oData;
    strm.avail_out = *oSize;

    int ret = inflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END)
    {
        inflateEnd(&strm);
        return -1;
    }

    *oSize = strm.total_out;
    inflateEnd(&strm);
    return 0;
}

static int _gzip_file_compress(char *source_file, char *dest_file)
{
    // 打开输入文件和输出gz文件
    FILE *src = fopen(source_file, "rb");
    gzFile dst = gzopen(dest_file, "wb");

    if (!src || !dst)
    {
        klog_err("File open error\n");
        return -1;
    }

    // 定义缓冲区
    char buffer[1024];
    int bytesRead;

    // 读取源文件，压缩并写入目标gz文件
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0)
    {
        gzwrite(dst, buffer, bytesRead);
    }

    fclose(src);
    gzclose(dst);
    return 0;
}

static int _gzip_file_decompress(char *source_file, char *dest_path)
{
    // 打开输入的gz文件和输出文件
    gzFile src = gzopen(source_file, "rb");

    char *file[128], dest_file[1024];
    strncpy(file, basename(source_file), sizeof(file) - 1);
    snprintf(dest_file, sizeof(dest_file), "%s/%s", dest_path, _get_file_name(file));

    FILE *dst = fopen(dest_file, "wb");

    if (!src || !dst)
    {
        klog_err("File open error\n");
        return -1;
    }

    // 定义缓冲区
    char buffer[1024];
    int bytesRead;

    // 读取源gz文件，解压并写入目标文件
    while ((bytesRead = gzread(src, buffer, sizeof(buffer))) > 0)
    {
        fwrite(buffer, 1, bytesRead, dst);
    }

    gzclose(src);
    fclose(dst);
    return 0;
}

/**************************************
 * bzip格式
 **************************************/

static int _bzip_data_compress(char *iData, int iSize, char *oData, int *oSize)
{
    int ret = BZ2_bzBuffToBuffCompress(oData, oSize, iData, iSize, 2, 0, 30);
    if (BZ_OK != ret)
    {
        klog_err("BZ2_bzBuffToBuffCompress fail. errno: %d\n", ret);
        return -1;
    }
    return 0;
}

static int _bzip_data_decompress(char *iData, int iSize, char *oData, int *oSize)
{
    int ret = BZ2_bzBuffToBuffDecompress(oData, oSize, iData, iSize, (iSize <= (1024 * 900)) ? 1 : 0, 0);
    if (BZ_OK != ret)
    {
        klog_err("BZ2_bzBuffToBuffDeCompress fail. errno: %d\n", ret);
        return -1;
    }
    return 0;
}

static int _bzip_file_compress(char *source_file, char *dest_file)
{
    FILE *in = fopen(source_file, "rb");
    if (!in)
    {
        klog_err("Can't open source file\n");
        return -1;
    }

    FILE *out = fopen(dest_file, "wb");
    if (!out)
    {
        fclose(in);
        klog_err("Can't open destination file\n");
        return -1;
    }

    BZFILE *bzFile = BZ2_bzWriteOpen(NULL, out, 9, 0, 0);
    if (!bzFile)
    {
        fclose(in);
        fclose(out);
        klog_err("Can't open BZ2 stream\n");
        return -1;
    }

    char buffer[4096];
    int bytesRead;
    while ((bytesRead = fread(buffer, 1, 4096, in)) > 0)
    {
        BZ2_bzWrite(NULL, bzFile, buffer, bytesRead);
    }

    BZ2_bzWriteClose(NULL, bzFile, 0, NULL, NULL);
    fclose(in);
    fclose(out);
    return 0;
}

static int _bzip_file_decompress(char *source_file, char *dest_path)
{
    FILE *in = fopen(source_file, "rb");
    if (!in)
    {
        klog_err("Can't open source file\n");
        return -1;
    }

    char *file[128], dest_file[1024];
    strncpy(file, basename(source_file), sizeof(file) - 1);
    snprintf(dest_file, sizeof(dest_file), "%s/%s", dest_path, _get_file_name(file));

    FILE *out = fopen(dest_file, "wb");
    if (!out)
    {
        fclose(in);
        klog_err("Can't open destination file\n");
        return -1;
    }

    int bz_error = BZ_OK;
    BZFILE *bzFile = BZ2_bzReadOpen(&bz_error, in, 0, 0, NULL, 0);
    if (!bzFile)
    {
        fclose(in);
        fclose(out);
        klog_err("Can't open BZ2 stream\n");
        return -1;
    }

    char buffer[4096];
    int bz_read = 0;
    while ((bz_read = BZ2_bzRead(&bz_error, bzFile, buffer, 4096)) > 0)
    {
        fwrite(buffer, 1, bz_read, out);
    }

    BZ2_bzReadClose(&bz_error, bzFile);
    fclose(in);
    fclose(out);
    return 0;
}

/**************************************
 * xz格式
 **************************************/

static int _xz_data_compress(char *iData, int iSize, char *oData, int *oSize)
{
    lzma_stream strm = LZMA_STREAM_INIT;
    if (lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC32) != LZMA_OK)
    {
        klog_err("Failed to initialize encoder\n");
        return -1;
    }
    strm.next_out = oData;
    strm.avail_out = oSize;
    strm.next_in = iData;
    strm.avail_in = iSize;

    lzma_ret ret = lzma_code(&strm, LZMA_FINISH);
    if (ret == LZMA_MEM_ERROR)
    {
        klog_err("Memory error\n");
        lzma_end(&strm);
        return -1;
    }
    lzma_end(&strm);

    return 0;
}

static int _xz_data_decompress(char *iData, int iSize, char *oData, int *oSize)
{
    lzma_stream strm = LZMA_STREAM_INIT;
    if (lzma_auto_decoder(&strm, UINT64_MAX, 0) != LZMA_OK)
    {
        klog_err("Failed to initialize encoder\n");
        return -1;
    }
    strm.next_out = oData;
    strm.avail_out = oSize;
    strm.next_in = iData;
    strm.avail_in = iSize;

    lzma_ret ret = lzma_code(&strm, LZMA_FINISH);
    if (ret == LZMA_MEM_ERROR)
    {
        klog_err("Memory error\n");
        lzma_end(&strm);
        return -1;
    }
    lzma_end(&strm);

    return 0;
}

static int _xz_file_compress(char *source_file, char *dest_file)
{
    FILE *in = fopen(source_file, "rb");
    if (!in)
    {
        klog_err("Failed to open input file\n");
        return -1;
    }

    FILE *out = fopen(dest_file, "wb");
    if (!out)
    {
        klog_err("Failed to open output file\n");
        fclose(in);
        return -1;
    }

    lzma_stream strm = LZMA_STREAM_INIT;
    if (lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC32) != LZMA_OK)
    {
        klog_err("Failed to initialize encoder\n");
        fclose(in);
        fclose(out);
        return -1;
    }
#define KLZMA_MARGIN(x) (uint8_t)(x * 1.1 + 12 + x)
    uint8_t in_buffer[8192];
    uint8_t out_buffer[KLZMA_MARGIN(8192)];
#undef KLZMA_MARGIN
    strm.next_out = out_buffer;
    strm.avail_out = sizeof(out_buffer);

    while (1)
    {
        strm.next_in = in_buffer;
        strm.avail_in = fread(in_buffer, 1, sizeof(in_buffer), in);
        if (ferror(in))
        {
            klog_err("Failed to read input file\n");
            lzma_end(&strm);
            fclose(in);
            fclose(out);
            return -1;
        }

        if (strm.avail_in == 0)
        {
            break;
        }

        while (strm.avail_in > 0)
        {
            lzma_ret ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
            if (ret == LZMA_MEM_ERROR)
            {
                klog_err("Memory error\n");
                lzma_end(&strm);
                fclose(in);
                fclose(out);
                return -1;
            }
            size_t write_size = sizeof(out_buffer) - strm.avail_out;
            fwrite(out_buffer, 1, write_size, out);
            strm.next_out = out_buffer;
            strm.avail_out = sizeof(out_buffer);
        }
    }

    lzma_code(&strm, LZMA_FINISH);
    size_t write_size = sizeof(out_buffer) - strm.avail_out;
    fwrite(out_buffer, 1, write_size, out);

    lzma_end(&strm);
    fclose(in);
    fclose(out);
    return 0;
}

static int _xz_file_decompress(char *source_file, char *dest_path)
{
    FILE *in = fopen(source_file, "rb");
    if (!in)
    {
        klog_err("Failed to open input file\n");
        return -1;
    }

    char *file[128], dest_file[1024];
    strncpy(file, basename(source_file), sizeof(file) - 1);
    snprintf(dest_file, sizeof(dest_file), "%s/%s", dest_path, _get_file_name(file));

    FILE *out = fopen(dest_file, "wb");
    if (!out)
    {
        klog_err("Failed to open output file\n");
        fclose(in);
        return -1;
    }

    lzma_stream strm = LZMA_STREAM_INIT;
    if (lzma_auto_decoder(&strm, UINT64_MAX, 0) != LZMA_OK)
    {
        klog_err("Failed to initialize decoder\n");
        fclose(in);
        fclose(out);
        return -1;
    }

    uint8_t in_buffer[8192];
    uint8_t out_buffer[8192];
    strm.next_out = out_buffer;
    strm.avail_out = sizeof(out_buffer);

    while (1)
    {
        strm.next_in = in_buffer;
        strm.avail_in = fread(in_buffer, 1, sizeof(in_buffer), in);
        if (ferror(in))
        {
            klog_err("Failed to read input file\n");
            lzma_end(&strm);
            fclose(in);
            fclose(out);
            return -1;
        }

        if (strm.avail_in == 0)
        {
            break;
        }

        while (strm.avail_in > 0)
        {
            lzma_ret ret = lzma_code(&strm, LZMA_RUN);
            if (ret == LZMA_MEM_ERROR)
            {
                klog_err("Memory error\n");
                lzma_end(&strm);
                fclose(in);
                fclose(out);
                return -1;
            }
            size_t write_size = sizeof(out_buffer) - strm.avail_out;
            fwrite(out_buffer, 1, write_size, out);
            strm.next_out = out_buffer;
            strm.avail_out = sizeof(out_buffer);
        }
    }

    lzma_code(&strm, LZMA_FINISH);
    size_t write_size = sizeof(out_buffer) - strm.avail_out;
    fwrite(out_buffer, 1, write_size, out);

    lzma_end(&strm);
    fclose(in);
    fclose(out);
    return 0;
}

/***************************************
 * lz4格式
 ***************************************/

#define IN_CHUNK_SIZE 16 * 1024

typedef struct
{
    int error;
    unsigned long long size_in;
    unsigned long long size_out;
} compressResult_t;

static void safe_fwrite(void *buf, size_t eltSize, size_t nbElt, FILE *f)
{
    size_t const writtenSize = fwrite(buf, eltSize, nbElt, f);
    size_t const expectedSize = eltSize * nbElt;
    if (expectedSize / nbElt != eltSize) /* check overflow */
    {
        return;
    }
    if (writtenSize < expectedSize)
    {
        if (ferror(f)) /* note : ferror() must follow fwrite */
            klog_err(stderr, "Write failed \n");
        else
            klog_err(stderr, "Short write \n");
    }
}

static size_t get_block_size(const LZ4F_frameInfo_t *info)
{
    switch (info->blockSizeID)
    {
    case LZ4F_default:
    case LZ4F_max64KB:
        return 1 << 16;
    case LZ4F_max256KB:
        return 1 << 18;
    case LZ4F_max1MB:
        return 1 << 20;
    case LZ4F_max4MB:
        return 1 << 22;
    default:
        klog_err("Impossible with expected frame specification (<=v1.6.1)\n");
        return 0;
    }
}

static compressResult_t compress_file_internal(FILE *f_in, FILE *f_out,
                                               LZ4F_compressionContext_t ctx,
                                               void *inBuff, size_t inChunkSize,
                                               void *outBuff, size_t outCapacity)
{
    compressResult_t result = {1, 0, 0}; /* result for an error */
    unsigned long long count_in = 0, count_out;

    if ((outCapacity < LZ4F_HEADER_SIZE_MAX) || (outCapacity < LZ4F_compressBound(inChunkSize, NULL)))
    {
        return result;
    }

    /* write frame header */
    {
        size_t const headerSize = LZ4F_compressBegin(ctx, outBuff, outCapacity, NULL);
        if (LZ4F_isError(headerSize))
        {
            klog_err("Failed to start compression: error %u \n", (unsigned)headerSize);
            return result;
        }
        count_out = headerSize;
        safe_fwrite(outBuff, 1, headerSize, f_out);
    }

    /* stream file */
    for (;;)
    {
        size_t const readSize = fread(inBuff, 1, IN_CHUNK_SIZE, f_in);
        if (readSize == 0)
            break; /* nothing left to read from input file */
        count_in += readSize;

        size_t const compressedSize = LZ4F_compressUpdate(ctx,
                                                          outBuff, outCapacity,
                                                          inBuff, readSize,
                                                          NULL);
        if (LZ4F_isError(compressedSize))
        {
            klog_err("Compression failed: error %u \n", (unsigned)compressedSize);
            return result;
        }

        if (compressedSize == 0)
            continue;
        safe_fwrite(outBuff, 1, compressedSize, f_out);
        count_out += compressedSize;
    }

    /* flush whatever remains within internal buffers */
    {
        size_t const compressedSize = LZ4F_compressEnd(ctx,
                                                       outBuff, outCapacity,
                                                       NULL);
        if (LZ4F_isError(compressedSize))
        {
            klog_err("Failed to end compression: error %u \n", (unsigned)compressedSize);
            return result;
        }

        safe_fwrite(outBuff, 1, compressedSize, f_out);
        count_out += compressedSize;
    }

    result.size_in = count_in;
    result.size_out = count_out;
    result.error = 0;
    return result;
}

/* @return : -1==error, 0==success */
static int decompress_file_internal(FILE *f_in, FILE *f_out,
                                    LZ4F_dctx *dctx,
                                    void *src, size_t srcCapacity, size_t filled, size_t alreadyConsumed,
                                    void *dst, size_t dstCapacity)
{
    int firstChunk = 1;
    size_t ret = 1;

    /* Decompression */
    while (ret != 0)
    {
        /* Load more input */
        size_t readSize = firstChunk ? filled : fread(src, 1, srcCapacity, f_in);
        firstChunk = 0;
        const void *srcPtr = (const char *)src + alreadyConsumed;
        alreadyConsumed = 0;
        const void *const srcEnd = (const char *)srcPtr + readSize;
        if (readSize == 0 || ferror(f_in))
        {
            klog_err("Decompress: not enough input or error reading file\n");
            return -1;
        }

        /* Decompress:
         * Continue while there is more input to read (srcPtr != srcEnd)
         * and the frame isn't over (ret != 0)
         */
        while (srcPtr < srcEnd && ret != 0)
        {
            /* Any data within dst has been flushed at this stage */
            size_t dstSize = dstCapacity;
            size_t srcSize = (const char *)srcEnd - (const char *)srcPtr;
            ret = LZ4F_decompress(dctx, dst, &dstSize, srcPtr, &srcSize, /* LZ4F_decompressOptions_t */ NULL);
            if (LZ4F_isError(ret))
            {
                klog_err("Decompression error: %s\n", LZ4F_getErrorName(ret));
                return -1;
            }
            /* Flush output */
            if (dstSize != 0)
                safe_fwrite(dst, 1, dstSize, f_out);
            /* Update input */
            srcPtr = (const char *)srcPtr + srcSize;
        }

        if (srcPtr > srcEnd)
        {
            return -1;
        }

        /* Ensure all input data has been consumed.
         * It is valid to have multiple frames in the same file,
         * but this example only supports one frame.
         */
        if (srcPtr < srcEnd)
        {
            klog_err("Decompress: Trailing data left in file after frame\n");
            return -1;
        }
    }

    /* Check that there isn't trailing data in the file after the frame.
     * It is valid to have multiple frames in the same file,
     * but this example only supports one frame.
     */
    {
        size_t const readSize = fread(src, 1, 1, f_in);
        if (readSize != 0 || !feof(f_in))
        {
            klog_err("Decompress: Trailing data left in file after frame\n");
            return 1;
        }
    }

    return 0;
}

static int decompress_file_allocDst(FILE *f_in, FILE *f_out,
                                    LZ4F_dctx *dctx,
                                    void *src, size_t srcCapacity)
{
    if (srcCapacity < LZ4F_HEADER_SIZE_MAX) /* ensure LZ4F_getFrameInfo() can read enough data */
    {
        klog_err("buffer too small\n");
        return -1;
    }
    /* Read Frame header */
    size_t const readSize = fread(src, 1, srcCapacity, f_in);
    if (readSize == 0 || ferror(f_in))
    {
        klog_err("Decompress: not enough input or error reading file\n");
        return 1;
    }

    LZ4F_frameInfo_t info;
    size_t consumedSize = readSize;
    {
        size_t const fires = LZ4F_getFrameInfo(dctx, &info, src, &consumedSize);
        if (LZ4F_isError(fires))
        {
            klog_err("LZ4F_getFrameInfo error: %s\n", LZ4F_getErrorName(fires));
            return 1;
        }
    }

    /* Allocating enough space for an entire block isn't necessary for
     * correctness, but it allows some memcpy's to be elided.
     */
    size_t const dstCapacity = get_block_size(&info);
    void *const dst = malloc(dstCapacity);
    if (!dst)
    {
        klog_err("decompress_file(dst)\n");
        return 1;
    }

    int const decompressionResult = decompress_file_internal(
        f_in, f_out,
        dctx,
        src, srcCapacity, readSize - consumedSize, consumedSize,
        dst, dstCapacity);

    free(dst);
    return decompressionResult;
}

static int _lz4_data_compress(char *iData, int iSize, char *oData, int *oSize)
{
    int need_size = LZ4_compressBound(iSize);
    if (*oSize < need_size)
    {
        klog_err("The output cache is too small, need %d bytes, output cache %d bytes\n", need_size, oSize);
        return -1;
    }

    *oSize = LZ4_compress_default(iData, oData, iSize, *oSize);
    if (*oSize <= 0)
    {
        klog_err("Compression failed\n");
        return -1;
    }

    return 0;
}

static int _lz4_data_decompress(char *iData, int iSize, char *oData, int *oSize)
{
    int need_size = iSize * 4;
    if (*oSize < need_size)
    {
        klog_err("The output cache is too small, need %d bytes, output cache %d bytes\n", need_size, *oSize);
        return -1;
    }

    *oSize = LZ4_decompress_safe(iData, oData, iSize, *oSize);
    if (*oSize <= 0)
    {
        klog_err("Compression failed\n");
        return -1;
    }

    return 0;
}

static int _lz4_file_compress(char *source_file, char *dest_file)
{
    // 打开输入文件
    FILE *in = fopen(source_file, "rb");
    if (!in)
    {
        klog_err("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    // 打开输出文件
    FILE *out = fopen(dest_file, "wb");
    if (!out)
    {
        klog_err("Failed to open output file\n");
        fclose(in);
        exit(EXIT_FAILURE);
    }

    /* ressource allocation */
    LZ4F_compressionContext_t ctx;
    size_t const ctxCreation = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
    void *const src = malloc(IN_CHUNK_SIZE);
    size_t const outbufCapacity = LZ4F_compressBound(IN_CHUNK_SIZE, NULL); /* large enough for any input <= IN_CHUNK_SIZE */
    void *const outbuff = malloc(outbufCapacity);
    compressResult_t result = {1, 0, 0}; /* == error (default) */

    if (!LZ4F_isError(ctxCreation) && src && outbuff)
    {
        result = compress_file_internal(in, out,
                                        ctx,
                                        src, IN_CHUNK_SIZE,
                                        outbuff, outbufCapacity);
    }
    else
    {
        klog_err("error : ressource allocation failed \n");
    }

    LZ4F_freeCompressionContext(ctx); /* supports free on NULL */
    free(src);
    free(outbuff);
    fclose(in);
    fclose(out);
    return 0;
}

static int _lz4_file_decompress(char *source_file, char *dest_path)
{
    FILE *in = fopen(source_file, "rb");
    if (!in)
    {
        klog_err("Failed to open input file\n");
        return -1;
    }

    char *file[128], dest_file[1024];
    strncpy(file, basename(source_file), sizeof(file) - 1);
    snprintf(dest_file, sizeof(dest_file), "%s/%s", dest_path, _get_file_name(file));

    FILE *out = fopen(dest_file, "wb");
    if (!out)
    {
        klog_err("Failed to open output file\n");
        fclose(in);
        return -1;
    }

    /* Ressource allocation */
    void *const src = malloc(IN_CHUNK_SIZE);
    if (!src)
    {
        klog_err("decompress_file(src)\n");
        return 1;
    }

    LZ4F_dctx *dctx;
    {
        size_t const dctxStatus = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
        if (LZ4F_isError(dctxStatus))
        {
            klog_err("LZ4F_dctx creation error: %s\n", LZ4F_getErrorName(dctxStatus));
        }
    }

    int const result = !dctx ? 1 /* error */ : decompress_file_allocDst(in, out, dctx, src, IN_CHUNK_SIZE);

    free(src);
    LZ4F_freeDecompressionContext(dctx); /* note : free works on NULL */

    fclose(in);
    fclose(out);
    return result;
}
#undef IN_CHUNK_SIZE

/***************************************
 * zstd格式
 ***************************************/

static int _zstd_data_compress(char *iData, int iSize, char *oData, int *oSize)
{
    size_t need_size = ZSTD_compressBound(iSize);
    if (*oSize < need_size)
    {
        klog_err("The output cache is too small, need %d bytes, output cache %d bytes\n", need_size, *oSize);
        return -1;
    }

    // 压缩数据
    *oSize = ZSTD_compress(oData, need_size, iData, iSize, 1);
    if (ZSTD_isError(*oSize))
    {
        klog_err("Compression failed\n");
        return -1;
    }

    return 0;
}

static int _zstd_data_decompress(char *iData, int iSize, char *oData, int *oSize)
{
    // 计算解压缩所需的缓冲区大小
    size_t need_size = ZSTD_getFrameContentSize(iData, iSize);
    if (need_size == ZSTD_CONTENTSIZE_UNKNOWN || need_size == ZSTD_CONTENTSIZE_ERROR)
    {
        klog_err("Unknown decompressed size\n");
        return -1;
    }

    if (*oSize < need_size)
    {
        klog_err("The output cache is too small, need %d bytes, output cache %d bytes\n", need_size, *oSize);
        return -1;
    }

    // 解压数据
    *oSize = ZSTD_decompress(oData, need_size, iData, iSize);
    if (ZSTD_isError(*oSize))
    {
        klog_err("Decompression failed\n");
        return -1;
    }

    return 0;
}

static int _zstd_file_compress(char *source_file, char *dest_file)
{
    FILE *in = fopen(source_file, "rb");
    if (!in)
    {
        klog_err("Failed to open input file\n");
        return -1;
    }

    FILE *out = fopen(dest_file, "wb");
    if (!out)
    {
        klog_err("Failed to open output file\n");
        fclose(in);
        return -1;
    }

    // 获取输入文件大小
    fseek(in, 0, SEEK_END);
    size_t file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    // 分配输入缓冲区
    void *in_buffer = malloc(file_size);
    if (!in_buffer)
    {
        klog_err("Memory allocation failed\n");
        fclose(in);
        fclose(out);
        return -1;
    }

    fread(in_buffer, 1, file_size, in);
    fclose(in);

    // 计算压缩后的最大大小
    size_t compressed_size = ZSTD_compressBound(file_size);
    void *out_buffer = malloc(compressed_size);
    if (!out_buffer)
    {
        klog_err("Memory allocation failed\n");
        free(in_buffer);
        fclose(out);
        return -1;
    }

    // 压缩数据
    compressed_size = ZSTD_compress(out_buffer, compressed_size, in_buffer, file_size, 1);
    if (ZSTD_isError(compressed_size))
    {
        klog_err("Compression failed\n");
        free(in_buffer);
        free(out_buffer);
        fclose(out);
        return -1;
    }

    // 将压缩数据写入输出文件
    fwrite(out_buffer, 1, compressed_size, out);
    fclose(out);

    free(in_buffer);
    free(out_buffer);
    return 0;
}

static int _zstd_file_decompress(char *source_file, char *dest_path)
{
    FILE *in = fopen(source_file, "rb");
    if (!in)
    {
        klog_err("Failed to open input file\n");
        return -1;
    }

    char *file[128], dest_file[1024];
    strncpy(file, basename(source_file), sizeof(file) - 1);
    snprintf(dest_file, sizeof(dest_file), "%s/%s", dest_path, _get_file_name(file));

    FILE *out = fopen(dest_file, "wb");
    if (!out)
    {
        klog_err("Failed to open output file\n");
        fclose(in);
        return -1;
    }

    // 读取压缩文件数据
    fseek(in, 0, SEEK_END);
    size_t compressed_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    // 分配输入缓冲区
    void *in_buffer = malloc(compressed_size);
    if (!in_buffer)
    {
        klog_err("Memory allocation failed\n");
        fclose(in);
        fclose(out);
        return -1;
    }

    fread(in_buffer, 1, compressed_size, in);
    fclose(in);

    // 计算解压缩所需的缓冲区大小
    size_t out_size = ZSTD_getFrameContentSize(in_buffer, compressed_size);
    if (out_size == ZSTD_CONTENTSIZE_UNKNOWN || out_size == ZSTD_CONTENTSIZE_ERROR)
    {
        klog_err("Unknown decompressed size\n");
        free(in_buffer);
        fclose(out);
        return -1;
    }

    // 为输出缓冲区分配内存
    void *out_buffer = malloc(out_size);
    if (!out_buffer)
    {
        klog_err("Memory allocation failed\n");
        free(in_buffer);
        fclose(out);
        return -1;
    }

    // 解压数据
    int decompressed_size = ZSTD_decompress(out_buffer, out_size, in_buffer, compressed_size);
    if (ZSTD_isError(decompressed_size))
    {
        klog_err("Decompression failed\n");
        free(in_buffer);
        free(out_buffer);
        fclose(out);
        return -1;
    }

    // 将解压数据写入文件
    fwrite(out_buffer, 1, decompressed_size, out);
    fclose(out);

    free(in_buffer);
    free(out_buffer);
    return 0;
}

/***************************************
 * 7z格式
 ***************************************/

static int _7z_data_compress(char *iData, int iSize, char *oData, int *oSize)
{
    return -2;
}

static int _7z_data_decompress(char *iData, int iSize, char *oData, int *oSize)
{
    return -2;
}

static int _7z_file_compress(char *source_file, char *dest_file)
{
    struct archive *a = archive_write_new();
    if (a == NULL)
    {
        klog_err("Failed to allocate archive object\n");
        return -1;
    }

    archive_write_set_format_7zip(a);

    // 打开目标文件以写入
    if (archive_write_open_filename(a, dest_file) != ARCHIVE_OK)
    {
        klog_err("Could not open output file %s: %s\n", dest_file, archive_error_string(a));
        archive_write_free(a);
        return -1;
    }

    char buff[8192];

    // 获取源文件的状态
    struct stat st;
    if (stat(source_file, &st) == -1)
    {
        klog_err("Could not stat file %s: %s\n", source_file, strerror(errno));
        archive_write_free(a);
        return -1;
    }

    // 创建新的档案条目
    struct archive_entry *entry = archive_entry_new();
    if (entry == NULL)
    {
        klog_err("Failed to allocate archive entry\n");
        archive_write_free(a);
        return -1;
    }

    // 设置条目的属性
    archive_entry_set_pathname(entry, basename(source_file)); // 使用basename获取文件名
    archive_entry_set_size(entry, st.st_size);
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);

    // 写入条目的头
    if (archive_write_header(a, entry) != ARCHIVE_OK)
    {
        klog_err("Could not write header: %s\n", archive_error_string(a));
        archive_entry_free(entry);
        archive_write_free(a);
        return -1;
    }

    // 打开源文件
    int fd = open(source_file, O_RDONLY);
    if (fd == -1)
    {
        klog_err("Could not open source file %s: %s\n", source_file, strerror(errno));
        archive_entry_free(entry);
        archive_write_free(a);
        return -1;
    }

    // 读取和写入数据
    ssize_t len;
    while ((len = read(fd, buff, sizeof(buff))) > 0)
    {
        if (archive_write_data(a, buff, len) < 0)
        {
            klog_err("Error writing data: %s\n", archive_error_string(a));
            close(fd);
            archive_entry_free(entry);
            archive_write_free(a);
            return -1;
        }
    }

    // 处理读取错误
    if (len < 0)
    {
        klog_err("Error reading from file: %s\n", strerror(errno));
    }

    // 关闭文件描述符
    close(fd);

    // 释放条目并结束写入
    archive_entry_free(entry);
    archive_write_close(a);
    archive_write_free(a);

    return 0;
}

static int _7z_file_decompress(char *source_file, char *dest_path)
{
    struct archive *a = archive_read_new();
    archive_read_support_format_7zip(a);

    if (ARCHIVE_OK != archive_read_open_filename(a, source_file, 10240))
    {
        klog_err("Error opening file: %s\n", archive_error_string(a));
        archive_read_free(a);
        return -1;
    }

    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
        char buffer[4096];
        ssize_t out_len;

        // 创建目标文件路径
        char dest_file[1024];
        snprintf(dest_file, sizeof(dest_file), "%s/%s", dest_path, archive_entry_pathname(entry));

        // 打开目标文件
        FILE *fd = fopen(dest_file, "wb");
        if (fd == NULL)
        {
            klog_err("Error opening output file: %s. %s\n", dest_file, strerror(errno));
            archive_read_free(a);
            return -1;
        }

        // 写入数据
        while ((out_len = archive_read_data(a, buffer, sizeof(buffer))) > 0)
        {
            if (fwrite(buffer, 1, out_len, fd) != out_len)
            {
                klog_err("Error writing to file: %s. %s\n", dest_file, strerror(errno));
                fclose(fd);
                archive_read_free(a);
                return -1;
            }
        }

        // 处理读取错误
        if (out_len < 0)
        {
            klog_err("Error reading file: %s\n", archive_error_string(a));
            fclose(fd);
            archive_read_free(a);
            return -1;
        }

        fclose(fd);

        archive_entry_clear(entry);
    }

    archive_read_free(a);

    return 0;
}

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/", strnlen("/", STRNLEN_MAX));
}

static char *_kdk_storage_get_Cache(BrowserEnum type)
{
    char *home = getenv("HOME");
    char result[1024];
    memset(result, 0, 1024);
    switch (type)
    {
    case Qax:
    {
        snprintf(result, sizeof(result), "%s/.config/qaxbrowser/Default/Cache", home);
    }
    break;
    case FireFox:
    {
        char profiles[1024];
        char canonical_filename[4096] = "\0";
        snprintf(profiles, sizeof(profiles), "%s/.mozilla/firefox/profiles.ini", home);
        if(!realpath(profiles, canonical_filename) || !verify_file(canonical_filename))
        {
            return NULL;
        }
        FILE *fp = fopen(canonical_filename, "r");
        if (NULL == fp)
        {
            klog_err("%s -> Read firefox profile failed: \n", __func__, strerror(errno));
            break;
        }
        char line[1024];
        int start = 0, def = 0;
        char configs[10][64];
        memset(configs, 0, 10 * 64);
        char path[64];
        memset(path, 0, 64);
        while (fgets(line, sizeof(line), fp))
        {
            line[strcspn(line, "\n")] = 0;

            // Parse key and value
            char *key = strtok(line, "=");
            char *value = strtok(NULL, "=");
            if (key != NULL && value != NULL)
            {
                if (0 == strcmp(key, "StartWithLastProfile"))
                    start = atoi(value);
                if (0 == strcmp(key, "Path"))
                {
                    strncpy(path, value, sizeof(path));
                }
                if (0 == strcmp(key, "Default"))
                {
                    def = atoi(value);
                }
            }
            else
            {
                memcpy(configs[def], path, 64);
                def = 0;
                memset(path, 0, 64);
            }
        }
        fclose(fp);
        if (start > 0)
            snprintf(result, sizeof(result), "%s/.mozilla/firefox/%s/cache2", home, configs[start]);
        else
        {
            klog_err("%s -> Not have default profile\n", __func__);
            return NULL;
        }
    }
    break;
    case Chrome:
    {
        snprintf(result, sizeof(result), "%s/.config/google-chrome/Default/Cache", home);
    }
    break;
    case Chromium:
    {
        snprintf(result, sizeof(result), "%s/.config/chromium/Default/Cache", home);
    }
    break;
    default:
    {
        klog_err("%s -> Unsupported browser\n", __func__);
    }
    break;
    }
    return strnlen(result, STRNLEN_MAX) > 0 ? strdup(result) : NULL;
}

static char *_kdk_storage_get_cookie(BrowserEnum type)
{
    char canonical_filename[4096] = "\0";
    char *home = getenv("HOME");
    char result[1024];
    memset(result, 0, 1024);
    switch (type)
    {
    case Qax:
    {
        snprintf(result, sizeof(result), "%s/.config/qaxbrowser/Default/Cookies", home);
    }
    break;
    case FireFox:
    {
        char profiles[1024];
        snprintf(profiles, sizeof(profiles), "%s/.mozilla/firefox/profiles.ini", home);
        if(!realpath(profiles, canonical_filename) || !verify_file(canonical_filename))
        {
            return NULL;
        }
        FILE *fp = fopen(canonical_filename, "r");
        if (NULL == fp)
        {
            klog_err("%s -> Read firefox profile failed: \n", __func__, strerror(errno));
            break;
        }
        char line[1024];
        int start = 0, def = 0;
        char configs[10][64];
        memset(configs, 0, 10 * 64);
        char path[64];
        memset(path, 0, 64);
        while (fgets(line, sizeof(line), fp))
        {
            line[strcspn(line, "\n")] = 0;

            // Parse key and value
            char *key = strtok(line, "=");
            char *value = strtok(NULL, "=");
            if (key != NULL && value != NULL)
            {
                if (0 == strcmp(key, "StartWithLastProfile"))
                    start = atoi(value);
                if (0 == strcmp(key, "Path"))
                {
                    strncpy(path, value, sizeof(path));
                }
                if (0 == strcmp(key, "Default"))
                {
                    def = atoi(value);
                }
            }
            else
            {
                memcpy(configs[def], path, 64);
                def = 0;
                memset(path, 0, 64);
            }
        }
        fclose(fp);
        if (start > 0)
            snprintf(result, sizeof(result), "%s/.mozilla/firefox/%s/cookies.sqlite", home, configs[start]);
        else
        {
            klog_err("%s -> Not have default profile\n", __func__);
            return NULL;
        }
    }
    break;
    case Chrome:
    {
        snprintf(result, sizeof(result), "%s/.config/google-chrome/Default/Cookies", home);
    }
    break;
    case Chromium:
    {
        snprintf(result, sizeof(result), "%s/.config/chromium/Default/Cookies", home);
    }
    break;
    default:
    {
        klog_err("%s -> Unsupported browser\n", __func__);
    }
    break;
    }
    return strnlen(result, STRNLEN_MAX) > 0 ? strdup(result) : NULL;
}

char *kdk_storage_get_default_cookie_path()
{
    char *default_browser = kdk_package_get_default_browser();
    if (NULL == default_browser)
        return NULL;

    klog_info("%s -> Default Browser %s\n", __func__, default_browser);
    char *result = NULL;
    if (strstr(default_browser, "Qax"))
        result = _kdk_storage_get_cookie(Qax);
    else if (strstr(default_browser, "Chrome"))
        result = _kdk_storage_get_cookie(Chrome);
    else if (strstr(default_browser, "Firefox"))
        result = _kdk_storage_get_cookie(FireFox);
    else if (strstr(default_browser, "Chromium"))
        result = _kdk_storage_get_cookie(Chromium);
    else
        result = _kdk_storage_get_cookie(None);

    free(default_browser);
    return result;
}

char *kdk_storage_get_default_desktop_path()
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), "桌面");
    if (0 == access(path, F_OK))
        return strdup(path);
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), "desktop");
    if (0 == access(path, F_OK))
        return strdup(path);
    return NULL;
}

char *kdk_storage_get_default_documents_path()
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), "文档");
    if (0 == access(path, F_OK))
        return strdup(path);
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), "documents");
    if (0 == access(path, F_OK))
        return strdup(path);
    return NULL;
}

char *kdk_storage_get_default_internetCache_path(char *name)
{
    if (NULL == name)
    {
        name = kdk_package_get_default_browser();
        if (NULL == name)
            return NULL;
    }
    char *result = NULL;
    if (strstr("qax", name))
        result = _kdk_storage_get_Cache(Qax);
    else if (strstr("chrome", name))
        result = _kdk_storage_get_Cache(Chrome);
    else if (strstr("firefox", name))
        result = _kdk_storage_get_Cache(FireFox);
    else if (strstr("chromeium", name))
        result = _kdk_storage_get_Cache(Chromium);
    else
        result = _kdk_storage_get_Cache(None);
    return result;
}

char *kdk_storage_get_default_download_path()
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), "下载");
    if (0 == access(path, F_OK))
        return strdup(path);
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), "download");
    if (0 == access(path, F_OK))
        return strdup(path);
    return NULL;
}

char *kdk_storage_get_file_ContentType(char *file_name)
{
    magic_t magic_cookie;
    const char *mime = NULL;

    magic_cookie = magic_open(MAGIC_MIME_TYPE);
    if (magic_cookie == NULL)
    {
        klog_err("%s -> Unable to initialize magic library\n", __func__);
        return NULL;
    }

    if (magic_load(magic_cookie, NULL) != 0)
    {
        klog_err("%s -> Cannot load magic database - %s\n", __func__, magic_error(magic_cookie));
        goto out;
    }

    mime = magic_file(magic_cookie, file_name);
    if (mime == NULL)
    {
        klog_err("%s -> Cannot determine file type\n", __func__);
        goto out;
    }
    mime = strdup(mime);

out:
    magic_close(magic_cookie);
    return mime;
}

char *kdk_storage_get_file_dateChange(char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed\n", __func__, strerror(errno));
        return NULL;
    }

    char time[32];
    strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_ctime));

    return strdup(time);
}

char *kdk_storage_get_file_dateModify(char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed\n", __func__, strerror(errno));
        return NULL;
    }

    char time[32];
    strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));

    return strdup(time);
}

char *kdk_storage_get_file_dateAccess(char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed\n", __func__, strerror(errno));
        return NULL;
    }

    char time[32];
    strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_atime));

    return strdup(time);
}

long kdk_storage_get_file_size(char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return 0;
    }

    return file_stat.st_size;
}

int kdk_storage_read_file(const char *file_name, char *buffer, unsigned long size)
{
    // if (-1 == access(file_name, F_OK))
    // {
    //     klog_err("%s -> File not exists\n", __func__);
    //     return -1;
    // }

    if (kdk_storage_get_file_size(file_name) > (size - 1))
    {
        klog_err("%s -> The buffer is insufficient\n", __func__);
        return -1;
    }

    int fd = open(file_name, O_RDONLY);
    if (-1 == fd)
    {
        klog_err("%s -> Open file failed: %s\n", __func__, strerror(errno));
        return -1;
    }

    int i = 0, ret = 1, count = 0;
    while (ret > 0)
    {
        ret = read(fd, buffer + (i++ * 2048), 2048);
        if (-1 == ret)
        {
            klog_err("%s -> Read file failed: %s\n", __func__, strerror(errno));
            memset(buffer, 0, count);
            count = -1;
            break;
        }
        count += ret;
    }
    close(fd);
    return count;
}

int kdk_storage_write_file(const char *file_name, const char *data, unsigned long length)
{
    // if (-1 == access(file_name, F_OK))
    // {
    //     klog_err("%s -> File not exists\n", __func__);
    //     return -1;
    // }

    int fd = open(file_name, O_WRONLY | O_APPEND);
    if (-1 == fd)
    {
        klog_err("%s -> Open file failed: %s\n", __func__, strerror(errno));
        return -1;
    }

    int ret = write(fd, data, length);
    if (-1 == ret)
    {
        klog_err("%s -> Write file failed: %s\n", __func__, strerror(errno));
    }

    close(fd);
    return ret;
}

int kdk_storage_move_file(const char *source, const char *destination)
{
    int ret = rename(source, destination);
    if (-1 == ret)
    {
        klog_err("%s -> Move file failed: %s\n", __func__, strerror(errno));
    }
    return ret;
}

char *kdk_storage_get_file_permissions(const char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return NULL;
    }
    unsigned int permission = 0;
    permission |= ((file_stat.st_mode & S_IRWXU) << 2);
    permission |= ((file_stat.st_mode & S_IRWXG) << 1);
    permission |= ((file_stat.st_mode & S_IRWXO));

    char result[4];
    snprintf(result, sizeof(result), "%x", permission);
    return strdup(result);
}

char *kdk_storage_get_file_owner(const char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return NULL;
    }

    struct passwd *pw = getpwuid(file_stat.st_uid);
    if (NULL == pw)
    {
        klog_err("%s -> Getting passwd failed: %s\n", __func__, strerror(errno));
        return NULL;
    }
    char *result = strdup(pw->pw_name);
    endpwent();
    return result;
}

char *kdk_storage_get_file_group(const char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return NULL;
    }

    struct group *gr = getgrgid(file_stat.st_gid);
    if (NULL == gr)
    {
        klog_err("%s -> Getting group failed: %s\n", __func__, strerror(errno));
        return NULL;
    }
    char *result = strdup(gr->gr_name);
    endgrent();
    return result;
}

FileInfo *kdk_storage_list_files(const char *path)
{
    FileInfo *info = NULL;
    DIR *dir;
    struct dirent *entry;

    // 打开当前目录
    dir = opendir(path);
    if (dir == NULL)
    {
        klog_err("%s -> Open directory fialed: %s\n", __func__, strerror(errno));
        return NULL;
    }

    // 读取目录中的文件和子目录
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") ^ strcmp(entry->d_name, ".."))
            continue;

        FileInfo *tmp = calloc(1, sizeof(FileInfo));
        if (NULL == tmp)
        {
            klog_err("%s -> Memory error\n", __func__);
            kdk_storage_free_file_info(info);
            info = NULL;
            break;
        }

        strncpy(tmp->name, entry->d_name, sizeof(tmp->name) - 1);
        tmp->is_dir = (DT_DIR == entry->d_type) ? 1 : 0;

        tmp->next = info;
        info = tmp;
    }

    // 关闭目录
    closedir(dir);
    return info;
}

void kdk_storage_free_file_info(FileInfo *info)
{
    FileInfo *tmp = info;
    while (info)
    {
        tmp = info;
        info = info->next;
        free(tmp);
    }
}

char *kdk_storage_resolve_symbolic_link(const char *file_name)
{
    char *result = NULL;

    struct stat file_stat;
    if (lstat(file_name, &file_stat) < 0)
    {
        klog_err("%s -> Getting stat failed: %s\n", __func__, strerror(errno));
        return NULL;
    }

    if (S_ISLNK(file_stat.st_mode))
    {
        char link_target[1024];
        memset(link_target, 0, 1024);
        ssize_t len = readlink(file_name, link_target, sizeof(link_target) - 1);
        if (-1 == len)
        {
            klog_err("%s -> Read link failed: %s\n", __func__, strerror(errno));
            return NULL;
        }
        result = strdup(link_target);
    }
    else if (file_stat.st_nlink > 1)
    {
        // 硬链接
    }
    else
    {
        klog_err("%s -> %s is neither a symbolic link nor a hard link\n", __func__, file_name);
    }

    return result;
}

int kdk_storage_data_compress(kdk_compress_algorithm algorithm, char *iData, int iSize, char *oData, int *oSize)
{
    if ((NULL == iData) || (NULL == oData))
        return -1;

    switch (algorithm)
    {
    case COMPRESS_ALGORITHM_ZIP:
        return _zip_data_compress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_GZIP:
        return _gzip_data_compress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_BZIP2:
        return _bzip_data_compress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_X:
        return _xz_data_compress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_LZ4:
        return _lz4_data_compress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_ZSTD:
        return _zstd_data_compress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_7Z:
        return _7z_data_compress(iData, iSize, oData, oSize);
    default:
        return -1;
    }
}

int kdk_storage_data_decompress(kdk_compress_algorithm algorithm, char *iData, int iSize, char *oData, int *oSize)
{
    if ((NULL == iData) || (NULL == oData))
        return -1;

    switch (algorithm)
    {
    case COMPRESS_ALGORITHM_ZIP:
        return _zip_data_decompress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_GZIP:
        return _gzip_data_decompress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_BZIP2:
        return _bzip_data_decompress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_X:
        return _xz_data_decompress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_LZ4:
        return _lz4_data_decompress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_ZSTD:
        return _zstd_data_decompress(iData, iSize, oData, oSize);
    case COMPRESS_ALGORITHM_7Z:
        return _7z_data_decompress(iData, iSize, oData, oSize);
    default:
        return -1;
    }
}

int kdk_storage_file_compress(kdk_compress_algorithm algorithm, char *source_file, char *dest_file)
{
    if ((NULL == source_file) || (NULL == dest_file))
        return -1;

    if (0 != access(source_file, F_OK))
    {
        return -1;
    }

    char tmp[512];
    strcpy(tmp, dest_file);
    if (0 != access(dirname(tmp), F_OK))
    {
        return -1;
        // if (0 != mkdir(dirname(tmp), 0755))
        // {
        //     klog_err("%s -> Create directory failed: %s\n", __func__, strerror(errno));
        //     return -1;
        // }
    }

    switch (algorithm)
    {
    case COMPRESS_ALGORITHM_ZIP:
        return _zip_file_compress(source_file, dest_file);
    case COMPRESS_ALGORITHM_GZIP:
        return _gzip_file_compress(source_file, dest_file);
    case COMPRESS_ALGORITHM_BZIP2:
        return _bzip_file_compress(source_file, dest_file);
    case COMPRESS_ALGORITHM_X:
        return _xz_file_compress(source_file, dest_file);
    case COMPRESS_ALGORITHM_LZ4:
        return _lz4_file_compress(source_file, dest_file);
    case COMPRESS_ALGORITHM_ZSTD:
        return _zstd_file_compress(source_file, dest_file);
    case COMPRESS_ALGORITHM_7Z:
        return _7z_file_compress(source_file, dest_file);
    default:
        return -1;
    }
}

int kdk_storage_file_decompress(kdk_compress_algorithm algorithm, char *source_file, char *dest_path)
{
    if ((NULL == source_file) || (NULL == dest_path))
        return -1;

    if (0 != access(source_file, F_OK))
    {
        return -1;
    }

    if (0 != access(dest_path, F_OK))
    {
        if (0 != mkdir(dest_path, 0755))
        {
            klog_err("%s -> Create directory failed: %s\n", __func__, strerror(errno));
            return -1;
        }
    }

    switch (algorithm)
    {
    case COMPRESS_ALGORITHM_ZIP:
        return _zip_file_decompress(source_file, dest_path);
    case COMPRESS_ALGORITHM_GZIP:
        return _gzip_file_decompress(source_file, dest_path);
    case COMPRESS_ALGORITHM_BZIP2:
        return _bzip_file_decompress(source_file, dest_path);
    case COMPRESS_ALGORITHM_X:
        return _xz_file_decompress(source_file, dest_path);
    case COMPRESS_ALGORITHM_LZ4:
        return _lz4_file_decompress(source_file, dest_path);
    case COMPRESS_ALGORITHM_ZSTD:
        return _zstd_file_decompress(source_file, dest_path);
    case COMPRESS_ALGORITHM_7Z:
        return _7z_file_decompress(source_file, dest_path);
    default:
        return -1;
    }
}

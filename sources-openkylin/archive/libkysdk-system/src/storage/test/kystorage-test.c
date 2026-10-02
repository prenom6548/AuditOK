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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "../libkystorage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

int main()
{
    char *cookie_path = kdk_storage_get_default_cookie_path();
    if (cookie_path)
    {
        printf("Cookie path: %s\n", cookie_path);
        free(cookie_path);
    }

    char *desktop_path = kdk_storage_get_default_desktop_path();
    if (desktop_path)
    {
        printf("Desktop path: %s\n", desktop_path);
        free(desktop_path);
    }

    char *documents_path = kdk_storage_get_default_documents_path();
    if (documents_path)
    {
        printf("Documents path: %s\n", documents_path);
        free(documents_path);
    }

    char *cache_path = kdk_storage_get_default_internetCache_path("firefox");
    if (cache_path)
    {
        printf("Cache path: %s\n", cache_path);
        free(cache_path);
    }

    char *download_path = kdk_storage_get_default_download_path();
    if (download_path)
    {
        printf("Download path: %s\n", download_path);
        free(download_path);
    }

    char *content_type = kdk_storage_get_file_ContentType("/home/kylin/test.yaml");
    if (content_type)
    {
        printf("Content type: %s\n", content_type);
        free(content_type);
    }

    char *changed_date = kdk_storage_get_file_dateChange("/home/kylin/test.yaml");
    if (changed_date)
    {
        printf("File change date: %s\n", changed_date);
        free(changed_date);
    }

    char *modify_date = kdk_storage_get_file_dateModify("/home/kylin/test.yaml");
    if (modify_date)
    {
        printf("File modify date: %s\n", modify_date);
        free(modify_date);
    }

    char *access_date = kdk_storage_get_file_dateAccess("/home/kylin/test.yaml");
    if (access_date)
    {
        printf("File access date: %s\n", access_date);
        free(access_date);
    }

    long size = kdk_storage_get_file_size("/home/kylin/test.yaml");
    printf("File size: %ld\n", size);

    char buffer[size + 1];
    memset(buffer, 0, size + 1);
    int ret = kdk_storage_read_file("/home/kylin/test.yaml", buffer, size + 1);
    if (-1 != ret)
    {
        printf("Read file success\n");
    }

    ret = kdk_storage_write_file("/home/kylin/dest.yaml", buffer, size);
    if (-1 != ret)
    {
        printf("Write file success\n");
    }

    // ret = kdk_storage_move_file("/home/kylin/dest.yaml", "/home/kylin/yaml/test.yaml");
    // if (-1 != ret)
    // {
    //     printf("Move file success\n");
    // }

    char *permission = kdk_storage_get_file_permissions("/home/kylin/test.yaml");
    if (permission)
    {
        printf("Permission: %s\n", permission);
        free(permission);
    }

    char *owner = kdk_storage_get_file_owner("/home/kylin/test.yaml");
    if (owner)
    {
        printf("Owner: %s\n", owner);
        free(owner);
    }

    char *group = kdk_storage_get_file_group("/home/kylin/test.yaml");
    if (group)
    {
        printf("Group: %s\n", group);
        free(group);
    }

    FileInfo *info = kdk_storage_list_files("/home/kylin");
    if (info)
    {
        FileInfo *tmp = info;
        while (tmp)
        {
            printf("\t%s: %s\n", tmp->is_dir ? "SubFolder" : "SubFile", tmp->name);
            tmp = tmp->next;
        }
        kdk_storage_free_file_info(info);
    }

    char *link = kdk_storage_resolve_symbolic_link("/usr/lib/libkysset.so");
    if (link)
    {
        printf("Link file: %s\n", link);
        free(link);
    }

    ret = 0;

    const char *result[3] = {"not support", "fail", "success"}; 

    char iData[1024 * 10] = "dsajklferklnwhahigjkrtonjbujhnkdnrbjehunkldrmkdfnj,sdfjkbeshujkejklsefmkdfnjdbjkabjkm,v.m,jkrwnjkljknjsdfnkldv vnmbnjlkbgnmf dnb vjk.abgf,z.vgbvjcvhkxlfu";
    char oData[1024 * 10];
    int iSize = strnlen(iData, STRNLEN_MAX), oSize = sizeof(oData);
    memset(oData, 0, oSize);

    // ZIP
    iSize = strnlen(iData, STRNLEN_MAX); oSize = sizeof(oData);
    ret = kdk_storage_data_compress(COMPRESS_ALGORITHM_ZIP, iData, iSize, oData, &oSize);
    printf("Zip data compress %s\n", result[ret + 2]);

    iSize = sizeof(iData);
    ret = kdk_storage_data_decompress(COMPRESS_ALGORITHM_ZIP, oData, oSize, iData, &iSize);
    printf("Zip data decompress %s\n", result[ret + 2]);

    ret = kdk_storage_file_compress(COMPRESS_ALGORITHM_ZIP, "/home/kylin/storage/testz.txt", "/home/kylin/storage/testz.zip");
    printf("Zip file compress %s\n", result[ret + 2]);

    ret = kdk_storage_file_decompress(COMPRESS_ALGORITHM_ZIP, "/home/kylin/storage/testz.zip", "/home/kylin/storage/out");
    printf("Zip file decompress %s\n", result[ret + 2]);

    // GZIP
    iSize = strnlen(iData, STRNLEN_MAX); oSize = sizeof(oData);
    ret = kdk_storage_data_compress(COMPRESS_ALGORITHM_GZIP, iData, iSize, oData, &oSize);
    printf("Gzip data compress %s\n", result[ret + 2]);

    iSize = sizeof(iData);
    ret = kdk_storage_data_decompress(COMPRESS_ALGORITHM_GZIP, oData, oSize, iData, &iSize);
    printf("Gzip data decompress %s\n", result[ret + 2]);

    ret = kdk_storage_file_compress(COMPRESS_ALGORITHM_GZIP, "/home/kylin/storage/testgz.txt", "/home/kylin/storage/testgz.gz");
    printf("Gzip file decompress %s\n", result[ret + 2]);

    ret = kdk_storage_file_decompress(COMPRESS_ALGORITHM_GZIP, "/home/kylin/storage/testgz.gz", "/home/kylin/storage/out");
    printf("Gzip file decompress %s\n", result[ret + 2]);

    // BZIP
    iSize = strnlen(iData, STRNLEN_MAX); oSize = sizeof(oData);
    ret = kdk_storage_data_compress(COMPRESS_ALGORITHM_BZIP2, iData, iSize, oData, &oSize);
    printf("Bzip2 data compress %s\n", result[ret + 2]);

    iSize = sizeof(iData);
    ret = kdk_storage_data_decompress(COMPRESS_ALGORITHM_BZIP2, oData, oSize, iData, &iSize);
    printf("Bzip2 data decompress %s\n", result[ret + 2]);

    ret = kdk_storage_file_compress(COMPRESS_ALGORITHM_BZIP2, "/home/kylin/storage/testbz.txt", "/home/kylin/storage/testbz.bz2");
    printf("Bzip2 file compress %s\n", result[ret + 2]);

    ret = kdk_storage_file_decompress(COMPRESS_ALGORITHM_BZIP2, "/home/kylin/storage/testbz.bz2", "/home/kylin/storage/out");
    printf("Bzip2 file decompress %s\n", result[ret + 2]);

    // XZIP
    iSize = strnlen(iData, STRNLEN_MAX); oSize = sizeof(oData);
    ret = kdk_storage_data_compress(COMPRESS_ALGORITHM_X, iData, iSize, oData, &oSize);
    printf("X data compress %s\n", result[ret + 2]);

    iSize = sizeof(iData);
    ret = kdk_storage_data_decompress(COMPRESS_ALGORITHM_X, oData, oSize, iData, &iSize);
    printf("X data decompress %s\n", result[ret + 2]);

    ret = kdk_storage_file_compress(COMPRESS_ALGORITHM_X, "/home/kylin/storage/testx.txt", "/home/kylin/storage/testx.xz");
    printf("X file compress %s\n", result[ret + 2]);

    ret = kdk_storage_file_decompress(COMPRESS_ALGORITHM_X, "/home/kylin/storage/testx.xz", "/home/kylin/storage/out");
    printf("X file decompress %s\n", result[ret + 2]);

    // LZ4
    iSize = strnlen(iData, STRNLEN_MAX); oSize = sizeof(oData);
    ret = kdk_storage_data_compress(COMPRESS_ALGORITHM_LZ4, iData, iSize, oData, &oSize);
    printf("Lz4 data compress %s\n", result[ret + 2]);

    iSize = sizeof(iData);
    ret = kdk_storage_data_decompress(COMPRESS_ALGORITHM_LZ4, oData, oSize, iData, &iSize);
    printf("Lz4 data decompress %s\n", result[ret + 2]);

    ret = kdk_storage_file_compress(COMPRESS_ALGORITHM_LZ4, "/home/kylin/storage/testlz.txt", "/home/kylin/storage/testlz.lz4");
    printf("Lz4 file compress %s\n", result[ret + 2]);

    ret = kdk_storage_file_decompress(COMPRESS_ALGORITHM_LZ4, "/home/kylin/storage/testlz.lz4", "/home/kylin/storage/out");
    printf("Lz4 file decompress %s\n", result[ret + 2]);

    // ZSTD
    iSize = strnlen(iData, STRNLEN_MAX); oSize = sizeof(oData);
    ret = kdk_storage_data_compress(COMPRESS_ALGORITHM_ZSTD, iData, iSize, oData, &oSize);
    printf("Zstd data compress %s\n", result[ret + 2]);

    iSize = sizeof(iData);
    ret = kdk_storage_data_decompress(COMPRESS_ALGORITHM_ZSTD, oData, oSize, iData, &iSize);
    printf("Zstd data decompress %s\n", result[ret + 2]);

    ret = kdk_storage_file_compress(COMPRESS_ALGORITHM_ZSTD, "/home/kylin/storage/testzs.txt", "/home/kylin/storage/testzs.zstd");
    printf("Zstd file compress %s\n", result[ret + 2]);

    ret = kdk_storage_file_decompress(COMPRESS_ALGORITHM_ZSTD, "/home/kylin/storage/testzs.zstd", "/home/kylin/storage/out");
    printf("Zstd file decompress %s\n", result[ret + 2]);

    // 7Z
    iSize = strnlen(iData, STRNLEN_MAX); oSize = sizeof(oData);
    ret = kdk_storage_data_compress(COMPRESS_ALGORITHM_7Z, iData, iSize, oData, &oSize);
    printf("7z data compress %s\n", result[ret + 2]);

    iSize = sizeof(iData);
    ret = kdk_storage_data_decompress(COMPRESS_ALGORITHM_7Z, oData, oSize, iData, &iSize);
    printf("7z data decompress %s\n", result[ret + 2]);

    ret = kdk_storage_file_compress(COMPRESS_ALGORITHM_7Z, "/home/kylin/storage/test7.txt", "/home/kylin/storage/test7.7z");
    printf("7z file compress %s\n", result[ret + 2]);

    ret = kdk_storage_file_decompress(COMPRESS_ALGORITHM_7Z, "/home/kylin/storage/test7.7z", "/home/kylin/storage/out");
    printf("7z file decompress %s\n", result[ret + 2]);
}

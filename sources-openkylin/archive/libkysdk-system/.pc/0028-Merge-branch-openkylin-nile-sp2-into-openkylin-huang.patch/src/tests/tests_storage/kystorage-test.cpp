/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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

extern "C"{
#include "../../storage/libkystorage.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtest/gtest.h>
#include "../cpp-stub/src/stub.h"
#include <kysdk/kysdk-system/libkypackages.h>

extern char* kdk_package_get_default_browser_stub(void)
{
    char *res = (char *)malloc(32);
    strncpy(res,"Chrome", sizeof(31));
    return res;
}

extern char* kdk_package_get_default_browser_stub1(void)
{
    char *res = (char *)malloc(32);
    strncpy(res,"Firefox", sizeof(31));
    return res;
}

extern char* kdk_package_get_default_browser_stub2(void)
{
    char *res = (char *)malloc(32);
    strncpy(res,"Chromium", sizeof(31));
    return res;
}

TEST(KdkStorageTest, StorageInfoStub)
{
    char *cache_path = kdk_storage_get_default_internetCache_path(NULL);
    if (cache_path)
    {
        printf("Cache path: %s\n", cache_path);
        free(cache_path);
    }
    Stub st;
    st.set(kdk_package_get_default_browser, kdk_package_get_default_browser_stub);
    char *cookie_path = kdk_storage_get_default_cookie_path();
    if (cookie_path)
    {
        printf("Cookie path: %s\n", cookie_path);
        free(cookie_path);
    }

    cache_path = kdk_storage_get_default_internetCache_path("chrome");
    if (cache_path)
    {
        printf("Cache path: %s\n", cache_path);
        free(cache_path);
    }

    cache_path = kdk_storage_get_default_internetCache_path("chromeium");
    if (cache_path)
    {
        printf("Cache path: %s\n", cache_path);
        free(cache_path);
    }
}

TEST(KdkStorageTest, StorageInfoStub1)
{
    Stub st;
    st.set(kdk_package_get_default_browser, kdk_package_get_default_browser_stub1);
    char *cookie_path = kdk_storage_get_default_cookie_path();
    if (cookie_path)
    {
        printf("Cookie path: %s\n", cookie_path);
        free(cookie_path);
    }
}

TEST(KdkStorageTest, StorageInfoStub2)
{
    Stub st;
    st.set(kdk_package_get_default_browser, kdk_package_get_default_browser_stub2);
    char *cookie_path = kdk_storage_get_default_cookie_path();
    if (cookie_path)
    {
        printf("Cookie path: %s\n", cookie_path);
        free(cookie_path);
    }
}

TEST(KdkStorageTest, StorageInfo)
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

    cache_path = kdk_storage_get_default_internetCache_path("");
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

    ret = kdk_storage_move_file("/home/kylin/test.yaml", "/home/kylin/yaml/");
    if (-1 != ret)
    {
        printf("Move file success\n");
    }

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
}
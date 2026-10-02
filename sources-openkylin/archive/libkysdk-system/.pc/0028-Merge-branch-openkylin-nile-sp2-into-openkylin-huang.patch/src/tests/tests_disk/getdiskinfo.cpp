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

#include <gtest/gtest.h>
#include "../../disk/libkydiskinfo.h"
#include <dbus-1.0/dbus/dbus.h>
#include <sys/ioctl.h>
#include <hd.h>

#include "../cpp-stub/src/stub.h"

using namespace testing;

dbus_bool_t dbus_connection_send_with_reply_stub(DBusConnection *connection, DBusMessage *message, DBusPendingCall **pending_return, int timeout_milliseconds)
{
    return false;
}

dbus_bool_t dbus_message_append_args_stub(DBusMessage *message, int first_arg_type, ...)
{
    return false;
}

DBusMessage *dbus_message_new_method_call_stub(const char *destination, const char *path, const char *iface, const char *method)
{
    return nullptr;
}

FILE *fopen_stub(const char *__restrict__ __filename, const char *__restrict__ __modes)
{
    return nullptr;
}

int strncmp_stub(const char *__s1, const char *__s2, size_t __n)
{
    return -1;
}

extern void *realloc_stub1 (void *__ptr, size_t __size)
{
    return nullptr;
}

// 测试 kdk_get_disklist
TEST(KdkDiskInfoTest, GetDiskList)
{
    char **disklist = kdk_get_disklist();
    ASSERT_NE(disklist, nullptr); // 检查返回值不为空

    int count = 0;
    while (disklist[count])
    {
        //EXPECT_NE(disklist[count], nullptr); // 检查每个元素不为空
        printf("Disk %d: %s\n", count + 1, disklist[count]);
        count++;
    }
    kdk_free_disklist(disklist);
}

// 测试 kdk_get_diskinfo
TEST(KdkDiskInfoTest, GetDiskInfo)
{
    char **disklist = kdk_get_disklist();
    ASSERT_NE(disklist, nullptr);

    for (int i = 0; disklist[i]; i++)
    {
        kdk_diskinfo *sdainfo = kdk_get_diskinfo(disklist[i]);
        if (!sdainfo)
        {
            printf("[%s] Get disk info error.\n", disklist[i]);
            continue;
        }

        //EXPECT_NE(sdainfo->name, "");
        //EXPECT_GT(sdainfo->sectors_num, 0);
        //EXPECT_GT(sdainfo->total_size_MiB, 0);
        //EXPECT_GT(sdainfo->sector_size, 0);
        //EXPECT_NE(sdainfo->model, "");
        //EXPECT_NE(sdainfo->serial, "");
        //EXPECT_GE(sdainfo->partition_nums, 0);
        //EXPECT_LE(sdainfo->disk_type, 4);
        //EXPECT_NE(sdainfo->fwrev, "");

        int run_time = kdk_get_hard_running_time(disklist[i]);
        //EXPECT_GE(run_time, 0);

        kdk_free_diskinfo(sdainfo);
    }
    kdk_free_disklist(disklist);

    char *size = kdk_get_hard_disk_size("/dev/nvme0n1");
    //EXPECT_STREQ(size, "488386.343750");
    free(size);

    char *fwrev = kdk_get_hard_fwrev("/dev/nvme0n1");
    //EXPECT_STREQ(fwrev, "E8FK11.C");
    free(fwrev);

    char *vendor = kdk_get_hard_vendor("/dev/nvme0n1");
    ASSERT_NE(vendor, "");
    free(vendor);

    char *type = kdk_get_hard_type("/dev/nvme0n1");
    ASSERT_NE(type, "");
    free(type);

    char *model = kdk_get_hard_model("/dev/nvme0n1");
    ASSERT_NE(model, "");
    free(model);

    char *serial = kdk_get_hard_serial("/dev/nvme0n1");
    ASSERT_NE(serial, "");
    free(serial);
}

// GTest Test Cases
TEST(KdkDiskInfoTest, NullInput)
{

    char **di = kdk_get_hard_disk();
    ASSERT_NE(di, nullptr);

    kdk_diskinfo *sdainfo = kdk_get_diskinfo("");
    //EXPECT_STREQ(sdainfo->name, "");
    //EXPECT_EQ(sdainfo->sectors_num, 0);
    //EXPECT_EQ(sdainfo->sector_size, 0);
    //EXPECT_EQ(sdainfo->total_size_MiB, 0);
    //EXPECT_STREQ(sdainfo->model, "");
    //EXPECT_STREQ(sdainfo->serial, "");
    //EXPECT_EQ(sdainfo->partition_nums, 0);
    //EXPECT_EQ(sdainfo->disk_type, 4);
    //EXPECT_STREQ(sdainfo->fwrev, "");

    char *size = kdk_get_hard_disk_size("");
    //EXPECT_STREQ(size, "");

    char *fwrev = kdk_get_hard_fwrev("");
    //EXPECT_STREQ(fwrev, "");

    char *vendor = kdk_get_hard_vendor("");
    ASSERT_NE(vendor, "");

    char *type = kdk_get_hard_type("");
    ASSERT_NE(type, "");

    char *model = kdk_get_hard_model("");
    ASSERT_NE(model, "");

    char *serial = kdk_get_hard_serial("");
    ASSERT_NE(serial, "");

    int run_time = kdk_get_hard_running_time("");
    //EXPECT_EQ(run_time, 0);
}

TEST(KdkDiskInfoTest, DiskStub)
{
    Stub st;
    st.set(dbus_message_new_method_call, dbus_message_new_method_call_stub);
    char **disklist = kdk_get_disklist();
    ASSERT_NE(disklist, nullptr);

    for (int i = 0; disklist[i]; i++)
    {
        kdk_diskinfo *sdainfo = kdk_get_diskinfo(disklist[i]);
        if (!sdainfo)
        {
            printf("[%s] Get disk info error.\n", disklist[i]);
            continue;
        }

        //EXPECT_NE(sdainfo->name, nullptr);
        //EXPECT_GT(sdainfo->sectors_num, 0);
        //EXPECT_GT(sdainfo->total_size_MiB, 0);
        //EXPECT_GT(sdainfo->sector_size, 0);
        //EXPECT_NE(sdainfo->model, "");
        //EXPECT_NE(sdainfo->serial, "");
        //EXPECT_GE(sdainfo->partition_nums, 0);
        //EXPECT_LE(sdainfo->disk_type, 4);
        //EXPECT_NE(sdainfo->fwrev, "");

        int run_time = kdk_get_hard_running_time(disklist[i]);
        //EXPECT_GE(run_time, 0);

        kdk_free_diskinfo(sdainfo);
    }
    kdk_free_disklist(disklist);

    char *size = kdk_get_hard_disk_size("/dev/nvme0n1");
    //EXPECT_STREQ(size, "488386.343750");
    free(size);

    char *fwrev = kdk_get_hard_fwrev("/dev/nvme0n1");
    //EXPECT_STREQ(fwrev, "E8FK11.C");
    free(fwrev);

    char *vendor = kdk_get_hard_vendor("/dev/nvme0n1");
    ASSERT_NE(vendor, "");
    free(vendor);

    char *type = kdk_get_hard_type("/dev/nvme0n1");
    ASSERT_NE(type, "");
    free(type);

    char *model = kdk_get_hard_model("/dev/nvme0n1");
    ASSERT_NE(model, "");
    free(model);

    char *serial = kdk_get_hard_serial("/dev/nvme0n1");
    ASSERT_NE(serial, "");
    free(serial);
}

TEST(KdkDiskInfoTest, DiskStub_1)
{
    Stub st;
    st.set(dbus_connection_send_with_reply, dbus_connection_send_with_reply_stub);
    char **disklist = kdk_get_disklist();
    ASSERT_NE(disklist, nullptr);

    for (int i = 0; disklist[i]; i++)
    {
        kdk_diskinfo *sdainfo = kdk_get_diskinfo(disklist[i]);
        if (!sdainfo)
        {
            printf("[%s] Get disk info error.\n", disklist[i]);
            continue;
        }

        //EXPECT_NE(sdainfo->name, nullptr);
        //EXPECT_GT(sdainfo->sectors_num, 0);
        //EXPECT_GT(sdainfo->total_size_MiB, 0);
        //EXPECT_GT(sdainfo->sector_size, 0);
        //EXPECT_NE(sdainfo->model, "");
        //EXPECT_NE(sdainfo->serial, "");
        //EXPECT_GE(sdainfo->partition_nums, 0);
        //EXPECT_LE(sdainfo->disk_type, 4);
        //EXPECT_NE(sdainfo->fwrev, "");

        int run_time = kdk_get_hard_running_time(disklist[i]);
        //EXPECT_GE(run_time, 0);

        kdk_free_diskinfo(sdainfo);
    }
    kdk_free_disklist(disklist);

    char *size = kdk_get_hard_disk_size("/dev/nvme0n1");
    //EXPECT_STREQ(size, "488386.343750");
    free(size);

    char *fwrev = kdk_get_hard_fwrev("/dev/nvme0n1");
    //EXPECT_STREQ(fwrev, "E8FK11.C");
    free(fwrev);

    char *vendor = kdk_get_hard_vendor("/dev/nvme0n1");
    ASSERT_NE(vendor, "");
    free(vendor);

    char *type = kdk_get_hard_type("/dev/nvme0n1");
    ASSERT_NE(type, "");
    free(type);

    char *model = kdk_get_hard_model("/dev/nvme0n1");
    ASSERT_NE(model, "");
    free(model);

    char *serial = kdk_get_hard_serial("/dev/nvme0n1");
    ASSERT_NE(serial, "");
    free(serial);
}

TEST(KdkDiskInfoTest, DiskStub_2)
{
    Stub st;
    st.set(dbus_message_append_args, dbus_message_append_args_stub);
    char **disklist = kdk_get_disklist();
    ASSERT_NE(disklist, nullptr);

    for (int i = 0; disklist[i]; i++)
    {
        kdk_diskinfo *sdainfo = kdk_get_diskinfo(disklist[i]);
        if (!sdainfo)
        {
            printf("[%s] Get disk info error.\n", disklist[i]);
            continue;
        }

        //EXPECT_NE(sdainfo->name, nullptr);
        //EXPECT_GT(sdainfo->sectors_num, 0);
        //EXPECT_GT(sdainfo->total_size_MiB, 0);
        //EXPECT_GT(sdainfo->sector_size, 0);
        //EXPECT_NE(sdainfo->model, "");
        //EXPECT_NE(sdainfo->serial, "");
        //EXPECT_GE(sdainfo->partition_nums, 0);
        //EXPECT_LE(sdainfo->disk_type, 4);
        //EXPECT_NE(sdainfo->fwrev, "");

        int run_time = kdk_get_hard_running_time(disklist[i]);
        //EXPECT_GE(run_time, 0);

        kdk_free_diskinfo(sdainfo);
    }
    kdk_free_disklist(disklist);

    char *size = kdk_get_hard_disk_size("/dev/nvme0n1");
    //EXPECT_STREQ(size, "488386.343750");
    free(size);

    char *fwrev = kdk_get_hard_fwrev("/dev/nvme0n1");
    //EXPECT_STREQ(fwrev, "E8FK11.C");
    free(fwrev);

    char *vendor = kdk_get_hard_vendor("/dev/nvme0n1");
    ASSERT_NE(vendor, "");
    free(vendor);

    char *type = kdk_get_hard_type("/dev/nvme0n1");
    ASSERT_NE(type, "");
    free(type);

    char *model = kdk_get_hard_model("/dev/nvme0n1");
    ASSERT_NE(model, "");
    free(model);

    char *serial = kdk_get_hard_serial("/dev/nvme0n1");
    ASSERT_NE(serial, "");
    free(serial);

    st.set(fopen, fopen_stub);
    serial = kdk_get_hard_serial("/dev/nvme0n1");
    ASSERT_NE(serial, "");
    free(serial);

    st.set(strncmp, strncmp_stub);
    serial = kdk_get_hard_serial("/dev/nvme0n1");
    ASSERT_NE(serial, "");
    free(serial);
}

hd_t *hd_list_stub(hd_data_t *hd_data, hd_hw_item_t item, int rescan, hd_t *hd_old)
{
    return nullptr;
}

TEST(KdkDiskInfoTest, GetDiskListStub)
{
    Stub st;
    st.set(realloc, realloc_stub1);
    char **disklist = kdk_get_disklist();

    Stub st1;
    st1.set(hd_list, hd_list_stub);
    char **di = kdk_get_hard_disk();
}
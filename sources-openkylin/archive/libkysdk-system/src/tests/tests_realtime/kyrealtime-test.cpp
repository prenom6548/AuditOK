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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include "stdio.h"
#include "../../realtime/libkyrealtimeinfo.h"
#include <gtest/gtest.h>
#include "../cpp-stub/src/stub.h"
#include <dbus-1.0/dbus/dbus.h>

DBusMessage *dbus_message_new_method_call_stub(const char *destination, const char *path, const char *iface, const char *method)
{
    return nullptr;
}

TEST(KdkRealtimeTest, RealtimeInfoStub)
{
    Stub st;
    st.set(dbus_message_new_method_call, dbus_message_new_method_call_stub);
    int diskTemp = kdk_real_get_disk_temperature("/dev/nvme0n1");
    printf("diskTemp : %d\n", diskTemp);
    EXPECT_NE(diskTemp, 0);
}

TEST(KdkRealtimeTest, RealtimeInfo)
{
    float netSpeed =  kdk_real_get_net_speed("enx00e099003295");
    printf("netSpeed : %f\n", netSpeed);

    float cpuTemp = kdk_real_get_cpu_temperature();
    printf("cpuTemp: %f\n", cpuTemp);

    int diskTemp = kdk_real_get_disk_temperature("/dev/nvme0n1");
    printf("diskTemp : %d\n", diskTemp);

    int diskRate = kdk_real_get_disk_rate("/dev/nvme0n1");
    printf("diskRate : %d\n", diskRate);

    int ifnetSpeed = kdk_real_get_if_speed("enx00e099003295");
    printf("ifnetSpeed : %f\n", ifnetSpeed);
}

FILE *fopen_stub(const char *__restrict__ __filename, const char *__restrict__ __modes)
{
    return nullptr;
}

TEST(KdkRealtimeTest, RealtimeInfoStub1)
{
    Stub st;
    st.set(fopen, fopen_stub);
    float cpuTemp = kdk_real_get_cpu_temperature();
    printf("cpuTemp: %f\n", cpuTemp);
}

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

#include "stdio.h"
#include "../../global/libkyglobal.h"
#include <stdlib.h>
#include <gtest/gtest.h>
#include "../cpp-stub/src/stub.h"
#include <dbus-1.0/dbus/dbus.h>

dbus_bool_t dbus_error_is_set_stub(const DBusError *error)
{
    return true;
}

DBusMessage *dbus_message_new_method_call_stub(const char *destination, const char *path, const char *iface, const char *method)
{
    return nullptr;
}

dbus_bool_t dbus_message_append_args_stub(DBusMessage *message, int first_arg_type, ...)
{
    return false;
}

dbus_bool_t dbus_connection_send_with_reply_stub(DBusConnection *connection, DBusMessage *message, DBusPendingCall **pending_return, int timeout_milliseconds)
{
    return false;
}

DBusMessage *dbus_pending_call_steal_reply_stub(DBusPendingCall *pending)
{
    return nullptr;
}

int dbus_message_get_type_stub(DBusMessage *message)
{
    return !DBUS_MESSAGE_TYPE_ERROR;
}

extern void *realloc_stub (void *__ptr, size_t __size)
{
    return nullptr;
}


TEST(KdkGlobalTest, GlobalInfoStub)
{
    Stub st;
    st.set(dbus_message_get_type, dbus_message_get_type_stub);
    // // char *eUser = kdk_system_get_eUser();
    char *system_language = kdk_global_get_system_language("kylin");
    printf("system_language = %s\n", system_language);
    // //EXPECT_EQ(system_language, nullptr);
}

TEST(KdkGlobalTest, GlobalInfoStub1)
{
    // Stub st;
    // st.set(dbus_pending_call_steal_reply, dbus_pending_call_steal_reply_stub);
    // // eUser = kdk_system_get_eUser();
    // char *system_language = kdk_global_get_system_language("kylin");
    // printf("system_language = %s\n", system_language);
    // //EXPECT_EQ(system_language, nullptr);
}

TEST(KdkGlobalTest, GlobalInfoStub2)
{
    Stub st;
    st.set(dbus_connection_send_with_reply, dbus_connection_send_with_reply_stub);
    // eUser = kdk_system_get_eUser();
    char *system_language = kdk_global_get_system_language("kylin");
    printf("system_language = %s\n", system_language);
    //EXPECT_EQ(system_language, nullptr);

    st.set(dbus_message_append_args, dbus_message_append_args_stub);
    system_language = kdk_global_get_system_language("kylin");
    printf("system_language = %s\n", system_language);
    //EXPECT_EQ(system_language, nullptr);

    st.set(dbus_message_new_method_call, dbus_message_new_method_call_stub);
    system_language = kdk_global_get_system_language("kylin");
    printf("system_language = %s\n", system_language);
    //EXPECT_EQ(system_language, nullptr);

    // st.set(dbus_error_is_set, dbus_error_is_set_stub);
    // system_language = kdk_global_get_system_language("kylin");
    // printf("system_language = %s\n", system_language);
    // //EXPECT_EQ(system_language, nullptr);
}

TEST(KdkGlobalTest, GlobalInfoStub3)
{
    Stub st;
    st.set(realloc, realloc_stub);
    int index = 0;
    char **language = kdk_global_get_system_support_language();
    if (language)
    {
        while (language[index])
        {
            printf("index = %d, language = %s\n", index, language[index]);
            //EXPECT_NE(language[index], nullptr);
            index++;
        }

        kdk_free_langlist(language);
    }
}

TEST(KdkGlobalTest, GlobalInfo)
{
    int index = 0;
    char **language = kdk_global_get_system_support_language();
    if (language)
    {
        while (language[index])
        {
            printf("index = %d, language = %s\n", index, language[index]);
            //EXPECT_NE(language[index], nullptr);
            index++;
        }

        kdk_free_langlist(language);
    }

    bool match = kdk_global_get_region_match_language();
    printf("match = %d\n", match);
    // //EXPECT_EQ(match, 1);

    bool rtl = kdk_global_get_rtl();
    printf("rtl = %d\n", rtl);
    //EXPECT_EQ(rtl, 0);

    int offset = kdk_global_get_raw_offset();
    printf("offset = %d\n", offset);
    //EXPECT_EQ(offset, 8);

    // char *"kylin" = kdk_system_get_eUser();
    char *system_language = kdk_global_get_system_language("kylin");
    printf("system_language = %s\n", system_language);
    //EXPECT_STREQ(system_language, "zh_CN");
}

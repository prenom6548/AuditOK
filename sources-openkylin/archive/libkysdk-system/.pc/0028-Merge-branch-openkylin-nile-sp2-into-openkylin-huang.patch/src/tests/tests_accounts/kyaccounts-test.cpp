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

#include <stdio.h>
#include "../../accounts/libkyaccounts.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "../cpp-stub/src/stub.h"
#include <dbus-1.0/dbus/dbus.h>

void callbackelogout(int out, void *userdata)
{
    printf("-----------%d\n", out);
}

void callbacklogin(kdk_userinfo *info, void *userdata)
{
    //     printf("subscribe = %s\n", info->subscribe);
    //     printf("data_sharing = %s\n", info->data_sharing);
    printf("kylinID = %s\n", info->kylinID);
    // printf("avatar = %s\n", info->avatar);
    printf("userName = %s\n", info->userName);
    printf("token = %s\n", info->token);
    printf("uid = %s\n", info->uid);
    printf("phone = %s\n", info->phone);
    // printf("identity = %s\n", info->identity);
    // printf("hasPhone = %s\n", info->hasPhone);
    // printf("region = %s\n", info->region);
    printf("email = %s\n", info->email);
    // printf("redirect_url = %s\n", info->redirect_url);
    printf("status = %s\n", info->status);
}

size_t strlen_stub1(const char *__s)
{
    return 1;
}

size_t strlen_stub2(const char *__s)
{
    return 1025;
}

dbus_bool_t dbus_error_is_set_stub(const DBusError *error)
{
    return true;
}

DBusMessage *dbus_message_new_method_call_stub(const char *destination, const char *path, const char *iface, const char *method)
{
    return nullptr;
}

dbus_bool_t dbus_message_append_args_stub(DBusMessage *message, int first_arg_type,...)
{
    return false;
}

dbus_bool_t dbus_connection_send_with_reply_stub(DBusConnection *connection, DBusMessage *message, DBusPendingCall **pending_return, int timeout_milliseconds)
{
    return false;
}

TEST(KdkAccountTest, AccountNull)
{
    char* ret = kdk_system_create_user(nullptr, nullptr, 0);
    printf("ret = %s\n", ret);
    //EXPECT_EQ(ret, nullptr);

    unsigned int err_num;
    bool pass = kdk_system_change_password("test-pc", nullptr, &err_num);
    printf("pass = %d, err_num = %d\n", pass, err_num);
    //EXPECT_EQ(pass, 0);

    pass = kdk_system_change_password(nullptr, nullptr, &err_num);
    printf("pass = %d, err_num = %d\n", pass, err_num);
    //EXPECT_EQ(pass, 0);

    pass = kdk_system_change_password(nullptr, "qwer$\"1234\"", &err_num);
    printf("pass = %d, err_num = %d\n", pass, err_num);
    //EXPECT_EQ(pass, 0);

    bool user = kdk_system_check_has_user(nullptr);
    printf("user = %d\n", user);
    //EXPECT_EQ(user, 0);

    bool result = kdk_login_send_password(nullptr, 0, 0);
    printf("result = %d\n", result);
    //EXPECT_EQ(result, 0);

    int ret_login = kdk_login_set_process_name(nullptr);
    //EXPECT_EQ(ret_login, 0);

    ret_login = kdk_login_send_auth_token(1, nullptr, NULL, 0, 1);
    printf("ret = %d\n", ret_login);
    //EXPECT_EQ(ret_login, -7);
}

TEST(KdkAccountTest, AccountInfo)
{

    char *ret = kdk_system_create_user("kylinTest", "kylinTest-pc", 0);
    printf("ret = %s\n", ret);
    //EXPECT_STREQ(ret, "/home/kylinTest");
    free(ret);

    unsigned int err_num;
    bool pass = kdk_system_change_password("test", "qwer$\"1234\"", &err_num);
    printf("pass = %d， err_num = %d\n", pass, err_num);
    //EXPECT_EQ(pass, 1);

    pass = kdk_system_change_password("ttttt", "qwer$\"1234\"", &err_num);
    printf("pass = %d， err_num = %d\n", pass, err_num);
    //EXPECT_EQ(pass, 0);

    bool user = kdk_system_check_has_user("kylinTest");
    printf("user = %d\n", user);
    //EXPECT_EQ(user, 1);

    user = kdk_system_check_has_user("tttt");
    printf("user = %d\n", user);
    //EXPECT_EQ(user, 0);

    char *pencrypt = kdk_login_get_public_encrypt();
    printf("pencrypt = %s\n", pencrypt);
    //EXPECT_NE(pencrypt, nullptr);


    char *secret = "hhhkkkkjjuojk1234";

    int len = strnlen(secret, 1024 * 1024 * 1024);
    unsigned char *byte_array = (unsigned char *)malloc(len + 1);
    if (byte_array)
    {
        for (int i = 0; i < len; i++)
    {
        byte_array[i] = (unsigned char)secret[i];
        printf("byte_array[i] = %d\n", byte_array[i]);
    }
    }    
    byte_array[len] = '\0';

    bool result = kdk_login_send_password("test", byte_array, len);
    printf("result = %d\n", result);
    //EXPECT_EQ(result, 0);

    char *login = kdk_kylinid_check_login();
    printf("login = %s\n", login);
    //EXPECT_EQ(login, nullptr);

    char *token = kdk_kylinid_get_user_token();
    printf("token = %s\n", token);
    //EXPECT_EQ(token, nullptr);

    kdk_kylinid_register_switch_logout_handle(callbackelogout, NULL);
    sleep(5);
    kdk_kylinid_unregister_switch_logout_handle();

    // sleep(5);
    // printf("test\n");

    kdk_kylinid_register_switch_login_handle(callbacklogin, NULL);
    sleep(10);
    kdk_kylinid_unregister_switch_login_handle();

    result = kdk_login_send_password("test", byte_array, len);
    printf("result = %d\n", result);\
    //EXPECT_EQ(result, 0);

    int ret_login = kdk_login_set_process_name("test");
    printf("ret_login = %d\n", ret_login);
    //EXPECT_GT(ret_login, 0);

    ret_login = kdk_login_send_auth_token(1, "test", NULL, 0, 1);
    printf("ret = %d\n", ret_login);
    //EXPECT_GT(ret_login, -7);
}

TEST(KdkAccountTest, AccountDbusStub)
{
    Stub st;
    st.set(dbus_connection_send_with_reply, dbus_connection_send_with_reply_stub);

    char *ret = kdk_system_create_user("ttt", "ttt-pc", 0);
    printf("ret = %s\n", ret);
    //EXPECT_STREQ(ret, nullptr);
    free(ret);

    unsigned int err_num;
    bool pass = kdk_system_change_password("kylinTest", "qwer$\"1234\"", &err_num);
    printf("pass = %d， err_num = %d\n", pass, err_num);
    //EXPECT_EQ(pass, 0);

    char *pencrypt = kdk_login_get_public_encrypt();
    printf("pencrypt = %s\n", pencrypt);
    //EXPECT_EQ(pencrypt, nullptr);

    char *secret = "hhhkkkkjjuojk1234";

    int len = strnlen(secret, 1024 * 1024 * 1024);
    unsigned char *byte_array = (unsigned char *)malloc(len + 1);
    if (byte_array)
    {
        for (int i = 0; i < len; i++)
    {
        byte_array[i] = (unsigned char)secret[i];
        // printf("byte_array[i] = %d\n", byte_array[i]);
    }
    }    
    byte_array[len] = '\0';

    bool result = kdk_login_send_password("test", byte_array, len);
    printf("result = %d\n", result);
    //EXPECT_EQ(result, 0);

    char *login = kdk_kylinid_check_login();
    printf("login = %s\n", login);
    //EXPECT_EQ(login, nullptr);

    char *token = kdk_kylinid_get_user_token();
    printf("token = %s\n", token);
    //EXPECT_EQ(token, nullptr);

    kdk_kylinid_register_switch_logout_handle(callbackelogout, NULL);
    sleep(5);
    kdk_kylinid_unregister_switch_logout_handle();

    // sleep(5);
    // printf("test\n");

    kdk_kylinid_register_switch_login_handle(callbacklogin, NULL);
    sleep(10);
    kdk_kylinid_unregister_switch_login_handle();

    int ret_login = kdk_login_set_process_name("test");
    printf("ret_login = %d\n", ret_login);
    //EXPECT_EQ(ret_login, 0);

    ret_login = kdk_login_send_auth_token(1, "test", NULL, 0, 1);
    printf("ret = %d\n", ret_login);
    //EXPECT_EQ(ret_login, -7);


    st.set(dbus_message_append_args, dbus_message_append_args_stub);

    ret = kdk_system_create_user("ttt", "ttt-pc", 0);
    printf("ret = %s\n", ret);
    //EXPECT_STREQ(ret, nullptr);
    // free(ret);

    pass = kdk_system_change_password("kylinTest", "qwer$\"1234\"", &err_num);
    printf("pass = %d， err_num = %d\n", pass, err_num);
    //EXPECT_EQ(pass, 0);

    ret_login = kdk_login_set_process_name("test");
    printf("ret_login = %d\n", ret_login);
    //EXPECT_EQ(ret_login, 0);
}

TEST(KdkAccountTest, AccountStub)
{
    Stub st;

    // st.set(dbus_error_is_set, dbus_error_is_set_stub);
    st.set(dbus_message_new_method_call, dbus_message_new_method_call_stub);
    char *ret = kdk_system_create_user("kylinTest", "kylinTest-pc", 0);


    unsigned int err_num;
    bool pass = kdk_system_change_password("test", "qwer$\"1234\"", &err_num);
    printf("pass = %d， err_num = %d\n", pass, err_num);

    pass = kdk_system_change_password("test", "qwer", &err_num);
    printf("pass = %d， err_num = %d\n", pass, err_num);
//     //EXPECT_EQ(pass, 0);

    char *pencrypt = kdk_login_get_public_encrypt();
    printf("pencrypt = %s\n", pencrypt);
    //EXPECT_EQ(pencrypt, nullptr);

    char *passwd = "hhhkkkkjjuojk1234";

    int len = strnlen(passwd, 1024 * 1024 * 1024);
    unsigned char *byte_array = (unsigned char *)malloc(len + 1);
    if (byte_array)
    {
        for (int i = 0; i < len; i++)
    {
        byte_array[i] = (unsigned char)passwd[i];
        // printf("byte_array[i] = %d\n", byte_array[i]);
    }
    }    
    byte_array[len] = '\0';

    bool result = kdk_login_send_password("test", byte_array, len);
    printf("result = %d\n", result);
    //EXPECT_EQ(result, 0);

    char *login = kdk_kylinid_check_login();
    printf("login = %s\n", login);

    char *token = kdk_kylinid_get_user_token();
    printf("token = %s\n", token);

//     kdk_kylinid_register_switch_logout_handle(callbackelogout, NULL);
//     sleep(5);
//     kdk_kylinid_unregister_switch_logout_handle();

//     // sleep(5);
//     // printf("test\n");

//     kdk_kylinid_register_switch_login_handle(callbacklogin, NULL);
//     sleep(10);
//     kdk_kylinid_unregister_switch_login_handle();

    int ret_login = kdk_login_set_process_name("test");
    printf("ret_login = %d\n", ret_login);
    //EXPECT_EQ(ret_login, 0);

    ret_login = kdk_login_send_auth_token(1, "test", NULL, 0, 1);
    printf("ret = %d\n", ret_login);
    //EXPECT_EQ(ret_login, -7);


//     st.set(strlen, strlen_stub1);
//     // unsigned int err_num;
//     pass = kdk_system_change_password("kylinTest", "qwer$\"1234\"", &err_num);
//     printf("pass = %d， err_num = %d\n", pass, err_num);
//     //EXPECT_EQ(pass, 0);

//     st.set(strlen, strlen_stub2);
//     pass = kdk_system_change_password("kylinTest", "qwer$\"1234\"", &err_num);
//     printf("pass = %d， err_num = %d\n", pass, err_num);
//     //EXPECT_EQ(pass, 0);
}

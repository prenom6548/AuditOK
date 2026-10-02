/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
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
#include "../libkyaccounts.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void callbackelogout(int out, void *userdata)
{
    printf("%d\n", out);
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

int main()
{

    char* ret = kdk_system_create_user("test", "test-pc", 0);
    printf("ret = %s\n", ret);
    free(ret);

    unsigned int err_num;
    bool pass = kdk_system_change_password("test", "qwer$\"1234\"", &err_num);
    printf("pass = %d， err_num = %d\n", pass, err_num);

    bool user = kdk_system_check_has_user("test");
    printf("user = %d\n", user);

    // char *pencrypt = kdk_login_get_public_encrypt();
    // printf("pencrypt = %s\n", pencrypt);

    char *passwd = "hhhkkkkjjuojk1234";

    // int len = strlen(passwd);
    // unsigned char *byte_array = (unsigned char *)malloc(len + 1);
    // if (!byte_array)
    // {
    //     return false;
    // }

    // for (int i = 0; i < len; i++) {  
    //     byte_array[i] = (unsigned char)passwd[i];
    //     printf("byte_array[i] = %d\n", byte_array[i]);
    // }  
    // byte_array[len] = '\0';

    // bool result = kdk_login_send_password("test", byte_array, len);
    // printf("result = %d\n", result);

    char* login = kdk_kylinid_check_login();
    printf("login = %s\n", login);

    char* token = kdk_kylinid_get_user_token();
    printf("token = %s\n", token);

    kdk_kylinid_register_switch_logout_handle(callbackelogout, NULL);
    sleep(15);
    kdk_kylinid_unregister_switch_logout_handle();

    sleep(5);
    printf("test\n");

    kdk_kylinid_register_switch_login_handle(callbacklogin, NULL);
    sleep(20);
    kdk_kylinid_unregister_switch_login_handle();
    
    // bool result = kdk_login_send_password("test", byte_array, len);
    // printf("result = %d\n", result);

    // int ret = kdk_login_set_process_name("test");

    ret = kdk_login_send_auth_token(1, "test", NULL, 0, 1);
    printf("ret = %d\n", ret);
}

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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#ifndef KYSDK_SYSTEM_SETTINGS_H__
#define KYSDK_SYSTEM_SETTINGS_H__

/**
 * @file        libkyaccounts.h
 * @brief       设置用户信息
 * @author      shaozhimin (shaozhimin@kylinos.cn)
 * @version     0.1
 * @date        2023-10-07
 * @copyright   Copyright (c) 2023
 * 
 * @since       2.4.1.0
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _kdk_userinfo{
    // char subscribe[16];  
    // char data_sharing[16];
    char kylinID[64];
    // char avatar[128];
    char userName[128];
    char token[2048];
    char uid[128];
    char phone[16];
    // char identity[16];
    // char hasPhone[16];
    // char region[128];
    char email[128];
    // char redirect_url[128];
    char status[16];
}kdk_userinfo;

/**
 * @brief 创建用户，并创建家目录
 * @param name 用户名
 * @param fullName 用户全名
 * @param accountType 创建的类型，0标准用户，1管理员用户
 * 
 * @return char* 成功返回用户家目录路径，失败返回NULL；返回的字符串需要被 free 释放。
 */
extern char* kdk_system_create_user(char* name, char* fullName, int accountType);

/**
 * @brief 修改用户密码
 * @param username 用户名
 * @param password 新的用户密码
 * @param err_num 0,修改密码成功；1,身份验证失败；2,用户不存在；3,密码不规范；4,参数为空。
 * 
 * @return bool 是否修改成功；false代表修改失败，true代表修改成功。
 */
extern bool kdk_system_change_password(char* username, char* password, unsigned int *err_num);

/**
 * @brief 获取用户是否存在
 * @param username 用户名
 * 
 * @return bool 返回用户是否存在；false代表不存在，true代表存在。
 */
extern bool kdk_system_check_has_user(char *username);

/**
 * @brief 获取加密使用的公钥
 * 
 * @return char* 返回非对称加密使用的公钥，加密使用rsa算法
 */
extern char* kdk_login_get_public_encrypt();

/**
 * @brief 发送用户名和密码进行登录
 * 
 * @param username 用户名
 * @param password 加密后的密码
 * @param length 密码长度
 * 
 * @return bool true表示开始拉起桌面会话，false表示未能开始拉起桌面会话
 */
extern bool kdk_login_send_password(const char* username, unsigned char* password, int length);

/**
 * @brief 检测麒麟ID是否登录
 * 
 * @return char* 登录返回麒麟ID名称，未登录返回-1；失败返回NULL。
 * @since 2.5.1.2
 */
extern char* kdk_kylinid_check_login();

/**
 * @brief 获取麒麟ID的用户token
 * 
 * @return char* 成功返回用户token，失败返回NULL。
 * @since 2.5.1.2
 */
extern char* kdk_kylinid_get_user_token();

/**
 * @brief 用户监听事件的回调函数原型
*/
typedef void (*CallBackLogout)(int out, void *user_data);

/**
 * @brief 注册监听麒麟ID退出事件的回调函数
 * @param call_back 指向回调函数的函数指针
 * @param user_data 传递给回调函数的用户数据结构指针
 * 
 * @return int 1 成功; 0 失败
 * @since 2.5.1.2
*/
extern int kdk_kylinid_register_switch_logout_handle(CallBackLogout call_back, void *user_data);

/**
 * @brief 注销监听麒麟ID退出事件的回调函数
 * @since 2.5.1.2
*/
extern void kdk_kylinid_unregister_switch_logout_handle();

/**
 * @brief 用户监听事件的回调函数原型
*/
typedef void (*CallBackLogin)(kdk_userinfo *info, void *user_data);

/**
 * @brief 注册监听麒麟ID登录事件的回调函数
 * @param call_back 指向回调函数的函数指针
 * @param user_data 传递给回调函数的用户数据结构指针
 * 
 * @return int 1 成功; 0 失败
 * @since 2.5.1.2
*/
extern int kdk_kylinid_register_switch_login_handle(CallBackLogin call_back, void *user_data);

/**
 * @brief 注销监听用麒麟ID登录事件的回调函数
 * @since 2.5.1.2
*/
extern void kdk_kylinid_unregister_switch_login_handle();

/**
 * @brief 传入调用进程名称，完成调用进程的授权设置；调用该接口需要root权限。
 * 
 * @param processName 调用进程绝对路径名称
 * 
 * @return int 唯一id，id大于0时代表设置成功，且需要保存下来，用于后面的kdk_login_set_auth_token接口，id为-1代表后端程序异常，id为-2时代表权限不足，id为-3时代表其他错误
 * @since 2.5.1.2
 */
extern int kdk_login_set_process_name(char *processName);

/**
 * @brief 传入认证token
 * 
 * @param id 认证时唯一id，调用kdk_login_set_process_name的返回值
 * @param username 用户名
 * @param token 加密后的用户密码，如果不是使用密码认证，则传空
 * @param length 用户密码长度，如果不是密码认证，则传0
 * @param authType 认证类型。0代表使用密码认证，1代表使用微信扫码。使用微信登陆时，密码需传空值，密码长度传0
 * 
 * @return int 返回认证结果。0代表认证通过；-1代表登陆程序后端异常，-2代表权限不足，-3代表认证失败；-4代表当前不处于登陆界面；-5代表进程名称和设置的进程名称不一致，-6代表id不一致，-7代表其他错误
 * @since 2.5.1.2
 */
extern int kdk_login_send_auth_token(int id, char *username, unsigned char *token, int length, int  authType);

#ifdef __cplusplus
}
#endif

#endif  // KYSDK_SYSTEM_SETTINGS_H__

/**
  * @}
  */

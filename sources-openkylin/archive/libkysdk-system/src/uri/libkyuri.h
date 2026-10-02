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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef KYSDK_SYSTEM_URI
#define KYSDK_SYSTEM_URI

/**
 * @file libkyuri.h
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief URL工具模块
 * @version 0.1
 * @date 2025-4-14
 * 
 * @copyright Copyright (c) 2025
 * @defgroup libkysdk-uri
 * @{
 * @since 3.0.1.0
 */

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    PARSE_SCHEMA = 1,
    PARSE_USER,
    PARSE_PASSWORD,
    PARSE_HOST,
    PARSE_PATH,
    PARSE_PORT,
    PARSE_QUERY,
    PARSE_FRAGMENT
} ParseAction;

typedef struct
{
    char *scheme;   // 指定URL的协议部分，例如 "https"。
    char *user;     // 包含用户身份验证信息，如 "user:password"（如果存在）。
    char *password; // 包含用户身份验证信息，如 "user:password"（如果存在）。
    char *host;     // 指定主机名或IP地址，例如 "example.com"。
    char *port;     // 指定端口号，例如 HTTP 的 80 或 HTTPS 的 443。
    char *path;     // 指定资源路径，例如 "/path/to/resource"。
    char *query;    // 包含查询字符串参数，例如 "key=value"（如果存在）。
    char *fragment; // 指定URL中的片段标识符，例如 "#section"（如果存在）。
} URLScheme;

/**
 * @brief 检查字符串是否是url（格式是否合法）
 *
 * @param url URL字符串
 * @return int 合法返回0，不合法返回-1
 * @since 3.0.1.0
 */
int kdk_uri_check_is_url(char *url);

/**
 * @brief 创建URL
 *
 * @param url URLScheam结构体
 * @return char* URL字符串,失败返回NULL
 * @since 3.0.1.0
 */
char *kdk_uri_create_url(URLScheme *url);

/**
 * @brief 用于将基本 URL和相对 URL合并成一个完整URL
 *
 * @param base_url 基本URL
 * @param relative_url 相对URL
 * @return char* 成功URL,失败返回NULL
 * @since 3.0.1.0
 */
char *kdk_uri_combine_url(char *base_url, char *relative_url);

/**
 * @brief 规范化指定的url
 *
 * @param url URL字符串
 * @return char* 规范后的URL字符串,失败返回NULL
 * @since 3.0.1.0
 */
char *kdk_uri_get_format(char *url);

/**
 * @brief 解析URL
 *
 * @param url URL字符串
 * @return URLScheme* URLScheme结构体,失败返回NULL
 * @since 3.0.1.0
 */
URLScheme *kdk_uri_get_parse_info(char *url);

/**
 * @brief 修改URL指定参数
 *
 * @param url URL字符串
 * @param action 参数类型
 * @param info 要设置的值
 * @return char* 修改后的URL,失败返回NULL
 * @since 3.0.1.0
 */
char *kdk_uri_set_parse_info(char *url, ParseAction action, char *info);

/**
 * @brief 获取url的指定参数
 *
 * @param url URL字符串
 * @param action 参数类型
 * @param info 参数的值
 * @return char * 指定参数的值，失败返回NULL
 * @since 3.0.1.0
 */
char *kdk_uri_get_parse_action_info(char *url, ParseAction action);

/**
 * @brief 回调函数 用于获取下载结果
 * @since 3.0.1.0
 *
 *  error_str: code
 *  "Transport error"   // [0,  100)
 *  "Informational"     // [100,200)
 *  "Successful"        // [200,300)
 *  "Redirection"       // [300,400)
 *  "Client error"      // [400,500)
 *  "Servererror"       // [500,600)
 */
typedef void (*kdk_url_download_callback)(int code, const char *err_string);

/**
 * @brief 从 Internet 下载数据并将其保存到文件中，此操作为异步
 *
 * @param url URL字符串
 * @param filepath 指向保存下载数据的文件路径的指针
 * @return int 成功返回0
 * @since 3.0.1.0
 */
int kdk_url_download_to_local(char *url, char *filepath, kdk_url_download_callback callback);

/**
 * @brief 释放内存
 *
 * @param scheme URLScheme指针
 * @since 3.0.1.0
 */
void kdk_uri_free_url_scheme(URLScheme *scheme);

#ifdef __cplusplus
}
#endif

#endif // KYSDK_SYSTEM_URI

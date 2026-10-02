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

#include "libkyuri.h"
#include "libkylog.h"
#include "cstring-extension.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <libsoup/soup.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

#define URI_FREE(x) \
    if ((x))        \
        free((x));
#define MAX_DOWNLOADS 10

typedef struct
{
    char *file;
    SoupSession *msg;
    kdk_url_download_callback callback;
} UriParam;

const char *errs[] = {"Transport error", // [0,  100)
                      "Informational",   // [100,200)
                      "Successful",      // [200,300)
                      "Redirection",     // [300,400)
                      "Client error",    // [400,500)
                      "Servererror"};    // [500,600)

static char *_strlowr(char *str)
{
    char *orign = str;
    for (; *str != '\0'; str++)
        *str = tolower(*str);
    return orign;
}

static char *_real_url_path(char *base_url, char *relative_url)
{
    if (relative_url[0] == '/')
        return strdup(relative_url);

    if ((NULL == base_url) && (relative_url[0] != '/'))
        return NULL;

    if (base_url[strnlen(base_url, STRNLEN_MAX) - 1] == '/')
        base_url[strnlen(base_url, STRNLEN_MAX) - 1] = '\0';

    while (relative_url[0] == '.')
    {
        if (0 == strstartswith(relative_url, "./"))
        {
            memset(relative_url, ' ', 2);
            strstripspace(relative_url);
        }
        if (0 == strstartswith(relative_url, "../"))
        {
            char *p = strrchr(base_url, '/');
            if (NULL == p)
                return NULL;
            *p = '\0';

            memset(relative_url, ' ', 3);
            strstripspace(relative_url);
        }
    }
    char *result = calloc(strnlen(base_url, STRNLEN_MAX) + strnlen(relative_url, STRNLEN_MAX) + 2, sizeof(char));
    snprintf(result, strnlen(base_url, STRNLEN_MAX) + strnlen(relative_url, STRNLEN_MAX) + 2, "%s/%s", base_url, relative_url);
    return result;
}

static void on_response(SoupSession *session, SoupMessage *msg, gpointer user_data)
{
    UriParam *param = (UriParam *)user_data;
    if (msg->status_code == SOUP_STATUS_OK)
    {
        FILE *out = fopen(param->file, "w");
        if (NULL == out)
        {
            param->callback(SOUP_STATUS_IO_ERROR, errs[SOUP_STATUS_IO_ERROR / 100]);
            goto out;
        }

        // 获取响应体的数据和长度
        fwrite(msg->response_body->data, 1, msg->response_body->length, out);
        fclose(out);
    }
    param->callback(msg->status_code, errs[msg->status_code / 100]);

out:
    // g_object_unref(param->msg);
    free(param);
}

int kdk_uri_check_is_url(char *url)
{
    if (!url)
        return -1;

    SoupURI *uri = soup_uri_new(url);
    if (!uri)
    {
        return -1;
    }

    // 检查必要的URL组成部分
    if (!uri->scheme || !uri->host)
    {
        soup_uri_free(uri);
        return -1;
    }

    // 检查scheme是否是http或https
    // if (g_ascii_strcasecmp(uri->scheme, "http") != 0 &&
    //     g_ascii_strcasecmp(uri->scheme, "https") != 0)
    // {
    //     soup_uri_free(uri);
    //     return -1;
    // }

    soup_uri_free(uri);
    return 0;
}

char *kdk_uri_create_url(URLScheme *url)
{
    if (!url || !url->scheme || !url->host)
        return NULL;

    int len = strnlen(url->scheme, STRNLEN_MAX) + 3;
    if (url->user && url->password)
        len += strnlen(url->user, STRNLEN_MAX) + strnlen(url->password, STRNLEN_MAX) + 2;
    len += strnlen(url->host, STRNLEN_MAX);
    if (url->port)
        len += strnlen(url->port, STRNLEN_MAX) + 1;
    if (url->path)
        len += strnlen(url->path, STRNLEN_MAX);
    if (url->query)
        len += strnlen(url->query, STRNLEN_MAX) + 1;
    if (url->fragment)
        len += strnlen(url->fragment, STRNLEN_MAX) + 1;
    len += 1;

    char *result = (char *)calloc(len, sizeof(char));

    if (url->user && url->password)
        snprintf(result, len, "%s://%s:%s@%s", url->scheme, url->user, url->password, url->host);
    else
        snprintf(result, len, "%s://%s", url->scheme, url->host);
    if (url->port)
    {
        strcat(result, ":");
        strcat(result, url->port);
    }
    if (url->path)
        strcat(result, url->path);
    if (url->query)
    {
        strcat(result, "?");
        strcat(result, url->query);
    }
    if (url->fragment)
    {
        strcat(result, "#");
        strcat(result, url->fragment);
    }

    if (0 != kdk_uri_check_is_url(result))
    {
        free(result);
        printf("Error: Invalid URL");
        return NULL;
    }

    return result;
}

char *kdk_uri_combine_url(char *base_url, char *relative_url)
{
    if (!base_url || !relative_url)
        return NULL;

    CURLU *curlu;
    char *absolute_url;

    // 初始化 libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // 创建一个 CURLU 实例
    curlu = curl_url();
    if (NULL == curlu)
    {
        return NULL;
    }

    // 设置基础 URL
    int uc = curl_url_set(curlu, CURLUPART_URL, base_url, 0);
    if (uc != CURLUE_OK)
    {
        return NULL;
    }

    char *base_path;
    curl_url_get(curlu, CURLUPART_PATH, &base_path, 0);
    char *real_path = _real_url_path(base_path, relative_url);
    curl_free(base_path);

    // 合并相对 URL
    uc = curl_url_set(curlu, CURLUPART_PATH, real_path, 0);
    free(real_path);
    if (uc != CURLUE_OK)
    {
        return NULL;
    }

    // 获取合并后的绝对 URL
    uc = curl_url_get(curlu, CURLUPART_URL, &absolute_url, 0);
    if (uc != CURLUE_OK)
    {
        return NULL;
    }

    // 清理资源
    // curl_free(absolute_url);
    curl_url_cleanup(curlu);
    curl_global_cleanup();

    return absolute_url;
}

char *kdk_uri_get_format(char *url)
{
    if (NULL == url)
        return NULL;

    URLScheme *scheme = kdk_uri_get_parse_info(url);
    if (NULL == scheme)
    {
        return NULL;
    }
    _strlowr(scheme->scheme);
    _strlowr(scheme->host);
    if (scheme->port && (0 == strcmp(scheme->port, "443")))
    {
        free(scheme->port);
        scheme->port = 0;
    }
    if (scheme->fragment)
    {
        free(scheme->fragment);
        scheme->fragment = 0;
    }

    char *result = kdk_uri_create_url(scheme);
    kdk_uri_free_url_scheme(scheme);

    return result;
}

URLScheme *kdk_uri_get_parse_info(char *url)
{
    if (!url)
        return NULL;

    CURLU *h;
    CURLUcode uc;

    h = curl_url();
    uc = curl_url_set(h, CURLUPART_URL, url, 0);
    if (uc)
    {
        klog_err("Error setting URL: %s\n", curl_easy_strerror(uc));
        curl_url_cleanup(h);
        return NULL;
    }

    URLScheme *result = calloc(1, sizeof *result);

    // 获取各个部分
    char *scheme, *user, *password, *host, *path, *query, *fragment;
    curl_url_get(h, CURLUPART_SCHEME, &result->scheme, 0);
    curl_url_get(h, CURLUPART_USER, &result->user, 0);
    curl_url_get(h, CURLUPART_PASSWORD, &result->password, 0);
    curl_url_get(h, CURLUPART_HOST, &result->host, 0);
    curl_url_get(h, CURLUPART_PORT, &result->port, 0);
    curl_url_get(h, CURLUPART_PATH, &result->path, 0);
    curl_url_get(h, CURLUPART_QUERY, &result->query, 0);
    curl_url_get(h, CURLUPART_FRAGMENT, &result->fragment, 0);

    // 释放资源
    curl_url_cleanup(h);

    return result;
}

char *kdk_uri_set_parse_info(char *url, ParseAction action, char *info)
{
    if (!url)
        return NULL;

    URLScheme *scheme = kdk_uri_get_parse_info(url);
    if (NULL == scheme)
        return NULL;

    switch (action)
    {
    case PARSE_SCHEMA:
    {
        URI_FREE(scheme->scheme)
        scheme->scheme = strdup(info);
    }
    break;
    case PARSE_USER:
    {
        URI_FREE(scheme->user)
        scheme->user = strdup(info);
    }
    break;
    case PARSE_PASSWORD:
    {
        URI_FREE(scheme->password)
        scheme->password = strdup(info);
    }
    break;
    case PARSE_HOST:
    {
        URI_FREE(scheme->host)
        scheme->host = strdup(info);
    }
    break;
    case PARSE_PORT:
    {
        URI_FREE(scheme->port)
        scheme->port = strdup(info);
    }
    break;
    case PARSE_PATH:
    {
        URI_FREE(scheme->path)
        scheme->path = strdup(info);
    }
    break;
    case PARSE_QUERY:
    {
        URI_FREE(scheme->query)
        scheme->query = strdup(info);
    }
    break;

    case PARSE_FRAGMENT:
    {
        URI_FREE(scheme->fragment)
        scheme->fragment = strdup(info);
    }
    break;
    default:
        break;
    }

    char *result = kdk_uri_create_url(scheme);
    kdk_uri_free_url_scheme(scheme);

    return result;
}

char *kdk_uri_get_parse_action_info(char *url, ParseAction action)
{
    if (!url)
        return NULL;

    URLScheme *scheme = kdk_uri_get_parse_info(url);
    if (NULL == scheme)
        return NULL;

    char *result = NULL;
    switch (action)
    {
    case PARSE_SCHEMA:
    {
        result = strdup(scheme->scheme);
    }
    break;
    case PARSE_USER:
    {
        result = strdup(scheme->user);
    }
    break;
    case PARSE_PASSWORD:
    {
        result = strdup(scheme->password);
    }
    break;
    case PARSE_HOST:
    {
        result = strdup(scheme->host);
    }
    break;
    case PARSE_PORT:
    {
        result = strdup(scheme->port);
    }
    break;
    case PARSE_PATH:
    {
        result = strdup(scheme->path);
    }
    break;
    case PARSE_QUERY:
    {
        result = strdup(scheme->query);
    }
    break;

    case PARSE_FRAGMENT:
    {
        result = strdup(scheme->fragment);
    }
    break;
    default:
        break;
    }

    kdk_uri_free_url_scheme(scheme);

    return result;
}

int kdk_url_download_to_local(char *url, char *filepath, kdk_url_download_callback callback)
{
    if (0 != kdk_uri_check_is_url(url))
        return -1;

    static SoupSession *session = NULL;
    UriParam *param = calloc(1, sizeof * param);

    if (NULL == session)
    {
        g_type_init();

        session = soup_session_new();
    }

    // 创建一个新的 GET 请求
    param->msg = soup_message_new("GET", url);
    param->file = filepath;
    param->callback = callback;

    GError *error;
    soup_session_queue_message(session, param->msg, on_response, (gpointer)param);

    return 0;
}

void kdk_uri_free_url_scheme(URLScheme *scheme)
{
    if (scheme)
    {
        URI_FREE(scheme->scheme)
        URI_FREE(scheme->user)
        URI_FREE(scheme->password)
        URI_FREE(scheme->host)
        URI_FREE(scheme->port)
        URI_FREE(scheme->path)
        URI_FREE(scheme->query)
        URI_FREE(scheme->fragment)
        free(scheme);
    }
}

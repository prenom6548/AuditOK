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

#include "../libkyuri.h"
#include "stdio.h"
#include "stdlib.h"
#include "glib.h"

GMainLoop *loop = NULL;
const char *url = "HTTPS://tianss:qwe123@www.Example.com:443/path?query=1#fragment";

void callback(int code, const char *error)
{
    printf("%d %s\n", code, error);
    g_main_loop_quit(loop);
}

int main()
{
    loop = g_main_loop_new(NULL, FALSE); 

    int success = 0;
    success = kdk_uri_check_is_url(url);
    printf("这个URL%s\n", success ? "错误" : "正确");

    URLScheme *scheme = kdk_uri_get_parse_info(url);
    printf("Scheme: %s\n", scheme->scheme);
    printf("User: %s\n", scheme->user);
    printf("Password: %s\n", scheme->password);
    printf("Host: %s\n", scheme->host);
    printf("Port: %s\n", scheme->port);
    printf("Path: %s\n", scheme->path);
    printf("Query: %s\n", scheme->query ? scheme->query : "None");

    char *rurl = kdk_uri_create_url(scheme);
    printf("Creat URL %s\n", rurl);
    free(rurl);

    rurl = kdk_uri_get_format(url);
    printf("Format URL %s\n", rurl);
    free(rurl);

    rurl = kdk_uri_combine_url("http://example.com/path", "file.html");
    printf("Combine URL %s\n", rurl);
    free(rurl);

    rurl = kdk_uri_set_parse_info(url, PARSE_USER, "tianshaoshuai");
    printf("Set parse URL %s\n", rurl);
    free(rurl);

    char *user = kdk_uri_get_parse_action_info(url, PARSE_USER);
    printf("Parse user %s\n", user);
    free(user);

    int id = kdk_url_download_to_local("https://marketplace.visualstudio.com/_apis/public/gallery/publishers/huizhou/vsextensions/githd/2.5.2/vspackage",
                                       "/home/kylin/huizhou.vsix",
                                       callback);
    g_main_loop_run(loop);
    return 0;
}

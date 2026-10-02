/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
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

#include "libkysearch.h"
#include "libkylog.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <magic.h>
#include <wand/magick_wand.h>
#include <sqlite3.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

const char *space_types[] = 
{
    "UndefinedColorspace",
    "RGBColorspace",
    "GRAYColorspace",
    "TransparentColorspace",
    "OHTAColorspace",
    "LabColorspace",
    "XYZColorspace",
    "YCbCrColorspace",
    "YCCColorspace",
    "YIQColorspace",
    "YPbPrColorspace",
    "YUVColorspace",
    "CMYKColorspace",
    "sRGBColorspace",
    "HSBColorspace",
    "HSLColorspace",
    "HWBColorspace",
    "Rec601LumaColorspace",
    "Rec601YCbCrColorspace",
    "Rec709LumaColorspace",
    "Rec709YCbCrColorspace",
    "LogColorspace",
    "CMYColorspace",
    "LuvColorspace",
    "HCLColorspace",
    "LCHColorspace",
    "LMSColorspace",
    "LCHabColorspace",
    "LCHuvColorspace",
    "scRGBColorspace",
    "HSIColorspace",
    "HSVColorspace",
    "HCLpColorspace",
    "YDbDrColorspace",
    "xyYColorspace",
    "LinearGRAYColorspace"
};

static int _check_filter(char *file, enum FileType file_type, enum FileSize file_size, enum FileTime file_time)
{
    if (file_type != FileType_ALL)
    {
        magic_t magic_cookie;

        // 初始化magic_cookie
        magic_cookie = magic_open(MAGIC_MIME_TYPE);
        if (magic_cookie == NULL)
        {
            klog_err("%s -> 无法初始化libmagic\n", __func__);
            return 0;
        }

        // 加载MIME数据库
        if (magic_load(magic_cookie, NULL) != 0)
        {
            klog_err("%s -> 无法加载MIME数据库: %s\n", __func__, magic_error(magic_cookie));
            magic_close(magic_cookie);
            return 0;
        }

        // 获取MIME类型
        const char *mime_type = magic_file(magic_cookie, file);
        if (mime_type == NULL)
        {
            klog_err("%s -> 无法获取MIME类型: %s\n", __func__, magic_error(magic_cookie));
            return 0;
        }

        int tmp = 0;
        if (strstr(mime_type, "inode"))
            tmp = FileType_FOLDER;
        else if (strstr(mime_type, "image"))
            tmp = FileType_IMAGE;
        else if (strstr(mime_type, "video"))
            tmp = FileType_VIDEO;
        else if (strstr(mime_type, "text"))
            tmp = FileType_TEXT;
        else if (strstr(mime_type, "audio"))
            tmp = FileType_AUDIO;
        else if (strstr(mime_type, "wps"))
            tmp = FileType_WPS;
        else
            tmp = FileType_OTHER;

        // 释放资源
        magic_close(magic_cookie);

        if (tmp != file_type)
            return 0;
    }

    struct stat file_stat;

    // 获取文件信息
    if (stat(file, &file_stat) != 0)
    {
        klog_err("%s -> Stat error\n", __func__);
        return 0;
    }

    if (file_size != FileSize_ALL)
    {
        long long size = file_stat.st_size;
        int tmp = FileSize_ALL;
        if (0 == size)
            tmp = FileSize_EMPTY;
        else if ((0 < size) && (size <= 16 * 1024))
            tmp = FileSize_MINIMUM;
        else if ((16 * 1024 < size) && (size <= 1024 * 1024))
            tmp = FileSize_SMALL;
        else if ((1024 * 1024 < size) && (size <= 128 * 1024 * 1024))
            tmp = FileSize_MEDIUM;
        else if ((128 * 1024 * 1024 < size) && (size <= 1024 * 1024 * 1024))
            tmp = FileSize_LARGE;
        else if ((1024 * 1024 * 1024 < size) && (size <= 4 * 1024 * 1024 * 1024))
            tmp = FileSize_HUGE;
        else
            tmp = FileSize_MAXIMUM;

        if (tmp != file_size)
            return 0;
    }

    if (file_size != FileSize_ALL)
    {
        time_t current_time = time(NULL);
        time_t creation_time = file_stat.st_ctime;
        double diff_time = difftime(current_time, creation_time);

        // 比较创建时间
        int tmp = FileTime_ALL;
        if (diff_time <= 86400)
            tmp = FileTime_TODAY;
        else if (diff_time <= 604800)
            tmp = FileTime_TWEEK;
        else if (diff_time <= 2592000)
            tmp = FileTime_TMONTH;
        else if (diff_time <= 31536000)
            tmp = FileTime_TYEAR;
        else
            tmp = FileTime_ONE_YEAR_AGO;

        if (tmp != file_time)
            return 0;
    }

    return 1;
}

static int _get_image_info(KdkImageInfo **info, char *file)
{
    MagickWand *wand;
    MagickBooleanType status;
    char *format;
    size_t width, height;
    size_t colorspace;

    // 初始化MagickWand
    MagickWandGenesis();

    // 创建MagickWand
    wand = NewMagickWand();

    // 读取图片文件
    status = MagickReadImage(wand, file);
    if (status == MagickFalse)
    {
        // klog_err("%s -> 无法打开图片文件\n", __func__);
        return 0;
    }

    KdkImageInfo *tmp = calloc(1, sizeof *tmp);
    tmp->next = *info;
    *info = tmp;
    tmp->path = strdup(file);

    // 获取图片的格式
    format = MagickGetImageFormat(wand);
    tmp->format = strdup(format);
    MagickRelinquishMemory(format);

    // 获取图片尺寸
    width = MagickGetImageWidth(wand);
    height = MagickGetImageHeight(wand);
    char size[32];
    memset(size, 0, sizeof(size));
    snprintf(size, sizeof(size), "%lux%lu", width, height);
    tmp->size = strdup(size);

    // 获取图片颜色模式
    colorspace = MagickGetImageColorspace(wand);
    tmp->color_mode = strdup(space_types[colorspace]);

    // 清理资源
    DestroyMagickWand(wand);
    MagickWandTerminus();

    return 1;
}

char **kdk_search_get_term_by_directory(char *directory, char *term)
{
    char cmd[2048];
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "locate %s/*%s*", directory, term);

    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return NULL;

    char **result = calloc(100, sizeof(char *));

    int i = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp))
    {
        int len = strnlen(line, STRNLEN_MAX);
        while ('/' != line[len - 1])
        {
            if ('\n' == line[len - 1])
                line[len - 1] = '\0';
            len--;
        }

        if (strstr(line + len, term))
        {
            result[i++] = strdup(line);
            if (i > 99)
                break;
        }
    }
    pclose(fp);

    return result;
}

char **kdk_search_full_get_term_by_directory(char *directory, char *term)
{
    char cmd[2048];
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "locate %s/*%s*", directory, term);

    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return NULL;

    char **result = calloc(100, sizeof(char *));

    int i = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp))
    {
        int len = strnlen(line, STRNLEN_MAX);
        while ('/' != line[len - 1])
        {
            if ('\n' == line[len - 1])
                line[len - 1] = '\0';
            len--;
        }

        if (0 == strcmp(line + len, term))
        {
            result[i++] = strdup(line);
            if (i > 99)
                break;
        }
    }
    pclose(fp);

    return result;
}

char **kdk_search_get_term_by_filter(char *directory, char *term, enum FileType file_type, enum FileSize file_size, enum FileTime file_time)
{
    char cmd[2048];
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "locate %s/*%s*", directory, term);

    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return NULL;

    char **result = calloc(100, sizeof(char *));

    int i = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp))
    {
        int len = strnlen(line, STRNLEN_MAX);
        while ('/' != line[len - 1])
        {
            if ('\n' == line[len - 1])
                line[len - 1] = '\0';
            len--;
        }
        if (strstr(line + len, term))
        {
            if (_check_filter(line, file_type, file_size, file_time))
            {
                result[i++] = strdup(line);
                if (i > 99)
                    break;
            }
        }
    }
    pclose(fp);

    return result;
}

KdkImageInfo *kdk_search_get_image_by_directory(char *directory, char *image)
{
    KdkImageInfo *result = NULL;

    char cmd[2048];
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "locate %s/*%s*", directory, image);

    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return NULL;

    int i = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp))
    {
        int len = strnlen(line, STRNLEN_MAX);
        while ('/' != line[len - 1])
        {
            if ('\n' == line[len - 1])
                line[len - 1] = '\0';
            len--;
        }

        if (strstr(line + len, image))
        {
            _get_image_info(&result, line);
        }
    }
    pclose(fp);

    return result;
}

KdkImageInfo *kdk_search_full_get_image_by_directory(char *directory, char *image)
{
    KdkImageInfo *result = NULL;

    char cmd[2048];
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "locate %s/*%s*", directory, image);

    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return NULL;

    int i = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp))
    {
        int len = strnlen(line, STRNLEN_MAX);
        while ('/' != line[len - 1])
        {
            if ('\n' == line[len - 1])
                line[len - 1] = '\0';
            len--;
        }

        if (0 == strcmp(line + len, image))
        {
            _get_image_info(&result, line);
        }
    }
    pclose(fp);

    return result;
}

void kdk_search_destory_image_info(KdkImageInfo *info)
{
    if (NULL == info)
        return;

    KdkImageInfo *tmp = NULL;
    while (info)
    {
        tmp = info;
        info = info->next;
        free(tmp->path);
        free(tmp->format);
        free(tmp->size);
        free(tmp->color_mode);
        free(tmp);
    }
}

void kdk_search_by_browser(char *key)
{
    char url[1024];
    memset(url, 0, sizeof(url));
    snprintf(url, sizeof(url), "https://www.baidu.com/s?wd=%s", key);

    pid_t pid = fork();
    if (pid == 0)
    {
        // 子进程中执行命令
        execl("/usr/bin/xdg-open", "xdg-open", url, NULL);
    }
    else if (pid > 0)
    {
        // 父进程退出
    }
    else
    {
        // 创建子进程失败
    }
}

void kdk_search_by_application(char *key)
{
    const char *exec_data = NULL;

    sqlite3 *db;
    char *errMsg = 0;
    sqlite3_stmt *stmt;
    char *sql[1024], db_file[1024];
    memset(sql, 0, sizeof(sql));

    char *home = getenv("HOME");
    snprintf(db_file, sizeof(db_file), "%s/.config/org.ukui/ukui-search/appdata/app-info.db", home);

    snprintf(sql, sizeof(sql), "SELECT EXEC FROM appInfo WHERE NAME_EN LIKE '%%%s%%' OR NAME_ZH LIKE '%%%s%%' OR PINYIN_NAME LIKE '%%%s%%' LIMIT 1;",
            key, key, key);

    // 打开数据库
    int rc = sqlite3_open(db_file, &db);
    if (rc)
    {
        klog_err("%s -> Can't open database: %s\n", __func__, sqlite3_errmsg(db));
        return;
    }

    // 准备查询
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        klog_err("%s -> Failed to prepare statement: %s\n", __func__, sqlite3_errmsg(db));
        return;
    }

    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        exec_data = strdup((const char *)sqlite3_column_text(stmt, 0));
    }
    // 释放资源
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (exec_data)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // 子进程中执行命令
            system(exec_data);
        }
        else if (pid > 0)
        {
            // 父进程退出
        }
        else
        {
            // 创建子进程失败
        }
        free(exec_data);
    }
}

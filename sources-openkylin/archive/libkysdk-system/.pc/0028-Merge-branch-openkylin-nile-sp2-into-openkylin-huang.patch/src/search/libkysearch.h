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

#ifndef KYSDK_SYSTEM_SEARCH_H_
#define KYSDK_SYSTEM_SEARCH_H_

/**
 * @file libkysearch.h
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief 搜索模块
 * @version 2.4
 * @date 2024-4-30
 *
 * @copyright Copyright (c) 2024
 * @defgroup libkysdk-search
 * @{
 * @since 2.5.1.0
 */

enum FileType
{
    FileType_ALL,    // 全部
    FileType_FOLDER, // 文件夹
    FileType_IMAGE,  // 图片
    FileType_VIDEO,  // 视频
    FileType_TEXT,   // 文本文档
    FileType_AUDIO,  // 音频
    FileType_WPS,    // wps文件
    FileType_OTHER   // 其它
};

enum FileSize
{
    FileSize_ALL,     // 全部
    FileSize_EMPTY,   // 空（0K）
    FileSize_MINIMUM, // 极小（0-16K）
    FileSize_SMALL,   // 很小（16K-1M）
    FileSize_MEDIUM,  // 中等（1M-128M）
    FileSize_LARGE,   // 大（128M-1G）
    FileSize_HUGE,    // 巨大（1G-4G）
    FileSize_MAXIMUM, // 极大（>4G）
};

enum FileTime
{
    FileTime_ALL,          // 全部
    FileTime_TODAY,        // 今天
    FileTime_TWEEK,        // 本周
    FileTime_TMONTH,       // 本月
    FileTime_TYEAR,        // 今年
    FileTime_ONE_YEAR_AGO, // 一年前
};

typedef struct _KdkImageInfo
{
    char *path;       // 路径
    char *format;     // 图片格式
    char *size;       // 图片尺寸
    char *color_mode; // 颜色模式
    struct _KdkImageInfo *next;
} KdkImageInfo;

/**
 * @brief 获取指定目录下的文件和文件夹路径(模糊查找)
 * 
 * @param directory 指定目录
 * @param term 要搜索的关键字
 * @return char** 符合关键字的路径，默认只输入100条
 */
char **kdk_search_get_term_by_directory(char *directory, char *term);

/**
 * @brief 获取指定目录下的文件和文件夹路径(精准查找)
 * 
 * @param directory 指定目录
 * @param term 要搜索的关键字
 * @return char** 符合关键字的路径，默认只输入100条
 */
char **kdk_search_full_get_term_by_directory(char *directory, char *term);

/**
 * @brief 有过滤条件的搜索路径
 * 
 * @param directory 指定目录
 * @param term 要搜索的关键字
 * @param file_type 文件类型枚举
 * @param file_size 文件大小枚举
 * @param file_time 文件修改时间枚举
 * @return char** 符合条件的路径
 */
char **kdk_search_get_term_by_filter(char *directory, char *term, enum FileType file_type, enum FileSize file_size, enum FileTime file_time);

/**
 * @brief 获取指定路径下的图片信息(模糊查找)
 * 
 * @param directory 指定目录
 * @param image 要搜索的图片
 * @return KdkImageInfo* 图片信息链表
 */
KdkImageInfo *kdk_search_get_image_by_directory(char *directory, char *image);

/**
 * @brief 获取指定路径下的图片信息(精准查找)
 * 
 * @param directory指定目录
 * @param image 要搜索的图片
 * @return KdkImageInfo* 图片信息链表
 */
KdkImageInfo *kdk_search_full_get_image_by_directory(char *directory, char *image);

/**
 * @brief 释放图片信息链表
 * 
 * @param info 图片信息链表指针
 */
void kdk_search_destory_image_info(KdkImageInfo *info);

/**
 * @brief 打开默认浏览器显示关键字联网搜索结果(默认百度)
 * 
 * @param key 搜索关键字
 */
void kdk_search_by_browser(char *key);

/**
 * @brief 搜索应用程序并启动
 * 
 * @param key 应用程序名称
 */
void kdk_search_by_application(char *key);

#endif // KYSDK_SYSTEM_SEARCH_H_
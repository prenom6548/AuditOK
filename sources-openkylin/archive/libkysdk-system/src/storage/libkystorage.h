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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef KYSDK_SYSTEM_SOTRAGE_H_
#define KYSDK_SYSTEM_SOTRAGE_H_

/**
 * @file libkystorage.h
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief 存储模块
 * @version 2.4
 * @date 2024-4-30
 *
 * @copyright Copyright (c) 2024
 * @defgroup libkysdk-storage
 * @{
 * @since 2.5.1.0
 */

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

typedef struct _FileInfo
{
    char name[256];
    int is_dir;
    struct _FileInfo *next;
} FileInfo;

typedef enum _kdk_compress_algorithm
{
    COMPRESS_ALGORITHM_ZIP,   // zip压缩
    COMPRESS_ALGORITHM_GZIP,  // gzip压缩
    COMPRESS_ALGORITHM_BZIP2, // bzip2压缩
    COMPRESS_ALGORITHM_X,     // xz压缩
    COMPRESS_ALGORITHM_LZ4,   // lz4压缩
    COMPRESS_ALGORITHM_ZSTD,  // zstd压缩
    COMPRESS_ALGORITHM_7Z     // 7z压缩
} kdk_compress_algorithm;

/**
 * @brief 获取默认浏览器 Cookie 的路径，目前仅支持qax，chrome，chromium和firefox
 *
 * @return char* 浏览器 Cookie 的路径
 */
extern char *kdk_storage_get_default_cookie_path();

/**
 * @brief 获取当前用户桌面文件夹的路径
 *
 * @return char* 桌面文件夹的路径
 */
extern char *kdk_storage_get_default_desktop_path();

/**
 * @brief 获取当前用户文档文件夹的路径
 *
 * @return char* 文档文件夹的路径
 */
extern char *kdk_storage_get_default_documents_path();

/**
 * @brief 获取当前用户指定浏览器的Internet 缓存文件夹的路径，目前仅支持qax，chrome，chromium和firefox
 *
 * @param name 浏览器名称，入参为null，获取默认浏览器
 * @return char* Internet 缓存文件夹的路径
 */
extern char *kdk_storage_get_default_internetCache_path(char *name);

/**
 * @brief 获取当前用户下载文件夹的路径
 *
 * @return char* 下载文件夹的路径
 */
extern char *kdk_storage_get_default_download_path();

/**
 * @brief 获取文件内容的 MIME 类型
 *
 * @param filename 文件路径
 * @return char* 文件内容的 MIME 类型
 */
extern char *kdk_storage_get_file_ContentType(char *filename);

/**
 * @brief 获取文件的最近改动的日期和时间（是指最近修改INode时间）
 *
 * @param filename 文件路径
 * @return char* 创建当前文件的日期和时间
 */
extern char *kdk_storage_get_file_dateChange(char *filename);

/**
 * @brief 获取文件最近更改的日期和时间（是指最近修改文件内容的时间）
 *
 * @param filename 文件路径
 * @return char* 最近更改的日期和时间
 */
extern char *kdk_storage_get_file_dateModify(char *filename);

/**
 * @brief 获取文件最近访问的日期和时间
 *
 * @param filename 文件路径
 * @return char* 最近访问的日期和时间
 */
extern char *kdk_storage_get_file_dateAccess(char *filename);

/**
 * @brief 获取文件大小
 *
 * @param filename 文件路径
 * @return long 文件大小
 */
extern long kdk_storage_get_file_size(char *filename);

/**
 * @brief 读取文件内容
 *
 * @param filename 要读取的文件的路径和名称
 * @param buffer 缓冲区
 * @return int 如果成功读取文件，则返回读取的字节数；如果发生错误，则返回 -1
 */
extern int kdk_storage_read_file(const char *filename, char *buffer, unsigned long size);

/**
 * @brief 将数据追加到指定的文件中
 *
 * @param filename 要追加的文件的路径和名称
 * @param data 要追加到文件中的数据
 * @param len 数据长度
 * @return int 如果成功写入文件，则返回 0；如果发生错误，则返回 -1
 */
extern int kdk_storage_write_file(const char *filename, const char *data, unsigned long length);

/**
 * @brief 移动文件（仅支持桌面、下载、文档、音乐或视频目录）
 *
 * @param source 要移动的源文件的路径和名称
 * @param destination 目标文件的路径和名称
 * @return int 如果成功移动文件，则返回 0；如果发生错误，则返回 -1
 */
extern int kdk_storage_move_file(const char *source, const char *destination);

/**
 * @brief 获取文件权限
 *
 * @param filename 要获取权限的文件的路径和名称
 * @return char* 文件权限
 */
extern char *kdk_storage_get_file_permissions(const char *filename);

/**
 * @brief 获取文件所有者
 *
 * @param filename 要获取所有者的文件的路径和名称
 * @return char* 文件所有者
 */
extern char *kdk_storage_get_file_owner(const char *filename);

/**
 * @brief 获取文件所属组
 *
 * @param filename 要获取所属组的文件的路径和名称
 * @return char* 文件所属组
 */
extern char *kdk_storage_get_file_group(const char *filename);

/**
 * @brief 列出指定路径下的所有文件和文件夹
 *
 * @param path 要列出的路径
 * @return  FileInfo 结构体
 */
extern FileInfo *kdk_storage_list_files(const char *path);

/**
 * @brief 释放FileInfo链表
 *
 * @param info FileInfo链表
 */
extern void kdk_storage_free_file_info(FileInfo *info);

/**
 * @brief 解析指定文件的符号链接（软链接），获取实际文件路径
 *
 * @param filename 指定的文件名
 * @return char* 实际文件路径
 */
extern char *kdk_storage_resolve_symbolic_link(const char *filename);

/**
 * @brief 压缩数据
 *
 * @param algorithm 压缩算法
 * @param iData 待压缩数据
 * @param iSize 待压缩数据大小
 * @param oData 压缩后数据
 * @param oSize 压缩后数据大小
 * @return int 成功返回0，失败返回-1, 不支持返回-2
 * @since 3.0.1.0
 */
extern int kdk_storage_data_compress(IN kdk_compress_algorithm algorithm, IN char *iData, IN int iSize, OUT char *oData, IN OUT int *oSize);

/**
 * @brief 解压数据
 *
 * @param algorithm 解压算法
 * @param iData 待解压数据
 * @param iSize 待解压数据大小
 * @param oData 解压后数据
 * @param oSize 解压后数据大小
 * @return int 成功返回0，失败返回-1, 不支持返回-2
 * @since 3.0.1.0
 */
extern int kdk_storage_data_decompress(IN kdk_compress_algorithm algorithm, IN char *iData, IN int iSize, OUT char *oData, IN OUT int *oSize);

/**
 * @brief 压缩文件(仅文件)
 *
 * @param algorithm 压缩算法
 * @param source_file 待压缩文件
 * @param dest_file 压缩文件
 * @return int 成功返回0，失败返回-1
 * @since 3.0.1.0
 */
extern int kdk_storage_file_compress(kdk_compress_algorithm algorithm, char *source_file, char *dest_file);

/**
 * @brief 解压文件(仅文件)
 *
 * @param algorithm 解压算法
 * @param source_file 待解压文件
 * @param dest_path 解压目录
 * @return int 成功返回0，失败返回-1
 * @since 3.0.1.0
 */
extern int kdk_storage_file_decompress(kdk_compress_algorithm algorithm, char *source_file, char *dest_path);

#endif // KYSDK_SYSTEM_SOTRAGE_H_

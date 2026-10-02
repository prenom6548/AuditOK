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
 * Authors: JingWen Wang <wangjingwen@kylinos.cn>
 *
 */

#ifndef _LIBKYMEDIA_H_
#define _LIBKYMEDIA_H_

/**
 * @file libkymedia.h
 * @author wangjingwen (wangjingwen@kylinos.cn)
 * @brief 音频设备管理模块
 * @version 1.0
 * @date 2025-4-2
 * 
 * @copyright Copyright (c) 2025
 * @defgroup libkysdk-media
 * @{
 * @since 3.0.1.0
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* 设备信息结构体 */
typedef struct {
    char name[256];           /* 设备名称 */
    char id[64];              /* 设备ID */
    char longname[256];       /* 设备长名称 */
    int subdevice;           /* 子设备编号 */
    int subdevices_count;    /* 子设备总数 */
    char subdevice_name[64]; /* 子设备名称 */
} kdk_device_info_t;

/* 声卡信息结构体 */
typedef struct {
    int card;                /* 声卡编号 */
    char name[256];          /* 声卡名称 */
    char longname[256];      /* 声卡长名称 */
    kdk_device_info_t* devices; /* 设备列表 */
    int device_count;        /* 设备数量 */
} kdk_sound_card_t;

/* 设备列表结构体 */
typedef struct {
    kdk_device_info_t* devices; /* 设备列表 */
    int count;                  /* 设备数量 */
} kdk_device_list_t;

/**
 * @brief 获取默认输入设备
 * @return 默认输入设备信息，如果失败返回NULL
 */
kdk_device_info_t* kdk_media_get_default_input_device(void);

/**
 * @brief 获取默认输出设备
 * @return 默认输出设备信息，如果失败返回NULL
 */
kdk_device_info_t* kdk_media_get_default_output_device(void);

/**
 * @brief 获取所有输入设备列表
 * @return 输入设备列表，如果失败返回NULL
 */
kdk_device_list_t* kdk_media_get_all_input_device(void);

/**
 * @brief 获取所有输出设备列表
 * @return 输出设备列表，如果失败返回NULL
 */
kdk_device_list_t* kdk_media_get_all_output_device(void);

/**
 * @brief 设置默认输入设备（不启用）
 * @param devicename 要设置的设备名称
 * @return 成功返回0，失败返回-1
 */
int kdk_media_set_default_input_device(const char* devicename);

/**
 * @brief 设置默认输出设备（不启用）
 * @param devicename 要设置的设备名称
 * @return 成功返回0，失败返回-1
 */
int kdk_media_set_default_output_device(const char* devicename);

/**
 * @brief 获取所有声卡信息
 * @return 声卡列表，如果失败返回NULL
 */
kdk_sound_card_t* kdk_media_get_all_sound_card(void);

/**
 * @brief 释放设备信息内存
 * @param device 要释放的设备信息
 */
void kdk_media_free_device_info(kdk_device_info_t* device);

/**
 * @brief 释放设备列表内存
 * @param list 要释放的设备列表
 */
void kdk_media_free_device_list(kdk_device_list_t* list);

/**
 * @brief 释放声卡信息内存
 * @param card 要释放的声卡信息
 */
void kdk_media_free_sound_card(kdk_sound_card_t* card);

#ifdef __cplusplus
}
#endif

#endif /* _LIBKYMEDIA_H_ */

/**
 * \example kysdk-system/src/media/test/kymedia-test.c
 * 
 */

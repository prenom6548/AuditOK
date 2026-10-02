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

#ifndef LIBKYJIMAGEPROC_H
#define LIBKYJIMAGEPROC_H

/**
 * @file libkyimageproc.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief jpeg编码模块
 * @version 0.1
 * @date 2023-9-28
 * 
 * @copyright Copyright (c) 2023
 * @defgroup libkysdk-imageproc
 * @{
 * @since 2.3.0.0
 */

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _AlphaType
{
    UNKNOWN,                    //未知透明度
    OPAQUE,                     //没有alpha或图片全透明
    PREMUL,                     //RGB前乘alpha
    UNPREMUL                    //RGB不前乘alpha
}AlphaType;

typedef enum _PixelMapFormat
{
    UNKNOWN_PIXEL,                    //未知格式
    RGB_565 = 2,                //格式为RGB_565
    RGBA_8888,                  //格式为RGBA_8888
    RGB_888 = 5,                //格式为RGB_888
    ALPHA_8,                    //格式为ALPHA_8
    RGBA_F169,                  //格式为RGBA_F16
    NV21                        //格式为NV21
}PixelMapFormat;

struct Size
{
    int height;                 //输出图片的高
    int width;                  //输出图片的宽
};

typedef struct _kdk_image_init
{
    AlphaType alphaType;                        //透明度
    PixelMapFormat pixelFormat;                 //像素格式
    struct Size size;                          //创建图片大小
}kdk_image_init;

typedef struct _kdk_image_info
{
    int xdensity;                                //像素密度，单位为ppi
    int ydensity;                                //像素密度，单位为ppi
    struct Size size;                          //创建图片大小
}kdk_image_info;

/**
 * @brief 初始化jpeg编码器
 * 
 * @return void* 返回编码器句柄
 */
extern void* kdk_jpeg_encoder_init();

/**
 * @brief jpeg图编码接口
 * 
 * @param handle 编码器句柄
 * @param srcFile 源文件 
 * @param destFile 输出文件 
 * @return int -1 则编码失败，0，编码成功
 */
extern int kdk_jpeg_encode_to_file(void *handle, char *srcFile, char *destFile);

/**
 * @brief 释放编码器内存
 * 
 * @param handle 编码器句柄
 */
extern void kdk_jpeg_encoder_release(void* handle);

/**
 * @brief 创建图片
 * 
 * @param init 创建图片的属性，包括透明度，尺寸，缩略值，像素格式
 * 
 * @return void* 图片句柄，失败句柄为空
 * @since 2.5.1.2
 */
extern void* kdk_image_create_pixelmap(kdk_image_init init);

/**
 * @brief 加载图片
 * 
 * @param srcpath 图片路径
 * 
 * @return void* 图片句柄，失败句柄为空
 * @since 2.5.1.2
 */
extern void* kdk_image_load_pixelmap(char * srcpath);

/**
 * @brief 获取图片的像素属性
 * 
 * @param handle 图片句柄
 * 
 * @return kdk_image_info* 图片的像素属性，比如宽高
 * @since 2.5.1.2
 */
extern kdk_image_info *kdk_image_get_image_info(void *handle);

/**
 * @brief 获取图片的像素密度【未实现】
 * 
 * @param path 图片路径
 * 
 * @return kdk_image_info* 图片的像素属性，比如像素密度单位为ppi
 * @since 2.5.1.2
 */
extern kdk_image_info *kdk_image_get_image_density(char *path);

/**
 * @brief 获取图像像素每行字节数
 * 
 * @param handle 图片句柄
 *  
 * @return int 图像像素每行字节数
 * @since 2.5.1.2
 */
extern int kdk_image_get_bytes_number_per_row(void *handle);

/**
 * @brief 获取图像像素的总字节数
 * 
 * @param handle 图片句柄
 *  
 * @return int 图像像素的总字节数
 * @since 2.5.1.2
 */
extern int kdk_image_get_bytes_number_total(void *handle);

/**
 * @brief 设置图片的透明度
 * 
 * @param handle 图片句柄
 * @param rate 透明比率的值，取值范围：0-1
 * @since 2.5.1.2
 */
extern void kdk_image_set_opacity(void *handle, float rate);

/**
 * @brief 设置图片的缩放
 * 
 * @param handle 图片句柄
 * @param xrate 宽度的缩放倍数
 * @param yrate 高度的缩放倍数
 * @since 2.5.1.2
 */
extern void kdk_image_set_scale(void *handle, int xrate, int yrate);

/**
 * @brief 设置图片的旋转角度
 * 
 * @param handle 图片句柄
 * @param angle 图片的旋转角度
 * @since 2.5.1.2
 */
extern void kdk_image_set_rotate(void *handle, int angle);

/**
 * @brief 设置图片的翻转
 * 
 * @param handle 图片句柄
 * @param horizontal 水平翻转
 * @param vertical 垂直翻转
 * @since 2.5.1.2
 */
extern void kdk_image_set_flip(void *handle, bool horizontal, bool vertical);

/**
 * @brief 剪裁图片
 * 
 * @param handle 图片句柄
 * @param x 剪裁的起始x坐标
 * @param y 剪裁的起始y坐标
 * @param height 剪裁后保留的长
 * @param width 剪裁后保留的宽
 * @since 2.5.1.2
 */
extern void kdk_image_set_crop(void *handle, int x, int y, int height, int width);

/**
 * @brief 保存图片
 * 
 * @param handle 图片句柄
 * @param dstpath 保存位置
 * 
 * @return int 是否保存成功；-1未成功，0成功
 * @since 2.5.1.2
 */
extern int kdk_image_save(void *handle, char *dstpath);

/**
 * @brief 释放由kdk_image_get_image_info返回的图片像素属性
 * 
 * @param disk 由kdk_image_get_image_info返回的结构体指针
 * @since 2.5.1.2
 */
extern void kdk_free_image_info(kdk_image_info *info);


#ifdef __cplusplus
}
#endif

#endif // LIBKYJIMAGEPROC_H

/*
 * libkysdk-thememanager's Library
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

#ifndef WATERMARK_H
#define WATERMARK_H

#include <QString>
#include <QVector>
#include <QQueue>

namespace kdk
{

/**
 * @defgroup WatermarkModule
 */

/**
 * @brief 水印控制类 提供屏幕水印功能
 * @since 3.0.1.0
 */

    struct WatermarkData
    {
        WatermarkData();
        QString handle;     // V11上创建水印返回的字符串
        QString path;       // 水印路径
        double opticity;    // 不透明度
        int x;              // x坐标
        int y;              // y坐标
        int expand;         // 图片模式 0正常，1重复，2全屏，3按比例全屏
        bool topmost;       // 显示层级 0水印层， 1置顶
    };

    class KWatermark
    {
    public:
        KWatermark();
        ~KWatermark();

    public:
        /**
         * @brief Create a Watermark object
         * 
         * @param path png图片路劲
         * @param opticity 不透明度0-1
         * @param x x坐标
         * @param y y坐标
         * @param expand 图片模式 0正常，1重复，2全屏，3按比例全屏
         * @param topmost 显示层级 0水印层， 1置顶
         * @return WatermarkData 水印句柄
         */
        WatermarkData createWatermark(QString path, double opticity, int x, int y, int expand, bool topmost);

        /**
         * @brief 修改水印参数
         * 
         * @param handle 水印句柄
         * @param path png图片路劲
         * @param opticity 不透明度0-1
         * @param x x坐标
         * @param y y坐标
         * @param expand 图片模式 0正常，1重复，2全屏，3按比例全屏
         * @param topmost 显示层级 0水印层， 1置顶
         * @return true 修改成功
         * @return false 修改失败
         */
        bool updateWatermark(WatermarkData &handle, QString path, double opticity, int x, int y, int expand, bool topmost);

        /**
         * @brief 取消水印
         * 
         * @param handle 水印句柄
         */
        void destroyWatermark(WatermarkData &handle);

    private:
        WatermarkData createWatermarkSP1(QString &path, double opticity, int x, int y, int expand, bool topmost);
        WatermarkData createWatermarkV11(QString &path, double opticity, int x, int y, int expand, bool topmost);
        bool updateWatermarkSP1(WatermarkData &handle, QString &path, double opticity, int x, int y, int expand, bool topmost);
        bool updateWatermarkV11(WatermarkData &handle, QString &path, double opticity, int x, int y, int expand, bool topmost);
        void destroyWatermarkSP1(WatermarkData &handle);
        void destroyWatermarkV11(WatermarkData &handle);
    };
}

#endif // WATERMARK_H

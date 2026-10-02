/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "imageoperationbeauty.h"
#include "../include/common.h"
#include <QDebug>


ImageOperationBeauty::ImageOperationBeauty(QImage *origin, QStack<QImage> *stackImage)
{
    m_origin = origin;
    m_stackImage = stackImage;
    isInterupt = false;
}

void ImageOperationBeauty::ImageOP(){
    QImage res = imageCopy(m_origin, m_stackImage);
    beauty(res);
    emit finished();
}

void ImageOperationBeauty::beauty(const QImage inputImage)
{
    // 创建输出图像
    QImage outputImage = inputImage;

    int width = inputImage.width();
    int height = inputImage.height();

    // 计算图像的平均亮度
    int sum = 0;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            QRgb pixel = inputImage.pixel(x, y);
            sum += qGray(pixel);
        }
    }
    int averageBrightness = sum / (width * height);

    // 提高图像的对比度和锐度，并根据平均亮度调整亮度
    for (int y = 1; y < height - 1; ++y)
    {
        for (int x = 1; x < width - 1; ++x)
        {
            // 获取中心像素值
            QRgb centerPixel = inputImage.pixel(x, y);
            int centerRed = qRed(centerPixel);
            int centerGreen = qGreen(centerPixel);
            int centerBlue = qBlue(centerPixel);

            // 计算邻近像素的平均颜色值
            sum = 0;
            for (int j = -1; j <= 1; ++j)
            {
                for (int i = -1; i <= 1; ++i)
                {
                    QRgb pixel = inputImage.pixel(x + i, y + j);
                    sum += qGray(pixel);
                }
            }
            int averageGray = sum / 9;

            // 计算高反差保留滤波后的颜色值
            int newRed = qBound(0, centerRed + (centerRed - averageGray), 255);
            int newGreen = qBound(0, centerGreen + (centerGreen - averageGray), 255);
            int newBlue = qBound(0, centerBlue + (centerBlue - averageGray), 255);

            // 根据平均亮度调整亮度
            int brightness = qBound(0, (newRed + newGreen + newBlue) / 3 + ((newRed + newGreen + newBlue) / 3 - averageBrightness) * 2, 255);

            // 猜想补全模糊的字体内容
            if (qGray(centerPixel) < averageGray)
            {
                // 补全模糊的字体内容
                brightness = qBound(0, (centerRed + centerGreen + centerBlue) / 3 + qAbs((centerRed + centerGreen + centerBlue) / 3 - averageGray) * 2, 255);
            }

            // USM锐化
            float usmAmount = 0.8; // USM锐化的强度，可以调整此值
            int usmRed = centerRed + static_cast<int>((centerRed - newRed) * usmAmount);
            int usmGreen = centerGreen + static_cast<int>((centerGreen - newGreen) * usmAmount);
            int usmBlue = centerBlue + static_cast<int>((centerBlue - newBlue) * usmAmount);

            // 亮度调整
            float brightnessAdjustment = 1.01; // 调整此值以改变亮度调整的强度
            int adjustedBrightness = qBound(0, static_cast<int>(brightness * brightnessAdjustment), 255);

            // 减小局部颜色变化
            float colorSmoothing = 0.01; // 调整此值以改变减小颜色变化的强度
            int finalRed = qBound(0, static_cast<int>(usmRed + (adjustedBrightness - usmRed) * colorSmoothing), 255);
            int finalGreen = qBound(0, static_cast<int>(usmGreen + (adjustedBrightness - usmGreen) * colorSmoothing), 255);
            int finalBlue = qBound(0, static_cast<int>(usmBlue + (adjustedBrightness - usmBlue) * colorSmoothing), 255);

            // 更新像素值
            outputImage.setPixel(x, y, qRgb(finalRed, finalGreen, finalBlue));
        }
    }

    outputImage.save(ScanningPicture);
}


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

#include "rectify.h"

#include <QDebug>

int ImageRectify(QImage *src)
{

    QTransform transform;
    double angle = caculateSkewAngle(src);
    qDebug() << "角度：" << angle;
    transform.rotate(angle);
    QImage correctedImage = src->transformed(transform).scaled(src->size(),Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    correctedImage.save(ScanningPicture);

    return 0;
}

double caculateSkewAngle(QImage *src)
{
    // 将图像转换为灰度图
    QImage grayImage = src->convertToFormat(QImage::Format_Grayscale8);

    // 计算水平和垂直梯度
    QVector<double> horizontalGradient;
    QVector<double> verticalGradient;

    int width = grayImage.width();
    int height = grayImage.height();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            QRgb pixel = grayImage.pixel(x, y);

            if (x == 0 || x == width - 1 || y == 0 || y == height - 1)
            {
                horizontalGradient.append(0);
                verticalGradient.append(0);
            }
            else
            {
                int leftPixel = qGray(grayImage.pixel(x - 1, y));
                int rightPixel = qGray(grayImage.pixel(x + 1, y));
                int topPixel = qGray(grayImage.pixel(x, y - 1));
                int bottomPixel = qGray(grayImage.pixel(x, y + 1));

                double gx = (rightPixel - leftPixel) / 2.0;
                double gy = (bottomPixel - topPixel) / 2.0;

                horizontalGradient.append(gx);
                verticalGradient.append(gy);
            }
        }
    }

    // 计算累积角度
    QVector<double> angles;
    for (int i = 0; i < horizontalGradient.size(); ++i)
    {
        double degrees = qAtan2(verticalGradient[i], horizontalGradient[i]);
        if(verticalGradient[i]==0 && horizontalGradient[i]==0){
            degrees = 0;
        }
        double angle = qRadiansToDegrees(degrees);
        angles.append(angle);
    }

    // 求解主方向
    int binSize = 300;  // 直方图中每个bin的大小，可以根据实际情况调整
    int numBins = 360 / binSize;
    QVector<int> histogram(numBins, 0);  // 初始化直方图为0

    for (double angle : angles)
    {
        int bin = qRound(angle / binSize);
        if (bin >= 0 && bin < histogram.size()) {
            histogram[bin]++;
        }
    }

    // 找到最大的直方图bin
    int maxBinIndex = 0;
    int maxBinValue = 0;

    for (int i = 0; i < numBins; ++i)
    {
        if (histogram[i] > maxBinValue)
        {
            maxBinValue = histogram[i];
            maxBinIndex = i;
        }
    }

    // 计算平均角度
    double totalAngle = 0;
    int count = 0;

    for (double angle : angles)
    {
        int bin = qRound(angle / binSize);

        if (bin == maxBinIndex)
        {
            totalAngle += angle;
            count++;
        }
    }

    double averageAngle = totalAngle / count;

    // 返回平均角度作为纠偏角度
    return averageAngle;
}

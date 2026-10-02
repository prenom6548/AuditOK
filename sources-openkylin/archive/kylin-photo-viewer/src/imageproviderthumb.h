/* Copyright (C) 2020, KylinSoft Co., Ltd.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef PQASYNCIMAGEPROVIDERTHUMB_H
#define PQASYNCIMAGEPROVIDERTHUMB_H

#include <QQuickAsyncImageProvider>
#include <QThreadPool>
#include <QPixmapCache>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QDebug>
#include <QIcon>
#include <QDate>
#include "./view/showimagewidget.h"
#include "./view/sizedate.h"
#include "global/variable.h"

class ScreenImageProvider : public QQuickImageProvider
{
public:
    ScreenImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize)
    //这个是自动调用的。路径"image://screen"，如需要使用id等参数，如"image://screen/id"等
    {
        qDebug()<<"请求图片 id="<<id <<" requestedSize="<<requestedSize << Variable::g_devicePixelRatio;
        if (tempImage.isNull()) {
            m_image.load("qrc:/res/res/damaged_img.png");
        } else if (Variable::g_isShowOrigImage && !origTempImage.isNull()) {
            m_image = origTempImage;
        } else {
            m_image = tempImage;
        }
        return m_image;
    }
    QImage m_image;
};

#endif // PQASYNCIMAGEPROVIDERTHUMB_H

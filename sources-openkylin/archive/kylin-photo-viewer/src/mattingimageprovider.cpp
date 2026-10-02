/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
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
#include "mattingimageprovider.h"

MattingImageProvider::MattingImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage MattingImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    if (m_image.isNull()) {
        m_image.load("qrc:/res/res/damaged_img.png");
    }

    return m_image;
}

void MattingImageProvider::setImage(QImage &img)
{
    m_image = img;
}

ShowMattingImage::ShowMattingImage(QObject *parent)
    : QObject(parent)
{
    m_pImgProvider = new MattingImageProvider();
}

void ShowMattingImage::setImage(QImage &img)
{
    getImgProvider()->setImage(img);
    Q_EMIT callQmlRefeshImg();
}

MattingImageProvider* ShowMattingImage::getImgProvider()
{
    return m_pImgProvider;
}

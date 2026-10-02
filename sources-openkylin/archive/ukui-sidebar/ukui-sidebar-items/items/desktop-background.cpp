/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

//
// Created by hxf on 22-9-23.
//

#include "desktop-background.h"
#include "pixmap-provider.h"

#include <QPainter>
#include <QtMath>

DesktopBackground::DesktopBackground(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    connect(PixmapProvider::instance(), &PixmapProvider::pixmapChanged,
            this, &DesktopBackground::updateBackground);

    connect(this, &DesktopBackground::widthChanged, this, &DesktopBackground::refresh);
    connect(this, &DesktopBackground::heightChanged, this, &DesktopBackground::refresh);
}

void DesktopBackground::paint(QPainter *painter)
{
    QRect rect {0, 0, static_cast<int>(size().width()), static_cast<int>(size().height())};
    painter->save();
    // 抗锯齿
    painter->setRenderHint(QPainter::Antialiasing);
    // 平滑过渡
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    painter->drawPixmap(rect, m_backgroundPixmap);
    painter->restore();
}

void DesktopBackground::updateBackground(const BackgroundType::Type &type, QPixmap pixmap)
{
    if (type == m_backgroundType ) {
        if (m_backgroundType == BackgroundType::Null) {
            qWarning() << "DesktopBackground is not init." << m_backgroundType;
            return;
        }
        rebuildBackgroundImage(pixmap.toImage());
        update();
    }
}
void DesktopBackground::rebuildBackgroundImage(const QImage &rawImage)
{
    QImage backgroundImage({static_cast<int>(size().width()), static_cast<int>(size().height())}, QImage::Format_ARGB32_Premultiplied);
    backgroundImage.fill({DEFAULT_COLOR});

    QString pictureOptions = (m_backgroundType == BackgroundType::DesktopPic) ? PixmapProvider::instance()->pictureOption() : "";

    if (pictureOptions.isEmpty()) {
        pictureOptions = "scaled";
    }

    QPainter painter(&backgroundImage);

    if (pictureOptions == "scaled") {
        painter.drawImage(backgroundImage.rect(), rawImage,
                          getSourceRect(backgroundImage.rect(), rawImage));

    } else if (pictureOptions == "centered") {
        painter.drawImage((backgroundImage.width() - rawImage.width()) / 2,
                          (backgroundImage.height() - rawImage.height()) / 2,
                          rawImage);

    } else if (pictureOptions == "stretched") {
        painter.drawImage(backgroundImage.rect(), rawImage, rawImage.rect());

    } else if (pictureOptions == "wallpaper") {
        int drewWidth;
        int drewHeight = 0;
        while (true) {
            drewWidth = 0;
            while (true) {
                painter.drawImage(drewWidth, drewHeight, rawImage);
                drewWidth += rawImage.width();
                if (drewWidth >= backgroundImage.width()) {
                    break;
                }
            }
            drewHeight += rawImage.height();
            if (drewHeight >= backgroundImage.height()) {
                break;
            }
        }
    } else {
        painter.drawImage(backgroundImage.rect(), rawImage,
                          getSourceRect(backgroundImage.rect(), rawImage));
    }

    QPixmap blurPixmap = QPixmap::fromImage(backgroundImage);
    m_backgroundPixmap.swap(blurPixmap);
}

QRect DesktopBackground::getSourceRect(const QRect &targetRect, const QImage &image)
{
    qreal screenScale = qreal(targetRect.width()) / qreal(targetRect.height());
    qreal width = image.width();
    qreal height = image.height();

    if ((width / height) == screenScale) {
        return image.rect();
    }

    bool isShortX = (width <= height);
    if (isShortX) {
        screenScale = qreal(targetRect.height()) / qreal(targetRect.width());
    }

    //使用长边与短边目的是屏蔽单独的宽与高概念，减少一部分判断逻辑
    qreal shortEdge = isShortX ? width : height;
    qreal longEdge = isShortX ? height : width;

    while (shortEdge > 1) {
        qint32 temp = qFloor(shortEdge * screenScale);
        if (temp <= longEdge) {
            longEdge = temp;
            break;
        }

        //每次递减5%进行逼近
        qint32 spacing = qRound(shortEdge / 20);
        if (spacing <= 0) {
            spacing = 1;
        }
        shortEdge -= spacing;
    }

    QSize sourceSize = image.size();
    if (shortEdge > 1 && longEdge > 1) {
        sourceSize.setWidth(static_cast<int>(isShortX ? shortEdge : longEdge));
        sourceSize.setHeight(static_cast<int>(isShortX ? longEdge : shortEdge));
    }

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (image.width() > sourceSize.width()) {
        offsetX = (image.width() - sourceSize.width()) / 2;
    }

    if (image.height() > sourceSize.height()) {
        offsetY = (image.height() - sourceSize.height()) / 2;
    }

    QPoint offsetPoint = image.rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return {offsetPoint, sourceSize};
}

void DesktopBackground::refresh()
{
    if (m_backgroundType == BackgroundType::Null) {
        qWarning() << "DesktopBackground is not init." << m_backgroundType;
        return;
    }
    rebuildBackgroundImage(PixmapProvider::instance()->getPixmap(m_backgroundType).toImage());
    update();
}

bool DesktopBackground::useDesktopBackground()
{
    return m_backgroundType == BackgroundType::DesktopPic;
}

void DesktopBackground::setUseDesktopBackground(bool used)
{
    setBackgroundType(used ? BackgroundType::DesktopPic : BackgroundType::ScreensaverPic);
}

BackgroundType::Type DesktopBackground::backgroundType()
{
    return m_backgroundType;
}

void DesktopBackground::setBackgroundType(BackgroundType::Type type)
{
    if (m_backgroundType != BackgroundType::Null) {
        return;
    }
    m_backgroundType = type;
    PixmapProvider::instance()->loadPixmap(m_backgroundType);
}

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "picturelabel.h"
#include <QPainter>
#include <QtMath>
#include <QApplication>
#include <QScreen>
#include <QDebug>

PictureLabel::PictureLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
        QLabel(text, parent, f)
{
}

PictureLabel::PictureLabel(QWidget *parent, Qt::WindowFlags f):
        QLabel(parent, f)
{
}

PictureLabel::~PictureLabel()
{

}

void PictureLabel::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (this->m_pixmap.isNull()) {
        return;
    }
    QPainter p(this);
    p.save();
    QScreen *m_screen = qApp->primaryScreen();
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    int timeScreenW = m_screen->size().width()  / this->width();
    int timeScreenH = m_screen->size().height() / this->height();
    QSize mSize(m_pixmap.width()/timeScreenW, m_pixmap.height()/timeScreenH);
    QPixmap pixmap = m_pixmap.scaled(mSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.fillRect(this->rect(), Qt::black);
    if (mode == "centered") {
        //居中

        p.drawPixmap((this->size().width() - pixmap.rect().width()) / 2,
                     (this->size().height() - pixmap.rect().height()) / 2,
                     pixmap);

    } else if (mode == "stretched") {
        //拉伸
        p.drawPixmap(this->rect(), pixmap, pixmap.rect());
    } else if (mode == "scaled") {
        //填充
        p.drawPixmap(this->rect(), pixmap, getSourceRect(pixmap));
    } else if (mode == "wallpaper") {
        //平铺
        int drawedWidth = 0;
        int drawedHeight = 0;
        while (1) {
            drawedWidth = 0;
            while (1) {
                p.drawPixmap(drawedWidth, drawedHeight, pixmap);
                drawedWidth += pixmap.width();
                if (drawedWidth >= this->size().width()) {
                    break;
                }
            }
            drawedHeight += pixmap.height();
            if (drawedHeight >= this->size().height()) {
                break;
            }
        }
    } else if (mode == "zoom") {
        p.drawPixmap(getDestRect(pixmap), pixmap, pixmap.rect());
    } else if (mode == "spanned") {
        p.drawPixmap(this->rect(), pixmap, getSourceRect(pixmap, this->geometry()));
    } else {
        p.drawPixmap(rect().adjusted(0, 0, 0, 0), pixmap, pixmap.rect());
    }
    p.restore();
}

void PictureLabel::setLocalPixmap(QPixmap pixmap)
{
    this->m_pixmap = pixmap;
}

QRect PictureLabel::getSourceRect(const QPixmap &pixmap)
{
    qreal screenScale = qreal(this->rect().width()) / qreal(this->rect().height());
    qreal width = pixmap.width();
    qreal height = pixmap.height();

    if ((width / height) == screenScale) {
        return pixmap.rect();
    }

    bool isShortX = (width <= height);
    if (isShortX) {
        screenScale = qreal(this->rect().height()) / qreal(this->rect().width());
    }

    qreal shortEdge = isShortX ? width : height;
    qreal longEdge = isShortX ? height : width;

    while (shortEdge > 1) {
        qint32 temp = qFloor(shortEdge * screenScale);
        if (temp <= longEdge) {
            longEdge = temp;
            break;
        }

        qint32 spacing = qRound(shortEdge / 20);
        if (spacing <= 0) {
            spacing = 1;
        }
        shortEdge -= spacing;
    }

    QSize sourceSize = pixmap.size();
    if (shortEdge > 1 && longEdge > 1) {
        sourceSize.setWidth(isShortX ? shortEdge : longEdge);
        sourceSize.setHeight(isShortX ? longEdge : shortEdge);
    }

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (pixmap.width() > sourceSize.width()) {
        offsetX = (pixmap.width() - sourceSize.width()) / 2;
    }

    if (pixmap.height() > sourceSize.height()) {
        offsetY = (pixmap.height() - sourceSize.height()) / 2;
    }

    QPoint offsetPoint = pixmap.rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);
    return QRect(offsetPoint, sourceSize);
}

QRect PictureLabel::getSourceRect(const QPixmap &pixmap, const QRect &screenGeometry)
{
    QRect virtualGeometry = this->geometry();
    qreal pixWidth = pixmap.width();
    qreal pixHeight = pixmap.height();

    QSize sourceSize = pixmap.size();
    sourceSize.setWidth(screenGeometry.width() / virtualGeometry.width() * pixWidth);
    sourceSize.setHeight(screenGeometry.height() / virtualGeometry.height() * pixHeight);

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (screenGeometry.x() > 0) {
        offsetX = (screenGeometry.x() / virtualGeometry.width() * pixWidth);
    }

    if (screenGeometry.y() > 0) {
        offsetY = (screenGeometry.y() / virtualGeometry.height() * pixHeight);
    }

    QPoint offsetPoint = pixmap.rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);
    return QRect(offsetPoint, sourceSize);
}

QRect PictureLabel::getDestRect(const QPixmap &pixmap)
{
    qreal screenScale = qreal(this->rect().width()) / qreal(this->rect().height());
    qreal pixmapScale = qreal(pixmap.width() / pixmap.height());
    qreal width = pixmap.width();
    qreal height = pixmap.height();

    if (pixmapScale == screenScale) {
        return this->rect();
    }

    qreal scaleWidth = this->rect().width() / width;
    qreal scaleHeight = this->rect().height() / height;
    qreal realPixmapWidth = 0;
    qreal realPixmapHeight = 0;

    if(pixmapScale < screenScale){
        //图片比例小于屏幕比例时，按照图片和屏幕高度比进行缩放
        realPixmapWidth = width * scaleHeight;
        realPixmapHeight = this->rect().height();
    }else{
        //图片比例大于屏幕比例时，按照图片与屏幕宽度比进行缩放
        realPixmapWidth = this->rect().width();
        realPixmapHeight = height * scaleWidth;
    }

    QSize sourceSize = this->size();
    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (this->rect().width() == realPixmapWidth) {
        offsetY = (this->rect().height() - realPixmapHeight) / 2;
        sourceSize.setHeight(realPixmapHeight);
    } else if (this->rect().height() == realPixmapHeight) {
        offsetX = (this->rect().width() - realPixmapWidth) / 2;
        sourceSize.setWidth(realPixmapWidth);
    }

    QPoint offsetPoint = this->rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

void PictureLabel::setMode(QString mode)
{
    this->mode = mode;
}

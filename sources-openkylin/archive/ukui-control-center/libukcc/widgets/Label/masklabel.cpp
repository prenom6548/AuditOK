/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 KylinSoft  Co., Ltd.
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
#include "masklabel.h"
#include <QPainter>
#include <QPainterPath>
#include <QIcon>

MaskLabel::MaskLabel(QWidget *parent, bool isControlSaver)
    : QLabel{parent},
      m_bControlSaver(isControlSaver)
{

}

QPixmap MaskLabel::drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor) {
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

void MaskLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if (!m_bControlSaver) {
        painter.setOpacity(0.45);
        painter.setClipping(true);
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor("#000000"));
        painter.setPen(Qt::transparent);
        painter.drawRoundedRect(rect().adjusted(0, 0, 0, 0), 8, 8);
        QRect widgetRect = this->rect();
        QPixmap prevIcon = drawSymbolicColoredPixmap(
            QIcon::fromTheme("software-installed-symbolic").pixmap(QSize(40, 40)), "white");
        QPoint centerPos = widgetRect.center() - QPoint(20, 20);
        painter.drawPixmap(centerPos, prevIcon);
    } else {
        QColor highlightColor = palette().color(QPalette::Highlight);
        QRectF borderRect = rect().adjusted(1, 1, -1, -1);
        QPainterPath path;
        path.addRoundedRect(borderRect, 8, 8);
        painter.setClipPath(path);
        painter.setClipping(false);
        QPen pen(highlightColor, 2);
        painter.setPen(pen);
        painter.drawPath(path);
    }
}

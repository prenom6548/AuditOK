/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "iconbutton.h"
#include <QApplication>
#include <QSvgRenderer>
#include <QPainter>

IconButton::IconButton(QString iconFileName, QIcon icon, QWidget *parent):
    QPushButton(parent)
{
    this->iconName = iconFileName;
    this->icon = icon;
    QString iconBtnQss = QString("QPushButton:checked{border:  none;}"
                                 "QPushButton:!checked{border: none;}");
    this->setStyleSheet(iconBtnQss);
}

IconButton::~IconButton()
{

}

void IconButton::reLoadIcon()
{
    if (icon.isNull()) {
        QString path = QString("://img/secondaryleftmenu/%1.svg").arg(iconName);
        QPixmap pix = loadSvg(path);
        this->setIcon(pix);
    } else {
        QPixmap pix = icon.pixmap(iconSize());
        this->setIcon(drawSymbolicColoredPixmap(pix));
    }
}

QPixmap IconButton::loadSvg(const QString &path, int size) {
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if (  ratio >= 2) {
        size += origSize;
    } else {
        size *= ratio;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap);
}

QPixmap IconButton::drawSymbolicColoredPixmap(const QPixmap &source) {
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                QColor colorSet = palette().color(QPalette::ButtonText);
                if (this->isChecked()) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                } else {
                    color.setRed(colorSet.red());
                    color.setGreen(colorSet.green());
                    color.setBlue(colorSet.blue());
                }
                img.setPixelColor(x, y, color);

            }
        }
    }
    return QPixmap::fromImage(img);
}

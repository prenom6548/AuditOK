/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "iconbutton.h"
#include <QPainter>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QStyleOption>
#include <QPainterPath>
#include <QPixmap>
#include <QDebug>

static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}

IconButton::IconButton(QWidget *parent) : QPushButton(parent) {}

IconButton::~IconButton() {}

void IconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOptionButton option;
    initStyleOption(&option);
    QStylePainter painter(this);
    if (iconType == ENTERICON) {
        painter.setRenderHint(QPainter::Antialiasing, true);
        if (option.state & QStyle::State_HasFocus) {

            painter.save();
            QColor color = option.palette.color(QPalette::Highlight);
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 12, 12);
            painter.restore();

            painter.save();
            QStyleOption opt;

            QColor borderColor = option.palette.color(QPalette::Text);
            painter.setPen(QPen(borderColor, 2));
            //        painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 12, 12);
            painter.restore();
        }

        int buttonWidget = this->width();
        int buttonHeight = this->height();
        QRect buttonRect(0, 0, buttonWidget, buttonHeight);

        QPixmap pixmap = option.icon.pixmap(option.iconSize, QIcon::Selected, QIcon::On);
        int pixmapWidth = static_cast<int>(pixmap.width());
        int pixmapHeight = static_cast<int>(pixmap.height());
        QRect pixmapRect(0, 0, pixmapWidth, pixmapHeight);

        int deltaX = 0;
        int deltaY = 0;
        if (pixmapRect.width() < buttonRect.width())
            deltaX = buttonRect.width() - pixmapRect.width();
        else
            deltaX = pixmapRect.width() - buttonRect.width();
        if (pixmapRect.height() < buttonRect.height())
            deltaY = buttonRect.height() - pixmapRect.height();
        else
            deltaY = pixmapRect.height() - buttonRect.height();

        painter.save();
        painter.translate(deltaX / 2, deltaY / 2);
        painter.drawPixmap(pixmapRect, pixmap);
        painter.restore();
    } else if (iconType == MODEICON) {
        option.state = QStyle::State_Enabled;
        painter.drawControl(QStyle::CE_PushButton, option);
    }
}

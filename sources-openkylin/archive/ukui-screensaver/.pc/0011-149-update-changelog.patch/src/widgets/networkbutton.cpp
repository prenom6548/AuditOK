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
#include "networkbutton.h"
#include <QStylePainter>
#include <QStyleOption>
#include <QApplication>
#include <QMouseEvent>
#include "commonfunc.h"

NetWorkButton::NetWorkButton(QWidget *parent) : KyNetworkIcon(parent)
{
    setFixedSize(48, 48);
    setIconSize(QSize(24, 24));
    setFocusPolicy(Qt::NoFocus);
}

void NetWorkButton::setClickedStatus(int status)
{
    curStatus = status;
    update();
}

void NetWorkButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QStyleOptionButton option;
    initStyleOption(&option);

    //    if (curStatus == NORMAL) {
    //        painter.save();
    //        painter.setPen(Qt::NoPen);
    //        painter.setBrush(Qt::white);
    //        painter.setOpacity(0);
    //        painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
    //        painter.restore();
    //    }

    //    if (option.state & QStyle::State_MouseOver) {
    //        painter.save();
    //        painter.setPen(Qt::NoPen);
    //        painter.setBrush(Qt::white);
    //        painter.setOpacity(0.25);
    //        painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
    //        painter.restore();
    //    }
    if (option.state & QStyle::State_MouseOver) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setOpacity(0.15);
        painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
        painter.restore();
    }

    if (curStatus == CLICKED) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setOpacity(0.35);
        painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
        painter.restore();
    }

    if (curStatus == SELECT) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setOpacity(0.25);
        painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
        painter.restore();

        painter.save();
        QStyleOption opt;
        QColor color = opt.palette.color(QPalette::Highlight);
        painter.setPen(QPen(color, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
        painter.restore();
    }

    if (option.state & QStyle::State_HasFocus) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setOpacity(0.25);
        painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
        painter.restore();
        painter.save();
        QStyleOption opt;
        QColor color = opt.palette.color(QPalette::Highlight);
        painter.setPen(QPen(color, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
        painter.restore();
    }
    // 绘制图片
    int buttonWidget = this->width();
    int buttonHeight = this->height();
    QRect buttonRect(0, 0, buttonWidget, buttonHeight);

    QPixmap pixmap = option.icon.pixmap(option.iconSize, QIcon::Active);
    pixmap = drawSymbolicColoredPixmap(pixmap, "white");
    int pixmapWidth = static_cast<int>(pixmap.width() / qApp->devicePixelRatio());
    int pixmapHeight = static_cast<int>(pixmap.height() / qApp->devicePixelRatio());
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
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.translate(deltaX / 2, deltaY / 2);
    painter.drawPixmap(pixmapRect, pixmap);
    painter.restore();
}

void NetWorkButton::mouseReleaseEvent(QMouseEvent *e)
{
    e->ignore();
}

void NetWorkButton::mousePressEvent(QMouseEvent *e)
{
    e->ignore();
}

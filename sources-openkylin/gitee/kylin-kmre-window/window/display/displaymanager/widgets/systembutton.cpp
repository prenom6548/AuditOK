/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "systembutton.h"
#include <QDebug>

SystemButton::SystemButton(QWidget *parent) :
    QPushButton(parent)
    , m_xOffSet(0)
    , m_yOffSet(0)
{
    this->setMouseTracking(false);
    this->setStyleSheet("QPushButton{background:transparent;}QToolTip {background-color: rgba(255,255,255,95%);border:1px solid rgba(255,255,255,20%); border-radius:4px; color:#000000;}");
    status = NORMAL;
    mouse_press = false;
}

void SystemButton::loadPixmap(const QString &normalPic, const QString &hoverPic, const QString &pressPic)
{
    m_normalPixmap = QPixmap(normalPic);
    m_hoverPixmap = QPixmap(hoverPic);
    m_pressPixmap = QPixmap(pressPic);
    btn_width = m_normalPixmap.width();
    btn_height = m_normalPixmap.height();
}

void SystemButton::setOffset(int x_offset, int y_offset)
{
    m_xOffSet = x_offset;
    m_yOffSet = y_offset;
}

void SystemButton::enterEvent(QEvent *)
{
    status = ENTER;
    update();
}

void SystemButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mouse_press = true;
        status = PRESS;
        update();
    }
}

void SystemButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (mouse_press && this->rect().contains(event->pos())) {
        mouse_press = false;
        status = ENTER;
        update();
        emit clicked();
    }
}

void SystemButton::leaveEvent(QEvent *)
{
    status = NORMAL;
    update();
}

void SystemButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(painter.renderHints() | QPainter::Antialiasing);
    QRect rect(this->rect().adjusted(0, 0, 0, 0));
    const int x = rect.left() + m_xOffSet;
    const int y = rect.top() + m_yOffSet;

    if (status == PRESS) {
        painter.fillRect(rect, QColor::fromRgbF(1, 1, 1, 0.1));//painter.fillRect(rect, Qt::transparent);
        painter.drawPixmap(x, y, m_pressPixmap);
    }
    else if (status == ENTER) {
        painter.fillRect(rect, QColor::fromRgbF(1, 1, 1, 0.05));
        painter.drawPixmap(x, y, m_hoverPixmap);
    }
    else {
        painter.drawPixmap(x, y, m_normalPixmap);
    }
}

/* Copyright (C) 2020, KylinSoft Co., Ltd.

*
* This file is part of Kylin-Screenshot.
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

#include "pinwidget.h"
#include "src/utils/confighandler.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QApplication>
#include <QShortcut>

PinWidget::PinWidget(const QPixmap &pixmap, QWidget *parent) :
    QWidget(parent),
    m_pixmap(pixmap)
{
#ifndef UNUI
    setWindowFlags(Qt::WindowStaysOnTopHint
                   | Qt::FramelessWindowHint);
    // set the bottom widget background transparent
    setAttribute(Qt::WA_TranslucentBackground);
    // setMinimumWidth(300);
    ConfigHandler conf;
    m_baseColor = conf.uiMainColorValue();
    m_hoverColor = conf.uiContrastColorValue();

    m_layout = new QVBoxLayout(this);
    const int margin = this->margin();
    m_layout->setContentsMargins(margin, margin, margin, margin);

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_hoverColor.setAlpha(180);
    m_shadowEffect->setColor(m_hoverColor);
    m_shadowEffect->setBlurRadius(2* margin);
    m_shadowEffect->setOffset(0, 0);
    setGraphicsEffect(m_shadowEffect);

    m_label = new QLabel();
    m_label->setPixmap(m_pixmap);
    m_layout->addWidget(m_label);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
#endif
}

int PinWidget::margin() const
{
    return 7;
}

void PinWidget::wheelEvent(QWheelEvent *e)
{
#ifndef UNUI
    int val = e->delta() > 0 ? 15 : -15;
    int newWidth = qBound(50, m_label->width() + val, maximumWidth());
    int newHeight = qBound(50, m_label->height() + val, maximumHeight());

    QSize size(newWidth, newHeight);
    setScaledPixmap(size);
    adjustSize();

    e->accept();
#endif
}

void PinWidget::enterEvent(QEvent *)
{
#ifndef UNUI
    m_hoverColor.setAlpha(180);
    m_shadowEffect->setColor(m_hoverColor);
#endif
}

void PinWidget::leaveEvent(QEvent *)
{
#ifndef UNUI
    m_hoverColor.setAlpha(180);
    m_shadowEffect->setColor(m_hoverColor);
#endif
}

void PinWidget::mouseDoubleClickEvent(QMouseEvent *)
{
#ifndef UNUI
    close();
#endif
}

void PinWidget::mousePressEvent(QMouseEvent *e)
{
#ifndef UNUI
    m_dragStart = e->globalPos();
    m_offsetX = e->localPos().x() / width();
    m_offsetY = e->localPos().y() / height();
#endif
}

void PinWidget::mouseMoveEvent(QMouseEvent *e)
{
#ifndef UNUI
    const QPoint delta = e->globalPos() - m_dragStart;
    int offsetW = width() * m_offsetX;
    int offsetH = height() * m_offsetY;
    move(m_dragStart.x() + delta.x() - offsetW, m_dragStart.y() + delta.y() - offsetH);
#endif
}

void PinWidget::setScaledPixmap(const QSize &size)
{
#ifndef UNUI
    const qreal scale = qApp->devicePixelRatio();
    QPixmap scaledPixmap = m_pixmap.scaled(size * scale, Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
    scaledPixmap.setDevicePixelRatio(scale);
    m_label->setPixmap(scaledPixmap);
#endif
}

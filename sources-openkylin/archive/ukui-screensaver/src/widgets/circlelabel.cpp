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
#include "circlelabel.h"
#include "commonfunc.h"

#include <QPainter>
#include <QResizeEvent>

CircleLabel::CircleLabel(QWidget *parent)
    : QLabel(parent)
{
}

CircleLabel::CircleLabel(QPixmap pixmap, QWidget *parent)
    : QLabel(parent)
{
    setPixmap(pixmap);
}

CircleLabel::~CircleLabel()
{
}

void CircleLabel::setPixmap(const QPixmap &pixmap)
{
    mPixmap = pixmap;
    m_cacheSize = QSize();
    rebuildCache();
    update();
}

void CircleLabel::rebuildCache()
{
    const int side = qMin(width(), height());
    const qreal dpr = devicePixelRatioF();
    if (side <= 0 || mPixmap.isNull()) {
        m_cachedPixmap = QPixmap();
        m_cacheSize = QSize();
        m_cacheDevicePixelRatio = dpr;
        return;
    }

    if (m_cacheSize.width() == side && m_cacheSize.height() == side
        && qFuzzyCompare(m_cacheDevicePixelRatio, dpr) && !m_cachedPixmap.isNull()) {
        return;
    }

    m_cacheSize = QSize(side, side);
    m_cacheDevicePixelRatio = dpr;

    const int pixelSide = qMax(1, qRound(side * dpr));
    QPixmap scaled = scaledSmoothPixmap(mPixmap, pixelSide, pixelSide);
    if (scaled.isNull()) {
        m_cachedPixmap = QPixmap();
        return;
    }

    scaled.setDevicePixelRatio(dpr);
    m_cachedPixmap = PixmapToRound(scaled, pixelSide / 2);
    m_cachedPixmap.setDevicePixelRatio(dpr);
}

void CircleLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    rebuildCache();
}

void CircleLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (m_cachedPixmap.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    const int side = qMin(width(), height());
    painter.drawPixmap(0, 0, side, side, m_cachedPixmap);
}

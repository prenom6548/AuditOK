/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#include "slider.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionSlider>

namespace {
constexpr qreal kGrooveHeight = 8.0;
}

Slider::Slider(QWidget *parent) : QSlider(parent) {
    setOrientation(Qt::Horizontal);
}

void Slider::setProgressColors(const QColor &played, const QColor &unplayed,
                               const QColor &handle)
{
    m_played_color = played;
    m_unplayed_color = unplayed;
    m_handle_color = handle;
    update();
}

void Slider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF groove(0.0, (height() - kGrooveHeight) / 2.0,
                        width(), kGrooveHeight);
    const qreal range = maximum() - minimum();
    const qreal progress = range > 0.0 ? (value() - minimum()) / range : 0.0;

    QStyleOptionSlider option;
    initStyleOption(&option);
    const bool playedFromRight = option.upsideDown;
    const qreal playedLength = qBound(0.0, progress, 1.0) * groove.width();
    // A full groove-height segment keeps the leading cap rounded even when
    // playback has only just started.
    const qreal displayedLength = playedLength > 0.0
        ? qMin(groove.width(), qMax(kGrooveHeight, playedLength))
        : 0.0;

    // Paint the unplayed groove first, then overlay a rounded played segment.
    // The exposed background around its trailing cap forms the matching
    // concave edge on the unplayed segment without leaving a transparent gap.
    QPainterPath groovePath;
    groovePath.addRoundedRect(groove, kGrooveHeight / 2.0,
                              kGrooveHeight / 2.0);
    painter.save();
    painter.setClipPath(groovePath);
    painter.fillRect(groove, m_unplayed_color);
    if (displayedLength > 0.0) {
        const QRectF playedRect(
            playedFromRight ? groove.right() - displayedLength : groove.left(),
            groove.top(), displayedLength, groove.height());
        QPainterPath playedPath;
        playedPath.addRoundedRect(playedRect, kGrooveHeight / 2.0,
                                  kGrooveHeight / 2.0);
        painter.fillPath(playedPath, m_played_color);
    }
    painter.restore();

    if (property("handleVisible").toBool()) {
        const qreal handleRadius = 6.0;
        qreal handleX = playedFromRight ? groove.right() - playedLength
                                       : groove.left() + playedLength;
        handleX = qBound(handleRadius, handleX, width() - handleRadius);
        painter.setBrush(m_handle_color);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(handleX, groove.center().y()),
                            handleRadius, handleRadius);
    }
}

void Slider::mousePressEvent(QMouseEvent *ev) {
    if (ev->button() != Qt::LeftButton) {
        QSlider::mousePressEvent(ev);
        return;
    }

    setSliderDown(true);
    updateValueFromMouse(ev->pos());
    ev->accept();
}

void Slider::mouseMoveEvent(QMouseEvent *ev) {
    if (!isSliderDown()) {
        QSlider::mouseMoveEvent(ev);
        return;
    }

    updateValueFromMouse(ev->pos());
    ev->accept();
}

void Slider::mouseReleaseEvent(QMouseEvent *ev) {
    if (ev->button() != Qt::LeftButton || !isSliderDown()) {
        QSlider::mouseReleaseEvent(ev);
        return;
    }

    updateValueFromMouse(ev->pos());
    setSliderDown(false);
    ev->accept();
}

void Slider::updateValueFromMouse(const QPoint &position) {
    QStyleOptionSlider option;
    initStyleOption(&option);
    const QRect groove = style()->subControlRect(QStyle::CC_Slider, &option,
                                                 QStyle::SC_SliderGroove, this);
    const QRect handle = style()->subControlRect(QStyle::CC_Slider, &option,
                                                 QStyle::SC_SliderHandle, this);
    const int sliderSpan = qMax(0, groove.width() - handle.width());
    const int sliderPosition = qBound(0,
                                      position.x() - groove.x() - handle.width() / 2,
                                      sliderSpan);
    const int value = QStyle::sliderValueFromPosition(minimum(), maximum(),
                                                      sliderPosition, sliderSpan,
                                                      option.upsideDown);
    setSliderPosition(value);
}

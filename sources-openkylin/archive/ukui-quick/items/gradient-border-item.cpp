/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */
#include "gradient-border-item.h"
#include <QtMath>
#include <QLinearGradient>
namespace UkuiQuick {
GradientBorderItem::GradientBorderItem(QQuickItem *parent)
        : QQuickPaintedItem(parent)
{
}

void GradientBorderItem::setBorderWidth(qreal width) {
    if(m_borderWidth != width) {
        m_borderWidth = width;
        update(); // 触发重绘
        Q_EMIT borderWidthChanged();
    }
}

void GradientBorderItem::setBorderRadius(qreal radius) {
    if(m_borderRadius != radius) {
        m_borderRadius = radius;
        update(); // 触发重绘
        Q_EMIT borderRadiusChanged();
    }
}

void GradientBorderItem::setDeg(qreal deg) {
    if(m_deg != deg) {
        m_deg = deg;
        update(); // 触发重绘
        Q_EMIT degChanged();
    }
}

void GradientBorderItem::setEnableSpecificBorder(bool enable) {
    if(m_enableSpecificBorder != enable) {
        m_enableSpecificBorder = enable;
        startCurveAnimation(m_enableSpecificBorder);
        Q_EMIT enableSpecificBorderChanged();
    }
}

void GradientBorderItem::setBorderColor(GradientColor *color)
{
    if (m_borderColor == nullptr && isComponentComplete()) {
        if (auto *theme = DtTheme::self(qmlEngine(this))) {
            disconnect(theme, &DtTheme::kLineComponentNormalChanged, this, &QQuickItem::update);
        }
    }
    if(color != nullptr && m_borderColor != color) {
        m_borderColor = color;
        update();
        Q_EMIT borderColorChanged();
    }
}

void GradientBorderItem::paint(QPainter *painter) {
    QRectF drawRect(m_borderWidth / 2.0, m_borderWidth / 2.0, width() - m_borderWidth, height() - m_borderWidth);
    // 启用抗锯齿，确保边缘平滑
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    if(m_enableSpecificBorder) {
        // 根据角度计算渐变方向
        QPointF start(
                (0.5 - 0.5 * qCos(m_deg * M_PI / 180)) * width(),
                (0.5 - 0.5 * qSin(m_deg * M_PI / 180)) * height()
        );
        QPointF end(
                (0.5 + 0.5 * qCos(m_deg * M_PI / 180)) * width(),
                (0.5 + 0.5 * qSin(m_deg * M_PI / 180)) * height()
        );
        QLinearGradient gradient(start,end);
        gradient.setColorAt(1.0, QColor("#0036F5"));
        gradient.setColorAt(0.66, QColor("#4E75FF"));
        gradient.setColorAt(0.29, QColor("#8A41FF"));
        gradient.setColorAt(0.0, QColor("#E888F8"));
        painter->setPen(QPen(gradient, m_borderWidth));
    } else {
        GradientColor* borderColor = m_borderColor ? m_borderColor : DtTheme::self(qmlEngine(this))->kLineComponentNormal();
        if(borderColor->colorType() == DtColorType::Type::Pure) {
            painter->setPen(QPen(borderColor->pureColor(), m_borderWidth));
        } else {
            //设置渐变色的方向
            QLinearGradient linearGradient(borderColor->getGradientStartPoint(borderColor->gradientDeg(), width(), height())
                    , borderColor->getGradientEndPoint(borderColor->gradientDeg(), width(), height()));
            //设置渐变色颜色
            linearGradient.setColorAt(0.0, borderColor->mixBackGroundColor(borderColor->gradientStart(), borderColor->gradientBackground(), 1));
            linearGradient.setColorAt(1.0, borderColor->mixBackGroundColor(borderColor->gradientEnd(), borderColor->gradientBackground(), 1));
            painter->setPen(QPen(linearGradient, m_borderWidth));
        }
    }
    // 绘制圆角矩形边框
    painter->drawRoundedRect(drawRect, m_borderRadius, m_borderRadius);
}

void GradientBorderItem::startCurveAnimation(bool start)
{
    if(m_animation == nullptr) {
        m_animation = new QPropertyAnimation(this, "deg", this);
        m_animation->setDuration(1200);
        m_animation->setStartValue(0);
        m_animation->setEndValue(540);

        // 设置贝塞尔曲线 cubic-bezier(0.3, 0.1, 0.3, 1)
        QEasingCurve curve(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(QPointF(0.3, 0.1), QPointF(0.3, 1), QPointF(1, 1));
        m_animation->setEasingCurve(curve);
        m_animation->setLoopCount(1);
    }
    if(start) {
        m_animation->start();
    } else {
        if(m_animation->state() != QAbstractAnimation::Stopped) {
            m_animation->stop();
        }
        update();
    }
}

void GradientBorderItem::componentComplete()
{
    QQuickItem::componentComplete();
    if (m_borderColor == nullptr) {
        if (auto *theme = DtTheme::self(qmlEngine(this))) {
            connect(theme, &DtTheme::kLineComponentNormalChanged, this, &QQuickItem::update);
        }
    }
}
}

/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2025 iaom <zhangpengfei@kylinos.cn>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "painted-rectangle-item.h"

#include <QPainter>
#include <QtMath>
#include <QPainterPath>


PaintedRectangleItem::PaintedRectangleItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

void PaintedRectangleItem::setRadius(const QVector4D& radius)
{
    m_radius = radius;
    update();
}

void PaintedRectangleItem::setPureColor(bool pureColor)
{
    m_pureColor = pureColor;
    update();
}

void PaintedRectangleItem::setStartColor(const QColor& color)
{
    m_startColor = color;
    update();
}

void PaintedRectangleItem::setEndColor(const QColor& color)
{
    m_endColor = color;
    update();
}

void PaintedRectangleItem::setAngle(qreal angle)
{
    m_angle = angle;
    update();
}

void PaintedRectangleItem::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void PaintedRectangleItem::setBorderColor(const QColor &color)
{
    m_borderColor = color;
    update();
}

void PaintedRectangleItem::setBorderWidth(qreal width)
{
    m_borderWidth = width;
    update();
}

void PaintedRectangleItem::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    qreal borderWidth = qFloor(m_borderWidth);
    qreal halfBorder = borderWidth / 2.0;

    // 计算填充区域（向内缩进边框宽度的一半）
    QRectF fillRect = boundingRect().adjusted(halfBorder, halfBorder, -halfBorder, -halfBorder);

    // 计算实际可用尺寸
    qreal effectiveWidth = fillRect.width();
    qreal effectiveHeight = fillRect.height();

    // 计算各圆角半径（确保不超过可用空间）
    float minDimension = qMin(effectiveWidth, effectiveHeight);
    qreal topLeftRadius = qMin(m_radius.w(), minDimension/2);
    qreal topRightRadius = qMin(m_radius.y(), minDimension/2);
    qreal bottomRightRadius = qMin(m_radius.x(), minDimension/2);
    qreal bottomLeftRadius = qMin(m_radius.z(), minDimension/2);

    // 构造圆角路径
    QPainterPath path;

    // 左上角
    path.moveTo(fillRect.left(), fillRect.top() + topLeftRadius);
    path.arcTo(fillRect.left(), fillRect.top(),
               topLeftRadius*2, topLeftRadius*2, 180, -90);

    // 右上角
    path.lineTo(fillRect.right() - topRightRadius, fillRect.top());
    path.arcTo(fillRect.right() - topRightRadius*2, fillRect.top(),
               topRightRadius*2, topRightRadius*2, 90, -90);

    // 右下角
    path.lineTo(fillRect.right(), fillRect.bottom() - bottomRightRadius);
    path.arcTo(fillRect.right() - bottomRightRadius*2, fillRect.bottom() - bottomRightRadius*2,
               bottomRightRadius*2, bottomRightRadius*2, 0, -90);

    // 左下角
    path.lineTo(fillRect.left() + bottomLeftRadius, fillRect.bottom());
    path.arcTo(fillRect.left(), fillRect.bottom() - bottomLeftRadius*2,
               bottomLeftRadius*2, bottomLeftRadius*2, -90, -90);

    path.closeSubpath();

    // 设置填充画刷
    QBrush brush;
    if (m_pureColor) {
        brush.setColor(m_color);
        brush.setStyle(Qt::SolidPattern);
    } else {
        // 计算渐变参数（基于填充区域）
        qreal gradW = fillRect.width();
        qreal gradH = fillRect.height();

        // 计算渐变方向向量
        const qreal radians = qDegreesToRadians(m_angle);
        qreal dirX = qCos(radians);
        qreal dirY = qSin(radians);

        // 计算起点终点（基于填充区域坐标系）
        QPointF startPoint(
            gradW * (0.5 - dirX * 0.5),
            gradH * (0.5 - dirY * 0.5)
        );
        QPointF endPoint(
            gradW * (0.5 + dirX * 0.5),
            gradH * (0.5 + dirY * 0.5)
        );

        // 创建线性渐变
        QLinearGradient gradient(
            fillRect.topLeft() + startPoint,
            fillRect.topLeft() + endPoint
        );
        gradient.setColorAt(0, m_startColor);
        gradient.setColorAt(1, m_endColor);
        brush = QBrush(gradient);
    }

    // 绘制填充
    painter->fillPath(path, brush);

    // 绘制边框
    if (borderWidth > 0) {
        QPen pen(m_borderColor, borderWidth);
        pen.setJoinStyle(Qt::RoundJoin);  // 圆角连接
        pen.setCapStyle(Qt::RoundCap);    // 圆角线帽
        painter->setPen(pen);
        painter->drawPath(path);
    }
}

/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2025 iaom <zhangpengfei@kylinos.cn>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef PAINTEDRECTANGLEITEM_H
#define PAINTEDRECTANGLEITEM_H

#include <QQuickPaintedItem>
#include <QVector4D>

/**
 * A rectangle with a border and rounded corners, rendered through QPainter.
 *
 * This is a helper used by ShadowedRectangle as fallback for when software
 * rendering is used, which means our shaders cannot be used.
 *
 * Since we cannot actually use QSGPaintedNode, we need to do some trickery
 * using QQuickPaintedItem as a child of ShadowedRectangle.
 *
 * \warning This item is **not** intended as a general purpose item.
 */
class PaintedRectangleItem : public QQuickPaintedItem
{
    Q_OBJECT
public:
    explicit PaintedRectangleItem(QQuickItem *parent = nullptr);

    void setRadius(const QVector4D &radius);
    void setColor(const QColor &color);
    void setPureColor(bool pureColor);
    void setStartColor(const QColor &color);
    void setEndColor(const QColor &color);
    void setAngle(qreal angle);
    void setBorderColor(const QColor &color);
    void setBorderWidth(qreal width);

    void paint(QPainter *painter) override;

private:
    QColor m_color;
    QVector4D m_radius = QVector4D{0.0, 0.0, 0.0, 0.0};
    QColor m_borderColor;
    qreal m_borderWidth = 0.0;
    QColor m_shadowColor = Qt::black;
    bool m_pureColor = true;
    QColor m_startColor = Qt::white;
    QColor m_endColor = Qt::white;
    float m_angle = 0.0;
};

#endif // PAINTEDRECTANGLEITEM_H

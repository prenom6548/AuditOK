/*
 * KWin Style UKUI
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "shadow-helper.h"

#include <QGraphicsEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPoint>

#include <QDebug>

#define INNERRECT_WIDTH 60

#define MIN_BORDER_RADIUS 2 // 圆角小于2时，按0处理

using namespace UKUI;

static ShadowHelper *global_instance = nullptr;

ShadowHelper *ShadowHelper::globalInstance()
{
    if (!global_instance)
        global_instance = new ShadowHelper;
    return global_instance;
}

void ShadowHelper::releaseShadows()
{
    for (auto shadow : m_shadowsCache) {
        shadow.clear();
    }
    m_shadowsCache.clear();
}

QSharedPointer<KDecoration2::DecorationShadow> ShadowHelper::getCreatedShadow(const ShadowInfo &info)
{
    int borderRadiusTopLeft = info.topLeft();
    int borderRadiusTopRight = info.topRight();
    int borderRadiusBottomLeft = info.bottomLeft();
    int borderRadiusBottomRight = info.bottomRight();
    qreal shadow_border = info.borderWidth();
    QPoint shadow_shift = info.shadowShift();
    bool ubrEnabled = info.ubr();

    if (borderRadiusTopLeft < 1) {
        borderRadiusTopLeft = 1;
    }
    if (borderRadiusTopRight < 1) {
        borderRadiusTopRight = 1;
    }
    if (borderRadiusBottomLeft < 1) {
        borderRadiusBottomLeft = 1;
    }
    if (borderRadiusBottomRight < 1) {
        borderRadiusBottomRight = 1;
    }

    if (m_shadowsCache.contains(info)) { // 如果在缓存中找到了相同唯一标识信息的阴影，直接返回该阴影
        auto shadow = m_shadowsCache.value(info);
        return shadow;
    } else {
        auto shadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        auto pix = this->getShadowPixmap(info.shadowColor(), info.borderColor(), shadow_border, borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight, shadow_shift, ubrEnabled);
        auto img = pix.toImage();
        shadow->setShadow(img);
        int maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
        int maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
        int maxRadius = qMax(maxTopRadius, maxBottomRadius);

        // Refering to Andriod 9-Patch image, set the area where can be stretched.
        QRect innerRect = QRect(img.width() / 2, 2 * shadow_border - shadow_shift.y() + INNERRECT_WIDTH - maxRadius - 1, 1, 1);
        shadow->setInnerShadowRect(innerRect);
        shadow->setPadding(QMargins(2 * shadow_border, 2 * shadow_border - shadow_shift.y(), 2 * shadow_border, 2 * shadow_border + shadow_shift.y()));

        m_shadowsCache.insert(info, shadow);
        return shadow;
    }
}

ShadowHelper::ShadowHelper()
{
}

static QImage applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent = 0)
{
    if (src.isNull())
        return QImage(); // No need to do anything else!
    if (!effect)
        return src; // No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size() + QSize(extent * 2, extent * 2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF(-extent, -extent, src.width() + extent * 2, src.height() + extent * 2));
    return res;
}

QPixmap ShadowHelper::getShadowPixmap(const QColor &shadowColor,
                                      const QColor &borderColor,
                                      int shadow_border,
                                      int borderRadiusTopLeft,
                                      int borderRadiusTopRight,
                                      int borderRadiusBottomLeft,
                                      int borderRadiusBottomRight,
                                      QPoint shadow_shift,
                                      bool ubrEnabled)
{
    // create a image to draw shadow
    QSize shadow_pix_size = QSize(4 * shadow_border + INNERRECT_WIDTH, 4 * shadow_border + INNERRECT_WIDTH);
    QPixmap pix(shadow_pix_size);
    pix.fill(Qt::transparent);

    // draw shadow
    QPainterPath windowRelativePath;
    windowRelativePath = caculateRelativePainterPath(borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight, ubrEnabled, false);
    windowRelativePath.setFillRule(Qt::WindingFill);

    QPainter painter(&pix);
    painter.save();
    painter.translate(2 * shadow_border - shadow_shift.x(), 2 * shadow_border - shadow_shift.y());
    painter.fillPath(windowRelativePath, shadowColor);
    painter.restore();

    QImage rawImg = pix.toImage();
    // return pix;

    QGraphicsDropShadowEffect *dropShadowEffect = new QGraphicsDropShadowEffect();
    dropShadowEffect->setBlurRadius(2 * shadow_border);
    dropShadowEffect->setColor(shadowColor);
    dropShadowEffect->setOffset(shadow_shift);

    QImage shadowImg = applyEffectToImage(rawImg, dropShadowEffect);
    // delete the shadow in the window area
    QPixmap target = QPixmap::fromImage(shadowImg);
    QPainter painter2(&target);
    painter2.save();
    painter2.setRenderHint(QPainter::HighQualityAntialiasing);
    painter2.translate(2 * shadow_border - shadow_shift.x(), 2 * shadow_border - shadow_shift.y());
    painter2.setCompositionMode(QPainter::CompositionMode_Clear);
    painter2.fillPath(windowRelativePath, Qt::transparent);
    painter2.restore();
    painter2.end();

    // draw window border
    QPixmap darkerTarget = QPixmap::fromImage(target.toImage());
    painter2.begin(&darkerTarget);

    auto borderPath = caculateRelativePainterPath(borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight, ubrEnabled, true);
    painter2.setCompositionMode(QPainter::CompositionMode_Source);
    painter2.setRenderHint(QPainter::HighQualityAntialiasing);
    painter2.setPen(QPen(borderColor, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter2.setBrush(Qt::NoBrush);
    painter2.translate(2 * shadow_border - shadow_shift.x() - 0.5, 2 * shadow_border - shadow_shift.y() - 0.5);
    painter2.drawPath(borderPath);

    return darkerTarget;
}

QPainterPath ShadowHelper::caculateRelativePainterPath(qreal borderRadiusTopLeft, qreal borderRadiusTopRight, qreal borderRadiusBottomLeft, qreal borderRadiusBottomRight, bool ubrEnabled, bool border)
{
    qreal maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
    qreal maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
    qreal maxRadius = qMax(maxTopRadius, maxBottomRadius);

    qreal squareWidth = INNERRECT_WIDTH;
    if (border)
        squareWidth += 1;

    QPainterPath windowRelativePath;
    windowRelativePath.setFillRule(Qt::WindingFill);
    QPointF currentPos;
    if (!ubrEnabled || maxRadius < MIN_BORDER_RADIUS) {
        windowRelativePath.moveTo(0, 0);
        windowRelativePath.lineTo(squareWidth, 0);
        windowRelativePath.lineTo(squareWidth, squareWidth);
        windowRelativePath.lineTo(0, squareWidth);
        windowRelativePath.lineTo(0, 0);
    } else {
        // move to top left arc start point
        windowRelativePath.moveTo(borderRadiusTopLeft, 0);
        // top left arc
        auto topLeftBorderRadiusRect = QRectF(0, 0, 2 * borderRadiusTopLeft, 2 * borderRadiusTopLeft);
        windowRelativePath.arcTo(topLeftBorderRadiusRect, 90, 90);
        // left
        windowRelativePath.lineTo(0, squareWidth - borderRadiusBottomLeft);
        // bottom left arc
        currentPos = QPointF(0, squareWidth - borderRadiusBottomLeft);
        auto bottomLeftRect = QRectF(0, squareWidth - 2 * borderRadiusBottomLeft, 2 * borderRadiusBottomLeft, 2 * borderRadiusBottomLeft);
        windowRelativePath.arcTo(bottomLeftRect, 180, 90);
        // bottom
        windowRelativePath.lineTo(squareWidth - borderRadiusBottomLeft, squareWidth);
        // bottom right squareWidth
        currentPos = QPointF(squareWidth - borderRadiusBottomLeft, squareWidth);
        auto bottomRightRect = QRectF(squareWidth - 2 * borderRadiusBottomLeft, squareWidth - 2 * borderRadiusBottomLeft, 2 * borderRadiusBottomRight, 2 * borderRadiusBottomRight);
        windowRelativePath.arcTo(bottomRightRect, 270, 90);
        // right
        windowRelativePath.lineTo(squareWidth, borderRadiusTopRight);
        // top right arc
        currentPos = QPointF(squareWidth, borderRadiusTopRight);
        auto topRightRect = QRectF(squareWidth - 2 * borderRadiusTopRight, 0, 2 * borderRadiusTopRight, 2 * borderRadiusTopRight);
        windowRelativePath.arcTo(topRightRect, 0, 90);
        // top
        windowRelativePath.lineTo(borderRadiusTopLeft, 0);
    }
    return windowRelativePath;
}

ShadowInfo::ShadowInfo(const QColor &shadowColor, const QColor &borderColor, int topLeft, int topRight, int bottomLeft, int bottomRight, int borderWidth, QPoint shadowShift, bool ubr)
    : m_shadowColor(shadowColor)
    , m_borderColor(borderColor)
    , m_topLeft(topLeft)
    , m_topRight(topRight)
    , m_bottomLeft(bottomLeft)
    , m_bottomRight(bottomRight)
    , m_borderWidth(borderWidth)
    , m_shadowShift(shadowShift)
    , m_ubr(ubr)
{
    m_uid.clear();
    m_uid.push_back(m_shadowColor.red());
    m_uid.push_back(m_shadowColor.green());
    m_uid.push_back(m_shadowColor.blue());
    m_uid.push_back(m_shadowColor.alpha());
    m_uid.push_back(m_borderColor.red());
    m_uid.push_back(m_borderColor.green());
    m_uid.push_back(m_borderColor.blue());
    m_uid.push_back(m_borderColor.alpha());
    m_uid.push_back(m_topLeft);
    m_uid.push_back(m_topRight);
    m_uid.push_back(m_bottomLeft);
    m_uid.push_back(m_bottomRight);
    m_uid.push_back(m_borderWidth);
    m_uid.push_back(m_shadowShift.x());
    m_uid.push_back(m_shadowShift.y());
    m_uid.push_back(m_ubr);
}

bool ShadowInfo::operator==(const ShadowInfo &info)
{
    return m_uid == info.uid();
}

bool ShadowInfo::operator<(const ShadowInfo &info) const
{
    return m_uid < info.uid();
}

QColor ShadowInfo::shadowColor() const
{
    return m_shadowColor;
}

QColor ShadowInfo::borderColor() const
{
    return m_borderColor;
}

int ShadowInfo::topLeft() const
{
    return m_topLeft;
}

int ShadowInfo::topRight() const
{
    return m_topRight;
}

int ShadowInfo::bottomLeft() const
{
    return m_bottomLeft;
}

int ShadowInfo::bottomRight() const
{
    return m_bottomRight;
}

int ShadowInfo::borderWidth() const
{
    return m_borderWidth;
}

QPoint ShadowInfo::shadowShift() const
{
    return m_shadowShift;
}

bool ShadowInfo::ubr() const
{
    return m_ubr;
}

QVector<int> ShadowInfo::uid() const
{
    return m_uid;
}
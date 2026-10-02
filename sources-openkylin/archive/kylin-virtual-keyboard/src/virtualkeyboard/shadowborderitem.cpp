#include "shadowborderitem.h"

ShadowBorderItem::ShadowBorderItem(QQuickItem *parent)
    : QQuickPaintedItem(parent), borderColor_(Qt::gray), outerRadius_(12.0),
      innerRadius_(8.0), verticalOffset_(2.0), keyRadius_(8.0) {
    setAntialiasing(true);
    calculateBorderRadius();
}

void ShadowBorderItem::paint(QPainter *painter) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

    if (verticalOffset_ >= 0) {
        drawBorder(painter, false);
    } else {
        drawBorder(painter, true);
    }
}

void ShadowBorderItem::drawBorder(QPainter *painter, bool isTop) {
    const qreal borderThickness = qAbs(verticalOffset_);
    QRectF outerRect(0, 0, width(), height());

    QRectF innerRect;
    if (isTop) {
        innerRect =
            QRectF(0, borderThickness, width(), height() - borderThickness);
    } else {
        innerRect = QRectF(0, 0, width(), height() - borderThickness);
    }

    // 创建外矩形路径
    QPainterPath outerPath;
    outerPath.addRoundedRect(outerRect, outerRadius_, outerRadius_);

    // 创建内矩形路径
    QPainterPath innerPath;
    innerPath.addRoundedRect(innerRect, innerRadius_, innerRadius_);

    // 计算相切后的描边路径（外矩形减去内矩形）
    QPainterPath borderPath = outerPath.subtracted(innerPath);

    // 简化路径以减少锯齿
    borderPath = borderPath.simplified();

    // 根据顶部或底部设置 mask 位置
    QPainterPath mask;
    if (isTop) {
        mask.addRect(0, 0, width(), borderThickness * 3);
    } else {
        mask.addRect(0, height() - borderThickness * 3, width(),
                     borderThickness * 3);
    }

    QPainterPath finalPath = borderPath.intersected(mask);

    // 根据顶部或底部设置渐变方向和颜色停止点
    QLinearGradient gradient;
    if (isTop) {
        gradient = QLinearGradient(0, 0, 0, borderThickness);
        gradient.setColorAt(0.0, borderColor_.darker(110));
        gradient.setColorAt(0.3, borderColor_);
        gradient.setColorAt(1.0, borderColor_.lighter(110));
    } else {
        gradient = QLinearGradient(0, height() - borderThickness, 0, height());
        gradient.setColorAt(0.0, borderColor_.lighter(110));
        gradient.setColorAt(0.7, borderColor_);
        gradient.setColorAt(1.0, borderColor_.darker(110));
    }
    gradient.setSpread(QGradient::PadSpread);

    painter->fillPath(finalPath, gradient);
}

QColor ShadowBorderItem::color() const { return borderColor_; }

void ShadowBorderItem::setColor(const QColor &color) {
    if (borderColor_ != color) {
        borderColor_ = color;
        update();
        emit colorChanged();
    }
}

qreal ShadowBorderItem::verticalOffset() const { return verticalOffset_; }

void ShadowBorderItem::setVerticalOffset(qreal offset) {
    if (!qFuzzyCompare(verticalOffset_, offset)) {
        verticalOffset_ = offset;
        calculateBorderRadius();
        update();
        emit verticalOffsetChanged();
    }
}

qreal ShadowBorderItem::keyRadius() const { return keyRadius_; }

void ShadowBorderItem::setKeyRadius(qreal radius) {
    if (!qFuzzyCompare(keyRadius_, radius)) {
        keyRadius_ = radius;
        calculateBorderRadius();
        update();
        emit keyRadiusChanged();
    }
}

void ShadowBorderItem::calculateBorderRadius() {
    const qreal borderThickness = qAbs(verticalOffset_);
    outerRadius_ = keyRadius_ + borderThickness * 2.0;
    innerRadius_ = keyRadius_ + borderThickness * 0.6;
}

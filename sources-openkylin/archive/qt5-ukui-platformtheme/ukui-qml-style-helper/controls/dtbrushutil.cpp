/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "dtbrushutil.h"

namespace UKUIQQC2Style {

DtColorInfo DtBrushUtil::fromBrush(const QBrush &brush)
{
    DtColorInfo info;

    if (brush.style() == Qt::NoBrush) {
        info.isSolidPattern = true;
        return info;
    }

    if (brush.style() == Qt::SolidPattern) {
        info.isSolidPattern = true;
        info.startColor = brush.color();
        info.endColor = brush.color();
        return info;
    }

    const QGradient *gradient = brush.gradient();
    if (!gradient || gradient->stops().isEmpty()) {
        info.isSolidPattern = true;
        info.startColor = brush.color();
        info.endColor = brush.color();
        return info;
    }

    info.startColor = gradient->stops().constFirst().second;
    if (gradient->stops().size() >= 2)
        info.endColor = gradient->stops().constLast().second;
    else
        info.endColor = info.startColor;

    info.isSolidPattern = (info.startColor == info.endColor);
    if (brush.style() == Qt::LinearGradientPattern && gradient->stops().size() == 2) {
        if (gradient->stops().at(0).second != gradient->stops().at(1).second)
            info.isSolidPattern = false;
    }

    return info;
}

bool DtBrushUtil::isSolidPattern(const QBrush &brush)
{
    return fromBrush(brush).isSolidPattern;
}

QColor DtBrushUtil::brushColor(const QBrush &brush)
{
    return brush.color();
}

QColor DtBrushUtil::startColor(const QBrush &brush)
{
    return fromBrush(brush).startColor;
}

QColor DtBrushUtil::endColor(const QBrush &brush)
{
    return fromBrush(brush).endColor;
}

} // namespace UKUIQQC2Style

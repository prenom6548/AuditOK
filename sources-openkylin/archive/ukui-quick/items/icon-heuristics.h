/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef UKUI_QUICK_ITEMS_ICON_HEURISTICS_H
#define UKUI_QUICK_ITEMS_ICON_HEURISTICS_H

#include <QImage>
#include <QPoint>
#include <QRect>

namespace UkuiQuick::IconHeuristics {

struct OpaqueBounds
{
    bool valid {false};
    QRect rect;
    QPoint topLeft;
    QPoint topRight;
    QPoint leftTop;
    QPoint leftBottom;
    QPoint rightTop;
    QPoint rightBottom;
    QPoint bottomLeft;
    QPoint bottomRight;
};

OpaqueBounds scanOpaqueBounds(const QImage &image, int alphaThreshold);
qreal alphaCoverage(const QImage &image, const QRect &rect);
bool hasThreeSimilarSides(int left, int right, int top, int bottom, int threshold);
bool hasOpaqueRectBackdrop(const QImage &image);
QImage cropNonShadowArea(QImage image, int strongAlphaThreshold = 200, int stripAlphaThreshold = 150);
bool shouldRepairBackdropCorners(const QImage &image, const OpaqueBounds &bounds, int gapAlphaThreshold = 200);
void repairBackdropCorners(QImage &image, int opaqueThreshold = 200, int gapAlphaThreshold = 200);
QRgb dominantOpaqueColor(const QImage &image);
qreal outerContentRatioWithDominant(const QImage &image, QRgb dominant);
int colorDistance(QRgb lhs, QRgb rhs);
QRgb mergeSimilarRgb(QRgb rgb);
qreal filledCircleFit(const QImage &image);
qreal circleFit(const QImage &image);

} // namespace UkuiQuick::IconHeuristics

#endif // UKUI_QUICK_ITEMS_ICON_HEURISTICS_H

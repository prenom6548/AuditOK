/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "icon-heuristics.h"

#include <QHash>
#include <QVector>
#include <QtMath>
#include <algorithm>

namespace {

constexpr int kBackdropDetectAlphaThreshold = 20; //阴影透明度误差
constexpr int kBackdropEdgeAlphaThreshold = 127; //边缘透明度误差，透明度低于该值需要填充距离最近不透明像素点的颜色
constexpr int kDominantOpaqueAlphaThreshold = 127; //主要颜色透明度阈值，获取图标主要颜色时使用
constexpr int kColorMergeStep = 32; //颜色合并步长，获取主要颜色时，对相近颜色进行合并
constexpr int kColorContrastThreshold = 96; //颜色对比阈值，用于计算两个颜色是否相近
constexpr qreal kCoreRectRatio = 0.6; //核心矩形比例，图片中心区域占图片的比例
constexpr qreal kBackdropCoverageThreshold = 0.58; //背景覆盖阈值，典型的无透明边距圆形图标在16*16时，图案边缘长度占据整个图片长度的比例
constexpr int kCircleFitAlphaThreshold = 32; //圆形轮廓检测的透明度阈值
constexpr int kFilledCircleFitAlphaThreshold = 32; //第一方兼容分支沿用原填充圆透明度阈值
constexpr qreal kCircleFitMinOuterRadiusRatio = 0.7; //忽略距离拟合轮廓过远的内部图案
constexpr qreal kCircleFitMinAngularCoverage = 0.6; //至少覆盖六成圆周才视为可拟合的开口圆形
constexpr qreal kCircleFitMaxPerfectError = 0.07; //小尺寸下允许的最大像素取整误差
constexpr qreal kCircleFitMaxError = 0.2; //超过该误差的角度不再贡献圆形拟合分

int maxContinuousOpaqueCountOnVerticalEdge(const QImage &image, int x, int startY, int endY, int alphaThreshold)
{
    int best = 0;
    int current = 0;
    const int step = startY <= endY ? 1 : -1;
    for (int y = startY; ; y += step) {
        if (qAlpha(image.pixel(x, y)) > alphaThreshold) {
            ++current;
        } else {
            best = std::max(best, current);
            current = 0;
        }

        if (y == endY) {
            break;
        }
    }

    return std::max(best, current);
}

int maxContinuousOpaqueCountOnHorizontalEdge(const QImage &image, int y, int startX, int endX, int alphaThreshold)
{
    int best = 0;
    int current = 0;
    const int step = startX <= endX ? 1 : -1;
    for (int x = startX; ; x += step) {
        if (qAlpha(image.pixel(x, y)) > alphaThreshold) {
            ++current;
        } else {
            best = std::max(best, current);
            current = 0;
        }

        if (x == endX) {
            break;
        }
    }

    return std::max(best, current);
}

bool hasRepairCandidatePixels(const QImage &image, const UkuiQuick::IconHeuristics::OpaqueBounds &bounds, int gapAlphaThreshold)
{
    for (int y = 0; y < bounds.leftTop.y(); ++y) {
        for (int x = 0; x < bounds.topLeft.x(); ++x) {
            if (qAlpha(image.pixel(x, y)) < gapAlphaThreshold) {
                return true;
            }
        }
    }

    for (int y = bounds.leftBottom.y(); y < image.height(); ++y) {
        for (int x = 0; x < bounds.bottomLeft.x(); ++x) {
            if (qAlpha(image.pixel(x, y)) < gapAlphaThreshold) {
                return true;
            }
        }
    }

    for (int y = 0; y < bounds.rightTop.y(); ++y) {
        for (int x = bounds.topRight.x(); x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) < gapAlphaThreshold) {
                return true;
            }
        }
    }

    for (int y = bounds.rightBottom.y(); y < image.height(); ++y) {
        for (int x = bounds.bottomRight.x(); x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) < gapAlphaThreshold) {
                return true;
            }
        }
    }

    return false;
}
} // namespace

namespace UkuiQuick::IconHeuristics {

OpaqueBounds scanOpaqueBounds(const QImage &image, int alphaThreshold)
{
    OpaqueBounds bounds;
    if (image.isNull()) {
        return bounds;
    }

    bool foundTopLeft = false;
    bool foundTopRight = false;
    for (int y = 0; y < image.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (!foundTopLeft && qAlpha(line[x]) > alphaThreshold) {
                bounds.topLeft = QPoint(x, y);
                foundTopLeft = true;
            }
            const int mirroredX = image.width() - x - 1;
            if (!foundTopRight && qAlpha(line[mirroredX]) > alphaThreshold) {
                bounds.topRight = QPoint(mirroredX, y);
                foundTopRight = true;
            }
            if (foundTopLeft && foundTopRight) {
                break;
            }
        }
        if (foundTopLeft && foundTopRight) {
            break;
        }
    }

    bool foundBottomLeft = false;
    bool foundBottomRight = false;
    for (int y = image.height() - 1; y >= 0; --y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (!foundBottomLeft && qAlpha(line[x]) > alphaThreshold) {
                bounds.bottomLeft = QPoint(x, y);
                foundBottomLeft = true;
            }
            const int mirroredX = image.width() - x - 1;
            if (!foundBottomRight && qAlpha(line[mirroredX]) > alphaThreshold) {
                bounds.bottomRight = QPoint(mirroredX, y);
                foundBottomRight = true;
            }
            if (foundBottomLeft && foundBottomRight) {
                break;
            }
        }
        if (foundBottomLeft && foundBottomRight) {
            break;
        }
    }

    bool foundLeftTop = false;
    bool foundLeftBottom = false;
    for (int x = 0; x < image.width(); ++x) {
        for (int y = 0; y < image.height(); ++y) {
            const QRgb *topLine = reinterpret_cast<const QRgb *>(image.constScanLine(y));
            const int mirroredY = image.height() - y - 1;
            const QRgb *bottomLine = reinterpret_cast<const QRgb *>(image.constScanLine(mirroredY));
            if (!foundLeftTop && qAlpha(topLine[x]) > alphaThreshold) {
                bounds.leftTop = QPoint(x, y);
                foundLeftTop = true;
            }
            if (!foundLeftBottom && qAlpha(bottomLine[x]) > alphaThreshold) {
                bounds.leftBottom = QPoint(x, mirroredY);
                foundLeftBottom = true;
            }
            if (foundLeftTop && foundLeftBottom) {
                break;
            }
        }
        if (foundLeftTop && foundLeftBottom) {
            break;
        }
    }

    bool foundRightTop = false;
    bool foundRightBottom = false;
    for (int x = image.width() - 1; x >= 0; --x) {
        for (int y = 0; y < image.height(); ++y) {
            const QRgb *topLine = reinterpret_cast<const QRgb *>(image.constScanLine(y));
            const int mirroredY = image.height() - y - 1;
            const QRgb *bottomLine = reinterpret_cast<const QRgb *>(image.constScanLine(mirroredY));
            if (!foundRightTop && qAlpha(topLine[x]) > alphaThreshold) {
                bounds.rightTop = QPoint(x, y);
                foundRightTop = true;
            }
            if (!foundRightBottom && qAlpha(bottomLine[x]) > alphaThreshold) {
                bounds.rightBottom = QPoint(x, mirroredY);
                foundRightBottom = true;
            }
            if (foundRightTop && foundRightBottom) {
                break;
            }
        }
        if (foundRightTop && foundRightBottom) {
            break;
        }
    }

    bounds.valid = foundTopLeft && foundTopRight && foundLeftTop && foundLeftBottom
                   && foundRightTop && foundRightBottom && foundBottomLeft && foundBottomRight;
    if (!bounds.valid) {
        return bounds;
    }

    const int left = std::min(bounds.leftTop.x(), bounds.leftBottom.x());
    const int right = std::max(bounds.rightTop.x(), bounds.rightBottom.x());
    const int top = std::min(bounds.topLeft.y(), bounds.topRight.y());
    const int bottom = std::max(bounds.bottomLeft.y(), bounds.bottomRight.y());
    bounds.rect = QRect(QPoint(left, top), QPoint(right, bottom));
    return bounds;
}

qreal alphaCoverage(const QImage &image, const QRect &rect)
{
    if (image.isNull()) {
        return 0.0;
    }

    const QRect boundedRect = rect.intersected(image.rect());
    if (boundedRect.isEmpty()) {
        return 0.0;
    }

    qreal alphaSum = 0.0;
    for (int y = boundedRect.top(); y <= boundedRect.bottom(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = boundedRect.left(); x <= boundedRect.right(); ++x) {
            alphaSum += qreal(qAlpha(line[x])) / 255.0;
        }
    }

    return alphaSum / (boundedRect.width() * boundedRect.height());
}

bool hasThreeSimilarSides(int left, int right, int top, int bottom, int threshold)
{
    const int sides[] = {left, right, top, bottom};
    for (int i = 0; i < 4; ++i) {
        int maxVal = 0;
        int minVal = INT_MAX;
        for (int j = 0; j < 4; ++j) {
            if (j == i) {
                continue;
            }
            maxVal = std::max(maxVal, sides[j]);
            minVal = std::min(minVal, sides[j]);
        }
        if (maxVal - minVal <= threshold) {
            return true;
        }
    }
    return false;
}

bool hasOpaqueRectBackdrop(const QImage &image)
{
    if (image.isNull()) {
        return false;
    }

    const OpaqueBounds bounds = scanOpaqueBounds(image, kBackdropDetectAlphaThreshold);
    if (!bounds.valid) {
        return false;
    }

    if (bounds.topLeft.y() != bounds.topRight.y()
        || bounds.leftTop.x() != bounds.leftBottom.x()
        || bounds.bottomLeft.y() != bounds.bottomRight.y()
        || bounds.rightTop.x() != bounds.rightBottom.x()) {
        return false;
    }

    const int threshold = image.width() >= 128 ? 5 : image.width() / 32 + 1;
    if (!hasThreeSimilarSides(bounds.leftBottom.y() - bounds.leftTop.y(),
                              bounds.rightBottom.y() - bounds.rightTop.y(),
                              bounds.topRight.x() - bounds.topLeft.x(),
                              bounds.bottomRight.x() - bounds.bottomLeft.x(),
                              threshold)) {
        return false;
    }

    const int topOpaqueCount = maxContinuousOpaqueCountOnHorizontalEdge(image, bounds.topLeft.y(),
                                                                        bounds.topLeft.x(), bounds.topRight.x(),
                                                                        kBackdropEdgeAlphaThreshold);
    const int bottomOpaqueCount = maxContinuousOpaqueCountOnHorizontalEdge(image, bounds.bottomLeft.y(),
                                                                           bounds.bottomLeft.x(), bounds.bottomRight.x(),
                                                                           kBackdropEdgeAlphaThreshold);
    const int leftOpaqueCount = maxContinuousOpaqueCountOnVerticalEdge(image, bounds.leftTop.x(),
                                                                       bounds.leftTop.y(), bounds.leftBottom.y(),
                                                                       kBackdropEdgeAlphaThreshold);
    const int rightOpaqueCount = maxContinuousOpaqueCountOnVerticalEdge(image, bounds.rightTop.x(),
                                                                        bounds.rightTop.y(), bounds.rightBottom.y(),
                                                                        kBackdropEdgeAlphaThreshold);

    return topOpaqueCount >= qRound((bounds.rightTop.x() - bounds.leftTop.x() + 1) * kBackdropCoverageThreshold)
           && bottomOpaqueCount >= qRound((bounds.rightBottom.x() - bounds.leftBottom.x() + 1) * kBackdropCoverageThreshold)
           && leftOpaqueCount >= qRound((bounds.bottomLeft.y() - bounds.topLeft.y() + 1) * kBackdropCoverageThreshold)
           && rightOpaqueCount >= qRound((bounds.bottomRight.y() - bounds.topRight.y() + 1) * kBackdropCoverageThreshold);
}

QImage cropNonShadowArea(QImage image, int strongAlphaThreshold, int stripAlphaThreshold)
{
    const OpaqueBounds bounds = scanOpaqueBounds(image, strongAlphaThreshold);
    if (!bounds.valid) {
        return image;
    }

    for (int y = 0; y < bounds.leftTop.y(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < bounds.topLeft.x(); ++x) {
            if (qAlpha(line[x]) < stripAlphaThreshold) {
                line[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    for (int y = bounds.leftBottom.y(); y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < bounds.bottomLeft.x(); ++x) {
            if (qAlpha(line[x]) < stripAlphaThreshold) {
                line[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    for (int y = 0; y < bounds.rightTop.y(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = bounds.topRight.x(); x < image.width(); ++x) {
            if (qAlpha(line[x]) < stripAlphaThreshold) {
                line[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    for (int y = bounds.rightBottom.y(); y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = bounds.bottomRight.x(); x < image.width(); ++x) {
            if (qAlpha(line[x]) < stripAlphaThreshold) {
                line[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    return image.copy(bounds.rect);
}

bool shouldRepairBackdropCorners(const QImage &image, const OpaqueBounds &bounds, int gapAlphaThreshold)
{
    if (image.isNull() || !bounds.valid) {
        return false;
    }

    if (bounds.rect.width() < 3 || bounds.rect.height() < 3) {
        return false;
    }

    if (!hasRepairCandidatePixels(image, bounds, gapAlphaThreshold)) {
        return false;
    }

    return true;
}

void repairBackdropCorners(QImage &image, int opaqueThreshold, int gapAlphaThreshold)
{
    if (image.isNull()) {
        return;
    }

    const OpaqueBounds bounds = scanOpaqueBounds(image, opaqueThreshold);
    if (!shouldRepairBackdropCorners(image, bounds, gapAlphaThreshold)) {
        return;
    }

    // 只补齐规则底板边角留白，不改写主体图案内容。
    const int top = std::max(bounds.topLeft.y(), bounds.topRight.y());
    const int bottom = std::min(bounds.bottomLeft.y(), bounds.bottomRight.y());
    const int left = std::max(bounds.leftTop.x(), bounds.leftBottom.x());
    const int right = std::min(bounds.rightTop.x(), bounds.rightBottom.x());

    for (int y = 0; y < top; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = bounds.topLeft.x(); x < bounds.topRight.x(); ++x) {
            if (qAlpha(line[x]) >= gapAlphaThreshold) {
                continue;
            }
            for (int i = y; i < image.height(); ++i) {
                QRgb *lineTemp = reinterpret_cast<QRgb *>(image.scanLine(i));
                if (qAlpha(lineTemp[x]) >= opaqueThreshold) {
                    line[x] = qRgba(qRed(lineTemp[x]), qGreen(lineTemp[x]), qBlue(lineTemp[x]), 255);
                    break;
                }
            }
        }
    }

    for (int y = bottom; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = bounds.bottomLeft.x(); x < bounds.bottomRight.x(); ++x) {
            if (qAlpha(line[x]) >= gapAlphaThreshold) {
                continue;
            }
            for (int i = y; i >= 0; --i) {
                QRgb *lineTemp = reinterpret_cast<QRgb *>(image.scanLine(i));
                if (qAlpha(lineTemp[x]) >= opaqueThreshold) {
                    line[x] = qRgba(qRed(lineTemp[x]), qGreen(lineTemp[x]), qBlue(lineTemp[x]), 255);
                    break;
                }
            }
        }
    }

    for (int y = bounds.leftTop.y(); y < bounds.leftBottom.y(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < left; ++x) {
            if (qAlpha(line[x]) >= gapAlphaThreshold) {
                continue;
            }
            for (int i = x + 1; i < image.width(); ++i) {
                if (qAlpha(line[i]) >= opaqueThreshold) {
                    line[x] = qRgba(qRed(line[i]), qGreen(line[i]), qBlue(line[i]), 255);
                    break;
                }
            }
        }
    }

    for (int y = bounds.rightTop.y(); y < bounds.rightBottom.y(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = image.width() - 1; x > right; --x) {
            if (qAlpha(line[x]) >= gapAlphaThreshold) {
                continue;
            }
            for (int i = x - 1; i >= 0; --i) {
                if (qAlpha(line[i]) >= opaqueThreshold) {
                    line[x] = qRgba(qRed(line[i]), qGreen(line[i]), qBlue(line[i]), 255);
                    break;
                }
            }
        }
    }

    for (int y = 0; y < bounds.leftTop.y(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < bounds.topLeft.x(); ++x) {
            if (qAlpha(line[x]) >= gapAlphaThreshold) {
                continue;
            }
            for (int i = x + 1; i < image.width(); ++i) {
                if (qAlpha(line[i]) >= opaqueThreshold) {
                    line[x] = qRgba(qRed(line[i]), qGreen(line[i]), qBlue(line[i]), 255);
                    break;
                }
            }
        }
    }

    for (int y = bounds.leftBottom.y(); y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < bounds.bottomLeft.x(); ++x) {
            if (qAlpha(line[x]) >= gapAlphaThreshold) {
                continue;
            }
            for (int i = x + 1; i < image.width(); ++i) {
                if (qAlpha(line[i]) >= opaqueThreshold) {
                    line[x] = qRgba(qRed(line[i]), qGreen(line[i]), qBlue(line[i]), 255);
                    break;
                }
            }
        }
    }

    for (int y = 0; y < bounds.rightTop.y(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = bounds.topRight.x(); x < image.width(); ++x) {
            if (qAlpha(line[x]) >= gapAlphaThreshold) {
                continue;
            }
            for (int i = x - 1; i >= 0; --i) {
                if (qAlpha(line[i]) >= opaqueThreshold) {
                    line[x] = qRgba(qRed(line[i]), qGreen(line[i]), qBlue(line[i]), 255);
                    break;
                }
            }
        }
    }

    for (int y = bounds.rightBottom.y(); y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = bounds.bottomRight.x(); x < image.width(); ++x) {
            if (qAlpha(line[x]) >= gapAlphaThreshold) {
                continue;
            }
            for (int i = x - 1; i >= 0; --i) {
                if (qAlpha(line[i]) >= opaqueThreshold) {
                    line[x] = qRgba(qRed(line[i]), qGreen(line[i]), qBlue(line[i]), 255);
                    break;
                }
            }
        }
    }
}

QRgb dominantOpaqueColor(const QImage &image)
{
    QHash<QRgb, int> counts;
    QRgb dominant = qRgb(0, 0, 0);
    int dominantCount = 0;

    for (int y = 0; y < image.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(line[x]) <= kDominantOpaqueAlphaThreshold) {
                continue;
            }
            const QRgb rgb = mergeSimilarRgb(line[x]);
            const int count = ++counts[rgb];
            if (count > dominantCount) {
                dominant = rgb;
                dominantCount = count;
            }
        }
    }

    return dominant;
}

qreal outerContentRatioWithDominant(const QImage &image, QRgb dominant)
{
    if (image.isNull()) {
        return 0.0;
    }

    const int width = qMax(1, qRound(image.width() * kCoreRectRatio));
    const int height = qMax(1, qRound(image.height() * kCoreRectRatio));
    const int left = (image.width() - width) / 2;
    const int top = (image.height() - height) / 2;
    const QRect coreRect(left, top, width, height);
    int opaquePixels = 0;
    int contrastingPixels = 0;

    for (int y = 0; y < image.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (coreRect.contains(x, y) || qAlpha(line[x]) <= kDominantOpaqueAlphaThreshold) {
                continue;
            }

            ++opaquePixels;
            if (colorDistance(mergeSimilarRgb(line[x]), dominant) >= kColorContrastThreshold) {
                ++contrastingPixels;
            }
        }
    }

    if (opaquePixels == 0) {
        return 0.0;
    }

    return qreal(contrastingPixels) / opaquePixels;
}

int colorDistance(QRgb lhs, QRgb rhs)
{
    return qAbs(qRed(lhs) - qRed(rhs))
           + qAbs(qGreen(lhs) - qGreen(rhs))
           + qAbs(qBlue(lhs) - qBlue(rhs));
}

QRgb mergeSimilarRgb(QRgb rgb)
{
    const int red = qRed(rgb) / kColorMergeStep * kColorMergeStep;
    const int green = qGreen(rgb) / kColorMergeStep * kColorMergeStep;
    const int blue = qBlue(rgb) / kColorMergeStep * kColorMergeStep;
    return qRgb(red, green, blue);
}

qreal filledCircleFit(const QImage &image)
{
    if (image.isNull()) {
        return 0.0;
    }

    const qreal radiusX = image.width() / 2.0;
    const qreal radiusY = image.height() / 2.0;
    const qreal centerX = radiusX;
    const qreal centerY = radiusY;
    qreal intersection = 0.0;
    qreal unionArea = 0.0;

    for (int y = 0; y < image.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const qreal nx = (x + 0.5 - centerX) / radiusX;
            const qreal ny = (y + 0.5 - centerY) / radiusY;
            const qreal expected = (nx * nx + ny * ny <= 1.0) ? 1.0 : 0.0;
            const qreal actual = qAlpha(line[x]) >= kFilledCircleFitAlphaThreshold ? 1.0 : 0.0;
            intersection += qMin(actual, expected);
            unionArea += qMax(actual, expected);
        }
    }

    if (unionArea <= 0.0) {
        return 0.0;
    }

    return intersection / unionArea;
}

qreal circleFit(const QImage &image)
{
    if (image.isNull()) {
        return 0.0;
    }

    const OpaqueBounds bounds = scanOpaqueBounds(image, kCircleFitAlphaThreshold);
    if (!bounds.valid) {
        return 0.0;
    }

    const int left = bounds.rect.left();
    const int top = bounds.rect.top();
    const int right = bounds.rect.right();
    const int bottom = bounds.rect.bottom();
    const int boundsWidth = right - left + 1;
    const int boundsHeight = bottom - top + 1;
    if (boundsWidth < 2 || boundsHeight < 2) {
        return 0.0;
    }

    const qreal radiusX = boundsWidth / 2.0;
    const qreal radiusY = boundsHeight / 2.0;
    const qreal perfectError = qMin(kCircleFitMaxPerfectError,
                                    1.0 / qMin(boundsWidth, boundsHeight));
    const qreal centerX = left + radiusX;
    const qreal centerY = top + radiusY;
    const int binCount = qBound(24, qMin(boundsWidth, boundsHeight) * 2, 180);
    //按角度只保留最外层像素，使实心圆、空心圆环和带内部图案的圆形得到相同轮廓。
    QVector<qreal> outerRadius(binCount, 0.0);
    //使用同尺寸的离散椭圆作为参照，抵消小尺寸下像素取整带来的系统误差。
    QVector<qreal> referenceRadius(binCount, 0.0);

    for (int y = top; y <= bottom; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = left; x <= right; ++x) {
            const qreal nx = (x + 0.5 - centerX) / radiusX;
            const qreal ny = (y + 0.5 - centerY) / radiusY;
            const qreal radialSquared = nx * nx + ny * ny;
            const bool isReferencePixel = radialSquared <= 1.0;
            const bool isOpaquePixel = qAlpha(line[x]) > kCircleFitAlphaThreshold;
            if (!isReferencePixel && !isOpaquePixel) {
                continue;
            }

            const qreal radial = qSqrt(radialSquared);
            qreal angle = qAtan2(ny, nx);
            if (angle < 0.0) {
                angle += M_PI * 2;
            }
            const int bin = qMin(binCount - 1,
                                 static_cast<int>(qFloor(angle / (M_PI * 2) * binCount)));
            if (isReferencePixel) {
                referenceRadius[bin] = qMax(referenceRadius.at(bin), radial);
            }
            if (isOpaquePixel) {
                outerRadius[bin] = qMax(outerRadius.at(bin), radial);
            }
        }
    }

    int referenceBins = 0;
    int supportedBins = 0;
    qreal radialFit = 0.0;
    for (int bin = 0; bin < binCount; ++bin) {
        const qreal reference = referenceRadius.at(bin);
        if (qFuzzyIsNull(reference)) {
            continue;
        }
        ++referenceBins;

        const qreal observed = outerRadius.at(bin);
        if (observed < reference * kCircleFitMinOuterRadiusRatio) {
            continue;
        }
        ++supportedBins;

        const qreal relativeError = qAbs(observed - reference) / reference;
        const qreal excessError = qMax<qreal>(0.0, relativeError - perfectError);
        radialFit += qMax<qreal>(0.0,
                                 1.0 - excessError / (kCircleFitMaxError - perfectError));
    }

    if (referenceBins == 0 || supportedBins == 0) {
        return 0.0;
    }

    radialFit /= supportedBins;
    //允许圆环存在开口，但开口过大时按实际角度覆盖率降低得分。
    const qreal angularCoverage = qreal(supportedBins) / referenceBins;
    if (angularCoverage < kCircleFitMinAngularCoverage) {
        radialFit *= angularCoverage / kCircleFitMinAngularCoverage;
    }
    return radialFit;
}

} // namespace UkuiQuick::IconHeuristics

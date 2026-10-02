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
 */
#include "wallpaperrenderer.h"
#include "commonfunc.h"

#include <QPainter>
#include <QPixmap>
#include <QRect>

namespace WallpaperRenderer {

namespace {

bool pixmapMatchesLayerSize(const QPixmap &pixmap, const QRect &layerRect)
{
    if (pixmap.isNull() || !layerRect.isValid()) {
        return false;
    }
    return pixmap.size().width() == layerRect.width() && pixmap.size().height() == layerRect.height();
}

} // namespace

void drawPrefilledWallpaper(QPainter &painter, const QPixmap &pixmap, const QRect &layerRect)
{
    if (pixmap.isNull() || !layerRect.isValid()) {
        return;
    }
    if (pixmapMatchesLayerSize(pixmap, layerRect)) {
        painter.drawPixmap(layerRect.topLeft(), pixmap);
        return;
    }
    drawPixmapWallpaperFill(painter, pixmap, layerRect);
}

void drawPrefilledWallpaperZoom(QPainter &painter, const QPixmap &pixmap, const QRect &layerRect,
                                const QRect &targetRect)
{
    if (pixmap.isNull() || !targetRect.isValid()) {
        return;
    }
    if (targetRect == layerRect && pixmapMatchesLayerSize(pixmap, layerRect)) {
        painter.drawPixmap(layerRect.topLeft(), pixmap);
        return;
    }
    drawPixmapWallpaperFill(painter, pixmap, targetRect);
}

void drawLockWallpaperLayers(QPainter &painter, const QPixmap &sharpPixmap, const QPixmap &blurPixmap,
                             const QRect &rect, qreal blurProgress, qreal scaleProgress, float layerAlpha)
{
    if (!rect.isValid()) {
        return;
    }

    blurProgress = qBound<qreal>(0.0, blurProgress, 1.0);
    scaleProgress = qBound<qreal>(0.0, scaleProgress, 1.0);

    if (qFuzzyIsNull(blurProgress) && qFuzzyIsNull(scaleProgress)) {
        painter.setOpacity(layerAlpha);
        drawPrefilledWallpaper(painter, sharpPixmap, rect);
        painter.setOpacity(1.0);
        return;
    }

    const qreal clearOpacity = (1.0 - blurProgress) * layerAlpha;
    const qreal blurOpacity = blurProgress * layerAlpha;
    const qreal zoomScale = 1.0 + 0.15 * scaleProgress;
    const int targetWidth = qRound(rect.width() * zoomScale);
    const int targetHeight = qRound(rect.height() * zoomScale);
    const QRect targetRect(
        rect.center().x() - targetWidth / 2,
        rect.center().y() - targetHeight / 2,
        targetWidth,
        targetHeight);

    if (clearOpacity > 0.001) {
        painter.setOpacity(clearOpacity);
        drawPrefilledWallpaperZoom(painter, sharpPixmap, rect, targetRect);
    }
    if (blurOpacity > 0.05 && !blurPixmap.isNull()) {
        painter.setOpacity(blurOpacity);
        drawPrefilledWallpaperZoom(painter, blurPixmap, rect, targetRect);
        painter.setOpacity(0.25 * blurProgress * layerAlpha);
        painter.fillRect(rect, QColor(QStringLiteral("#000000")));
    }
    painter.setOpacity(1.0);
}

} // namespace WallpaperRenderer

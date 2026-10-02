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
 * 锁屏壁纸绘制统一入口（SCALED 填充，与 rootWindowBackground 一致）。
 */
#ifndef WALLPAPERRENDERER_H
#define WALLPAPERRENDERER_H

#include <QtGlobal>

class QPainter;
class QPixmap;
class QRect;

namespace WallpaperRenderer {

/** 清晰/模糊双层 + 缩放进度；终态 blur=0、scale=0 时仅绘制清晰层 SCALED 填充 */
void drawLockWallpaperLayers(QPainter &painter, const QPixmap &sharpPixmap, const QPixmap &blurPixmap,
                             const QRect &rect, qreal blurProgress, qreal scaleProgress, float layerAlpha = 1.0f);

/**
 * 绘制已按 layerRect 做过 SCALED 填充的位图（scalePixmapWallpaperFill / getSharpBackground 等）。
 * layerRect 与 pixmap 逻辑尺寸一致时直接贴图，避免重复 cover 裁剪。
 */
void drawPrefilledWallpaper(QPainter &painter, const QPixmap &pixmap, const QRect &layerRect);

/** 在 layerRect 基础上按 targetRect（如 zoom 放大区域）绘制预填充位图 */
void drawPrefilledWallpaperZoom(QPainter &painter, const QPixmap &pixmap, const QRect &layerRect,
                                const QRect &targetRect);

} // namespace WallpaperRenderer

#endif // WALLPAPERRENDERER_H

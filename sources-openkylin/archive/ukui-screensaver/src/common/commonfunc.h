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
 **/
#ifndef COMMONFUNC_H
#define COMMONFUNC_H

#include <QList>
#include <QPixmap>
#include <QColor>
#include <QRect>
#include <QSize>

class QPainter;

class QScreen;

/** 多屏时过滤 name 为空或 FALLBACK 的占位屏；单屏时原样返回 */
QList<QScreen *> effectiveScreens(const QList<QScreen *> &screens);
/** 对 QGuiApplication::screens() 应用 effectiveScreens */
QList<QScreen *> effectiveApplicationScreens();

// 设置一下鼠标的居于主屏中央
void setCursorCenter();

QPixmap scaledPixmap(QPixmap src);

QPixmap PixmapToRound(const QPixmap &src, int radius);

QPixmap PixmapToRound(const QPixmap &src, int leftTop, int rightTop, int leftBottom, int rightBottom);

const QPixmap loadSvg(const QString &path, const QString color, int size);

QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor);

QPixmap getLoadingIcon(int size);

QPixmap loadFromFile(QString strPath);

/** 与 rootWindowBackground WNDBG_OPTION_SCALED（填充）一致的源图裁剪区域 */
QRect wallpaperCoverSourceRect(const QSize &destination, const QSize &source);

/** 居中裁剪后缩放到目标尺寸，铺满目标区域（填充） */
QPixmap scalePixmapWallpaperFill(const QPixmap &pixmap, int width, int height);

/** 与 rootWindowBackground copyImageToPixmap(SCALED) 一致：按目标区域填充绘制 */
void drawPixmapWallpaperFill(QPainter &painter, const QPixmap &pixmap, const QRect &destRect);

QPixmap blurPixmap(QPixmap pixmap);

QPixmap scaleBlurPixmap(int width, int height, QString url);

/** 清晰壁纸按目标尺寸 SCALED 填充（无模糊），与锁屏/桌面 rootWindowBackground 一致 */
QPixmap scaleWallpaperFillFromFile(int width, int height, const QString &url);

QPixmap scaledSmoothPixmap(const QPixmap &src, int nWidth, int nHeight);

QStringList getAudioFiles(const QString &directoryPath);

QString getFirstImageFileIterator(const QString &dirPath);

QString mapToThumbnailPath(const QString& originalPath);

QPixmap filledSymbolicColoredPix(const QPixmap &source, const QColor &baseColor, const QColor &symbolicColor = QColor(26, 26, 26), int colorDifference = 15);

#endif // COMMONFUNC_H

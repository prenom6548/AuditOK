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

#include <QPixmap>

// 设置一下鼠标的居于主屏中央
void setCursorCenter();

QPixmap scaledPixmap(QPixmap src);

QPixmap PixmapToRound(const QPixmap &src, int radius);

QPixmap PixmapToRound(const QPixmap &src, int leftTop, int rightTop, int leftBottom, int rightBottom);

const QPixmap loadSvg(const QString &path, const QString color, int size);

QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor);

QPixmap getLoadingIcon(int size);

QPixmap blurPixmap(QPixmap pixmap);

QPixmap scaleBlurPixmap(int width, int height, QString url);

QPixmap scaledSmoothPixmap(const QPixmap &src, int nWidth, int nHeight);

#endif // COMMONFUNC_H

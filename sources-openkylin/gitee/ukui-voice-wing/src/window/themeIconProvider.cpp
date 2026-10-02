/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "themeIconProvider.h"

#include <QBuffer>
#include <QFile>
#include <QIcon>
#include <QImage>

namespace {

QImage applyInvertedColor(const QImage &source)
{
    QImage image = source.convertToFormat(QImage::Format_ARGB32);
    for (int x = 0; x < image.width(); ++x) {
        for (int y = 0; y < image.height(); ++y) {
            QColor color = image.pixelColor(x, y);
            if (color.alpha() > 0) {
                color.setRed(255 - color.red());
                color.setGreen(255 - color.green());
                color.setBlue(255 - color.blue());
                image.setPixelColor(x, y, color);
            }
        }
    }
    return image;
}

} // namespace

QString ThemeIconHelper::buildIconDataUrl(const QString &iconName, bool invertColors, int size)
{
    if (iconName.isEmpty() || size <= 0) {
        return QString();
    }

    const QIcon sourceIcon = QIcon::fromTheme(iconName);
    if (sourceIcon.isNull()) {
        return QString();
    }

    QImage sourceImage = sourceIcon.pixmap(size, size).toImage();
    if (sourceImage.isNull()) {
        return QString();
    }

    if (invertColors) {
        sourceImage = applyInvertedColor(sourceImage);
    }

    QByteArray imageData;
    QBuffer buffer(&imageData);
    if (!buffer.open(QIODevice::WriteOnly) || !sourceImage.save(&buffer, "PNG")) {
        return QString();
    }

    return QStringLiteral("data:image/png;base64,") + imageData.toBase64();
}

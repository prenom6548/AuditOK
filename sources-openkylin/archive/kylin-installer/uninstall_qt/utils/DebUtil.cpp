/*
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
 */
#include "./utils/DebUtil.h"

QPixmap DebUtil::loadAndResizeImage(const QString &filePath, const QSize &size)
{
    QPixmap pixmap;
    QStringList supportedFormats = {"bmp", "gif", "jpg", "jpeg", "png", "svg", "tiff", "ico", "webp", "heic", "psd", "raw", "jpeg2000", "apng"};

    QFileInfo fileInfo(filePath);
    QString fileSuffix = fileInfo.suffix().toLower();

    if (!fileInfo.exists())
    {
        qWarning() << "File does not exist:" << filePath;
        return pixmap;
    }
    if (!supportedFormats.contains(fileSuffix))
    {
        qWarning() << "Unsupported file path:" << filePath;
        return pixmap;
    }

    if (fileSuffix == "svg")
    {
        QSvgRenderer svgRenderer(filePath);
        if (!svgRenderer.isValid()) 
        {
            qWarning() << "Invalid SVG file:" << filePath;
            return pixmap;
        }
        pixmap = QPixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        svgRenderer.render(&painter);
    }
    else
    {
        QImageReader reader(filePath);
        if (!reader.canRead())
        {
            qWarning() << "Cannot read image file:" << filePath;
        }
        reader.setScaledSize(size);
        QImage image = reader.read();
        if (image.isNull())
        {
            qWarning() << "Failed to read image:" << filePath;
        }
        pixmap = QPixmap::fromImage(image);
    }
    return pixmap;
}
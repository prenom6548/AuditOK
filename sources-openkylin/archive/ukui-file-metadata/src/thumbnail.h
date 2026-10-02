/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */


#ifndef UKUI_FILE_METADATA_THUMBNAIL_H
#define UKUI_FILE_METADATA_THUMBNAIL_H
#include <QUrl>
#include <QSize>
#include "ukui-file-metadata_global.h"
class QString;
class QImage;
class ThumbnailRequestPrivate;
class ThumbnailPrivate;
namespace UkuiFileMetadata {
class UKUIFILEMETADATA_EXPORT ThumbnailRequest
{
public:
    ThumbnailRequest();
    explicit ThumbnailRequest(const QSize &targetSize, qreal dpr);
    ThumbnailRequest(const ThumbnailRequest &other);
    ThumbnailRequest &operator=(const ThumbnailRequest &other);
    ThumbnailRequest &operator=(ThumbnailRequest &&other) noexcept;
    ~ThumbnailRequest();

    QSize targetSize() const;
    qreal devicePixelRatio() const;
private:
    ThumbnailRequestPrivate *d;
};

class UKUIFILEMETADATA_EXPORT Thumbnail
{
public:
    explicit Thumbnail(const QImage &image);
    Thumbnail();
    Thumbnail(const Thumbnail &other);
    Thumbnail &operator=(const Thumbnail &other);
    Thumbnail &operator=(Thumbnail &&other) noexcept;
    ~Thumbnail();

    QImage image() const;
    bool isValid() const;

private:
    ThumbnailPrivate *d;
};

} // UkuiFileMetadata

#endif //UKUI_FILE_METADATA_THUMBNAIL_H

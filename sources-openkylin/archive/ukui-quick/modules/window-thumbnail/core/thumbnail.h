/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: sundagao <sundagao@kylinos.cn>
 *
 */
#ifndef ABSTRACTTHUMBNAIL_H
#define ABSTRACTTHUMBNAIL_H

#include "thumbnail-buffer.h"
namespace UkuiThumbnail {
class Thumbnail : public QObject
{
    Q_OBJECT
public:
    explicit Thumbnail(QObject *parent = nullptr) : QObject(parent) {}
    ~Thumbnail() override = default;

    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void setUuid(const QString &uuid) = 0;
    virtual void setOutputId(const QString& outputId) = 0;
    virtual QSharedPointer<ThumbnailBuffer> currentBuffer() = 0;
    virtual void releaseBuffer(bool release) = 0;
    virtual QList<QSharedPointer<ThumbnailBuffer>> takeRetiredBuffers() = 0;
    virtual QList<QSharedPointer<ThumbnailBuffer>> takeAllBuffers() = 0;
Q_SIGNALS:
    void bufferUpdate();
};
}

#endif //THUMBNAIL_H

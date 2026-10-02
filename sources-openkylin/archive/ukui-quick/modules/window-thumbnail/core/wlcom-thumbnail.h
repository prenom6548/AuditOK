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

#ifndef WLCOMTHUMBNAIL_H
#define WLCOMTHUMBNAIL_H

#include <QList>
#include "thumbnail.h"
namespace UkuiQuick::WaylandClient {
    class KywcCaptureManager;
    class KywcCaptureFrame;
}

namespace UkuiThumbnail
{
    class WlcomThumbnail : public Thumbnail
    {
    public:
        enum Type {
            Output,
            Toplevel,
            Workspace,
        };

        explicit WlcomThumbnail(QObject *parent = nullptr);
        ~WlcomThumbnail() override;
        bool start() override;
        void stop() override;
        void setUuid(const QString &uuid) override;
        void setOutputId(const QString& outputId) override;
        QSharedPointer<ThumbnailBuffer> currentBuffer() override;
        void releaseBuffer(bool release) override;
        QList<QSharedPointer<ThumbnailBuffer>> takeRetiredBuffers() override;
        QList<QSharedPointer<ThumbnailBuffer>> takeAllBuffers() override;
        void setType(Type type);
    private:
        class Private;
        Private *d{nullptr};
    };
}
#endif //WLCOMTHUMBNAIL_H

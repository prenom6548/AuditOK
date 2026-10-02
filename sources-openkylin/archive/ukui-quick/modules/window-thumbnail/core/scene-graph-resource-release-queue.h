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
#ifndef UKUI_QUICK_SCENEGRAPHRESOURCERELEASEQUEUE_H
#define UKUI_QUICK_SCENEGRAPHRESOURCERELEASEQUEUE_H

#include <QList>
#include <QMutex>
#include <QPointer>
#include <QQuickWindow>
#include <QSharedPointer>

#include "thumbnail-buffer.h"

namespace UkuiThumbnail
{
    class SceneGraphResourceReleaseQueue final : public QObject
    {
        Q_OBJECT

    public:
        using Buffer = QSharedPointer<ThumbnailBuffer>;
        using Buffers = QList<Buffer>;

        static SceneGraphResourceReleaseQueue *fromWindow(QQuickWindow *window);
        ~SceneGraphResourceReleaseQueue() override = default;

        void enqueue(const Buffers &buffers, bool schedule = true);
        void enqueue(Buffers &&buffers, bool schedule = true);

    private:
        explicit SceneGraphResourceReleaseQueue(QQuickWindow *window);

        struct State
        {
            QMutex mutex;
            Buffers buffers;
        };

        void requestSchedule();
        void schedule();
        void immediate();
        QPointer<QQuickWindow> m_window;
        QSharedPointer<State> m_state;
    };

    inline SceneGraphResourceReleaseQueue *SceneGraphResourceReleaseQueue::fromWindow(QQuickWindow *window)
    {
        if (!window) {
            return nullptr;
        }

        auto *queue = window->findChild<SceneGraphResourceReleaseQueue *>(
            QString(), Qt::FindDirectChildrenOnly);
        if (!queue) {
            queue = new SceneGraphResourceReleaseQueue(window);
        }

        return queue;
    }
}

#endif // UKUI_QUICK_SCENEGRAPHRESOURCERELEASEQUEUE_H

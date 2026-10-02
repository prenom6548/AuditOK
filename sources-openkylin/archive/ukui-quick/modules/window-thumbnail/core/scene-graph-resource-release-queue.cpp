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
#include "scene-graph-resource-release-queue.h"

#include <QOpenGLContext>
#include <QMetaObject>
#include <QMutexLocker>
#include <QThread>

#include <utility>

#include "discard-texture-resources-runnable.h"

using namespace UkuiThumbnail;

SceneGraphResourceReleaseQueue::SceneGraphResourceReleaseQueue(QQuickWindow *window)
    : QObject(window)
    , m_window(window)
    , m_state(QSharedPointer<State>::create())
{
    connect(m_window, &QQuickWindow::sceneGraphInitialized, this, &SceneGraphResourceReleaseQueue::requestSchedule);
    connect(m_window, &QQuickWindow::visibleChanged, this, [this](bool visible) {
        if (visible) {
            requestSchedule();
        }
    });
}

void SceneGraphResourceReleaseQueue::enqueue(const Buffers &buffers, bool schedule)
{
    enqueue(Buffers(buffers), schedule);
}

void SceneGraphResourceReleaseQueue::enqueue(Buffers &&buffers, bool schedule)
{
    if (buffers.isEmpty()) {
        return;
    }

    {
        QMutexLocker locker(&m_state->mutex);
        m_state->buffers.append(std::move(buffers));
    }
    if (schedule) {
        requestSchedule();
    } else {
        immediate();
    }
}

void SceneGraphResourceReleaseQueue::requestSchedule()
{
    if (QThread::currentThread() == thread()) {
        schedule();
        return;
    }

    QMetaObject::invokeMethod(this, [this]() {
        schedule();
    }, Qt::QueuedConnection);
}

void SceneGraphResourceReleaseQueue::schedule()
{
    auto *window = m_window.data();
    if (!window ||
        !window->isVisible() ||
        !window->isSceneGraphInitialized()) {
        qWarning() << "SceneGraphResourceReleaseQueue::schedule failed, window is null or not visible or scene graph is not initialized";
        return;
    }

    {
        QMutexLocker locker(&m_state->mutex);
        if (m_state->buffers.isEmpty()) {

            return;
        }
    }

    window->scheduleRenderJob( new DiscardTextureResourcesRunnable([this]() {
        immediate();
    }), QQuickWindow::AfterRenderingStage);
}

void SceneGraphResourceReleaseQueue::immediate()
{
    auto *context = QOpenGLContext::currentContext();
    if (!context || !context->isValid()) {
        return;
    }

    Buffers buffers;
    {
        QMutexLocker locker(&m_state->mutex);
        buffers.swap(m_state->buffers);
    }

    for (int index = 0; index < buffers.size(); ++index) {
        const auto &buffer = buffers.at(index);
        if (!buffer) {
            continue;
        }
        if (buffer->releaseTextureResources()) {
            continue;
        }
        // 未释放成功，将剩余的 buffer 放回队列
        QMutexLocker locker(&m_state->mutex);
        for (int i = buffers.size() - 1; i >= index; --i) {
            m_state->buffers.prepend(buffers.at(i));
        }
        break;
    }
}

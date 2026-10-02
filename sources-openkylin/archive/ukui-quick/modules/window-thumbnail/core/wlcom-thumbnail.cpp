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


#include <utility>
#include <QThread>
#include <QMutexLocker>
#include <qpa/qplatformnativeinterface.h>
#include <QGuiApplication>
#include "wlcom-thumbnail.h"
#include "graphics-memory-buffer.h"
#include <KWayland/Client/connection_thread.h>
#include "wayland/registry.h"
#include "wayland/kywc-capture-manager.h"

using namespace UkuiThumbnail;
using namespace UkuiQuick::WaylandClient;

class WaylandBackendContext
{
public:
    static WaylandBackendContext &instance()
    {
        static WaylandBackendContext context;
        return context;
    }

    ~WaylandBackendContext()
    {
        if (m_captureManager) {
            m_captureManager->deleteLater();
            m_captureManager = nullptr;
        }
        if (m_registry) {
            m_registry->deleteLater();
            m_registry = nullptr;
        }
    }

    KywcCaptureManager *captureManager()
    {
        ensureInitialized();
        return m_captureManager;
    }

private:
    void ensureInitialized()
    {
        if (m_captureManager) {
            return;
        }

        // backend context 在进程内共享，避免每个 session 都重复做 registry/roundtrip。
        auto *connection = KWayland::Client::ConnectionThread::fromApplication(qGuiApp);
        if (!connection) {
            qWarning() << "Failed getting Wayland connection from QPA";
            return;
        }

        m_registry = new Registry();
        m_registry->create(connection);
        m_registry->setup();
        connection->roundtrip();

        const auto [name, version] = m_registry->interface(Registry::Interface::KywcCaptureManager);
        if (name == 0) {
            qWarning() << "KywcCaptureManager interface not found";
            return;
        }
        m_captureManager = m_registry->createKywcCaptureManager(name, version);
    }

    KywcCaptureManager* m_captureManager{nullptr};
    Registry* m_registry{nullptr};
};


class WlcomThumbnail::Private
{
public:
    Private(WlcomThumbnail *thumbnail);
    ~Private();
    void setUuid(const QString& uuid);
    void setOutputId(const QString& outputId);
    bool start();
    void stop();
    void setType(Type type);
    void retireCurrentBuffer();
    void replaceCurrentBuffer(ThumbnailBuffer* buffer);
    QSharedPointer<ThumbnailBuffer> currentBuffer() const;
    Type type() const;
    QString uuid() const;
    QString outputUuid() const;
    QList<QSharedPointer<ThumbnailBuffer>> takeRetiredBuffers();
    QList<QSharedPointer<ThumbnailBuffer>> takeAllBuffers();
    void releaseBuffer(bool release);

    Type m_type {Toplevel};
    bool m_withoutDecoration {false};
    QString m_uuid;
    QString m_outputUuid;
    // 是否需要重新构建 capture frame
    bool m_rebuildFrame {true};
    QScopedPointer<KywcCaptureFrame> m_captureFrame;
    QSharedPointer<ThumbnailBuffer> m_buffer;
    QList<QSharedPointer<ThumbnailBuffer>> m_retiredBuffers;
    mutable QMutex m_bufferMutex;

private:
    WlcomThumbnail *q;
};

WlcomThumbnail::Private::Private(WlcomThumbnail *thumbnail) : q(thumbnail)
{
}

WlcomThumbnail::Private::~Private()
{

}

void WlcomThumbnail::Private::setUuid(const QString& uuid)
{
    if (uuid == m_uuid) {
        return;
    }
    m_uuid = uuid;
    stop();
}

void WlcomThumbnail::Private::setOutputId(const QString& outputId)
{
    if (m_outputUuid == outputId) {
        return;
    }
    m_outputUuid = outputId;
    stop();
}

bool WlcomThumbnail::Private::start()
{
    if (!m_rebuildFrame) {
        return true;
    }
    if (m_captureFrame) {
        m_captureFrame.reset();
    }
    auto* captureManager = WaylandBackendContext::instance().captureManager();
    if (!captureManager) {
        qWarning() << "Capture manager is not available";
        return false;
    }

    switch (m_type) {
    case Output:
        m_captureFrame.reset(captureManager->captureOutput(false, m_uuid, q));
        break;
    case Toplevel:
        m_captureFrame.reset(captureManager->captureToplevel(m_uuid, m_withoutDecoration, q));
        break;
    case Workspace:
        m_captureFrame.reset(captureManager->captureWorkspace(m_uuid, m_outputUuid, q));
        break;
    }

    if (!m_captureFrame) {
        qWarning() << "create thumbnail failed, type:" << m_type << ", uuid:" << m_outputUuid << ", outputId:" << m_outputUuid;
        return false;
    }

    const auto* frame = m_captureFrame.data();
    connect(frame, &KywcCaptureFrame::updateBuffer, q, [this](const KywcCaptureFrame::BufferInfo &info) {
        replaceCurrentBuffer(new GraphicsMemoryBuffer(info));
    });
    QObject::connect(frame, &KywcCaptureFrame::failed, q, [this]() {
        m_rebuildFrame = true;
        qWarning() << "Capture failed";
    });
    QObject::connect(frame, &KywcCaptureFrame::cancelled, q, [this]() {
        m_rebuildFrame = true;
        qWarning() << "Capture cancelled";
    });

    m_rebuildFrame = false;
    return true;
}

void WlcomThumbnail::Private::stop()
{
    m_rebuildFrame = true;
    m_captureFrame.reset();

    QMutexLocker locker(&m_bufferMutex);
    if (!m_buffer && m_retiredBuffers.isEmpty()) {
        return;
    }
    retireCurrentBuffer();
}

void WlcomThumbnail::Private::setType(Type type)
{
    if (m_type == type) {
        return;
    }
    m_type = type;
    stop();
}

void WlcomThumbnail::Private::retireCurrentBuffer()
{
    if (m_buffer) {
        m_retiredBuffers.append(m_buffer);
        m_buffer.reset();
    }
}

void WlcomThumbnail::Private::replaceCurrentBuffer(ThumbnailBuffer* buffer)
{
    {
        QMutexLocker locker(&m_bufferMutex);
        retireCurrentBuffer();
        m_buffer.reset(buffer);
    }
    Q_EMIT q->bufferUpdate();
}

QSharedPointer<ThumbnailBuffer> WlcomThumbnail::Private::currentBuffer() const
{
    const QMutexLocker locker(&m_bufferMutex);
    return m_buffer;
}

WlcomThumbnail::Type WlcomThumbnail::Private::type() const
{
    return m_type;
}

QString WlcomThumbnail::Private::uuid() const
{
    return m_uuid;
}

QString WlcomThumbnail::Private::outputUuid() const
{
    return m_outputUuid;
}

QList<QSharedPointer<ThumbnailBuffer>> WlcomThumbnail::Private::takeRetiredBuffers()
{
    QMutexLocker locker(&m_bufferMutex);
    return std::exchange(m_retiredBuffers, {});
}

QList<QSharedPointer<ThumbnailBuffer>> WlcomThumbnail::Private::takeAllBuffers()
{
    QMutexLocker locker(&m_bufferMutex);
    auto buffers = std::exchange(m_retiredBuffers, {});
    if (m_buffer) {
        buffers.append(m_buffer);
        m_buffer.reset();
    }
    return buffers;
}

void WlcomThumbnail::Private::releaseBuffer(bool release)
{
    QPointer<KywcCaptureFrame> frame = m_captureFrame.data();
    if (!frame) {
        return;
    }
    if (frame->thread() == QThread::currentThread()) {
        frame->releaseBuffer(release);
        return;
    }
    QMetaObject::invokeMethod(frame, [frame, release]() {
        if (frame) {
            frame->releaseBuffer(release);
        }
    }, Qt::QueuedConnection);
}

WlcomThumbnail::WlcomThumbnail(QObject *parent) : d(new Private(this))
{

}

WlcomThumbnail::~WlcomThumbnail()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

bool WlcomThumbnail::start()
{
    return d->start();
}

void WlcomThumbnail::stop()
{
    d->stop();
}

void WlcomThumbnail::setUuid(const QString& uuid)
{
    d->setUuid(uuid);
}

void WlcomThumbnail::setOutputId(const QString& outputId)
{
    d->setOutputId(outputId);
}

void WlcomThumbnail::setType(Type type)
{
    d->setType(type);
}

QSharedPointer<ThumbnailBuffer> WlcomThumbnail::currentBuffer()
{
    return d->currentBuffer();
}

void WlcomThumbnail::releaseBuffer(bool release)
{
    d->releaseBuffer(release);
}

QList<QSharedPointer<ThumbnailBuffer>> WlcomThumbnail::takeRetiredBuffers()
{
    return d->takeRetiredBuffers();
}

QList<QSharedPointer<ThumbnailBuffer>> WlcomThumbnail::takeAllBuffers()
{
    return d->takeAllBuffers();
}

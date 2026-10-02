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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "kywc-capture-manager.h"

#include "wayland-pointer_p.h"
#include "wayland-kywc-capture-v1-client-protocol.h"
#include <algorithm>
#include <QDebug>
#include <unistd.h>

namespace UkuiQuick::WaylandClient {

class KywcCaptureFrame::Private
{
public:
    Private(KywcCaptureFrame *q);

    WaylandPointer<kywc_capture_frame_v1, kywc_capture_frame_v1_destroy> frame;
    KywcCaptureFrame *q;
    BufferInfo currentBuffer;
    bool bufferInfoComplete = false;
    KywcCaptureManager *manager = nullptr;

    void init_listener(kywc_capture_frame_v1 *frame);

    static struct kywc_capture_frame_v1_listener s_frameListener;
    static void failedCallback(void *data, kywc_capture_frame_v1 *frame);
    static void cancelledCallback(void *data, kywc_capture_frame_v1 *frame);
    static void bufferCallback(void *data, kywc_capture_frame_v1 *frame, int32_t fd, uint32_t format,
                                uint32_t width, uint32_t height, uint32_t offset, uint32_t stride,
                                uint32_t modifier_hi, uint32_t modifier_lo, uint32_t flags);
    static void bufferWithPlaneCallback(void *data, kywc_capture_frame_v1 *frame, uint32_t index,
                                          int32_t fd, uint32_t offset, uint32_t stride);
    static void bufferDoneCallback(void *data, kywc_capture_frame_v1 *frame);
};

kywc_capture_frame_v1_listener KywcCaptureFrame::Private::s_frameListener = {
    .failed = failedCallback,
    .cancelled = cancelledCallback,
    .buffer = bufferCallback,
    .buffer_with_plane = bufferWithPlaneCallback,
    .buffer_done = bufferDoneCallback
};

class KywcCaptureManager::Private
{
public:
    Private(KywcCaptureManager *q);

    WaylandPointer<kywc_capture_manager_v1, kywc_capture_manager_v1_destroy> manager;
    KWayland::Client::EventQueue *queue = nullptr;
    KywcCaptureManager *q;
};

KywcCaptureFrame::Private::Private(KywcCaptureFrame *q) : q(q)
{
}

KywcCaptureFrame::KywcCaptureFrame(KywcCaptureManager *manager, QObject *parent) : QObject(parent), d(new Private(this))
{
    d->manager = manager;
}

KywcCaptureFrame::~KywcCaptureFrame()
{
    release();
}

void KywcCaptureFrame::setup(kywc_capture_frame_v1 *frame)
{
    Q_ASSERT(!d->frame);
    Q_ASSERT(frame);
    d->frame.setup(frame);
    d->init_listener(frame);
}

void KywcCaptureFrame::release()
{
    if (d->frame) {
        d->frame.release();
    }
}

void KywcCaptureFrame::destroy()
{
    if (d->frame) {
        d->frame.destroy();
    }
}

bool KywcCaptureFrame::isValid() const
{
    return d->frame.isValid();
}

void KywcCaptureFrame::releaseBuffer(uint32_t wantBuffer)
{
    if (!d->frame) {
        return;
    }
    kywc_capture_frame_v1_release_buffer(d->frame, wantBuffer);
    wl_display_flush(wl_proxy_get_display(d->frame));

    const int planeCount = std::clamp(d->currentBuffer.n_planes, 0, 4);
    for (int i = 0; i < planeCount; i++) {
        if (d->currentBuffer.planes[i].fd >= 0) {
            close(d->currentBuffer.planes[i].fd);
            d->currentBuffer.planes[i].fd = -1;
        }
    }
    d->currentBuffer.fd = -1;
    d->currentBuffer.n_planes = 0;
}

void KywcCaptureFrame::Private::init_listener(kywc_capture_frame_v1 *frame)
{
    kywc_capture_frame_v1_add_listener(frame, &s_frameListener, this);
}

void KywcCaptureFrame::Private::failedCallback(void *data, kywc_capture_frame_v1 *frame)
{
    auto p = reinterpret_cast<KywcCaptureFrame::Private *>(data);
    if (p->frame != frame) {
        return;
    }
    Q_EMIT p->q->failed();
}

void KywcCaptureFrame::Private::cancelledCallback(void *data, kywc_capture_frame_v1 *frame)
{
    auto p = reinterpret_cast<KywcCaptureFrame::Private *>(data);
    if (p->frame != frame) {
        return;
    }
    Q_EMIT p->q->cancelled();
}

void KywcCaptureFrame::Private::bufferCallback(void *data, kywc_capture_frame_v1 *frame,
                                                int32_t fd, uint32_t format,
                                                uint32_t width, uint32_t height,
                                                uint32_t offset, uint32_t stride,
                                                uint32_t modifier_hi, uint32_t modifier_lo,
                                                uint32_t flags)
{
    auto p = reinterpret_cast<KywcCaptureFrame::Private *>(data);
    if (p->frame != frame) {
        close(fd);
        return;
    }

    BufferInfo info;
    info.fd = fd;
    info.format = format;
    info.size = QSize(width, height);
    info.offset = offset;
    info.stride = stride;
    info.modifier = (static_cast<uint64_t>(modifier_hi) << 32) | modifier_lo;
    info.flags = static_cast<BufferFlags>(flags);
    info.n_planes = 1;
    info.planes[0] = { fd, offset, stride };

    p->currentBuffer = info;
    p->bufferInfoComplete = true;

    if (kywc_capture_frame_v1_get_version(frame) >= KYWC_CAPTURE_FRAME_V1_BUFFER_DONE_SINCE_VERSION) {
        return;
    }
    Q_EMIT p->q->updateBuffer(info);
}

void KywcCaptureFrame::Private::bufferWithPlaneCallback(void *data, kywc_capture_frame_v1 *frame,
                                                         uint32_t index, int32_t fd,
                                                         uint32_t offset, uint32_t stride)
{
    auto p = reinterpret_cast<KywcCaptureFrame::Private *>(data);
    if (p->frame != frame) {
        close(fd);
        return;
    }

    if (index >= 4) {
        close(fd);
        return;
    }

    p->currentBuffer.planes[index].fd = fd;
    p->currentBuffer.planes[index].offset = offset;
    p->currentBuffer.planes[index].stride = stride;
    p->currentBuffer.n_planes = index + 1;
}

void KywcCaptureFrame::Private::bufferDoneCallback(void *data, kywc_capture_frame_v1 *frame)
{
    auto p = reinterpret_cast<KywcCaptureFrame::Private *>(data);
    if (p->frame != frame) {
        return;
    }

    Q_EMIT p->q->updateBuffer(p->currentBuffer);
}

KywcCaptureManager::Private::Private(KywcCaptureManager *q) : q(q)
{
}

KywcCaptureManager::KywcCaptureManager(QObject *parent) : QObject(parent), d(new Private(this))
{
}

KywcCaptureManager::~KywcCaptureManager()
{
    release();
}

bool KywcCaptureManager::isValid() const
{
    return d->manager.isValid();
}

void KywcCaptureManager::release()
{
    if (!d->manager) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->manager.release();
}

void KywcCaptureManager::destroy()
{
    if (!d->manager) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->manager.destroy();
}

void KywcCaptureManager::setup(kywc_capture_manager_v1 *manager)
{
    Q_ASSERT(!d->manager);
    Q_ASSERT(manager);
    d->manager.setup(manager);
}

void KywcCaptureManager::setEventQueue(KWayland::Client::EventQueue *queue)
{
    d->queue = queue;
}

KWayland::Client::EventQueue *KywcCaptureManager::eventQueue() const
{
    return d->queue;
}

KywcCaptureFrame* KywcCaptureManager::captureOutput(bool overlayCursor, const QString &outputId, QObject *parent)
{
    if (!d->manager) {
        return nullptr;
    }
    auto frame = kywc_capture_manager_v1_capture_output(d->manager, overlayCursor ? 1 : 0, qPrintable(outputId));
    if (!frame) {
        return nullptr;
    }
    auto captureFrame = new KywcCaptureFrame(this, parent);
    if (d->queue) {
        d->queue->addProxy(frame);
    }
    captureFrame->setup(frame);
    return captureFrame;
}

KywcCaptureFrame* KywcCaptureManager::captureWorkspace(const QString &workspaceId, const QString &outputId, QObject *parent)
{
    if (!d->manager) {
        return nullptr;
    }
    auto frame = kywc_capture_manager_v1_capture_workspace(d->manager, qPrintable(workspaceId), qPrintable(outputId));
    if (!frame) {
        return nullptr;
    }
    auto captureFrame = new KywcCaptureFrame(this, parent);
    if (d->queue) {
        d->queue->addProxy(frame);
    }
    captureFrame->setup(frame);
    return captureFrame;
}

KywcCaptureFrame* KywcCaptureManager::captureToplevel(const QString &toplevelId, bool withoutDecoration, QObject *parent)
{
    if (!d->manager) {
        return nullptr;
    }
    auto frame = kywc_capture_manager_v1_capture_toplevel(d->manager, qPrintable(toplevelId), withoutDecoration ? 1 : 0);
    if (!frame) {
        return nullptr;
    }
    auto captureFrame = new KywcCaptureFrame(this, parent);
    if (d->queue) {
        d->queue->addProxy(frame);
    }
    captureFrame->setup(frame);
    return captureFrame;
}

}

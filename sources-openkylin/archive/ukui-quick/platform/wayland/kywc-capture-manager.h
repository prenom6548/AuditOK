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

#ifndef KYWC_CAPTURE_MANAGER_H
#define KYWC_CAPTURE_MANAGER_H

#include <QObject>
#include <QList>
#include <QSize>
#include <KWayland/Client/event_queue.h>

struct kywc_capture_manager_v1;
struct kywc_capture_frame_v1;

namespace KWayland::Client {
class EventQueue;
class Seat;
}

namespace UkuiQuick::WaylandClient {

class KywcCaptureFrame;
class KywcCaptureCursor;

class KywcCaptureManager : public QObject
{
    Q_OBJECT
public:
    explicit KywcCaptureManager(QObject *parent = nullptr);
    ~KywcCaptureManager() override;

    bool isValid() const;
    void release();
    void destroy();
    void setup(kywc_capture_manager_v1 *manager);
    void setEventQueue(KWayland::Client::EventQueue *queue);

    KWayland::Client::EventQueue *eventQueue() const;

    KywcCaptureFrame* captureOutput(bool overlayCursor, const QString &outputId, QObject *parent = nullptr);
    KywcCaptureFrame* captureWorkspace(const QString &workspaceId, const QString &outputId, QObject *parent = nullptr);
    KywcCaptureFrame* captureToplevel(const QString &toplevelId, bool withoutDecoration, QObject *parent = nullptr);

Q_SIGNALS:
    void interfaceAboutToBeReleased();
    void interfaceAboutToBeDestroyed();
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

class KywcCaptureFrame : public QObject
{
    Q_OBJECT
public:
    ~KywcCaptureFrame() override;

    void setup(kywc_capture_frame_v1 *frame);
    void release();
    void destroy();
    bool isValid() const;

    void releaseBuffer(uint32_t wantBuffer);

    enum class BufferFlag {
        Dmabuf = 1,
        Reused = 2
    };
    Q_DECLARE_FLAGS(BufferFlags, BufferFlag)

    struct BufferInfo {
        int fd = -1;
        uint32_t format = 0;
        QSize size;
        uint32_t offset = 0;
        uint32_t stride = 0;
        uint64_t modifier = 0;
        BufferFlags flags;
        int n_planes = 0;
        struct {
            int32_t fd = -1;
            uint32_t offset, stride;
        } planes[4]{};
    };

Q_SIGNALS:
    void failed();
    void cancelled();
    void updateBuffer(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo &info);

private:
    friend class KywcCaptureManager;
    explicit KywcCaptureFrame(KywcCaptureManager *manager, QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}

#endif //KYWC_CAPTURE_MANAGER_H

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

#ifndef KYWC_CAPTURE_THUMBNAIL_ITEM_H
#define KYWC_CAPTURE_THUMBNAIL_ITEM_H

#include <QQuickItem>
#include <QImage>
#include <QOpenGLTexture>
#include <QSize>
#include <QPoint>
#include <qsgtexture.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/mman.h>

#include "kywc-capture-manager.h"

class KywcCaptureThumbnailItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(CaptureType captureType READ captureType WRITE setCaptureType NOTIFY captureTypeChanged)
    Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
    Q_PROPERTY(QString outputUuid READ outputUuid WRITE setOutputUuid NOTIFY outputUuidChanged)
    Q_PROPERTY(bool overlayCursor READ overlayCursor WRITE setOverlayCursor NOTIFY overlayCursorChanged)
    Q_PROPERTY(bool withoutDecoration READ withoutDecoration WRITE setWithoutDecoration NOTIFY withoutDecorationChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)

public:
    enum CaptureType {
        Toplevel,
        Output,
        Workspace
    };
    Q_ENUM(CaptureType)

    explicit KywcCaptureThumbnailItem(QQuickItem *parent = nullptr);
    ~KywcCaptureThumbnailItem() override;

    CaptureType captureType() const;
    void setCaptureType(CaptureType type);

    QString uuid() const;
    void setUuid(const QString &uuid);

    QString outputUuid() const;
    void setOutputUuid(const QString &outputUuid);

    bool overlayCursor() const;
    void setOverlayCursor(bool overlay);

    bool withoutDecoration() const;
    void setWithoutDecoration(bool without);

    bool isActive() const;
    void setActive(bool active);

    qreal paintedWidth() const;
    qreal paintedHeight() const;

    void componentComplete() override;
    void releaseResources() override;

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

Q_SIGNALS:
    void captureTypeChanged();
    void uuidChanged();
    void outputUuidChanged();
    void overlayCursorChanged();
    void withoutDecorationChanged();
    void activeChanged();
    void paintedSizeChanged();
    void frameReceived(const QSize &size);
    void captureFailed();

private Q_SLOTS:
    void onBufferReceived(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo &info);
    void onBufferDone();
    void onCaptureFailed();
    void onCaptureCancelled();
    void onAfterRendering();
    void onRenderCompleted();

private:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void startCapture();
    void stopCapture();
    void createTextureFromBuffer(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo &info);
    void createTextureFromDmabuf(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo&info);
    void createTextureFromMemfd(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo&info);

    QSGTexture* createOpenGLTexture();



    CaptureType m_captureType = Toplevel;
    QString m_uuid;
    QString m_outputUuid;
    bool m_overlayCursor = true;
    bool m_withoutDecoration = false;
    bool m_active = false;

    UkuiQuick::WaylandClient::KywcCaptureManager *m_captureManager = nullptr;
    UkuiQuick::WaylandClient::KywcCaptureFrame *m_captureFrame = nullptr;

    EGLImage m_eglImage = EGL_NO_IMAGE_KHR;
    QImage m_qImage;
    uint32_t m_bufferFormat = 0;
    UkuiQuick::WaylandClient::KywcCaptureFrame::BufferFlags m_bufferFlags;
    uint64_t m_bufferModifier = 0;

    QScopedPointer<QOpenGLTexture> m_texture;
    QSizeF m_paintedSize;
    uint8_t *m_mmapData = nullptr;
    size_t m_mmapSize = 0;
    QSize m_bufferSize;
    bool m_firstFrame = true;
    wl_display *m_display = nullptr;

    bool m_renderPending = false;
    bool m_connectedWindow = false;
};

#endif //KYWC_CAPTURE_THUMBNAIL_ITEM_H
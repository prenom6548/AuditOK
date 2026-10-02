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

#include "kywc-capture-thumbnail-item.h"

#include <QQuickWindow>
#include <QSGImageNode>
#include <QOpenGLContext>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <QDir>
#include <KF5/KWayland/Client/connection_thread.h>

#include <libdrm/drm_fourcc.h>

#include "registry.h"

KywcCaptureThumbnailItem::KywcCaptureThumbnailItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);

    auto connection = KWayland::Client::ConnectionThread::fromApplication(qGuiApp);
    if (!connection) {
        qWarning() << "Failed getting Wayland connection from QPA";
        return;
    }

    auto registry = new UkuiQuick::WaylandClient::Registry(qGuiApp);
    registry->create(connection);
    registry->setup();
    connection->roundtrip();

    const UkuiQuick::WaylandClient::Registry::AnnouncedInterface interface = registry->interface(UkuiQuick::WaylandClient::Registry::Interface::KywcCaptureManager);

    if (interface.name == 0) {
        qDebug() << "KywcCaptureManager interface not found";
        return;
    }

    m_captureManager = registry->createKywcCaptureManager(interface.name, interface.version, this);

    qDebug() << "KywcCaptureThumbnailItem created" << m_captureManager;
    connect(m_captureManager, &UkuiQuick::WaylandClient::KywcCaptureManager::removed, this, [this]() {
        m_captureManager->deleteLater();
    });

    connect(this, &QQuickItem::visibleChanged, this, [this]() {
        if (isVisible() && m_active) {
            startCapture();
        } else {
            stopCapture();
        }
    });
}

KywcCaptureThumbnailItem::~KywcCaptureThumbnailItem()
{
    releaseResources();
}

KywcCaptureThumbnailItem::CaptureType KywcCaptureThumbnailItem::captureType() const
{
    return m_captureType;
}

void KywcCaptureThumbnailItem::setCaptureType(CaptureType type)
{
    if (m_captureType != type) {
        m_captureType = type;
        Q_EMIT captureTypeChanged();
    }
}

QString KywcCaptureThumbnailItem::uuid() const
{
    return m_uuid;
}

void KywcCaptureThumbnailItem::setUuid(const QString &uuid)
{
    if (m_uuid != uuid) {
        m_uuid = uuid;
        Q_EMIT uuidChanged();
    }
}

QString KywcCaptureThumbnailItem::outputUuid() const
{
    return m_outputUuid;
}

void KywcCaptureThumbnailItem::setOutputUuid(const QString &outputUuid)
{
    if (m_outputUuid != outputUuid) {
        m_outputUuid = outputUuid;
        Q_EMIT outputUuidChanged();
    }
}

bool KywcCaptureThumbnailItem::overlayCursor() const
{
    return m_overlayCursor;
}

void KywcCaptureThumbnailItem::setOverlayCursor(bool overlay)
{
    if (m_overlayCursor != overlay) {
        m_overlayCursor = overlay;
        Q_EMIT overlayCursorChanged();
    }
}

bool KywcCaptureThumbnailItem::withoutDecoration() const
{
    return m_withoutDecoration;
}

void KywcCaptureThumbnailItem::setWithoutDecoration(bool without)
{
    if (m_withoutDecoration != without) {
        m_withoutDecoration = without;
        Q_EMIT withoutDecorationChanged();
    }
}

bool KywcCaptureThumbnailItem::isActive() const
{
    return m_active;
}

void KywcCaptureThumbnailItem::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        Q_EMIT activeChanged();
        if (m_active && isVisible()) {
            startCapture();
        } else {
            stopCapture();
        }
    }
}

qreal KywcCaptureThumbnailItem::paintedWidth() const
{
    return m_paintedSize.width();
}

qreal KywcCaptureThumbnailItem::paintedHeight() const
{
    return m_paintedSize.height();
}

void KywcCaptureThumbnailItem::componentComplete()
{
    QQuickItem::componentComplete();
    if (m_active && isVisible()) {
        startCapture();
    }
}

void KywcCaptureThumbnailItem::releaseResources()
{
    if (m_mmapData) {
        munmap(m_mmapData, m_mmapSize);
        m_mmapData = nullptr;
        m_mmapSize = 0;
    }
}

void KywcCaptureThumbnailItem::onBufferReceived(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo &info)
{
    qDebug() << "Buffer received" << info.size << info.format << info.flags;
    m_bufferSize = info.size;
    m_bufferFormat = info.format;
    m_bufferFlags = info.flags;
    m_bufferModifier = info.modifier;

    createTextureFromBuffer(info);

    if (window() && window()->isVisible()) {
        update();
    }
}

void KywcCaptureThumbnailItem::onBufferDone()
{
    if (m_captureFrame) {
        m_captureFrame->releaseBuffer(1);
    }
}

void KywcCaptureThumbnailItem::onCaptureFailed()
{
    qWarning() << "Capture failed";
    Q_EMIT captureFailed();
}

void KywcCaptureThumbnailItem::onCaptureCancelled()
{
    qWarning() << "Capture cancelled";
}

void KywcCaptureThumbnailItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data)
{
    if (change == ItemSceneChange && !data.window) {
        releaseResources();
    }
    QQuickItem::itemChange(change, data);
}

void KywcCaptureThumbnailItem::startCapture()
{
    if (!m_captureManager) {
        qWarning() << "Capture manager is not created";
        return;
    }
    if (!m_captureManager->isValid()) {
        qWarning() << "Capture manager is not valid";
        return;
    }

    if (m_captureFrame) {
        m_captureFrame->deleteLater();
        m_captureFrame = nullptr;
    }
    m_firstFrame = true;

    switch (m_captureType) {
    case Toplevel:
        m_captureFrame = m_captureManager->captureToplevel(m_uuid, m_withoutDecoration, this);
        break;

    case Output:
        m_captureFrame = m_captureManager->captureOutput(m_overlayCursor, m_uuid, this);
        break;

    case Workspace:
        m_captureFrame = m_captureManager->captureWorkspace(m_uuid, m_outputUuid, this);
        break;
    }
    connect(m_captureFrame, &UkuiQuick::WaylandClient::KywcCaptureFrame::updateBuffer, this, &KywcCaptureThumbnailItem::onBufferReceived);
    connect(m_captureFrame, &UkuiQuick::WaylandClient::KywcCaptureFrame::failed, this, &KywcCaptureThumbnailItem::onCaptureFailed);
    connect(m_captureFrame, &UkuiQuick::WaylandClient::KywcCaptureFrame::cancelled, this, &KywcCaptureThumbnailItem::onCaptureCancelled);
}

void KywcCaptureThumbnailItem::stopCapture()
{
    if (m_captureFrame) {
        disconnect(m_captureFrame);
        m_captureFrame->deleteLater();
        m_captureFrame = nullptr;
    }
}

#define ADD_ATTRIB(name, value)                                                                    \
do {                                                                                           \
attribs[num_attribs++] = (name);                                                           \
attribs[num_attribs++] = (value);                                                          \
attribs[num_attribs] = EGL_NONE;                                                           \
} while (0)

void KywcCaptureThumbnailItem::createTextureFromBuffer(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo &info)
{
    if (info.flags & UkuiQuick::WaylandClient::KywcCaptureFrame::BufferFlag::Dmabuf) {
        createTextureFromDmabuf(info);
    } else {
        createTextureFromMemfd(info);
    }
}



void KywcCaptureThumbnailItem::createTextureFromDmabuf(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo &info)
{
    EGLDisplay display = EGL_NO_DISPLAY;
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    if (native) {
        display = static_cast<EGLDisplay>(
            native->nativeResourceForIntegration("egldisplay"));
    }

    if (display == EGL_NO_DISPLAY) {
        qWarning() << "Failed to get EGL display";
        return;
    }

    static PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = nullptr;

    if (!eglCreateImageKHR) {
        eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    }

    if (!eglCreateImageKHR) {
        qWarning() << "EGL image extension not supported";
        return;
    }

    if (m_eglImage) {
        if (m_bufferFlags & UkuiQuick::WaylandClient::KywcCaptureFrame::BufferFlag::Reused)
            return;
        static auto eglDestroyImageKHR =
            (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
        eglDestroyImageKHR(display, m_eglImage);
        m_eglImage = nullptr;
    }

    EGLint attribs[50] = { EGL_NONE };
    int num_attribs = 0;


    ADD_ATTRIB(EGL_WIDTH, info.size.width());
    ADD_ATTRIB(EGL_HEIGHT, info.size.height());
    ADD_ATTRIB(EGL_LINUX_DRM_FOURCC_EXT, info.format);

    struct {
        EGLint fd, offset, pitch, mod_lo, mod_hi;
    } attr_names[4] = {
        { EGL_DMA_BUF_PLANE0_FD_EXT, EGL_DMA_BUF_PLANE0_OFFSET_EXT, EGL_DMA_BUF_PLANE0_PITCH_EXT,
          EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT },
        { EGL_DMA_BUF_PLANE1_FD_EXT, EGL_DMA_BUF_PLANE1_OFFSET_EXT, EGL_DMA_BUF_PLANE1_PITCH_EXT,
          EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT },
        { EGL_DMA_BUF_PLANE2_FD_EXT, EGL_DMA_BUF_PLANE2_OFFSET_EXT, EGL_DMA_BUF_PLANE2_PITCH_EXT,
          EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT },
        { EGL_DMA_BUF_PLANE3_FD_EXT, EGL_DMA_BUF_PLANE3_OFFSET_EXT, EGL_DMA_BUF_PLANE3_PITCH_EXT,
          EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT }
    };

    for (uint32_t i = 0; i < info.n_planes; i++) {
        ADD_ATTRIB(attr_names[i].fd, info.planes[i].fd);
        ADD_ATTRIB(attr_names[i].offset, info.planes[i].offset);
        ADD_ATTRIB(attr_names[i].pitch, info.planes[i].stride);
        if (info.modifier != DRM_FORMAT_MOD_INVALID) {
            ADD_ATTRIB(attr_names[i].mod_lo, info.modifier & 0xFFFFFFFF);
            ADD_ATTRIB(attr_names[i].mod_hi, info.modifier >> 32);
        }
    }
    ADD_ATTRIB(EGL_IMAGE_PRESERVED_KHR, EGL_TRUE);


    m_bufferFormat = info.format;

    m_eglImage = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);

    if (m_eglImage == EGL_NO_IMAGE_KHR) {
        EGLint error = eglGetError();
        const char *errorStr = "Unknown";
        switch (error) {
            case EGL_SUCCESS: errorStr = "EGL_SUCCESS"; break;
            case EGL_NOT_INITIALIZED: errorStr = "EGL_NOT_INITIALIZED"; break;
            case EGL_BAD_ACCESS: errorStr = "EGL_BAD_ACCESS"; break;
            case EGL_BAD_ALLOC: errorStr = "EGL_BAD_ALLOC"; break;
            case EGL_BAD_ATTRIBUTE: errorStr = "EGL_BAD_ATTRIBUTE"; break;
            case EGL_BAD_CONFIG: errorStr = "EGL_BAD_CONFIG"; break;
            case EGL_BAD_CONTEXT: errorStr = "EGL_BAD_CONTEXT"; break;
            case EGL_BAD_CURRENT_SURFACE: errorStr = "EGL_BAD_CURRENT_SURFACE"; break;
            case EGL_BAD_DISPLAY: errorStr = "EGL_BAD_DISPLAY"; break;
            case EGL_BAD_MATCH: errorStr = "EGL_BAD_MATCH - Format/planes/modifier mismatch"; break;
            case EGL_BAD_NATIVE_PIXMAP: errorStr = "EGL_BAD_NATIVE_PIXMAP"; break;
            case EGL_BAD_NATIVE_WINDOW: errorStr = "EGL_BAD_NATIVE_WINDOW"; break;
            case EGL_BAD_PARAMETER: errorStr = "EGL_BAD_PARAMETER - Invalid buffer attributes"; break;
            case EGL_BAD_SURFACE: errorStr = "EGL_BAD_SURFACE"; break;
            case EGL_CONTEXT_LOST: errorStr = "EGL_CONTEXT_LOST"; break;
        }
        qWarning() << "=== EGL Error ===";
        qWarning() << "  EGL error:" << Qt::hex << error << "(" << errorStr << ")";
    }
}

void KywcCaptureThumbnailItem::createTextureFromMemfd(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo &info)
{
    if (m_mmapData && !(m_bufferFlags & UkuiQuick::WaylandClient::KywcCaptureFrame::BufferFlag::Reused)) {
        munmap(m_mmapData, m_mmapSize);
        m_mmapData = nullptr;
    }

    m_mmapSize = info.size.height() * info.stride + info.offset;

    m_mmapData = static_cast<uint8_t *>(mmap(nullptr, m_mmapSize, PROT_READ, MAP_PRIVATE, info.fd, 0));

    if (m_mmapData == MAP_FAILED) {
        qWarning() << "Failed to mmap buffer:" << strerror(errno);
        qWarning() << "  This means the buffer cannot be accessed at all";
        return;
    }

    QImage::Format qtFormat = QImage::Format_ARGB32_Premultiplied;
    if (info.stride / info.size.width() == 3) {
        qtFormat = QImage::Format_RGB888;
    }

    QImage img(m_mmapData + info.offset, info.size.width(), info.size.height(), info.stride, qtFormat);
    m_qImage = img.copy();
}

QSGNode *KywcCaptureThumbnailItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    QSGTexture *texture = nullptr;
    if (m_bufferFlags & UkuiQuick::WaylandClient::KywcCaptureFrame::BufferFlag::Dmabuf) {
        if (m_eglImage == EGL_NO_IMAGE_KHR) {
            QImage placeholder(width() > 0 ? width() : 200, height() > 0 ? height() : 150,
                            QImage::Format_ARGB32_Premultiplied);
            placeholder.fill(Qt::transparent);
            texture = window()->createTextureFromImage(placeholder);
        } else {
            texture = createOpenGLTexture();
        }
    } else {
        if (m_qImage.isNull()) {
            QImage placeholder(width() > 0 ? width() : 200, height() > 0 ? height() : 150,
                            QImage::Format_ARGB32_Premultiplied);
            placeholder.fill(Qt::transparent);
            texture = window()->createTextureFromImage(placeholder);
        } else {
            QQuickWindow::CreateTextureOption textureOption = m_bufferFormat == DRM_FORMAT_ARGB8888
                                                      ? QQuickWindow::TextureHasAlphaChannel
                                                      : QQuickWindow::TextureIsOpaque;
            texture = window()->createTextureFromImage(m_qImage, textureOption);
        }
    }

    if (!texture) {
        return oldNode;
    }

    QSGImageNode *node = static_cast<QSGImageNode *>(oldNode);
    if (!node) {
        node = window()->createImageNode();
        node->setOwnsTexture(true);
        node->setFiltering(QSGTexture::Linear);
    }

    node->setTexture(texture);

    QSizeF targetSize = boundingRect().size();
    QSizeF textureSize = QSizeF(m_bufferSize.width(), m_bufferSize.height());

    if (m_firstFrame && !m_bufferSize.isEmpty()) {
        setImplicitWidth(m_bufferSize.width());
        setImplicitHeight(m_bufferSize.height());
        m_firstFrame = false;
    }

    QSizeF scaledSize = textureSize.scaled(targetSize, Qt::KeepAspectRatio);
    if (scaledSize != m_paintedSize) {
        m_paintedSize = scaledSize;
        Q_EMIT paintedSizeChanged();
    }

    QRectF rect(QPointF(), scaledSize);
    rect.moveCenter(boundingRect().center());
    node->setRect(rect);
    if (!m_connectedWindow) {
        m_connectedWindow = true;
        connect(window(), &QQuickWindow::afterRendering, this, &KywcCaptureThumbnailItem::onAfterRendering, Qt::DirectConnection);
    }
    m_renderPending = true;

    return node;
}

QSGTexture* KywcCaptureThumbnailItem::createOpenGLTexture()
{
    if (!window()) {
        return nullptr;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QOpenGLContext *context = const_cast<QOpenGLContext *>(
        static_cast<const QOpenGLContext *>(
            window()->rendererInterface()->getResource(window(), QSGRendererInterface::OpenGLContextResource)));
#else
    QOpenGLContext *context = window()->openglContext();
#endif

    if (!context || !context->isValid()) {
        qWarning() << "OpenGL context is not valid";
        return nullptr;
    }

    if (!m_texture) {
        m_texture.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
        m_texture->create();
    }

    m_texture->bind();

    static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES =
        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");

    if (glEGLImageTargetTexture2DOES) {
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)m_eglImage);
    }

    m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    m_texture->release();

    m_texture->setSize(m_bufferSize.width(), m_bufferSize.height());
    int textureId = m_texture->textureId();

    QQuickWindow::CreateTextureOption textureOption =
            m_bufferFormat == DRM_FORMAT_ARGB8888 ? QQuickWindow::TextureHasAlphaChannel
                                            : QQuickWindow::TextureIsOpaque;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto texture = window()->createTextureFromNativeObject(QQuickWindow::NativeObjectTexture,
                                                      &textureId, 0 /*a vulkan thing?*/,
                                                     m_bufferSize, textureOption);
#else
    auto texture = QNativeInterface::QSGOpenGLTexture::fromNative(textureId, window(), m_bufferSize, textureOption);
#endif
    return texture;
}

void KywcCaptureThumbnailItem::onAfterRendering()
{
    if (m_renderPending) {
        m_renderPending = false;
        QMetaObject::invokeMethod(this, "onRenderCompleted", Qt::QueuedConnection);
    }
}

void KywcCaptureThumbnailItem::onRenderCompleted()
{
    if (m_captureFrame) {
        m_captureFrame->releaseBuffer(true);
    }
}

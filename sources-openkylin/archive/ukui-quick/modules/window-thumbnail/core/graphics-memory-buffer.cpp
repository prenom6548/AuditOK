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

#include "graphics-memory-buffer.h"

#include <QDebug>
#include <qpa/qplatformnativeinterface.h>
#include <QGuiApplication>

#include <libdrm/drm_fourcc.h>
#include <limits>
#include <sys/mman.h>
#include <sys/stat.h>
#include <QQuickWindow>
#include <QOpenGLContext>
using namespace UkuiThumbnail;

GraphicsMemoryBuffer::GraphicsMemoryBuffer(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo& info)
    : m_bufferInfo(info)
{
    createResources();
}

QSGTexture* GraphicsMemoryBuffer::createTexture(const QQuickItem* item)
{
    if (!item || !item->window()) {
        return nullptr;
    }
    if (m_qsgTexture) {
        return m_qsgTexture;
    }
    return hasDmabuf() ? createTextureFromOpenGLTexture(item) : createTextureFromQImage(item);
}

bool GraphicsMemoryBuffer::releaseTextureResources()
{
    auto* context =  QOpenGLContext::currentContext();
    if (!context || !context->isValid()) {
        qWarning() << "GraphicsMemoryBuffer::releaseTextureResources failed, context is null or invalid";
        return false;
    }
    if (m_qsgTexture) {
        delete m_qsgTexture;
        m_qsgTexture = nullptr;
    }
    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }
    if (m_image != EGL_NO_IMAGE_KHR) {
        static auto eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
        eglDestroyImageKHR(m_display, m_image);
        m_image = EGL_NO_IMAGE_KHR;
    }
    return true;
}
#define ADD_ATTRIB(name, value)                                                                    \
do {                                                                                           \
attribs[num_attribs++] = (name);                                                           \
attribs[num_attribs++] = (value);                                                          \
attribs[num_attribs] = EGL_NONE;                                                           \
} while (0)

bool GraphicsMemoryBuffer::createDmabufResources()
{
    if (m_bufferInfo.size.width() <= 0 || m_bufferInfo.size.height() <= 0
        || m_bufferInfo.n_planes <= 0 || m_bufferInfo.n_planes > 4) {
        qWarning() << "Invalid DMA-BUF thumbnail metadata";
        return false;
    }

    auto display  = static_cast<EGLDisplay>(
            QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("egldisplay"));

    if (display == EGL_NO_DISPLAY) {
        qWarning() << "egl get display failed ";
        return false;
    }
    m_display = display;

    EGLint attribs[50] = { EGL_NONE };
    int num_attribs = 0;

    ADD_ATTRIB(EGL_WIDTH, m_bufferInfo.size.width());
    ADD_ATTRIB(EGL_HEIGHT, m_bufferInfo.size.height());
    ADD_ATTRIB(EGL_LINUX_DRM_FOURCC_EXT, m_bufferInfo.format);

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

    constexpr uint32_t maxEglInt = static_cast<uint32_t>(std::numeric_limits<EGLint>::max());
    for (int i = 0; i < m_bufferInfo.n_planes; i++) {
        const auto &plane = m_bufferInfo.planes[i];
        if (plane.fd < 0 || plane.offset > maxEglInt || plane.stride == 0 || plane.stride > maxEglInt) {
            qWarning() << "Invalid DMA-BUF plane metadata";
            return false;
        }

        ADD_ATTRIB(attr_names[i].fd, plane.fd);
        ADD_ATTRIB(attr_names[i].offset, plane.offset);
        ADD_ATTRIB(attr_names[i].pitch, plane.stride);
        if (m_bufferInfo.modifier != DRM_FORMAT_MOD_INVALID) {
            ADD_ATTRIB(attr_names[i].mod_lo, m_bufferInfo.modifier & 0xFFFFFFFF);
            ADD_ATTRIB(attr_names[i].mod_hi, m_bufferInfo.modifier >> 32);
        }
    }
    ADD_ATTRIB(EGL_IMAGE_PRESERVED_KHR, EGL_TRUE);

    static auto eglCreateImageKHR =
            (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    Q_ASSERT(eglCreateImageKHR);
    m_image = eglCreateImageKHR(m_display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, attribs);
    if (m_image == EGL_NO_IMAGE_KHR) {
        qWarning() << "invalid eglCreateImageKHR" << eglGetError();
        return false;
    }
    return true;
}

bool GraphicsMemoryBuffer::createMemfdResources()
{
    const int width = m_bufferInfo.size.width();
    const int height = m_bufferInfo.size.height();
    const quint64 stride = m_bufferInfo.stride;
    const quint64 offset = m_bufferInfo.offset;
    const int fd = m_bufferInfo.fd;
    const quint64 maximumSize = std::numeric_limits<size_t>::max();

    if (fd < 0 || width <= 0 || height <= 0 || stride == 0
        || stride > static_cast<quint64>(std::numeric_limits<int>::max())) {
        qWarning() << "Invalid thumbnail buffer dimensions";
        return false;
    }

    const QImage::Format format = stride / static_cast<quint64>(width) == 3
            ? QImage::Format_RGB888 : QImage::Format_ARGB32;
    const quint64 bytesPerPixel = format == QImage::Format_RGB888 ? 3 : 4;
    if (stride < static_cast<quint64>(width) * bytesPerPixel
        || offset > maximumSize
        || static_cast<quint64>(height) > (maximumSize - offset) / stride) {
        qWarning() << "Invalid thumbnail buffer layout";
        return false;
    }

    const quint64 dataSizeValue = offset + static_cast<quint64>(height) * stride;
    if (dataSizeValue > maximumSize) {
        qWarning() << "Invalid thumbnail buffer size";
        return false;
    }
    const size_t dataSize = static_cast<size_t>(dataSizeValue);

    struct stat bufferStat;
    if (fstat(fd, &bufferStat) != 0 || bufferStat.st_size < 0
        || static_cast<quint64>(bufferStat.st_size) < dataSize) {
        qWarning() << "Invalid thumbnail buffer file";
        return false;
    }

    void *map = mmap(nullptr, dataSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED || !map) {
        qWarning() << "Failed to mmap the memory: " << strerror(errno);
        return false;
    }

    const QImage image(static_cast<uchar *>(map) + static_cast<size_t>(offset), width, height,
                       static_cast<int>(stride), format);
    m_qImage = image.copy();
    munmap(map, dataSize);
    return !m_qImage.isNull();
}

bool GraphicsMemoryBuffer::createResources()
{
    //创建 Texture 需要的资源
    if (isReused()) {
        if ((hasDmabuf() && m_image != EGL_NO_IMAGE_KHR) ||
            (!hasDmabuf() && !m_qImage.isNull())) {
            return true;
        }
    }
    if (hasDmabuf()) {
        return createDmabufResources();
    }
    return createMemfdResources();
}

QSGTexture* GraphicsMemoryBuffer::createTextureFromOpenGLTexture(const QQuickItem* item)
{
    const auto* window = item->window();
    if (!window) {
        return nullptr;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const auto context = window->openglContext();
#else
    const auto context = static_cast<QOpenGLContext*>(window->rendererInterface()->getResource(
        window(), QSGRendererInterface::OpenGLContextResource));
#endif
    if (!context || !context->isValid()) {
        qWarning() << "OpenGL context is not valid.";
        return nullptr;
    }

    // 创建OpenGL纹理
    if (!m_texture) {
        m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        if (!m_texture->create()) {
            qWarning() << "Failed to create OpenGL texture";
            GraphicsMemoryBuffer::releaseTextureResources();
            return nullptr;
        }
    }
    // 将EGL图像绑定到纹理
    static auto s_glEGLImageTargetTexture2DOES =
        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!s_glEGLImageTargetTexture2DOES) {
        qWarning() << "glEGLImageTargetTexture2DOES is not available";
        return nullptr;
    }
    m_texture->bind();
    s_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, static_cast<GLeglImageOES>(m_image));
    m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    m_texture->release();
    m_texture->setSize(item->boundingRect().width(), item->boundingRect().height());

    int textureId = m_texture->textureId();

    QQuickWindow::CreateTextureOption textureOption =
            m_bufferInfo.format == DRM_FORMAT_ARGB8888 ? QQuickWindow::TextureHasAlphaChannel
                                            : QQuickWindow::TextureIsOpaque;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_qsgTexture = window->createTextureFromNativeObject(QQuickWindow::NativeObjectTexture,
                                                         &textureId, 0 /*a vulkan thing?*/,
                                                         m_bufferInfo.size, textureOption);
#else
    m_qsgTexture = QNativeInterface::QSGOpenGLTexture::fromNative(textureId, window, m_bufferInfo.size, textureOption);
#endif
    return m_qsgTexture;
}

QSGTexture* GraphicsMemoryBuffer::createTextureFromQImage(const QQuickItem* item)
{
    const auto* window = item->window();
    if (!window) {
        return nullptr;
    }
    m_qsgTexture = window->createTextureFromImage(m_qImage, QQuickWindow::TextureIsOpaque);
    return m_qsgTexture;
}


bool GraphicsMemoryBuffer::hasDmabuf() const
{
    return m_bufferInfo.flags.testFlag(UkuiQuick::WaylandClient::KywcCaptureFrame::BufferFlag::Dmabuf);
}

bool GraphicsMemoryBuffer::isReused() const
{
    return m_bufferInfo.flags.testFlag(UkuiQuick::WaylandClient::KywcCaptureFrame::BufferFlag::Reused);
}

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
#ifndef UKUI_QUICK_GRAPHICSMEMORYBUFFER_H
#define UKUI_QUICK_GRAPHICSMEMORYBUFFER_H


#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <QOpenGLTexture>
#include <QMutexLocker>
#include "thumbnail-buffer.h"
#include "wayland/kywc-capture-manager.h"
namespace UkuiThumbnail
{
    class GraphicsMemoryBuffer : public ThumbnailBuffer
    {
    public:
        // buffer info 过多时，可用桥接模式
        GraphicsMemoryBuffer(const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo &info);
        ~GraphicsMemoryBuffer() override = default;
        QSGTexture* createTexture(const QQuickItem* item) override;
        bool releaseTextureResources() override;
    private:
        bool createDmabufResources();
        bool createMemfdResources();
        bool createResources();
        QSGTexture* createTextureFromOpenGLTexture(const QQuickItem* item);
        QSGTexture* createTextureFromQImage(const QQuickItem* item);

        bool hasDmabuf() const;
        bool isReused() const;

        const UkuiQuick::WaylandClient::KywcCaptureFrame::BufferInfo m_bufferInfo;
        EGLImage m_image = EGL_NO_IMAGE_KHR;
        EGLDisplay m_display = EGL_NO_DISPLAY;
        QOpenGLTexture *m_texture = nullptr;
        QPointer<QSGTexture> m_qsgTexture;
        QImage m_qImage;

    };
}
#endif //UKUI_QUICK_GRAPHICSMEMORYBUFFER_H

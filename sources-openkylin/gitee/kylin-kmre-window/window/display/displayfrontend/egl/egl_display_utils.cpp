/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "egl_display_utils.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QGuiApplication>
#include <QThread>
#include <QSize>

#include <qpa/qplatformnativeinterface.h>

#include <QtEglSupport/private/qeglconvenience_p.h>
#include <QtPlatformHeaders/QEGLNativeContext>

#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xf86drm.h>
#include <drm_fourcc.h>

#include <sys/syslog.h>

#ifndef RENDER_TEXTURE_TO_FRAMEBUFFER
//#define RENDER_TEXTURE_TO_FRAMEBUFFER
#endif

struct EglDisplayUtils::Data
{
    Data(DisplayWidget* widget)
        : widget(widget)
    {}

    std::map<int, sp<Texture>> textures;
    sp<Texture> currentTexture;

    sp<QOpenGLContext> context;
    sp<QOffscreenSurface> surface;
    DisplayWidget* widget;

    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLContext eglContext = EGL_NO_CONTEXT;
    bool initialized = false;

#ifdef RENDER_TEXTURE_TO_FRAMEBUFFER
    GLuint tex = 0;
    QSize framebufferSize;
    sp<QOpenGLFramebufferObject> renderFbo;
    sp<QOpenGLFramebufferObject> displayFbo;
    bool fboInitialized = false;
    sp<QOpenGLShaderProgram> shaderProgram;
    sp<QOpenGLBuffer> vbo;
    sp<QOpenGLBuffer> ebo;
#endif
};

#ifdef RENDER_TEXTURE_TO_FRAMEBUFFER
static QByteArray versionedShaderCode(const char *src)
{
    QByteArray versionedSrc;

    if (QOpenGLContext::currentContext()->isOpenGLES())
        versionedSrc.append(QByteArrayLiteral("#version 100\nprecision mediump float;\n"));
    else
        versionedSrc.append(QByteArrayLiteral("#version 330\n"));

    versionedSrc.append(src);
    return versionedSrc;
}

static void makeObjectForFramebufferObject(sp<EglDisplayUtils::Data> data)
{
    if (!data->context) {
        return;
    }

    QVector<GLfloat> vertData;

    // Image for framebuffer object is up-side-down flipped.
    // For orientation 0, texture coordination
    // (0.0, 0.0) <-> (0.0, 1.0) and (1.0, 1.0) <-> (1.0, 0.0)
    vertData << -1.0f << 1.0f  << 0.0f << 0.0f << 1.0f;
    vertData << -1.0f << -1.0f << 0.0f << 0.0f << 0.0f;
    vertData << 1.0f  << -1.0f << 0.0f << 1.0f << 0.0f;
    vertData << 1.0f  << 1.0f  << 0.0f << 1.0f << 1.0f;

    if (data->vbo) {
        if (data->vbo->isCreated()) {
            data->vbo->destroy();
        }
        data->vbo.reset();
    }
    data->vbo = std::make_shared<QOpenGLBuffer>();
    data->vbo->create();
    data->vbo->bind();
    data->vbo->allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));

    QVector<GLushort> indices;
    indices << 0 << 1 << 2 << 0 << 2 << 3;

    if (data->ebo) {
        if (data->ebo->isCreated()) {
            data->ebo->destroy();
        }
        data->ebo.reset();
    }
    data->ebo = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
    data->ebo->create();
    data->ebo->bind();
    data->ebo->allocate(indices.constData(), indices.count() * sizeof(GLushort));

}

static void initializeRenderFboData(sp<EglDisplayUtils::Data> data)
{
    if (!data->context) {
        return;
    }

    makeObjectForFramebufferObject(data);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const  char  vShaderStr[] =
        "attribute vec3 a_position;                 \n"
        "attribute vec2 a_texCoord;                 \n"
        "varying vec2 v_texCoord;                   \n"
        "void main()                                \n"
        "{                                          \n"
        "   gl_Position = vec4(a_position, 1.0);    \n"
        "   v_texCoord = a_texCoord;                \n"
        "}                                          \n";

    const  char fShaderStr[] =
        "varying vec2 v_texCoord;                            \n"
        "uniform sampler2D s_texture;                        \n"
        "void main()                                         \n"
        "{                                                   \n"
        "  gl_FragColor = texture2D(s_texture, v_texCoord);  \n"
        "}                                                   \n";

    if (data->shaderProgram) {
        data->shaderProgram.reset();
    }

    data->shaderProgram = std::make_shared<QOpenGLShaderProgram>();
    data->shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, versionedShaderCode(vShaderStr));
    data->shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, versionedShaderCode(fShaderStr));
    data->shaderProgram->link();
    data->shaderProgram->bind();
    data->shaderProgram->setUniformValue("s_texture", 0);
}

static void renderFramebuffer(sp<EglDisplayUtils::Data> data)
{
    if (!data->context) {
        return;
    }

    if (!data->currentTexture) {
        return;
    }

    if (!data->renderFbo || !data->displayFbo) {
        return;
    }

    GLuint textureId = 0;
    int offset = 0;
    int stride = 5 * sizeof(GLfloat);

    data->renderFbo->bind();

    glViewport(0, 0, data->framebufferSize.width(), data->framebufferSize.height());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    data->vbo->bind();
    data->ebo->bind();

    offset = 0;

    data->shaderProgram->bind();
    data->shaderProgram->enableAttributeArray(0);
    data->shaderProgram->setAttributeBuffer(0, GL_FLOAT, offset, 3, stride);

    offset += (3 * sizeof(GLfloat));
    data->shaderProgram->enableAttributeArray(1);
    data->shaderProgram->setAttributeBuffer(1, GL_FLOAT, offset, 2, stride);

    textureId = data->currentTexture->getTextureId();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glFlush();

    data->renderFbo->release();

    data->tex = data->displayFbo->texture();

    std::swap(data->renderFbo, data->displayFbo);
}

#endif

EglDisplayUtils::EglDisplayUtils(DisplayWidget *widget, QObject* parent)
    : QObject(parent),
      mData(nullptr)
{
    mData = std::make_shared<Data>(widget);
}

EglDisplayUtils::~EglDisplayUtils()
{
    destroyEglContext();
}

bool EglDisplayUtils::initializeUtils()
{
    // initializeUtils runs in work thread of DisplayWorkManager
    QThread* currentThread = QThread::currentThread();

    if (mData && mData->initialized) {
        return true;
    }

    EGLContext shareCtx = EGL_NO_CONTEXT;

    QVariant nativeHandle;
    if (!mData->widget) {
        syslog(LOG_ERR, "EglDisplayUtils: Requires a display widget.");
        return false;
    }

    QOpenGLContext* ctx = mData->widget->context();
    if (!ctx) {
        syslog(LOG_ERR, "EglDisplayUtils: Requires a QOpenGLContext.");
        return false;
    }

    nativeHandle = mData->widget->context()->nativeHandle();
    QString platformName = qGuiApp->platformName();
    qDebug() << "platformName: " << platformName;
    if (platformName == "wayland" || platformName == "wayland-egl") {
        shareCtx = (EGLContext) qGuiApp->platformNativeInterface()->nativeResourceForContext(QByteArrayLiteral("eglcontext"), mData->widget->context());
    } else {
        if (!nativeHandle.canConvert<QEGLNativeContext>()) {
            syslog(LOG_ERR, "EglDisplayUtils: Requires a QEGLNativeContext");
            return false;
        }
        shareCtx = nativeHandle.value<QEGLNativeContext>().context();
    }

    mData->eglDisplay = (EGLDisplay) qGuiApp->platformNativeInterface()->nativeResourceForIntegration(
        QByteArrayLiteral("egldisplay"));
    if (!mData->eglDisplay) {
        syslog(LOG_ERR, "EglDisplayUtils: Requires a EGLDisplay.");
        return false;
    }

    QSurfaceFormat fmt = mData->widget->format();
    EGLConfig config = q_configFromGLFormat(mData->eglDisplay, fmt);
    if (config == EGL_NO_CONFIG_KHR) {
        syslog(LOG_ERR, "EglDisplayUtils: Failed to get egl config.");
        return false;
    }

    QVector<EGLint> contextAttrs;
    contextAttrs.append(EGL_CONTEXT_CLIENT_VERSION);
    contextAttrs.append(fmt.majorVersion());
    contextAttrs.append(EGL_NONE);

    eglBindAPI(EGL_OPENGL_ES_API);

    mData->eglContext = eglCreateContext(mData->eglDisplay, config, shareCtx, contextAttrs.constData());
    mData->context = std::make_shared<QOpenGLContext>();
    mData->context->setNativeHandle(
        QVariant::fromValue<QEGLNativeContext>(QEGLNativeContext(mData->eglContext, mData->eglDisplay)));
    mData->context->setShareContext(mData->widget->context());
    mData->context->setFormat(mData->widget->format());
    mData->context->moveToThread(currentThread);
    mData->context->create();

    mData->surface = std::make_shared<QOffscreenSurface>();
    mData->surface->setFormat(mData->context->format());
    mData->context->moveToThread(currentThread);
    mData->surface->create();

    mData->initialized = true;

    return true;
}

void EglDisplayUtils::importImage(int name, int width, int height, int stride, int bpp)
{
    if (mData->currentTexture) {
        mData->currentTexture.reset();
    }

    if (!mData->initialized) {
        return;
    }

    if (!mData->context || !mData->surface) {
        return;
    }

    auto texture = mData->textures.find(name);
    if (texture != mData->textures.end()) {
        mData->currentTexture = texture->second;
        return;
    }

    if (mData->textures.size() > 3) {
        syslog(LOG_WARNING, "[%s] Too many textures! Clear it now.", __func__);
        mData->textures.clear();
    }

    mData->context->makeCurrent(mData->surface.get());
    Texture::ImageBuf imageBuf = { name, width, height, stride, bpp };
    mData->currentTexture = std::make_shared<Texture>(
        mData->eglDisplay, mData->eglContext, std::move(imageBuf));
    mData->textures[name] = mData->currentTexture;

    mData->context->doneCurrent();
}

void EglDisplayUtils::updateImage(int name, int width, int height, int stride, int bpp)
{
    importImage(name, width, height, stride, bpp);

#ifdef RENDER_TEXTURE_TO_FRAMEBUFFER
    mData->context->makeCurrent(mData->surface.get());
    if (!mData->fboInitialized) {
        GLuint textureId = 0;
        mData->framebufferSize = QSize(width, height);
        mData->renderFbo = std::make_shared<QOpenGLFramebufferObject>(mData->framebufferSize);
        /*
         * Default GL_TEXTURE_MIN_FILTER and GL_TEXTURE_MAG_FILTER for texture of QOpenGLFramebuffer is GL_NEAREST,
         * we need to set it to GL_LINEAR here.
         */
        textureId = mData->renderFbo->texture();
        if (textureId != 0) {
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        mData->displayFbo = std::make_shared<QOpenGLFramebufferObject>(mData->framebufferSize);
        textureId = mData->displayFbo->texture();
        if (textureId != 0) {
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        initializeRenderFboData(mData);
        mData->fboInitialized = true;
    }

    renderFramebuffer(mData);

    mData->context->doneCurrent();
#endif
}

void EglDisplayUtils::drawDisplay()
{
    if (mData->widget) {
        QMetaObject::invokeMethod(mData->widget, "update");
    }
}

void EglDisplayUtils::destroyEglContext()
{
    if (!mData->initialized) {
        return;
    }

    if (!mData->context || !mData->surface) {
        return;
    }

    mData->context->makeCurrent(mData->surface.get());
    mData->currentTexture.reset();
    mData->textures.clear();
#ifdef RENDER_TEXTURE_TO_FRAMEBUFFER
    mData->renderFbo.reset();
    mData->displayFbo.reset();
    mData->vbo->destroy();
    mData->vbo.reset();
    mData->ebo->destroy();
    mData->ebo.reset();
    mData->shaderProgram.reset();
    mData->tex = 0;
#endif
    mData->context->doneCurrent();
    mData->context.reset();
    mData->surface.reset();

    eglDestroyContext(mData->eglDisplay, mData->eglContext);
    mData->initialized = false;
}

GLuint EglDisplayUtils::framebufferTextureId()
{
#ifdef RENDER_TEXTURE_TO_FRAMEBUFFER
    if (mData->tex != 0) {
        return mData->tex;
    }
#endif

    if (mData->currentTexture) {
        return mData->currentTexture->getTextureId();
    }

    return 0;
}

QSize EglDisplayUtils::framebufferSize()
{
    if (mData->currentTexture) {
        int width = mData->currentTexture->getWidth();
        int height = mData->currentTexture->getHeight();
        return QSize(width, height);
    }

    return QSize(0, 0);
}

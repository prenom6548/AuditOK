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

#include "egldisplaywidget.h"
#include "egl/egl_display_work_manager.h"
#include "egl/egl_display_helper.h"
#include "displaybackend/displaybackend.h"
#include "kmrewindow.h"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QPainter>
#include <QSize>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syslog.h>

#include <cmath>

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

static inline GLint offsetFromOrientation(int orientation)
{
    GLint result = 0;
    switch(orientation) {
        case 0:
            result = 0;
            break;
        case 1:
            result = 20 * sizeof(GLfloat);
            break;
        case 2:
            result = 40 * sizeof(GLfloat);
            break;
        case 3:
            result = 60 * sizeof(GLfloat);
            break;
        default:
            result = 0;
            break;
    }

    return result;
}

EglDisplayWidget::EglDisplayWidget(KmreWindow* window, int id, int width, int height, int orientation, const QString& displayName)
    : DisplayWidget(window, id, width, height, orientation, displayName),
      mDefaultShaderProgram(nullptr),
      mBlurShaderProgram(nullptr),
      mCurrentShaderProgram(nullptr),
      mCurrentOrientation(orientation),
      ebo(QOpenGLBuffer::IndexBuffer)
{

}


EglDisplayWidget::~EglDisplayWidget()
{
    makeCurrent();
    vbo.destroy();
    ebo.destroy();

    if (mDefaultShaderProgram) {
        delete mDefaultShaderProgram;
    }

    if (mBlurShaderProgram) {
        delete mBlurShaderProgram;
    }

    doneCurrent();
}

void EglDisplayWidget::setOrientation(int orientation)
{
    mCurrentOrientation = orientation;
}

void EglDisplayWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
}

void EglDisplayWidget::initializeGL()
{
    initializeOpenGLFunctions();

    makeObject();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);


    const  char  default_vShaderStr[] =
        "attribute vec3 a_position;                 \n"
        "attribute vec2 a_texCoord;                 \n"
        "varying vec2 v_texCoord;                   \n"
        "void main()                                \n"
        "{                                          \n"
        "   gl_Position = vec4(a_position, 1.0);    \n"
        "   v_texCoord = a_texCoord;                \n"
        "}                                          \n";

    const  char default_fShaderStr[] =
        "varying vec2 v_texCoord;                            \n"
        "uniform sampler2D s_texture;                        \n"
        "void main()                                         \n"
        "{                                                   \n"
        "  gl_FragColor = texture2D(s_texture, v_texCoord);  \n"
        "}                                                   \n";

    const char blur_fShaderStr[] =
        "precision mediump float;                                                                           \n"
        "varying vec2 v_texCoord;                                                                           \n"
        "uniform sampler2D s_texture;                                                                       \n"
        "uniform float blur_size;                                                                           \n"
        // 注意这里 kernel_size 的大小一定要和 GAUSS_KERNEL_SIZE 一致 ！
        "const int kernel_size = 9;                                                                         \n"
        "uniform float kernel[kernel_size];                                                                 \n"
        "void main()                                                                                        \n"
        "{                                                                                                  \n"
        "   vec4 color = vec4(0.0);                                                                         \n"
        "   for (int i = 0; i < kernel_size; i++)                                                           \n"
        "   {                                                                                               \n"
        "       for (int j = 0; j < kernel_size; j++)                                                       \n"
        "       {                                                                                           \n"
        "           vec2 offset = vec2(float(i - kernel_size / 2), float(j - kernel_size / 2)) * blur_size; \n"
        "           color += texture2D(s_texture, v_texCoord + offset) * kernel[i] * kernel[j];             \n"
        "       }                                                                                           \n"
        "   }                                                                                               \n"
        "   gl_FragColor = color;                                                                           \n"
        "}                                                                                                  \n";

    if (mDefaultShaderProgram) {
        delete mDefaultShaderProgram;
        mDefaultShaderProgram = nullptr;
    }

    if (mBlurShaderProgram) {
        delete mBlurShaderProgram;
        mBlurShaderProgram = nullptr;
    }

    mDefaultShaderProgram = new QOpenGLShaderProgram;
    mDefaultShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, versionedShaderCode(default_vShaderStr));
    mDefaultShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, versionedShaderCode(default_fShaderStr));
    mDefaultShaderProgram->link();
    mDefaultShaderProgram->bind();
    mDefaultShaderProgram->setUniformValue("s_texture", 0);
    mDefaultShaderProgram->release();

    mCurrentShaderProgram = mDefaultShaderProgram;

    if (isDDSSupported()) {
        mBlurShaderProgram = new QOpenGLShaderProgram;
        mBlurShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, versionedShaderCode(default_vShaderStr));
        mBlurShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, versionedShaderCode(blur_fShaderStr));
        mBlurShaderProgram->link();
        mBlurShaderProgram->bind();
        mBlurShaderProgram->setUniformValue("s_texture", 0);
        mBlurShaderProgram->release();
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void EglDisplayWidget::paintGL()
{
    if (mIsFirstShow) {
        QPainter p(this);
        // draw background
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);//default
        p.drawRect(rect());
        // draw icon
        QPixmap pixmap(mMainWindow->getIconPath());
        if (!pixmap.isNull()) {
            QSize widgetSize = rect().size();
            QSize pixmapSize(80, 80);
            int x = (widgetSize.width() - pixmapSize.width()) / 2;
            int y = (widgetSize.height() - pixmapSize.height()) / 2;
            p.drawPixmap(x, y, pixmapSize.width(), pixmapSize.height(), pixmap);
        }
        mIsFirstShow = false;
    }

    drawDisplay();
}

void EglDisplayWidget::makeObject()
{
    QVector<GLfloat> vertData;
    // orientation 0
    vertData << -1.0f << 1.0f  << 0.0f << 0.0f << 0.0f;
    vertData << -1.0f << -1.0f << 0.0f << 0.0f << 1.0f;
    vertData << 1.0f  << -1.0f << 0.0f << 1.0f << 1.0f;
    vertData << 1.0f  << 1.0f  << 0.0f << 1.0f << 0.0f;

    // orientation 1
    vertData << -1.0f << 1.0f  << 0.0f << 1.0f << 0.0f;
    vertData << -1.0f << -1.0f << 0.0f << 0.0f << 0.0f;
    vertData << 1.0f  << -1.0f << 0.0f << 0.0f << 1.0f;
    vertData << 1.0f  << 1.0f  << 0.0f << 1.0f << 1.0f;

    // orientation 2
    vertData << -1.0f << 1.0f  << 0.0f << 1.0f << 1.0f;
    vertData << -1.0f << -1.0f << 0.0f << 1.0f << 0.0f;
    vertData << 1.0f  << -1.0f << 0.0f << 0.0f << 0.0f;
    vertData << 1.0f  << 1.0f  << 0.0f << 0.0f << 1.0f;

    // orientation 3
    vertData << -1.0f << 1.0f  << 0.0f << 0.0f << 1.0f;
    vertData << -1.0f << -1.0f << 0.0f << 1.0f << 1.0f;
    vertData << 1.0f  << -1.0f << 0.0f << 1.0f << 0.0f;
    vertData << 1.0f  << 1.0f  << 0.0f << 0.0f << 0.0f;

    vbo.create();
    vbo.bind();
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
    vbo.release();
    memcpy(mVertices, vertData.constData(), vertData.count() * sizeof(GLfloat));

    QVector<GLushort> indices;
    indices << 0 << 1 << 2 << 0 << 2 << 3;

    ebo.create();
    ebo.bind();
    ebo.allocate(indices.constData(), indices.count() * sizeof(GLushort));
    ebo.release();
}

void EglDisplayWidget::drawDisplay()
{
    GLuint textureId = 0;
    int offset = 0;
    int stride = 5 * sizeof(GLfloat);
    QSize framebufferSize(0, 0);
    bool blurEnabled = false;

    EglDisplayWorkManager* eglDisplayWorkManager = nullptr;
    sp<DisplayWorkManager> displayWorkManager = nullptr;
    EglDisplayHelper* eglDisplayHelper = nullptr;

    displayWorkManager = DisplayBackend::getInstance()->getWorkManagerForDisplay(getDisplayId());
    if (displayWorkManager) {
        eglDisplayWorkManager = dynamic_cast<EglDisplayWorkManager*>(displayWorkManager.get());
    }

    if (!eglDisplayWorkManager) {
        return;
    }

    eglDisplayHelper = eglDisplayWorkManager->getDisplayHelper();
    if (!eglDisplayHelper) {
        return;
    }
    
    eglDisplayHelper->lock();
    textureId = eglDisplayHelper->framebufferTextureId();
    
    if (textureId == 0) {
        goto out;
    }

    if (isDDSSupported()) {
        framebufferSize = eglDisplayHelper->framebufferSize();
        if (framebufferSize.width() == 0 || framebufferSize.height() == 0) {
            goto out;
        }

        blurEnabled = eglDisplayHelper->blurEnabled();
    }

    makeCurrent();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (isDDSSupported()) {
        if (blurEnabled) {
            mCurrentShaderProgram = mBlurShaderProgram;
        } else {
            mCurrentShaderProgram = mDefaultShaderProgram;
        }
        updateVbo(framebufferSize, blurEnabled);
        updateBlurShaderData(framebufferSize);
    }

    vbo.bind();
    ebo.bind();

    offset = offsetFromOrientation(mCurrentOrientation);

    mCurrentShaderProgram->bind();
    mCurrentShaderProgram->enableAttributeArray(0);
    mCurrentShaderProgram->setAttributeBuffer(0, GL_FLOAT, offset, 3, stride);

    offset += (3 * sizeof(GLfloat));

    mCurrentShaderProgram->enableAttributeArray(1);
    mCurrentShaderProgram->setAttributeBuffer(1, GL_FLOAT, offset, 2, stride);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    
    doneCurrent();

out:
    eglDisplayHelper->unlock();
}

void EglDisplayWidget::updateVbo(const QSize &framebufferSize, bool enableBlur)
{
    int index = mCurrentOrientation;
    int offset = sizeof(GLfloat) * VERTICE_DATA_SIZE * index;
    QSize showSize;
    if (enableBlur) {
        showSize = getWidgetMinSize();
        if (showSize == QSize(0, 0)) {
            showSize = getWidgetRealSize();
        }
    } else {
        showSize = getWidgetRealSize();
    }
    int showWidth = showSize.width();
    int showHeight = showSize.height();
    int imageWidth = framebufferSize.width();
    int imageHeight = framebufferSize.height();

    GLfloat widthRatio = (GLfloat)showWidth / (GLfloat)(imageWidth);
    GLfloat heightRatio = (GLfloat)showHeight / (GLfloat)(imageHeight);

    if ((widthRatio > 0) && (widthRatio <= 1) && (heightRatio > 0) && (heightRatio <= 1)) {
        mVertices[eDegree_0][3]    = 0.0f;
        mVertices[eDegree_0][4]    = 0.0f;
        mVertices[eDegree_0][8]    = 0.0f;
        mVertices[eDegree_0][9]    = heightRatio;
        mVertices[eDegree_0][13]   = widthRatio;
        mVertices[eDegree_0][14]   = heightRatio;
        mVertices[eDegree_0][18]   = widthRatio;
        mVertices[eDegree_0][19]   = 0.0f;

        mVertices[eDegree_90][3]    = widthRatio;
        mVertices[eDegree_90][4]    = 0.0f;
        mVertices[eDegree_90][8]    = 0.0f;
        mVertices[eDegree_90][9]    = 0.0f;
        mVertices[eDegree_90][13]   = 0.0f;
        mVertices[eDegree_90][14]   = heightRatio;
        mVertices[eDegree_90][18]   = widthRatio;
        mVertices[eDegree_90][19]   = heightRatio;

        mVertices[eDegree_180][3]    = widthRatio;
        mVertices[eDegree_180][4]    = heightRatio;
        mVertices[eDegree_180][8]    = widthRatio;
        mVertices[eDegree_180][9]    = 0.0f;
        mVertices[eDegree_180][13]   = 0.0f;
        mVertices[eDegree_180][14]   = 0.0f;
        mVertices[eDegree_180][18]   = 0.0f;
        mVertices[eDegree_180][19]   = heightRatio;

        mVertices[eDegree_270][3]    = 0.0f;
        mVertices[eDegree_270][4]    = heightRatio;
        mVertices[eDegree_270][8]    = widthRatio;
        mVertices[eDegree_270][9]    = heightRatio;
        mVertices[eDegree_270][13]   = widthRatio;
        mVertices[eDegree_270][14]   = 0.0f;
        mVertices[eDegree_270][18]   = 0.0f;
        mVertices[eDegree_270][19]   = 0.0f;

        vbo.bind();
        vbo.write(offset, mVertices[index], sizeof(mVertices[0]));
    }
}

static void generateGaussKernel(float *kernelBuf, int size, float sigma)
{
    float sum = 0.f;

    // generate gauss kernel
    for (int i = 0; i < size; i++) {
        float x = (float)i - (size / 2.f);
        kernelBuf[i] = exp(-(x * x) / (2.0 * sigma * sigma));
        sum += kernelBuf[i];
    }
    // normalize gauss kernel
    for (int i = 0; i < size; i++) {
        kernelBuf[i] /= sum;
    }
}

void EglDisplayWidget::updateBlurShaderData(const QSize& framebufferSize)
{
    float blurSize = 0.001;

    if (!mBlurShaderProgram) {
        return;
    }

    mBlurShaderProgram->bind();

    generateGaussKernel(mKernel, GAUSS_KERNEL_SIZE, GAUSS_KERNEL_SIGMA);
    mBlurShaderProgram->setUniformValueArray("kernel", mKernel, GAUSS_KERNEL_SIZE, 1);

    blurSize = 1.f / (framebufferSize.width() + framebufferSize.height()) * 2;
    mBlurShaderProgram->setUniformValue("blur_size", blurSize);
}

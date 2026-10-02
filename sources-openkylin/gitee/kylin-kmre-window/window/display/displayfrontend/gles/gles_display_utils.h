/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#ifndef GLES_DISPLAY_UTILS_H
#define GLES_DISPLAY_UTILS_H

#include <stdint.h>
#include <QObject>
#include <memory>
#include <vector>
#include <X11/Xlib.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include "typedef.h"

#define VERTICE_DATA_SIZE 20
#define GAUSS_KERNEL_SIZE 9 // 修改该值时需同步修改着色器中的对应值！
#define GAUSS_KERNEL_SIGMA 10

class Shader;
class Texture;

class GLESDisplayUtils : public QObject
{
public:
    GLESDisplayUtils(QObject *parent = nullptr);
    ~GLESDisplayUtils();

    enum RotateDegree {
        eDegree_0 = 0,
        eDegree_90,
        eDegree_180,
        eDegree_270,
        eDegree_Max
    };

    void drawDisplay(int width, int height);
    void setOrientation(int orientation);
    void updateImage(uint32_t name, int width, int height, int stride, int bpp);
    void updateShowRegion(int image_width, int image_height, int show_width, int show_height);
    void enableBlur(bool enable);
    bool initGles(unsigned long winId);
    void destroyGles();

private:
    bool initContext(unsigned long winId);
    bool attachWindow(EGLNativeWindowType winId);
    EGLint getContextRenderableType();
    void bindVerticeData();
    void updateBlurShaderData();

private:
    struct UserData {
        GLuint vbo;
        GLuint ebo;
        kmre::sp<Shader> defaultShader;
        kmre::sp<Shader> blurShader;
        kmre::sp<Shader> currentShader;
        kmre::sp<Texture> currentTexture;
        std::vector<kmre::sp<Texture>> textures;
        RotateDegree currentRotate;
        GLfloat vertices[eDegree_Max][VERTICE_DATA_SIZE];
        GLfloat kernel[GAUSS_KERNEL_SIZE];
        bool verticeChanged;
    }mUserData;

    struct Context {
        EGLNativeDisplayType eglNativeDisplay;
        EGLNativeWindowType  eglNativeWindow;
        EGLDisplay  eglDisplay;
        EGLContext  eglContext;
        EGLSurface  eglSurface;
    } mContext;
};

#endif // GLES_DISPLAY_UTILS_H

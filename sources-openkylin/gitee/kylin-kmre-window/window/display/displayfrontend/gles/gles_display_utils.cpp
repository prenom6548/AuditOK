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

#include "gles_display_utils.h"
#include "shader.h"
#include "texture.h"

#include <GLES2/gl2ext.h>
#include <EGL/eglext.h>
#include <sys/syslog.h>
#include <vector>
#include <cmath>

using namespace kmre;

const char defaultVShaderStr[] =
    "attribute vec3 a_position;                 \n"
    "attribute vec2 a_texCoord;                 \n"
    "varying vec2 v_texCoord;                   \n"
    "void main()                                \n"
    "{                                          \n"
    "   gl_Position = vec4(a_position, 1.0);    \n"
    "   v_texCoord = a_texCoord;                \n"
    "}                                          \n";

const char defaultFShaderStr[] =
    "precision mediump float;                            \n"
    "varying vec2 v_texCoord;                            \n"
    "uniform sampler2D s_texture;                        \n"
    "void main()                                         \n"
    "{                                                   \n"
    "  gl_FragColor = texture2D(s_texture, v_texCoord);  \n"
    "}                                                   \n";


const char blurFShaderStr[] =
    "precision mediump float;\n"
    "varying vec2 v_texCoord;\n"
    "uniform sampler2D s_texture;\n"
    "uniform float blur_size;\n"
    "const int kernel_size = 9;\n" // 注意这里 kernel_size 的大小一定要和 GAUSS_KERNEL_SIZE 一致 ！
    "uniform float kernel[kernel_size];\n" 
    "void main()\n"
    "{\n"
    "   vec4 color = vec4(0.0);\n"
    "   for (int i = 0; i < kernel_size; i++)\n"
    "   {\n"
    "       for (int j = 0; j < kernel_size; j++)\n"
    "       {\n"
    "           vec2 offset = vec2(float(i - kernel_size / 2), float(j - kernel_size / 2)) * blur_size;\n"
    "           color += texture2D(s_texture, v_texCoord + offset) * kernel[i] * kernel[j];\n"
    "       }\n"
    "   }\n"
    "   gl_FragColor = color;\n"
    "}\n";


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

static const GLfloat DefaultVertices[GLESDisplayUtils::eDegree_Max][VERTICE_DATA_SIZE] = {
    {// Rotate 0 degree
        -1.0f,  1.0f, 0.0f,    // Position 0
        0.0f,  0.0f,          // TexCoord 0 
        -1.0f, -1.0f, 0.0f,    // Position 1
        0.0f,  1.0f,          // TexCoord 1
        1.0f, -1.0f, 0.0f,    // Position 2
        1.0f,  1.0f,          // TexCoord 2
        1.0f,  1.0f, 0.0f,    // Position 3
        1.0f,  0.0f,          // TexCoord 3
    },
    {// Rotate 90 degree
        -1.0f,  1.0f, 0.0f,    // Position 0
        1.0f,  0.0f,          // TexCoord 0 
        -1.0f, -1.0f, 0.0f,    // Position 1
        0.0f,  0.0f,          // TexCoord 1
        1.0f, -1.0f, 0.0f,    // Position 2
        0.0f,  1.0f,          // TexCoord 2
        1.0f,  1.0f, 0.0f,    // Position 3
        1.0f,  1.0f,          // TexCoord 3
    },
    {// Rotate 180 degree
        -1.0f,  1.0f,0.0f,     // Position 0
        1.0f,  1.0f,          // TexCoord 0 
        -1.0f, -1.0f, 0.0f,    // Position 1
        1.0f,  0.0f,          // TexCoord 1
        1.0f, -1.0f, 0.0f,    // Position 2
        0.0f,  0.0f,          // TexCoord 2
        1.0f,  1.0f, 0.0f,    // Position 3
        0.0f,  1.0f,          // TexCoord 3
    },
    {// Rotate 270 degree
        -1.0f,  1.0f,0.0f,     // Position 0
        0.0f,  1.0f,          // TexCoord 0 
        -1.0f, -1.0f, 0.0f,    // Position 1
        1.0f,  1.0f,          // TexCoord 1
        1.0f, -1.0f, 0.0f,    // Position 2
        1.0f,  0.0f,          // TexCoord 2
        1.0f, 1.0f, 0.0f,     // Position 3
        0.0f,  0.0f           // TexCoord 3
    }
};

GLESDisplayUtils::GLESDisplayUtils(QObject *parent)
    : QObject(parent)
{

}

GLESDisplayUtils::~GLESDisplayUtils()
{
    destroyGles();
}

void GLESDisplayUtils::drawDisplay(int width, int height)
{
    if (!mUserData.currentTexture || !mUserData.currentTexture->isValid()) {
        // texture have not ready!
        return;
    }

    glViewport(0, 0, width, height);
    bindVerticeData();
    mUserData.currentTexture->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    eglSwapBuffers(mContext.eglDisplay, mContext.eglSurface);
}

void GLESDisplayUtils::destroyGles()
{
    if (mUserData.vbo) glDeleteBuffers(1, &mUserData.vbo);
    if (mUserData.ebo) glDeleteBuffers(1, &mUserData.ebo);

    eglMakeCurrent(mContext.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (mContext.eglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(mContext.eglDisplay, mContext.eglContext);
    }

    if (mContext.eglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(mContext.eglDisplay, mContext.eglSurface);
    }

    // if (mContext.eglDisplay != EGL_NO_DISPLAY) {
    //     eglTerminate(mContext.eglDisplay);
    // }
}

void GLESDisplayUtils::setOrientation(int orientation)
{
    switch(orientation) {
        case 0:
            mUserData.currentRotate = eDegree_0;
            break;
        case 1:
            mUserData.currentRotate = eDegree_90;
            break;
        case 2:
            mUserData.currentRotate = eDegree_180;
            break;
        case 3:
            mUserData.currentRotate = eDegree_270;
            break;
        default:
            mUserData.currentRotate = eDegree_0;
            break;
    }

    mUserData.verticeChanged = true;
}

void GLESDisplayUtils::updateImage(uint32_t name, int width, int height, int stride, int bpp)
{
    if (mUserData.currentTexture) {
        mUserData.currentTexture.reset();
    }

    for (auto &texture : mUserData.textures) {
        if (texture->getName() == name) {
            mUserData.currentTexture = texture;
            return;
        }
    }

    if (mUserData.textures.size() > 3) {
        syslog(LOG_WARNING, "[%s] Too many textures! Clear it now.", __func__);
        mUserData.textures.clear();
    }

    Texture::ImageBuf imageBuf = {name, width, height, stride, bpp};
    mUserData.currentTexture = std::make_shared<Texture>(
        mContext.eglDisplay, mContext.eglContext, std::move(imageBuf));
    mUserData.textures.push_back(mUserData.currentTexture);
}

void GLESDisplayUtils::updateShowRegion(int image_width, int image_height, int show_width, int show_height)
{
    GLfloat width_ratio = show_width / (GLfloat)(image_width);
    GLfloat height_ratio = show_height / (GLfloat)(image_height);

    if ((width_ratio > 0) && (width_ratio <= 1) && (height_ratio > 0) && (height_ratio <= 1)) {
        mUserData.vertices[eDegree_0][3]    = 0.0f;                 // TexCoord 0(x)
        mUserData.vertices[eDegree_0][4]    = 0.0f;                 // TexCoord 0(y)
        mUserData.vertices[eDegree_0][8]    = 0.0f;                 // TexCoord 1(x)
        mUserData.vertices[eDegree_0][9]    = height_ratio;         // TexCoord 1(y)
        mUserData.vertices[eDegree_0][13]   = width_ratio;          // TexCoord 2(x)
        mUserData.vertices[eDegree_0][14]   = height_ratio;         // TexCoord 2(y)
        mUserData.vertices[eDegree_0][18]   = width_ratio;          // TexCoord 3(x)
        mUserData.vertices[eDegree_0][19]   = 0.0f;                 // TexCoord 3(y)

        mUserData.vertices[eDegree_90][3]    = width_ratio;          // TexCoord 0(x)
        mUserData.vertices[eDegree_90][4]    = 0.0f;                 // TexCoord 0(y)
        mUserData.vertices[eDegree_90][8]    = 0.0f;                 // TexCoord 1(x)
        mUserData.vertices[eDegree_90][9]    = 0.0f;                 // TexCoord 1(y)
        mUserData.vertices[eDegree_90][13]   = 0.0f;                 // TexCoord 2(x)
        mUserData.vertices[eDegree_90][14]   = height_ratio;         // TexCoord 2(y)
        mUserData.vertices[eDegree_90][18]   = width_ratio;          // TexCoord 3(x)
        mUserData.vertices[eDegree_90][19]   = height_ratio;         // TexCoord 3(y)

        mUserData.vertices[eDegree_180][3]    = width_ratio;          // TexCoord 0(x)
        mUserData.vertices[eDegree_180][4]    = height_ratio;         // TexCoord 0(y)
        mUserData.vertices[eDegree_180][8]    = width_ratio;          // TexCoord 1(x)
        mUserData.vertices[eDegree_180][9]    = 0.0f;                 // TexCoord 1(y)
        mUserData.vertices[eDegree_180][13]   = 0.0f;                 // TexCoord 2(x)
        mUserData.vertices[eDegree_180][14]   = 0.0f;                 // TexCoord 2(y)
        mUserData.vertices[eDegree_180][18]   = 0.0f;                 // TexCoord 3(x)
        mUserData.vertices[eDegree_180][19]   = height_ratio;         // TexCoord 3(y)

        mUserData.vertices[eDegree_270][3]    = 0.0f;                 // TexCoord 0(x)
        mUserData.vertices[eDegree_270][4]    = height_ratio;         // TexCoord 0(y)
        mUserData.vertices[eDegree_270][8]    = width_ratio;          // TexCoord 1(x)
        mUserData.vertices[eDegree_270][9]    = height_ratio;         // TexCoord 1(y)
        mUserData.vertices[eDegree_270][13]   = width_ratio;          // TexCoord 2(x)
        mUserData.vertices[eDegree_270][14]   = 0.0f;                 // TexCoord 2(y)
        mUserData.vertices[eDegree_270][18]   = 0.0f;                 // TexCoord 3(x)
        mUserData.vertices[eDegree_270][19]   = 0.0f;                 // TexCoord 3(y)

        mUserData.verticeChanged = true;
    }
}

bool GLESDisplayUtils::initGles(unsigned long winId)
{
    mUserData.currentRotate = eDegree_0;
    mUserData.textures.clear();
    mUserData.vbo = 0;
    mUserData.ebo = 0;
    mUserData.verticeChanged = false;
    // init default vertices
    memcpy(mUserData.vertices, DefaultVertices, sizeof(DefaultVertices));
    
    if (!initContext(winId)) {
        syslog(LOG_CRIT, "[%s] Init GLES context failed!", __func__);
        return false;
    }

    return true;
}

bool GLESDisplayUtils::initContext(unsigned long winId)
{
    if (!attachWindow((EGLNativeWindowType)winId)) {
        syslog(LOG_ERR, "[%s] Failed to attach window:%lu.", __func__, winId);
        return false;
    }

    // Load the shader
    mUserData.defaultShader = std::make_shared<Shader>(defaultVShaderStr, defaultFShaderStr);
    mUserData.blurShader = std::make_shared<Shader>(defaultVShaderStr, blurFShaderStr);
    mUserData.currentShader = mUserData.defaultShader;

    // init vbo
    glGenBuffers(1, &mUserData.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mUserData.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mUserData.vertices[0]), mUserData.vertices[0], GL_STATIC_DRAW);

    GLuint pos = mUserData.currentShader->getAttribLoc("a_position");
    GLuint coord = mUserData.currentShader->getAttribLoc("a_texCoord");
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(coord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(coord);

    // init ebo
    glGenBuffers(1, &mUserData.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mUserData.ebo);
    const GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Reset state.
    //glDisableVertexAttribArray(pos);
    //glDisableVertexAttribArray(coord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glActiveTexture(GL_TEXTURE0);

    return true;
}

bool GLESDisplayUtils::attachWindow(EGLNativeWindowType winId)
{
    static Display *X11Display = nullptr;
    
    if (!X11Display) {
        X11Display = XOpenDisplay(NULL);
        if (!X11Display) {
            syslog(LOG_ERR, "[%s] Failed to get X11 display!", __func__);
            return GL_FALSE;
        }
    }

    mContext.eglDisplay = eglGetDisplay(X11Display);
    mContext.eglNativeWindow = winId;
    if (mContext.eglDisplay == EGL_NO_DISPLAY ) {
        syslog(LOG_ERR, "[%s] Failed to get egl display!", __func__);
        return GL_FALSE;
    }

    // Initialize EGL
    EGLint majorVersion, minorVersion;
    if (!eglInitialize(mContext.eglDisplay, &majorVersion, &minorVersion)) {
        syslog(LOG_ERR, "[%s] Failed to initialize egl!", __func__);
        return GL_FALSE;
    }

    EGLint attribList[] = {
        EGL_RED_SIZE,       5,
        EGL_GREEN_SIZE,     6,
        EGL_BLUE_SIZE,      5,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, 0,
        // if EGL_KHR_create_context extension is supported, then we will use
        // EGL_OPENGL_ES3_BIT_KHR instead of EGL_OPENGL_ES2_BIT in the attribute list
        EGL_RENDERABLE_TYPE, getContextRenderableType(),
        EGL_NONE
    };

    // Choose config
    EGLConfig config;
    EGLint numConfigs = 0;
    if (!eglChooseConfig (mContext.eglDisplay, attribList, &config, 1, &numConfigs)) {
        syslog(LOG_ERR, "[%s] Failed to choose egl config!", __func__);
        return GL_FALSE;
    }

    if (numConfigs < 1) {
        syslog(LOG_ERR, "[%s] Can't find available egl config!", __func__);
        return GL_FALSE;
    }

    // Create a surface
    mContext.eglSurface = eglCreateWindowSurface (mContext.eglDisplay, config, 
                                                    mContext.eglNativeWindow, NULL );

    if (mContext.eglSurface == EGL_NO_SURFACE) {
        syslog(LOG_ERR, "[%s] Create window surface failed!", __func__);
        return GL_FALSE;
    }

    // Create a GL context
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    mContext.eglContext = eglCreateContext(mContext.eglDisplay, config, 
                                                EGL_NO_CONTEXT, contextAttribs );

    if (mContext.eglContext == EGL_NO_CONTEXT) {
        syslog(LOG_ERR, "[%s] Create egl context failed!", __func__);
        return GL_FALSE;
    }

    // Make the context current
    if (!eglMakeCurrent (mContext.eglDisplay, mContext.eglSurface, 
                        mContext.eglSurface, mContext.eglContext )) {
        syslog(LOG_ERR, "[%s] Make current egl context failed!", __func__);
        return GL_FALSE;
    }

    return GL_TRUE;
}

EGLint GLESDisplayUtils::getContextRenderableType()
{
#ifdef EGL_KHR_create_context
   const char *extensions = eglQueryString (mContext.eglDisplay, EGL_EXTENSIONS);
   // check whether EGL_KHR_create_context is in the extension string
   if ((extensions != nullptr) && strstr(extensions, "EGL_KHR_create_context")) {
      // extension is supported
      return EGL_OPENGL_ES3_BIT_KHR;
   }
#endif
   // extension is not supported
   return EGL_OPENGL_ES2_BIT;
}

void GLESDisplayUtils::bindVerticeData()
{
    mUserData.currentShader->use();

    glBindBuffer(GL_ARRAY_BUFFER, mUserData.vbo);
    if (mUserData.verticeChanged) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mUserData.vertices[0]), mUserData.vertices[mUserData.currentRotate]);
        mUserData.verticeChanged = false;
    }

    GLuint pos =  mUserData.currentShader->getAttribLoc("a_position");
    GLuint coord =  mUserData.currentShader->getAttribLoc("a_texCoord");
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(coord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(coord);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mUserData.ebo);
}

void GLESDisplayUtils::updateBlurShaderData()
{
    mUserData.currentShader->use();

    GLuint kernelLoc =  mUserData.currentShader->getUniformLoc("kernel");
    GLuint blurSizeLoc =  mUserData.currentShader->getUniformLoc("blur_size");

    generateGaussKernel(mUserData.kernel, GAUSS_KERNEL_SIZE, GAUSS_KERNEL_SIGMA);
    glUniform1fv(kernelLoc, GAUSS_KERNEL_SIZE, mUserData.kernel);

    float blurSize = 0.001;
    if (mUserData.currentTexture && mUserData.currentTexture->isValid()) {
        blurSize = 1.f / (mUserData.currentTexture->getWidth() + mUserData.currentTexture->getHeight()) * 2;
    }
    glUniform1f(blurSizeLoc, blurSize);
}

void GLESDisplayUtils::enableBlur(bool enable) 
{
    mUserData.currentShader = enable ? mUserData.blurShader : mUserData.defaultShader;
    if (enable) {
        updateBlurShaderData();
    }
}
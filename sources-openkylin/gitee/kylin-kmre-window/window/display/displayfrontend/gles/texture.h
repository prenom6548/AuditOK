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

#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>


class Texture
{
public:
    struct ImageBuf{
        uint32_t name;
        int32_t width;
        int32_t height;
        int32_t stride;
        int32_t bpp;
    };

	Texture(EGLDisplay display, EGLContext context, ImageBuf &&buffer);
	virtual ~Texture();
    
    void bind();
    bool isValid() {return mIsValid;}
    uint32_t getName() {return mName;}
    uint32_t getWidth() {return mWidth;}
    uint32_t getHeight() {return mHeight;}
    GLuint getTextureId() { return mId; };

private:
    GLboolean initEglImageKHRFuncs();
    GLboolean createTexture(ImageBuf &imageBuf);
    GLboolean createEglImageFromBo(ImageBuf &imageBuf);
    GLboolean createEglImageFromDma(ImageBuf &imageBuf);

private:
    GLuint mId;
    uint32_t mName;
    uint32_t mWidth;
    uint32_t mHeight;
    EGLImageKHR mEglImage;
    EGLDisplay mEglDisplay;
    EGLContext mEglContext;
	bool mIsValid;
   
};

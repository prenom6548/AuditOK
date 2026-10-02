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

#include "texture.h"
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <xf86drm.h>
#include <drm_fourcc.h>

#define GL_GLEXT_PROTOTYPES  //for get glEGLImageTargetTexture2DOES
#define EGL_EGLEXT_PROTOTYPES

#ifdef GL_OES_EGL_image
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
#endif

#ifdef EGL_KHR_image
static PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = nullptr;
static PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = nullptr;
#endif

extern "C" {
extern int drm_helper_get_dmabuf_from_name(uint32_t name);
}

Texture::Texture(EGLDisplay display, EGLContext context, ImageBuf &&buffer) 
	: mId(0)
	, mName(buffer.name)
    , mWidth(buffer.width)
    , mHeight(buffer.height)
	, mEglImage(EGL_NO_IMAGE_KHR)
	, mEglDisplay(display)
	, mEglContext(context)
	, mIsValid(false)
{
	if (!initEglImageKHRFuncs()) {
		return;
	}

	if (!createTexture(buffer)) {
		return;
	}

	mIsValid = true;
}

Texture::~Texture()
{
	if (mEglImage != EGL_NO_IMAGE_KHR) {
		eglDestroyImageKHR(mEglDisplay, mEglImage);
		mEglImage = EGL_NO_IMAGE_KHR;
	}

	if (mId) {
		glDeleteTextures(1, &mId);
		mId = 0;
	}
}

void Texture::bind() 
{
	if (mIsValid) {
		glBindTexture(GL_TEXTURE_2D, mId);
	}
}

GLboolean Texture::createTexture(ImageBuf &imageBuf)
{
    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &mId);
    glBindTexture(GL_TEXTURE_2D, mId);

	if (!createEglImageFromBo(imageBuf)) {
		if (!createEglImageFromDma(imageBuf)) {
			syslog(LOG_ERR, "[%s] Failed to create texture!", __func__);
			return false;
		}
	}

	glGetError();// clear err
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, mEglImage);
	if (glGetError() != GL_NO_ERROR) {
		syslog(LOG_ERR, "[%s] glEGLImageTargetTexture2DOES failed!", __func__);
		return false;
	}

    // Set the filtering mode
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;

}

GLboolean Texture::createEglImageFromBo(ImageBuf &imageBuf)
{
    int32_t stride = (imageBuf.stride >= (imageBuf.width * imageBuf.bpp)) ? \
						(imageBuf.stride / imageBuf.bpp) : imageBuf.width;

    //syslog(LOG_DEBUG, "[%s] imageBuf: name = %d, width = %d, height = %d, stride = %d", 
    //        __func__, imageBuf.name, imageBuf.width, imageBuf.height, imageBuf.stride);

    const EGLint hostAttribs[] = {
        EGL_WIDTH, imageBuf.width,
        EGL_HEIGHT, imageBuf.height,
        EGL_DRM_BUFFER_FORMAT_MESA, EGL_DRM_BUFFER_FORMAT_ARGB32_MESA,
        EGL_DRM_BUFFER_STRIDE_MESA, stride,
        EGL_NONE
    };

    mEglImage = eglCreateImageKHR(mEglDisplay, mEglContext,
                                EGL_DRM_BUFFER_MESA,
                                (EGLClientBuffer)(uintptr_t)imageBuf.name, 
                                hostAttribs);

    if (mEglImage == EGL_NO_IMAGE_KHR) {
		syslog(LOG_ERR, "[%s] Failed to create EGL image from bo: %d", __func__, imageBuf.name);
		return false;
    }

	return true;
}

GLboolean Texture::createEglImageFromDma(ImageBuf &imageBuf)
{
    int bufFd = drm_helper_get_dmabuf_from_name(imageBuf.name);
    if (bufFd < 0) {
        return false;
    }
    //syslog(LOG_DEBUG, "[%s] imageBuf: name = %d, width = %d, height = %d, stride = %d", 
    //        __func__, imageBuf.name, imageBuf.width, imageBuf.height, imageBuf.stride);
            
    EGLint const attribute_list[] = {
        EGL_WIDTH, imageBuf.width,
        EGL_HEIGHT, imageBuf.height,
        EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_XRGB8888,
        EGL_DMA_BUF_PLANE0_FD_EXT, bufFd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, imageBuf.stride,
        EGL_NONE
    };

    mEglImage = eglCreateImageKHR(mEglDisplay,
                                EGL_NO_CONTEXT,
                                EGL_LINUX_DMA_BUF_EXT,
                                (EGLClientBuffer)NULL, 
								attribute_list);

    if (mEglImage == EGL_NO_IMAGE_KHR) {
		syslog(LOG_ERR, "[%s] Failed to create EGL image from dma: %d", __func__, imageBuf.name);
        close(bufFd);
		return false;
    }

    return true;
}

GLboolean Texture::initEglImageKHRFuncs()
{
    if (!glEGLImageTargetTexture2DOES) {
        glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) 
                                        eglGetProcAddress("glEGLImageTargetTexture2DOES");
        if (!glEGLImageTargetTexture2DOES) {
            syslog(LOG_CRIT, "[%s] Failed to get glEGLImageTargetTexture2DOES func!", __func__);
            return false;
        }
    }

    if (!eglCreateImageKHR) {
        eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
        if (!eglCreateImageKHR) {
            syslog(LOG_CRIT, "[%s] Failed to get eglCreateImageKHR func!", __func__);
            return false;
        }
    }

    if (!eglDestroyImageKHR) {
        eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
        if (!eglDestroyImageKHR) {
            syslog(LOG_CRIT, "[%s] Failed to get eglDestroyImageKHR func!", __func__);
            return false;
        }
    }

    return true;
}

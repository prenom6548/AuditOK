// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _GNU_SOURCE
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <gbm.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "linux-dmabuf-v1-client-protocol.h"
#include "wlu_private.h"

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

static inline int create_anonymous_file(size_t size)
{
    int fd = memfd_create("wlu-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING | MFD_NOEXEC_SEAL);
    if (fd < 0 && errno == EINVAL && MFD_NOEXEC_SEAL != 0) {
        fd = memfd_create("wlu-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    }
    if (fd >= 0) {
        fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK);
    }

    if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static void compositor_release_buffer(void *data, struct wl_buffer *buffer)
{
    struct wlu_buffer *wlu_buffer = data;
    wlu_buffer->compositor_using = false;

    // 服务器超时占用跳过渲染，释放后尝试渲染
    wlu_window_try_invoke_draw(wlu_buffer->window);
}

static const struct wl_buffer_listener buffer_listener = {
    .release = compositor_release_buffer,
};

static bool buffer_init_shm(struct wlu_buffer *buffer, struct wlu_display *display)
{
    const int32_t width = buffer->width;
    const int32_t height = buffer->height;
    const int32_t stride = width * display->shm_formats[buffer->format].bytes_per_pixel;
    size_t size = stride * height;
    int fd = create_anonymous_file(size);
    if (fd < 0) {
        fprintf(stderr, "creating a buffer file for %ld B failed: %s\n", size, strerror(errno));
        return false;
    }

    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        close(fd);
        return false;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(display->shm, fd, size);
    struct wl_buffer *wl_buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride,
                                                            wlu_format_to_shm(buffer->format));
    wl_buffer_add_listener(wl_buffer, &buffer_listener, buffer);
    wl_shm_pool_destroy(pool);
    close(fd);

    buffer->buffer = wl_buffer;
    buffer->backend.shm.size = size;
    buffer->backend.shm.data = data;
    return true;
}

static void linux_buffer_create_succeeded(void *data, struct zwp_linux_buffer_params_v1 *params,
                                          struct wl_buffer *new_buffer)
{
    struct wlu_buffer *buffer = data;
    buffer->buffer = new_buffer;
    wl_buffer_add_listener(buffer->buffer, &buffer_listener, buffer);
    zwp_linux_buffer_params_v1_destroy(params);
}

static void linux_buffer_create_failed(void *data, struct zwp_linux_buffer_params_v1 *params)
{
    struct wlu_buffer *buffer = data;
    buffer->buffer = NULL;
    zwp_linux_buffer_params_v1_destroy(params);
}

static const struct zwp_linux_buffer_params_v1_listener linux_buffer_params_listener = {
    .created = linux_buffer_create_succeeded,
    .failed = linux_buffer_create_failed,
};

static bool buffer_init_dmabuf(struct wlu_buffer *buffer, struct wlu_display *display)
{
    int32_t format = wlu_format_to_drm(buffer->format);

    struct gbm_bo *gbm_bo = gbm_bo_create(display->gbm.device, buffer->width, buffer->height,
                                          format, GBM_BO_USE_RENDERING);
    if (!gbm_bo) {
        fprintf(stderr, "Failed to create gbm_bo\n");
        return false;
    }
    int fd = gbm_bo_get_fd(gbm_bo);
    uint32_t stride = gbm_bo_get_stride(gbm_bo);
    uint32_t offset = gbm_bo_get_offset(gbm_bo, 0);
    uint64_t modifier = gbm_bo_get_modifier(gbm_bo);

    struct zwp_linux_buffer_params_v1 *params = zwp_linux_dmabuf_v1_create_params(display->dmabuf);
    zwp_linux_buffer_params_v1_add(params, fd, 0, offset, stride, modifier >> 32,
                                   modifier & 0xffffffff);
    zwp_linux_buffer_params_v1_add_listener(params, &linux_buffer_params_listener, buffer);
    zwp_linux_buffer_params_v1_create(params, buffer->width, buffer->height, format, 0);
    // 立马执行linux_buffer_create_succeeded
    wl_display_roundtrip(display->display);
    if (buffer->buffer == NULL) {
        fprintf(stderr, "Failed to create zwp_linux_buffer_params\n");
        gbm_bo_destroy(gbm_bo);
        return false;
    }

    // clang-format off
    EGLint  image_attribs[] = {
		EGL_WIDTH, buffer->width,
		EGL_HEIGHT, buffer->height,
		EGL_LINUX_DRM_FOURCC_EXT, format,
        EGL_DMA_BUF_PLANE0_FD_EXT, fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_NONE
	};
    // clang-format on
    EGLImageKHR egl_image = buffer->window->egl.create_image(
        display->egl_display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, image_attribs);
    close(fd);
    if (egl_image == EGL_NO_IMAGE_KHR) {
        fprintf(stderr, "EGLImageKHR creation failed\n");
        gbm_bo_destroy(gbm_bo);
        return false;
    }

    GLuint gl_texture;
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    buffer->window->egl.image_target_texture_2d(GL_TEXTURE_2D, egl_image);

    GLuint gl_fbo;
    glGenFramebuffers(1, &gl_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gl_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "FBO creation failed\n");
        eglDestroyImage(display->egl_display, egl_image);
        gbm_bo_destroy(gbm_bo);
        return false;
    }

    buffer->backend.gl.bo = gbm_bo;
    buffer->backend.gl.image = egl_image;
    buffer->backend.gl.texture = gl_texture;
    buffer->backend.gl.fbo = gl_fbo;
    return true;
}

struct wlu_buffer *wlu_buffer_create(struct wlu_window *window)
{
    struct wlu_buffer *buffer = calloc(1, sizeof(*buffer));
    if (buffer == NULL) {
        return NULL;
    }
    buffer->width = window->width;
    buffer->height = window->height;
    buffer->format = window->buffer_format;
    buffer->window = window;
    buffer->type = window->buffer_type;

    if (buffer->type == WLU_BUFFER_TYPE_SHM) {
        if (!buffer_init_shm(buffer, window->display)) {
            wlu_buffer_destroy(buffer);
            return NULL;
        }
    } else if (buffer->type == WLU_BUFFER_TYPE_DMABUF_GL) {
        if (!buffer_init_dmabuf(buffer, window->display)) {
            wlu_buffer_destroy(buffer);
            return NULL;
        }
    }

    return buffer;
}

void wlu_buffer_destroy(struct wlu_buffer *buffer)
{
    if (!buffer) {
        return;
    }

    if (buffer->buffer) {
        wl_buffer_destroy(buffer->buffer);
    }

    if (buffer->type == WLU_BUFFER_TYPE_SHM) {
        if (buffer->backend.shm.data) {
            munmap(buffer->backend.shm.data, buffer->backend.shm.size);
        }
    } else if (buffer->type == WLU_BUFFER_TYPE_DMABUF_GL) {
        if (buffer->backend.gl.fbo > 0) {
            glDeleteFramebuffers(1, &buffer->backend.gl.fbo);
        }
        if (buffer->backend.gl.texture > 0) {
            glDeleteTextures(1, &buffer->backend.gl.texture);
        }
        if (buffer->backend.gl.image) {
            buffer->window->egl.destroy_image(buffer->window->display->egl_display,
                                              buffer->backend.gl.image);
        }
        if (buffer->backend.gl.bo) {
            gbm_bo_destroy(buffer->backend.gl.bo);
        }
    }

    free(buffer);
}

int32_t wlu_buffer_get_width(struct wlu_buffer *buffer)
{
    return buffer->width;
}

int32_t wlu_buffer_get_height(struct wlu_buffer *buffer)
{
    return buffer->height;
}

enum wlu_buffer_type wlu_buffer_get_type(struct wlu_buffer *buffer)
{
    return buffer->type;
}

void *wlu_buffer_get_shm_data(struct wlu_buffer *buffer)
{
    if (buffer->type == WLU_BUFFER_TYPE_SHM) {
        return buffer->backend.shm.data;
    }
    return NULL;
}

uint32_t wlu_buffer_get_gl_fbo(struct wlu_buffer *buffer)
{
    if (buffer->type == WLU_BUFFER_TYPE_DMABUF_GL) {
        return buffer->backend.gl.fbo;
    }
    return 0;
}

uint32_t wlu_buffer_get_gl_texture(struct wlu_buffer *buffer)
{
    if (buffer->type == WLU_BUFFER_TYPE_DMABUF_GL) {
        return buffer->backend.gl.texture;
    }
    return 0;
}

struct wlu_buffer *wlu_buffer_import_shmbuf(struct wlu_window *window, struct wlu_shmbuf *shmbuf)
{
    size_t size = shmbuf->height * shmbuf->stride + shmbuf->offset;
    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmbuf->fd, 0);
    if (data == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        return NULL;
    }

    struct wlu_buffer *buffer = calloc(1, sizeof(*buffer));
    if (buffer == NULL) {
        return NULL;
    }
    buffer->width = shmbuf->width;
    buffer->height = shmbuf->height;
    buffer->format = wlu_format_from_shm(shmbuf->format);
    buffer->window = window;
    buffer->type = WLU_BUFFER_TYPE_SHM;
    buffer->backend.shm.data = data;
    buffer->backend.shm.size = size;

    return buffer;
}

struct wlu_buffer *wlu_buffer_import_shmbuf_gl(struct wlu_window *window, struct wlu_shmbuf *shmbuf)
{
    size_t size = shmbuf->height * shmbuf->stride + shmbuf->offset;
    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmbuf->fd, 0);
    if (data == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        return NULL;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    enum wlu_format format = wlu_format_from_shm(shmbuf->format);
    GLenum gl_format;
    GLenum gl_internal_format;
    switch (format) {
    case WLU_FORMAT_BGRA8888:
    case WLU_FORMAT_BGRX8888:
        gl_internal_format = GL_RGBA;
        gl_format = GL_RGBA;
        break;
    default:
        gl_internal_format = GL_RGBA;
        gl_format = GL_BGRA_EXT;
        break;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, shmbuf->width, shmbuf->height, 0, gl_format,
                 GL_UNSIGNED_BYTE, data);

    munmap(data, size);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteTextures(1, &texture);
        glDeleteFramebuffers(1, &fbo);
        fprintf(stderr, "FBO creation failed for memfd\n");
        return NULL;
    }

    struct wlu_buffer *buffer = calloc(1, sizeof(*buffer));
    if (buffer == NULL) {
        return NULL;
    }
    buffer->width = shmbuf->width;
    buffer->height = shmbuf->height;
    buffer->format = format;
    buffer->window = window;
    buffer->type = WLU_BUFFER_TYPE_DMABUF_GL;
    buffer->backend.gl.texture = texture;
    buffer->backend.gl.fbo = fbo;

    return buffer;
}

struct wlu_buffer *wlu_buffer_import_dmabuf(struct wlu_window *window, struct wlu_dmabuf *dmabuf)
{
    if (!window->egl.has_image_dma_buf_import || dmabuf->planes_count == 0) {
        return NULL;
    }

    static const int general_attribs = 3;
    static const int plane_attribs = 5;
    static const int entries_per_attrib = 2;
    EGLint
        attribs[(general_attribs + plane_attribs * WLU_MAX_DMABUF_PLANES) * entries_per_attrib + 1];
    unsigned int atti = 0;

    attribs[atti++] = EGL_WIDTH;
    attribs[atti++] = dmabuf->width;
    attribs[atti++] = EGL_HEIGHT;
    attribs[atti++] = dmabuf->height;
    attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
    attribs[atti++] = dmabuf->format;

#define ADD_PLANE_ATTRIBS(plane_idx)                                                               \
    {                                                                                              \
        attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_FD_EXT;                                   \
        attribs[atti++] = dmabuf->planes[plane_idx].fd;                                            \
        attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_OFFSET_EXT;                               \
        attribs[atti++] = (int)dmabuf->planes[plane_idx].offset;                                   \
        attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_PITCH_EXT;                                \
        attribs[atti++] = (int)dmabuf->planes[plane_idx].stride;                                   \
        if (window->egl.has_image_dma_buf_import_modifiers) {                                      \
            attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_MODIFIER_LO_EXT;                      \
            attribs[atti++] = dmabuf->modifier & 0xFFFFFFFF;                                       \
            attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_MODIFIER_HI_EXT;                      \
            attribs[atti++] = dmabuf->modifier >> 32;                                              \
        }                                                                                          \
    }

    ADD_PLANE_ATTRIBS(0);
    if (dmabuf->planes_count > 1)
        ADD_PLANE_ATTRIBS(1);
    if (dmabuf->planes_count > 2)
        ADD_PLANE_ATTRIBS(2);
    if (dmabuf->planes_count > 3)
        ADD_PLANE_ATTRIBS(3);

#undef ADD_PLANE_ATTRIBS

    attribs[atti] = EGL_NONE;

    assert(atti < ARRAY_LENGTH(attribs));

    EGLImageKHR image = window->egl.create_image(window->display->egl_display, EGL_NO_CONTEXT,
                                                 EGL_LINUX_DMA_BUF_EXT, NULL, attribs);
    if (image == EGL_NO_IMAGE_KHR) {
        fprintf(stderr, "EGLImageKHR creation failed for dmabuf\n");
        return NULL;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    window->egl.image_target_texture_2d(GL_TEXTURE_2D, image);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        window->egl.destroy_image(window->display->egl_display, image);
        glDeleteTextures(1, &texture);
        glDeleteFramebuffers(1, &fbo);
        fprintf(stderr, "FBO creation failed for dmabuf\n");
        return NULL;
    }

    struct wlu_buffer *buffer = calloc(1, sizeof(*buffer));
    if (buffer == NULL) {
        return NULL;
    }
    buffer->width = dmabuf->width;
    buffer->height = dmabuf->height;
    buffer->format = wlu_format_from_drm(dmabuf->format);
    buffer->window = window;
    buffer->type = WLU_BUFFER_TYPE_DMABUF_GL;
    buffer->backend.gl.image = image;
    buffer->backend.gl.texture = texture;
    buffer->backend.gl.fbo = fbo;

    return buffer;
}

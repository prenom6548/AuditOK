// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>

#include <wlr/render/wlr_texture.h>

#include <kywc/log.h>

#include "render/opengl.h"
#include "render/pixel_format.h"

static const struct wlr_texture_impl texture_impl;

bool wlr_texture_is_opengl(struct wlr_texture *wlr_texture)
{
    return wlr_texture->impl == &texture_impl;
}

struct opengl_texture *opengl_texture_from_wlr_texture(struct wlr_texture *wlr_texture)
{
    assert(wlr_texture->impl == &texture_impl);
    struct opengl_texture *texture = wl_container_of(wlr_texture, texture, wlr_texture);
    return texture;
}

static bool gl_texture_update_from_buffer(struct wlr_texture *wlr_texture,
                                          struct wlr_buffer *buffer,
                                          const pixman_region32_t *damage)
{
    struct opengl_texture *texture = opengl_texture_from_wlr_texture(wlr_texture);

    if (texture->drm_format == DRM_FORMAT_INVALID) {
        return false;
    }

    void *data;
    uint32_t format;
    size_t stride;
    if (!wlr_buffer_begin_data_ptr_access(buffer, WLR_BUFFER_DATA_PTR_ACCESS_READ, &data, &format,
                                          &stride)) {
        return false;
    }

    if (format != texture->drm_format) {
        wlr_buffer_end_data_ptr_access(buffer);
        return false;
    }

    const struct pixel_format *fmt = pixel_format_from_drm(texture->drm_format);
    assert(fmt);

    if (pixel_format_pixels_per_block(fmt) != 1) {
        wlr_buffer_end_data_ptr_access(buffer);
        kywc_log(KYWC_ERROR, "Cannot update texture: block formats are not supported");
        return false;
    }

    if (!pixel_format_check_stride(fmt, stride, buffer->width)) {
        wlr_buffer_end_data_ptr_access(buffer);
        return false;
    }

    struct egl_context prev_ctx;
    egl_make_current(texture->renderer->egl, &prev_ctx);

    opengl_push_debug(texture->renderer);

    glBindTexture(GL_TEXTURE_2D, texture->tex);

    int rects_len = 0;
    const pixman_box32_t *rects = pixman_region32_rectangles(damage, &rects_len);

    for (int i = 0; i < rects_len; i++) {
        pixman_box32_t rect = rects[i];

        glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, stride / fmt->bytes_per_block);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS_EXT, rect.x1);
        glPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, rect.y1);

        int width = rect.x2 - rect.x1;
        int height = rect.y2 - rect.y1;
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x1, rect.y1, width, height, fmt->gl_format,
                        fmt->gl_type, data);
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS_EXT, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    opengl_pop_debug(texture->renderer);

    egl_restore_context(&prev_ctx);

    wlr_buffer_end_data_ptr_access(buffer);

    return true;
}

bool opengl_texture_invalidate(struct opengl_texture *texture)
{
    if (texture->image == EGL_NO_IMAGE_KHR) {
        return false;
    }
    if (texture->target == GL_TEXTURE_EXTERNAL_OES) {
        // External changes are immediately made visible by the GL implementation
        return true;
    }

    struct egl_context prev_ctx;
    egl_make_current(texture->renderer->egl, &prev_ctx);

    opengl_push_debug(texture->renderer);

    glBindTexture(texture->target, texture->tex);
    glEGLImageTargetTexture2DOES(texture->target, texture->image);
    glBindTexture(texture->target, 0);

    opengl_pop_debug(texture->renderer);

    egl_restore_context(&prev_ctx);

    return true;
}

void opengl_texture_destroy(struct opengl_texture *texture)
{
    wl_list_remove(&texture->link);
    if (texture->buffer != NULL) {
        wlr_addon_finish(&texture->buffer_addon);
    }

    struct egl_context prev_ctx;
    egl_make_current(texture->renderer->egl, &prev_ctx);

    opengl_push_debug(texture->renderer);

    glDeleteTextures(1, &texture->tex);
    if (texture->fbo) {
        glDeleteFramebuffers(1, &texture->fbo);
    }
    egl_destroy_image(texture->renderer->egl, texture->image);

    opengl_pop_debug(texture->renderer);

    egl_restore_context(&prev_ctx);

    free(texture);
}

static void gl_texture_unref(struct wlr_texture *wlr_texture)
{
    struct opengl_texture *texture = opengl_texture_from_wlr_texture(wlr_texture);
    if (texture->buffer != NULL) {
        // Keep the texture around, in case the buffer is re-used later. We're
        // still listening to the buffer's destroy event.
        wlr_buffer_unlock(texture->buffer);
    } else {
        opengl_texture_destroy(texture);
    }
}

static const struct wlr_texture_impl texture_impl = {
    .update_from_buffer = gl_texture_update_from_buffer,
    .destroy = gl_texture_unref,
};

struct opengl_texture *opengl_texture_create(struct opengl_renderer *renderer, uint32_t width,
                                             uint32_t height)
{
    struct opengl_texture *texture = calloc(1, sizeof(*texture));
    if (texture == NULL) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        return NULL;
    }
    wlr_texture_init(&texture->wlr_texture, &renderer->wlr_renderer, &texture_impl, width, height);
    texture->renderer = renderer;
    wl_list_insert(&renderer->textures, &texture->link);
    return texture;
}

static struct wlr_texture *gl_texture_from_pixels(struct wlr_renderer *wlr_renderer,
                                                  uint32_t drm_format, uint32_t stride,
                                                  uint32_t width, uint32_t height, const void *data)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);

    const struct pixel_format *fmt = pixel_format_from_drm(drm_format);
    if (fmt == NULL || !pixel_format_is_support_by_gl(fmt, renderer)) {
        kywc_log(KYWC_ERROR, "Unsupported pixel format 0x%" PRIX32, drm_format);
        return NULL;
    }

    if (pixel_format_pixels_per_block(fmt) != 1) {
        kywc_log(KYWC_ERROR, "Cannot upload texture: block formats are not supported");
        return NULL;
    }

    if (!pixel_format_check_stride(fmt, stride, width)) {
        return NULL;
    }

    struct opengl_texture *texture = opengl_texture_create(renderer, width, height);
    if (texture == NULL) {
        return NULL;
    }
    texture->target = GL_TEXTURE_2D;
    texture->has_alpha = fmt->has_alpha;
    texture->drm_format = fmt->drm_format;

    GLint internal_format = fmt->gl_internalformat;
    if (!internal_format) {
        /* on OpenGL ES there is a requirement that internalFormat == format */
        internal_format = fmt->gl_format;
    }
    /* fix the internal_format in OpenGL */
    if (!renderer->egl->is_gles) {
        if (internal_format == GL_BGRA_EXT || internal_format == GL_ABGR_EXT) {
            internal_format = GL_RGBA;
        } else if (internal_format == GL_BGR) {
            internal_format = GL_RGB;
        }
    }

    struct egl_context prev_ctx;
    egl_make_current(renderer->egl, &prev_ctx);

    opengl_push_debug(renderer);

    glGenTextures(1, &texture->tex);
    glBindTexture(GL_TEXTURE_2D, texture->tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, stride / fmt->bytes_per_block);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, fmt->gl_format, fmt->gl_type,
                 data);
    glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    opengl_pop_debug(renderer);

    egl_restore_context(&prev_ctx);

    return &texture->wlr_texture;
}

static struct wlr_texture *gl_texture_from_dmabuf(struct wlr_renderer *wlr_renderer,
                                                  struct wlr_dmabuf_attributes *attribs)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);
    /* for glEGLImageTargetTexture2DOES */
    if (!renderer->exts.OES_egl_image) {
        return NULL;
    }

    struct opengl_texture *texture =
        opengl_texture_create(renderer, attribs->width, attribs->height);
    if (texture == NULL) {
        return NULL;
    }
    texture->drm_format = DRM_FORMAT_INVALID; // texture can't be written anyways

    texture->has_alpha = !pixel_format_is_opaque(attribs->format);

    struct egl_context prev_ctx;
    egl_make_current(renderer->egl, &prev_ctx);

    bool external_only = renderer->exts.OES_egl_image_external;
    texture->image = egl_create_image_from_dmabuf(renderer->egl, attribs, &external_only);
    if (texture->image == EGL_NO_IMAGE_KHR) {
        kywc_log_ratelimited(KYWC_ERROR, "Failed to create EGL image from DMA-BUF");
        egl_restore_context(&prev_ctx);
        wl_list_remove(&texture->link);
        free(texture);
        return NULL;
    }

    texture->target = external_only ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D;

    opengl_push_debug(renderer);

    glGenTextures(1, &texture->tex);
    glBindTexture(texture->target, texture->tex);
    glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glEGLImageTargetTexture2DOES(texture->target, texture->image);
    glBindTexture(texture->target, 0);

    opengl_pop_debug(renderer);

    egl_restore_context(&prev_ctx);

    return &texture->wlr_texture;
}

static void texture_handle_buffer_destroy(struct wlr_addon *addon)
{
    struct opengl_texture *texture = wl_container_of(addon, texture, buffer_addon);
    opengl_texture_destroy(texture);
}

static const struct wlr_addon_interface texture_addon_impl = {
    .name = "opengl_texture",
    .destroy = texture_handle_buffer_destroy,
};

static struct wlr_texture *gl_texture_from_dmabuf_buffer(struct opengl_renderer *renderer,
                                                         struct wlr_buffer *buffer,
                                                         struct wlr_dmabuf_attributes *dmabuf)
{
    struct wlr_addon *addon = wlr_addon_find(&buffer->addons, renderer, &texture_addon_impl);
    if (addon != NULL) {
        struct opengl_texture *texture = wl_container_of(addon, texture, buffer_addon);
        if (!opengl_texture_invalidate(texture)) {
            kywc_log(KYWC_ERROR, "Failed to invalidate texture");
            return NULL;
        }
        wlr_buffer_lock(texture->buffer);
        return &texture->wlr_texture;
    }

    struct wlr_texture *wlr_texture = gl_texture_from_dmabuf(&renderer->wlr_renderer, dmabuf);
    if (wlr_texture == NULL) {
        return NULL;
    }

    struct opengl_texture *texture = opengl_texture_from_wlr_texture(wlr_texture);
    texture->buffer = wlr_buffer_lock(buffer);
    wlr_addon_init(&texture->buffer_addon, &buffer->addons, renderer, &texture_addon_impl);

    return &texture->wlr_texture;
}

struct wlr_texture *opengl_texture_from_buffer(struct wlr_renderer *wlr_renderer,
                                               struct wlr_buffer *buffer)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);

    void *data;
    uint32_t format;
    size_t stride;
    struct wlr_dmabuf_attributes dmabuf;
    if (wlr_buffer_get_dmabuf(buffer, &dmabuf)) {
        return gl_texture_from_dmabuf_buffer(renderer, buffer, &dmabuf);
    } else if (wlr_buffer_is_wayland_buffer(buffer)) {
        return wlr_texture_from_wayland_buffer(renderer, buffer);
    } else if (wlr_buffer_begin_data_ptr_access(buffer, WLR_BUFFER_DATA_PTR_ACCESS_READ, &data,
                                                &format, &stride)) {
        struct wlr_texture *tex = gl_texture_from_pixels(wlr_renderer, format, stride,
                                                         buffer->width, buffer->height, data);
        wlr_buffer_end_data_ptr_access(buffer);
        return tex;
    } else {
        return NULL;
    }
}

struct opengl_texture *opengl_texture_create_with_fbo(struct opengl_renderer *renderer,
                                                      uint32_t width, uint32_t height,
                                                      GLenum format, GLint filter)
{
    struct opengl_texture *gl_texture = opengl_texture_create(renderer, width, height);
    if (!gl_texture) {
        return NULL;
    }

    struct egl_context prev_ctx;
    egl_make_current(renderer->egl, &prev_ctx);
    gl_texture->target = GL_TEXTURE_2D;
    gl_texture->drm_format = DRM_FORMAT_INVALID; // GPU-render target, no CPU uploads
    gl_texture->has_alpha = format == GL_RGBA || format == GL_ABGR_EXT || format == GL_BGRA_EXT;

    GLint internal_format = format;
    /* fix the internal_format in OpenGL */
    if (!renderer->egl->is_gles) {
        if (internal_format == GL_BGRA_EXT || internal_format == GL_ABGR_EXT) {
            internal_format = GL_RGBA;
        } else if (internal_format == GL_BGR) {
            internal_format = GL_RGB;
        }
    }
    /* generate texture ID */
    glGenTextures(1, &gl_texture->tex);
    glBindTexture(GL_TEXTURE_2D, gl_texture->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLint old_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
    glGenFramebuffers(1, &gl_texture->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gl_texture->fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_texture->tex, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        kywc_log(KYWC_ERROR,
                 "opengl texture failed to fbo(%d) attach texture(%d). status=0x%x, tex=%dx%d",
                 gl_texture->fbo, gl_texture->tex, status, width, height);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);

    egl_restore_context(&prev_ctx);
    return gl_texture;
}

void opengl_texture_get_attribs(struct wlr_texture *wlr_texture,
                                struct opengl_texture_attribs *attribs)
{
    struct opengl_texture *texture = opengl_texture_from_wlr_texture(wlr_texture);
    *attribs = (struct opengl_texture_attribs){
        .target = texture->target,
        .tex = texture->tex,
        .has_alpha = texture->has_alpha,
    };
}

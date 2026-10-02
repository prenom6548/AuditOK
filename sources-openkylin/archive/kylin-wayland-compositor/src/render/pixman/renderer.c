// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "render/pixel_format.h"
#include "render/pixman.h"

bool pixman_begin_data_ptr_access(struct wlr_buffer *wlr_buffer, pixman_image_t **image_ptr,
                                  uint32_t flags)
{
    void *data = NULL;
    uint32_t drm_format;
    size_t stride;
    if (!wlr_buffer_begin_data_ptr_access(wlr_buffer, flags, &data, &drm_format, &stride)) {
        return false;
    }

    pixman_image_t *image = *image_ptr;
    // If the data pointer has changed, re-create the Pixman image. This can
    // happen if it's a client buffer and the wl_shm_pool has been resized.
    if (data != pixman_image_get_data(image)) {
        const struct pixel_format *info = pixel_format_from_drm(drm_format);
        assert(info && info->pixman_format != 0);

        pixman_image_t *new_image = pixman_image_create_bits_no_clear(
            info->pixman_format, wlr_buffer->width, wlr_buffer->height, data, stride);
        if (!new_image) {
            wlr_buffer_end_data_ptr_access(wlr_buffer);
            return false;
        }

        pixman_image_unref(image);
        image = new_image;
    }

    *image_ptr = image;
    return true;
}

static struct pixman_buffer *get_buffer(struct pixman_renderer *renderer,
                                        struct wlr_buffer *wlr_buffer)
{
    struct pixman_buffer *buffer;
    wl_list_for_each(buffer, &renderer->buffers, link) {
        if (buffer->buffer == wlr_buffer) {
            return buffer;
        }
    }
    return NULL;
}

static struct pixman_texture *get_texture(struct wlr_texture *wlr_texture)
{
    assert(wlr_texture_is_pixman_texture(wlr_texture));
    struct pixman_texture *texture = wl_container_of(wlr_texture, texture, wlr_texture);
    return texture;
}

static void texture_destroy(struct wlr_texture *wlr_texture)
{
    struct pixman_texture *texture = get_texture(wlr_texture);
    wl_list_remove(&texture->link);
    pixman_image_unref(texture->image);
    wlr_buffer_unlock(texture->buffer);
    free(texture);
}

static const struct wlr_texture_impl texture_impl = {
    .destroy = texture_destroy,
};

bool wlr_texture_is_pixman_texture(struct wlr_texture *texture)
{
    return texture->impl == &texture_impl;
}

static void destroy_buffer(struct pixman_buffer *buffer)
{
    wl_list_remove(&buffer->link);
    wl_list_remove(&buffer->buffer_destroy.link);
    pixman_image_unref(buffer->image);
    free(buffer);
}

static void handle_destroy_buffer(struct wl_listener *listener, void *data)
{
    struct pixman_buffer *buffer = wl_container_of(listener, buffer, buffer_destroy);
    destroy_buffer(buffer);
}

static struct pixman_buffer *create_buffer(struct pixman_renderer *renderer,
                                           struct wlr_buffer *wlr_buffer)
{
    struct pixman_buffer *buffer = calloc(1, sizeof(*buffer));
    if (!buffer) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        return NULL;
    }

    buffer->buffer = wlr_buffer;
    buffer->renderer = renderer;

    void *data = NULL;
    uint32_t drm_format;
    size_t stride;
    if (!wlr_buffer_begin_data_ptr_access(
            wlr_buffer, WLR_BUFFER_DATA_PTR_ACCESS_READ | WLR_BUFFER_DATA_PTR_ACCESS_WRITE, &data,
            &drm_format, &stride)) {
        kywc_log(KYWC_ERROR, "Failed to get buffer data");
        free(buffer);
        return NULL;
    }
    wlr_buffer_end_data_ptr_access(wlr_buffer);

    const struct pixel_format *info = pixel_format_from_drm(drm_format);
    if (!info || info->pixman_format == 0) {
        kywc_log(KYWC_ERROR, "Unsupported pixman drm format 0x%" PRIX32, drm_format);
        free(buffer);
        return NULL;
    }

    buffer->image = pixman_image_create_bits(info->pixman_format, wlr_buffer->width,
                                             wlr_buffer->height, data, stride);
    if (!buffer->image) {
        kywc_log(KYWC_ERROR, "Failed to allocate pixman image");
        free(buffer);
        return NULL;
    }

    buffer->buffer_destroy.notify = handle_destroy_buffer;
    wl_signal_add(&wlr_buffer->events.destroy, &buffer->buffer_destroy);

    wl_list_insert(&renderer->buffers, &buffer->link);
    kywc_log(KYWC_DEBUG, "Created pixman buffer %dx%d", wlr_buffer->width, wlr_buffer->height);

    return buffer;
}

static bool pixman_begin(struct wlr_renderer *wlr_renderer, uint32_t width, uint32_t height)
{
    struct pixman_renderer *renderer = pixman_renderer_from_wlr_renderer(wlr_renderer);
    renderer->width = width;
    renderer->height = height;

    struct pixman_buffer *buffer = renderer->current_buffer;
    assert(buffer != NULL);

    return pixman_begin_data_ptr_access(buffer->buffer, &buffer->image,
                                        WLR_BUFFER_DATA_PTR_ACCESS_READ |
                                            WLR_BUFFER_DATA_PTR_ACCESS_WRITE);
}

static void pixman_end(struct wlr_renderer *wlr_renderer)
{
    struct pixman_renderer *renderer = pixman_renderer_from_wlr_renderer(wlr_renderer);
    assert(renderer->current_buffer != NULL);
    wlr_buffer_end_data_ptr_access(renderer->current_buffer->buffer);
}

static const uint32_t *pixman_get_shm_texture_formats(struct wlr_renderer *wlr_renderer,
                                                      size_t *len)
{
    return pixel_format_get_drm_formats_for_pixman(len);
}

static const struct wlr_drm_format_set *pixman_get_render_formats(struct wlr_renderer *wlr_renderer)
{
    struct pixman_renderer *renderer = pixman_renderer_from_wlr_renderer(wlr_renderer);
    return &renderer->drm_formats;
}

static void pixman_destroy(struct wlr_renderer *wlr_renderer)
{
    struct pixman_renderer *renderer = pixman_renderer_from_wlr_renderer(wlr_renderer);

    struct pixman_buffer *buffer, *buffer_tmp;
    wl_list_for_each_safe(buffer, buffer_tmp, &renderer->buffers, link) {
        destroy_buffer(buffer);
    }

    struct pixman_texture *tex, *tex_tmp;
    wl_list_for_each_safe(tex, tex_tmp, &renderer->textures, link) {
        wlr_texture_destroy(&tex->wlr_texture);
    }

    wlr_drm_format_set_finish(&renderer->drm_formats);

    free(renderer);
}

static struct pixman_texture *pixman_texture_create(struct pixman_renderer *renderer,
                                                    uint32_t drm_format, uint32_t width,
                                                    uint32_t height)
{
    struct pixman_texture *texture = calloc(1, sizeof(*texture));
    if (!texture) {
        kywc_log_errno(KYWC_ERROR, "Failed to allocate pixman texture");
        return NULL;
    }

    wlr_texture_init(&texture->wlr_texture, &renderer->wlr_renderer, &texture_impl, width, height);

    texture->format_info = pixel_format_from_drm(drm_format);
    if (!texture->format_info) {
        kywc_log(KYWC_ERROR, "Unsupported drm format 0x%" PRIX32, drm_format);
        free(texture);
        return NULL;
    }

    texture->format = texture->format_info->pixman_format;
    if (texture->format == 0) {
        kywc_log(KYWC_ERROR, "Unsupported pixman drm format 0x%" PRIX32, drm_format);
        free(texture);
        return NULL;
    }

    wl_list_insert(&renderer->textures, &texture->link);

    return texture;
}

static struct wlr_texture *pixman_texture_from_buffer(struct wlr_renderer *wlr_renderer,
                                                      struct wlr_buffer *wlr_buffer)
{
    struct pixman_renderer *renderer = pixman_renderer_from_wlr_renderer(wlr_renderer);
    // reuse the buffer
    struct pixman_buffer *buffer = get_buffer(renderer, wlr_buffer);

    void *data = NULL;
    uint32_t drm_format;
    size_t stride;

    if (!buffer) {
        if (!wlr_buffer_begin_data_ptr_access(wlr_buffer, WLR_BUFFER_DATA_PTR_ACCESS_READ, &data,
                                              &drm_format, &stride)) {
            return NULL;
        }
        wlr_buffer_end_data_ptr_access(wlr_buffer);
    } else {
        pixman_format_code_t format = pixman_image_get_format(buffer->image);
        const struct pixel_format *info = pixel_format_from_pixman(format);
        if (!info) {
            return NULL;
        }
        drm_format = info->drm_format;
    }

    struct pixman_texture *texture =
        pixman_texture_create(renderer, drm_format, wlr_buffer->width, wlr_buffer->height);
    if (texture == NULL) {
        return NULL;
    }

    if (!buffer) {
        texture->image = pixman_image_create_bits_no_clear(texture->format, wlr_buffer->width,
                                                           wlr_buffer->height, data, stride);
        if (!texture->image) {
            kywc_log(KYWC_ERROR, "Failed to create pixman image");
            wl_list_remove(&texture->link);
            free(texture);
            return NULL;
        }
    } else {
        texture->image = pixman_image_ref(buffer->image);
    }

    texture->buffer = wlr_buffer_lock(wlr_buffer);
    return &texture->wlr_texture;
}

static bool pixman_bind_buffer(struct wlr_renderer *wlr_renderer, struct wlr_buffer *wlr_buffer)
{
    struct pixman_renderer *renderer = pixman_renderer_from_wlr_renderer(wlr_renderer);

    if (renderer->current_buffer) {
        wlr_buffer_unlock(renderer->current_buffer->buffer);
        renderer->current_buffer = NULL;
    }

    if (!wlr_buffer) {
        return true;
    }

    struct pixman_buffer *buffer = get_buffer(renderer, wlr_buffer);
    if (!buffer) {
        buffer = create_buffer(renderer, wlr_buffer);
    }
    if (!buffer) {
        return false;
    }

    wlr_buffer_lock(wlr_buffer);
    renderer->current_buffer = buffer;

    return true;
}

static uint32_t pixman_preferred_read_format(struct wlr_renderer *wlr_renderer)
{
    struct pixman_renderer *renderer = pixman_renderer_from_wlr_renderer(wlr_renderer);
    struct pixman_buffer *buffer = renderer->current_buffer;

    pixman_format_code_t pixman_format = pixman_image_get_format(buffer->image);
    const struct pixel_format *info = pixel_format_from_pixman(pixman_format);
    return info ? info->drm_format : DRM_FORMAT_INVALID;
}

static bool pixman_read_pixels(struct wlr_renderer *wlr_renderer, uint32_t drm_format,
                               uint32_t stride, uint32_t width, uint32_t height, uint32_t src_x,
                               uint32_t src_y, uint32_t dst_x, uint32_t dst_y, void *data)
{
    struct pixman_renderer *renderer = pixman_renderer_from_wlr_renderer(wlr_renderer);
    struct pixman_buffer *buffer = renderer->current_buffer;

    const struct pixel_format *info = pixel_format_from_drm(drm_format);
    if (!info || info->pixman_format == 0) {
        kywc_log(KYWC_ERROR, "Cannot read pixels: unsupported pixel format");
        return false;
    }

    void *p = (char *)data + pixel_format_min_stride(info, dst_x) + dst_y * stride;

    pixman_image_t *dst =
        pixman_image_create_bits_no_clear(info->pixman_format, width, height, p, stride);

    pixman_image_composite32(PIXMAN_OP_SRC, buffer->image, NULL, dst, src_x, src_y, 0, 0, 0, 0,
                             width, height);

    pixman_image_unref(dst);

    return true;
}

static uint32_t pixman_get_render_buffer_caps(struct wlr_renderer *renderer)
{
    return WLR_BUFFER_CAP_DATA_PTR;
}

static struct wlr_render_pass *
pixman_begin_buffer_pass(struct wlr_renderer *wlr_renderer, struct wlr_buffer *wlr_buffer,
                         const struct wlr_buffer_pass_options *options)
{
    struct pixman_renderer *renderer = pixman_renderer_from_wlr_renderer(wlr_renderer);

    struct pixman_buffer *buffer = get_buffer(renderer, wlr_buffer);
    if (!buffer) {
        buffer = create_buffer(renderer, wlr_buffer);
    }
    if (!buffer) {
        return NULL;
    }

    struct pixman_render_pass *pass = pixman_begin_render_pass(buffer);
    if (!pass) {
        return NULL;
    }

    return &pass->base;
}

static const struct wlr_renderer_impl renderer_impl = {
    .bind_buffer = pixman_bind_buffer,
    .begin = pixman_begin,
    .end = pixman_end,
    .get_shm_texture_formats = pixman_get_shm_texture_formats,
    .get_render_formats = pixman_get_render_formats,
    .preferred_read_format = pixman_preferred_read_format,
    .read_pixels = pixman_read_pixels,
    .destroy = pixman_destroy,
    .get_render_buffer_caps = pixman_get_render_buffer_caps,
    .texture_from_buffer = pixman_texture_from_buffer,
    .begin_buffer_pass = pixman_begin_buffer_pass,
};

bool wlr_renderer_is_pixman_renderer(struct wlr_renderer *wlr_renderer)
{
    return wlr_renderer->impl == &renderer_impl;
}

struct pixman_renderer *pixman_renderer_from_wlr_renderer(struct wlr_renderer *wlr_renderer)
{
    assert(wlr_renderer_is_pixman_renderer(wlr_renderer));
    struct pixman_renderer *renderer = wl_container_of(wlr_renderer, renderer, wlr_renderer);
    return renderer;
}

struct wlr_renderer *pixman_renderer_create(void)
{
    struct pixman_renderer *renderer = calloc(1, sizeof(*renderer));
    if (!renderer) {
        return NULL;
    }

    kywc_log(KYWC_INFO, "Creating pixman renderer with libpixman %s (Compiled with %s)",
             pixman_version_string(), PIXMAN_VERSION_STRING);

    wlr_renderer_init(&renderer->wlr_renderer, &renderer_impl);
    wl_list_init(&renderer->buffers);
    wl_list_init(&renderer->textures);

    size_t len = 0;
    const uint32_t *formats = pixel_format_get_drm_formats_for_pixman(&len);
    for (size_t i = 0; i < len; ++i) {
        wlr_drm_format_set_add(&renderer->drm_formats, formats[i], DRM_FORMAT_MOD_INVALID);
        wlr_drm_format_set_add(&renderer->drm_formats, formats[i], DRM_FORMAT_MOD_LINEAR);
    }

    return &renderer->wlr_renderer;
}

// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _RENDER_PIXMAN_H_
#define _RENDER_PIXMAN_H_

#include <pixman.h>

#include <wlr/render/drm_format_set.h>
#include <wlr/render/interface.h>
#include <wlr/render/wlr_renderer.h>

#include "pass.h"

struct pixman_buffer {
    struct wl_list link;
    struct pixman_renderer *renderer;

    struct wlr_buffer *buffer;
    struct wl_listener buffer_destroy;

    pixman_image_t *image;
};

struct pixman_texture {
    struct wlr_texture wlr_texture;

    struct wl_list link;
    struct pixman_renderer *renderer;

    pixman_image_t *image;
    pixman_format_code_t format;
    const struct pixel_format *format_info;

    struct wlr_buffer *buffer;
};

struct pixman_renderer {
    struct wlr_renderer wlr_renderer;

    struct wl_list buffers;  // pixman_buffer.link
    struct wl_list textures; // pixman_texture.link

    struct wlr_drm_format_set drm_formats;

    struct pixman_buffer *current_buffer;
    int32_t width, height;
};

struct pixman_render_pass {
    struct wlr_render_pass base;
    const struct render_pass_impl *impl;
    struct pixman_buffer *buffer;
};

struct wlr_renderer *pixman_renderer_create(void);

bool wlr_renderer_is_pixman_renderer(struct wlr_renderer *wlr_renderer);

bool wlr_texture_is_pixman_texture(struct wlr_texture *texture);

struct pixman_renderer *pixman_renderer_from_wlr_renderer(struct wlr_renderer *wlr_renderer);

bool wlr_render_pass_is_pixman(struct wlr_render_pass *render_pass);

struct pixman_render_pass *
pixman_renderer_pass_from_wlr_render_pass(struct wlr_render_pass *wlr_pass);

struct pixman_render_pass *pixman_begin_render_pass(struct pixman_buffer *buffer);

void pixman_render_pass_save(struct pixman_render_pass *pass);

void pixman_render_pass_restore(struct pixman_render_pass *pass);

bool pixman_begin_data_ptr_access(struct wlr_buffer *wlr_buffer, pixman_image_t **image_ptr,
                                  uint32_t flags);

#endif /* _RENDER_PIXMAN_H_ */

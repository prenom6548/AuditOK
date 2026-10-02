// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <wayland-server-core.h>

struct server;
struct wlr_backend;
struct wlr_allocator;
struct wlr_linux_dmabuf_v1;

struct wlr_renderer *renderer_autocreate(struct wlr_backend *backend, struct server *server);

bool renderer_init_wl_display(struct wlr_renderer *renderer, struct wlr_backend *backend,
                              struct wl_display *wl_display,
                              struct wlr_linux_dmabuf_v1 **linux_dmabuf_v1);

const struct wlr_drm_format_set *renderer_get_render_formats(struct wlr_renderer *renderer);

const struct wlr_drm_format *renderer_get_render_format(struct wlr_renderer *renderer,
                                                        uint32_t fmt);

struct wlr_buffer *renderer_create_buffer(struct wlr_renderer *renderer,
                                          struct wlr_allocator *alloc, int width, int height,
                                          uint32_t format, bool single_plane);

bool renderer_is_software(struct wlr_renderer *renderer);

bool renderer_support_timeline(struct wlr_renderer *renderer);

struct wlr_buffer *renderer_upload_pixels(struct wlr_renderer *renderer,
                                          struct wlr_allocator *alloc, int width, int height,
                                          struct wlr_buffer *pixels);

bool renderer_check_render_buffer(struct wlr_renderer *renderer, struct wlr_buffer *buffer);

bool render_buffer_is_opaque(struct wlr_buffer *buffer);

#endif /* _RENDER_H_ */

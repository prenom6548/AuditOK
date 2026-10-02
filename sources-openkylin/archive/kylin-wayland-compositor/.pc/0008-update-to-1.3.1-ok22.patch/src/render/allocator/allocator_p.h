// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _ALLOCATOR_P_H_
#define _ALLOCATOR_P_H_

#include <wayland-server-core.h>
#include <wlr/render/drm_format_set.h>

struct gbm_alloc;
struct dumb_alloc;
struct wlr_renderer;

struct dumb_alloc *dumb_alloc_create(int drm_fd);

struct gbm_alloc *gbm_alloc_create(struct wlr_renderer *renderer, int fd);

struct wlr_buffer *dumb_buffer_create(struct dumb_alloc *alloc, int width, int height,
                                      const struct wlr_drm_format *drm_format);

struct wlr_buffer *gbm_buffer_create(struct gbm_alloc *alloc, int width, int height,
                                     const struct wlr_drm_format *format, uint32_t flags);

struct wlr_buffer *shm_buffer_create(int width, int height, uint32_t fmt);

void gbm_alloc_destroy(struct gbm_alloc *alloc);

void dumb_alloc_destroy(struct dumb_alloc *alloc);

#endif /* _ALLOCATOR_P_H_ */

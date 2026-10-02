// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include <wayland-server-core.h>
#include <wlr/render/allocator.h>

enum allocator_buffer_type {
    ALLOCATOR_BUFFER_TYPE_GBM,
    ALLOCATOR_BUFFER_TYPE_DUMB,
    ALLOCATOR_BUFFER_TYPE_SHM,
};

enum allocator_buffer_flags {
    ALLOCATOR_BUFFER_AUTO = 0,
    ALLOCATOR_BUFFER_RENDERING = 1 << 0,
    ALLOCATOR_BUFFER_NO_SCANOUT = 1 << 1,
    ALLOCATOR_BUFFER_CURSOR = 1 << 2,
    ALLOCATOR_BUFFER_LINEAR = 1 << 3,
    ALLOCATOR_BUFFER_SINGLE_PLANE = 1 << 4,
};

struct wlr_allocator *allocator_create(struct wlr_backend *backend, struct wlr_renderer *renderer);

struct wlr_buffer *allocator_create_buffer(struct wlr_allocator *wlr_allocator,
                                           enum allocator_buffer_type type, int width, int height,
                                           const struct wlr_drm_format *format, uint32_t flags);

struct swapchain *allocator_create_swapchain(struct wlr_allocator *alloc,
                                             enum allocator_buffer_type type, int width, int height,
                                             const struct wlr_drm_format *format, uint32_t flags);

struct wlr_buffer *swapchain_acquire(struct swapchain *swapchain, int *age);

void swapchain_destroy(struct swapchain *swapchain);

void swapchain_set_buffer_submitted(struct swapchain *swapchain, struct wlr_buffer *buffer);

#endif /* _ALLOCATOR_H_ */

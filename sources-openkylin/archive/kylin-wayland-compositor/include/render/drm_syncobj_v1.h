// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef RENDER_DRM_SYNCOBJ_V1_H
#define RENDER_DRM_SYNCOBJ_V1_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>

#include "render/drm_syncobj.h"

struct wlr_buffer;
struct wlr_surface;

struct drm_syncobj_surface_v1_state {
    uint64_t acquire_point;
    struct drm_syncobj_timeline *acquire_timeline;

    bool committed;
    uint64_t release_point;
    struct drm_syncobj_timeline *release_timeline;

    struct drm_syncobj_merger *release_merger;
};

struct drm_syncobj_manager_v1 {
    int drm_fd;
    struct wl_global *global;

    struct wl_listener display_destroy;
};

struct drm_syncobj_manager_v1 *drm_syncobj_manager_v1_create(struct wl_display *display, int drm_fd);

struct drm_syncobj_surface_v1_state *drm_syncobj_v1_get_surface_state(struct wlr_surface *surface);

bool drm_syncobj_v1_state_signal_release_with_buffer(struct drm_syncobj_surface_v1_state *state,
                                                     struct wlr_buffer *buffer);

/**
 * Register a release point for buffer usage.
 *
 * This function may be called multiple times for the same commit. The client's
 * release point will be signalled when all registered points are signalled, and
 * a new buffer has been committed.
 *
 * Because the given release point may not be materialized, a wl_event_loop must
 * be supplied to schedule a wait internally, if needed
 */
bool drm_syncobj_v1_state_add_release_point(struct drm_syncobj_surface_v1_state *state,
                                            struct drm_syncobj_timeline *release_timeline,
                                            uint64_t release_point,
                                            struct wl_event_loop *event_loop);

#endif

// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef RENDER_DRM_SYNCOBJ_MERGER_H
#define RENDER_DRM_SYNCOBJ_MERGER_H

#include <wayland-server-core.h>

struct wlr_buffer;

/**
 * Accumulate timeline points, to have a destination timeline point be
 * signalled when all inputs are
 */
struct drm_syncobj_merger {
    int n_ref;
    struct drm_syncobj_timeline *dst_timeline;
    uint64_t dst_point;
    int sync_fd;
};

/**
 * Create a new merger.
 *
 * The given timeline point will be signalled when all input points are
 * signalled and the merger is destroyed.
 */
struct drm_syncobj_merger *drm_syncobj_merger_create(struct drm_syncobj_timeline *dst_timeline,
                                                     uint64_t dst_point);

struct drm_syncobj_merger *drm_syncobj_merger_ref(struct drm_syncobj_merger *merger);

/**
 * Target timeline point is materialized when all inputs are, and the merger is
 * destroyed.
 */
void drm_syncobj_merger_unref(struct drm_syncobj_merger *merger);

/**
 * Add a new timeline point to wait for.
 *
 * If the point is not materialized, the supplied event loop is used to schedule
 * a wait.
 */
bool drm_syncobj_merger_add(struct drm_syncobj_merger *merger,
                            struct drm_syncobj_timeline *dst_timeline, uint64_t dst_point,
                            struct wl_event_loop *loop);

/**
 * Add a new sync file to wait for.
 *
 * Ownership of fd is transferred to the merger.
 */
bool drm_syncobj_merger_add_sync_file(struct drm_syncobj_merger *merger, int fd);

#endif

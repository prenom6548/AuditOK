// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _RENDER_DRM_SYNCOBJ_H_
#define _RENDER_DRM_SYNCOBJ_H_

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>

struct drm_syncobj_timeline_waiter;
typedef void (*drm_syncobj_timeline_ready_callback)(struct drm_syncobj_timeline_waiter *waiter);

struct drm_syncobj_timeline {
    int drm_fd;
    uint32_t handle;

    size_t n_refs;
};

struct drm_syncobj_timeline_waiter {
    int ev_fd;
    struct wl_event_source *event_source;
    drm_syncobj_timeline_ready_callback callback;
};

struct drm_syncobj_timeline *drm_syncobj_timeline_create(int drm_fd);

int drm_syncobj_timeline_export(struct drm_syncobj_timeline *timeline);

struct drm_syncobj_timeline *drm_syncobj_timeline_import(int drm_fd, int drm_syncobj_fd);

struct drm_syncobj_timeline *drm_syncobj_timeline_ref(struct drm_syncobj_timeline *timeline);

void drm_syncobj_timeline_unref(struct drm_syncobj_timeline *timeline);

bool drm_syncobj_timeline_transfer(struct drm_syncobj_timeline *dst, uint64_t dst_point,
                                   struct drm_syncobj_timeline *src, uint64_t src_point);

bool drm_syncobj_timeline_signal(struct drm_syncobj_timeline *timeline, uint64_t point);

bool drm_syncobj_timeline_check(struct drm_syncobj_timeline *timeline, uint64_t point,
                                uint32_t flags, bool *result);

bool drm_syncobj_timeline_waiter_init(struct drm_syncobj_timeline_waiter *waiter,
                                      struct drm_syncobj_timeline *timeline, uint64_t point,
                                      uint32_t flags, struct wl_event_loop *loop,
                                      drm_syncobj_timeline_ready_callback callback);

void drm_syncobj_timeline_waiter_finish(struct drm_syncobj_timeline_waiter *waiter);

int drm_syncobj_timeline_export_sync_file(struct drm_syncobj_timeline *timeline,
                                          uint64_t src_point);
bool drm_syncobj_timeline_import_sync_file(struct drm_syncobj_timeline *timeline,
                                           uint64_t dst_point, int sync_file_fd);

#endif

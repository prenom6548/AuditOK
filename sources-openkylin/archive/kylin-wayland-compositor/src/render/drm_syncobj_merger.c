// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <linux/dma-buf.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/config.h>
#include <wlr/render/dmabuf.h>

#include <wlr/types/wlr_buffer.h>
#include <xf86drm.h>

#include <kywc/log.h>

#include "render/drm_syncobj.h"
#include "render/drm_syncobj_merger.h"

#if HAVE_LINUX_SYNC_FILE

#include <linux/sync_file.h>
#include <sys/ioctl.h>

static int sync_file_merge(int fd1, int fd2)
{
    // The kernel will automatically prune signalled fences
    struct sync_merge_data merge_data = { .fd2 = fd2 };
    if (ioctl(fd1, SYNC_IOC_MERGE, &merge_data) < 0) {
        kywc_log_errno(KYWC_ERROR, "ioctl(SYNC_IOC_MERGE) failed");
        return -1;
    }

    return merge_data.fence;
}

#else

static int sync_file_merge(int fd1, int fd2)
{
    kywc_log(KYWC_ERROR, "sync_file support is unavailable");
    return -1;
}

#endif

struct drm_syncobj_merger *drm_syncobj_merger_create(struct drm_syncobj_timeline *dst_timeline,
                                                     uint64_t dst_point)
{
    struct drm_syncobj_merger *merger = calloc(1, sizeof(*merger));
    if (merger == NULL) {
        return NULL;
    }
    merger->n_ref = 1;
    merger->dst_timeline = drm_syncobj_timeline_ref(dst_timeline);
    merger->dst_point = dst_point;
    merger->sync_fd = -1;
    return merger;
}

struct drm_syncobj_merger *drm_syncobj_merger_ref(struct drm_syncobj_merger *merger)
{
    assert(merger->n_ref > 0);
    merger->n_ref++;
    return merger;
}

void drm_syncobj_merger_unref(struct drm_syncobj_merger *merger)
{
    if (merger == NULL) {
        return;
    }
    assert(merger->n_ref > 0);
    merger->n_ref--;
    if (merger->n_ref > 0) {
        return;
    }

    if (merger->sync_fd != -1) {
        drm_syncobj_timeline_import_sync_file(merger->dst_timeline, merger->dst_point,
                                              merger->sync_fd);
        close(merger->sync_fd);
    } else {
        drm_syncobj_timeline_signal(merger->dst_timeline, merger->dst_point);
    }
    drm_syncobj_timeline_unref(merger->dst_timeline);
    free(merger);
}

static bool merger_add_exportable(struct drm_syncobj_merger *merger,
                                  struct drm_syncobj_timeline *src_timeline, uint64_t src_point)
{
    int new_sync = drm_syncobj_timeline_export_sync_file(src_timeline, src_point);
    return drm_syncobj_merger_add_sync_file(merger, new_sync);
}

struct export_waiter {
    struct drm_syncobj_timeline_waiter waiter;
    struct drm_syncobj_merger *merger;
    struct drm_syncobj_timeline *src_timeline;
    uint64_t src_point;
};

static void export_waiter_handle_ready(struct drm_syncobj_timeline_waiter *waiter)
{
    struct export_waiter *add = wl_container_of(waiter, add, waiter);
    merger_add_exportable(add->merger, add->src_timeline, add->src_point);
    drm_syncobj_merger_unref(add->merger);
    drm_syncobj_timeline_unref(add->src_timeline);
    drm_syncobj_timeline_waiter_finish(&add->waiter);
    free(add);
}

bool drm_syncobj_merger_add(struct drm_syncobj_merger *merger,
                            struct drm_syncobj_timeline *src_timeline, uint64_t src_point,
                            struct wl_event_loop *loop)
{
    assert(loop != NULL);
    bool exportable = false;
    int flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_AVAILABLE;
    if (!drm_syncobj_timeline_check(src_timeline, src_point, flags, &exportable)) {
        return false;
    }
    if (exportable) {
        return merger_add_exportable(merger, src_timeline, src_point);
    }
    struct export_waiter *add = calloc(1, sizeof(*add));
    if (add == NULL) {
        return false;
    }
    if (!drm_syncobj_timeline_waiter_init(&add->waiter, src_timeline, src_point, flags, loop,
                                          export_waiter_handle_ready)) {
        free(add);
        return false;
    }
    add->merger = merger;
    add->src_timeline = drm_syncobj_timeline_ref(src_timeline);
    add->src_point = src_point;
    merger->n_ref++;
    return true;
}

bool drm_syncobj_merger_add_sync_file(struct drm_syncobj_merger *merger, int fd)
{
    int new_sync = fd;
    if (merger->sync_fd != -1) {
        new_sync = sync_file_merge(merger->sync_fd, fd);
        close(fd);
        close(merger->sync_fd);
    }
    merger->sync_fd = new_sync;
    return merger->sync_fd != -1;
}

// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <wlr/backend/interface.h>
#include <wlr/render/interface.h>

#include <kywc/log.h>

#include "allocator_p.h"
#include "render/allocator.h"
#include "util/quirks.h"

struct allocator {
    struct wlr_allocator base;

    struct dumb_alloc *dumb;
    struct gbm_alloc *gbm;

    struct wl_listener render_destroy;
};

/* Re-open the DRM node to avoid GEM handle ref'counting issues. See:
 * https://gitlab.freedesktop.org/mesa/drm/-/merge_requests/110
 */
static int reopen_drm_node(int drm_fd, bool allow_render_node)
{
    if (drmIsMaster(drm_fd)) {
        // Only recent kernels support empty leases
        uint32_t lessee_id;
        int lease_fd = drmModeCreateLease(drm_fd, NULL, 0, O_CLOEXEC, &lessee_id);
        if (lease_fd >= 0) {
            return lease_fd;
        } else if (lease_fd != -EINVAL && lease_fd != -EOPNOTSUPP) {
            kywc_log_errno(KYWC_ERROR, "DrmModeCreateLease failed");
            return -1;
        }
        kywc_log(KYWC_DEBUG, "drmModeCreateLease failed, "
                             "falling back to plain open");
    }

    char *name = NULL;
    if (allow_render_node) {
        name = drmGetRenderDeviceNameFromFd(drm_fd);
    }
    if (name == NULL) {
        // Either the DRM device has no render node, either the caller wants
        // a primary node
        name = drmGetDeviceNameFromFd2(drm_fd);
        if (name == NULL) {
            kywc_log(KYWC_ERROR, "DrmGetDeviceNameFromFd2 failed");
            return -1;
        }
    }

    int new_fd = open(name, O_RDWR | O_CLOEXEC);
    if (new_fd < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to open DRM node '%s'", name);
        free(name);
        return -1;
    }

    free(name);

    // If we're using a DRM primary node and we are DRM master (e.g. because
    // we're running under the DRM backend), we need to use the legacy DRM
    // authentication mechanism to have the permission to manipulate DRM dumb
    // buffers.
    if (drmIsMaster(drm_fd) && drmGetNodeTypeFromFd(new_fd) == DRM_NODE_PRIMARY) {
        drm_magic_t magic;
        if (drmGetMagic(new_fd, &magic) < 0) {
            kywc_log_errno(KYWC_ERROR, "DrmGetMagic failed");
            close(new_fd);
            return -1;
        }

        if (drmAuthMagic(drm_fd, magic) < 0) {
            kywc_log_errno(KYWC_ERROR, "DrmAuthMagic failed");
            close(new_fd);
            return -1;
        }
    }

    return new_fd;
}

static uint32_t backend_get_buffer_caps(struct wlr_backend *backend)
{
    if (!backend->impl->get_buffer_caps) {
        return 0;
    }

    return backend->impl->get_buffer_caps(backend);
}

static uint32_t renderer_get_render_buffer_caps(struct wlr_renderer *r)
{
    return r->impl->get_render_buffer_caps(r);
}

static const struct wlr_allocator_interface allocator_impl;

static struct allocator *allocator_from_wlr_alloc(struct wlr_allocator *wlr_alloc)
{
    assert(wlr_alloc->impl == &allocator_impl);
    struct allocator *alloc = wl_container_of(wlr_alloc, alloc, base);
    return alloc;
}

static void wlr_alloc_destroy(struct wlr_allocator *wlr_allocator)
{
    struct allocator *alloc = allocator_from_wlr_alloc(wlr_allocator);

    if (alloc->gbm) {
        gbm_alloc_destroy(alloc->gbm);
        alloc->gbm = NULL;
    }

    if (alloc->dumb) {
        dumb_alloc_destroy(alloc->dumb);
        alloc->dumb = NULL;
    }

    wl_list_remove(&alloc->render_destroy.link);
    free(alloc);
}

static void handle_renderer_destroy(struct wl_listener *listener, void *data)
{
    struct allocator *alloc = wl_container_of(listener, alloc, render_destroy);
    wl_list_remove(&alloc->render_destroy.link);
    wl_list_init(&alloc->render_destroy.link);

    if (alloc->gbm) {
        gbm_alloc_destroy(alloc->gbm);
        alloc->gbm = NULL;
    }
}

static struct wlr_buffer *wlr_alloc_create_buffer(struct wlr_allocator *wlr_allocator, int width,
                                                  int height, const struct wlr_drm_format *format)
{
    if (!format) {
        return NULL;
    }

    struct allocator *alloc = allocator_from_wlr_alloc(wlr_allocator);
    if (wlr_allocator->buffer_caps == WLR_BUFFER_CAP_DMABUF) {
        return gbm_buffer_create(alloc->gbm, width, height, format, 0);
    } else if (wlr_allocator->buffer_caps & WLR_BUFFER_CAP_SHM) {
        return shm_buffer_create(width, height, format->format);
    } else {
        return dumb_buffer_create(alloc->dumb, width, height, format);
    }
}

struct wlr_buffer *allocator_create_buffer(struct wlr_allocator *wlr_allocator,
                                           enum allocator_buffer_type type, int width, int height,
                                           const struct wlr_drm_format *format, uint32_t flags)
{
    if (!format) {
        return NULL;
    }

    struct allocator *alloc = allocator_from_wlr_alloc(wlr_allocator);
    switch (type) {
    case ALLOCATOR_BUFFER_TYPE_GBM:
        return gbm_buffer_create(alloc->gbm, width, height, format, flags);
    case ALLOCATOR_BUFFER_TYPE_DUMB:
        return dumb_buffer_create(alloc->dumb, width, height, format);
    case ALLOCATOR_BUFFER_TYPE_SHM:
        return shm_buffer_create(width, height, format->format);
    default:
        return NULL;
    }
}

static struct gbm_alloc *gbm_allocator_create(struct wlr_renderer *renderer, int fd)
{
    struct gbm_alloc *alloc = NULL;

    int gbm_fd = reopen_drm_node(fd, true);
    if (gbm_fd < 0) {
        goto error;
    }

    if ((alloc = gbm_alloc_create(renderer, gbm_fd)) != NULL) {
        return alloc;
    }

    close(gbm_fd);
error:
    kywc_log(KYWC_WARN, "Failed to create gbm allocator");
    return alloc;
}

static struct dumb_alloc *drm_dumb_allocator_create(int drm_fd)
{
    struct dumb_alloc *alloc = NULL;

    int dumb_fd = reopen_drm_node(drm_fd, false);
    if (dumb_fd < 0) {
        goto error;
    }

    if ((alloc = dumb_alloc_create(dumb_fd)) != NULL) {
        return alloc;
    }

    close(dumb_fd);
error:
    kywc_log(KYWC_WARN, "Failed to create drm dumb allocator");
    return alloc;
}

static const struct wlr_allocator_interface allocator_impl = {
    .destroy = wlr_alloc_destroy,
    .create_buffer = wlr_alloc_create_buffer,
};

struct wlr_allocator *allocator_create(struct wlr_backend *backend, struct wlr_renderer *renderer)
{
    struct allocator *alloc = calloc(1, sizeof(*alloc));
    if (!alloc) {
        return NULL;
    }

    uint32_t backend_caps = backend_get_buffer_caps(backend);
    uint32_t renderer_caps = renderer_get_render_buffer_caps(renderer);

    // Note, drm_fd may be negative if unavailable
    int drm_fd = wlr_backend_get_drm_fd(backend);
    if (drm_fd < 0) {
        drm_fd = wlr_renderer_get_drm_fd(renderer);
    }

    uint32_t buffer_caps = 0;
    uint32_t quirks = quirks_by_backend(drm_fd);

    uint32_t gbm_caps = WLR_BUFFER_CAP_DMABUF;
    if ((backend_caps & gbm_caps) && (renderer_caps & gbm_caps) && drm_fd >= 0) {
        alloc->gbm = gbm_allocator_create(renderer, drm_fd);
    }

    if (alloc->gbm) {
        buffer_caps |= gbm_caps;
    }

    uint32_t drm_caps = WLR_BUFFER_CAP_DMABUF | WLR_BUFFER_CAP_DATA_PTR;
    if ((backend_caps & drm_caps) && (renderer_caps & drm_caps) && drm_fd >= 0 &&
        drmIsMaster(drm_fd)) {
        alloc->dumb = drm_dumb_allocator_create(drm_fd);
    }

    if (alloc->dumb && (quirks & QUIRKS_MASK_PREFER_DRM_DUMB)) {
        buffer_caps |= drm_caps;
    }

    uint32_t shm_caps = WLR_BUFFER_CAP_SHM | WLR_BUFFER_CAP_DATA_PTR;
    if ((backend_caps & shm_caps) && (renderer_caps & shm_caps) && !buffer_caps) {
        buffer_caps |= shm_caps;
    }

    wlr_allocator_init(&alloc->base, &allocator_impl, buffer_caps);

    alloc->render_destroy.notify = handle_renderer_destroy;
    wl_signal_add(&renderer->events.destroy, &alloc->render_destroy);

    return &alloc->base;
}

// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <xf86drm.h>

#include <wlr/backend/interface.h>

#include <kywc/log.h>

#include "backend/drm.h"
#include "drm_p.h"

struct drm_backend *drm_backend_from_wlr_backend(struct wlr_backend *wlr_backend)
{
    assert(wlr_backend_is_drm(wlr_backend));
    struct drm_backend *backend = wl_container_of(wlr_backend, backend, wlr_backend);
    return backend;
}

static uint32_t get_buffer_caps(struct wlr_backend *wlr_backend)
{
    return WLR_BUFFER_CAP_DMABUF | WLR_BUFFER_CAP_SHM | WLR_BUFFER_CAP_DATA_PTR;
}

static bool backend_start(struct wlr_backend *wlr_backend)
{
    struct drm_backend *backend = drm_backend_from_wlr_backend(wlr_backend);
    drm_scan_connectors(backend->drm, NULL);
    return true;
}

static int backend_get_drm_fd(struct wlr_backend *wlr_backend)
{
    struct drm_backend *backend = drm_backend_from_wlr_backend(wlr_backend);
    return backend->drm->fd;
}

static void backend_destroy(struct wlr_backend *wlr_backend)
{
    if (!wlr_backend) {
        return;
    }

    struct drm_backend *backend = drm_backend_from_wlr_backend(wlr_backend);
    wlr_backend_finish(wlr_backend);

    wl_list_remove(&backend->display_destroy.link);
    wl_list_remove(&backend->session_destroy.link);
    wl_list_remove(&backend->session_active.link);
    wl_list_remove(&backend->parent_destroy.link);

    wl_list_remove(&backend->device_change.link);
    wl_list_remove(&backend->device_remove.link);

    drm_device_destroy(backend->drm);
    wlr_session_close_file(backend->session, backend->dev);

    free(backend);
}

static const struct wlr_backend_impl backend_impl = {
    .start = backend_start,
    .destroy = backend_destroy,
    .get_buffer_caps = get_buffer_caps,
    .get_drm_fd = backend_get_drm_fd,
};

static void handle_session_active(struct wl_listener *listener, void *data)
{
    struct drm_backend *backend = wl_container_of(listener, backend, session_active);
    struct wlr_session *session = backend->session;
    kywc_log(KYWC_INFO, "DRM device %s %s", backend->drm->name,
             session->active ? "resumed" : "paused");

    backend->drm->session_active = session->active;
    if (session->active) {
        drm_restore_connectors(backend->drm);
    }
}

static void handle_device_change(struct wl_listener *listener, void *data)
{
    struct drm_backend *backend = wl_container_of(listener, backend, device_change);
    struct wlr_device_change_event *change = data;

    if (!backend->session->active) {
        if (change->type == WLR_DEVICE_HOTPLUG) {
            drm_update_connector(backend->drm, &change->hotplug);
        }
        return;
    }

    switch (change->type) {
    case WLR_DEVICE_HOTPLUG:
        kywc_log(KYWC_DEBUG, "Received hotplug event for %s", backend->drm->name);
        drm_scan_connectors(backend->drm, &change->hotplug);
        break;
    case WLR_DEVICE_LEASE:
        kywc_log(KYWC_DEBUG, "Received lease event for %s", backend->drm->name);
        drm_scan_leases(backend->drm);
        break;
    default:
        kywc_log(KYWC_DEBUG, "Received unknown change event for %s", backend->drm->name);
    }
}

static void handle_device_remove(struct wl_listener *listener, void *data)
{
    struct drm_backend *backend = wl_container_of(listener, backend, device_remove);

    kywc_log(KYWC_INFO, "Destroying DRM backend for %s", backend->drm->name);
    backend_destroy(&backend->wlr_backend);
}

static void handle_session_destroy(struct wl_listener *listener, void *data)
{
    struct drm_backend *backend = wl_container_of(listener, backend, session_destroy);
    backend_destroy(&backend->wlr_backend);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct drm_backend *backend = wl_container_of(listener, backend, display_destroy);
    backend_destroy(&backend->wlr_backend);
}

static void handle_parent_destroy(struct wl_listener *listener, void *data)
{
    struct drm_backend *backend = wl_container_of(listener, backend, parent_destroy);
    backend_destroy(&backend->wlr_backend);
}

bool wlr_backend_is_drm(struct wlr_backend *backend)
{
    return backend->impl == &backend_impl;
}

int drm_backend_get_non_master_fd(struct wlr_backend *wlr_backend)
{
    assert(wlr_backend);
    struct drm_backend *backend = drm_backend_from_wlr_backend(wlr_backend);
    int fd = open(backend->drm->name, O_RDWR | O_CLOEXEC);

    if (fd < 0) {
        kywc_log_errno(KYWC_ERROR, "Unable to clone DRM fd for client fd");
        return -1;
    }

    if (drmIsMaster(fd) && drmDropMaster(fd) < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to drop master");
        return -1;
    }

    return fd;
}

struct wlr_backend *drm_backend_create(struct wlr_session *session, struct wlr_device *dev,
                                       struct wlr_backend *parent)
{
    struct drm_backend *backend = calloc(1, sizeof(*backend));
    if (!backend) {
        kywc_log(KYWC_ERROR, "Failed to allocate drm_backend");
        return NULL;
    }

    struct drm_device *drm = drm_device_create(dev->fd, &backend->wlr_backend, session->display);
    if (!drm) {
        kywc_log(KYWC_ERROR, "Failed to create drm device");
        free(backend);
        return NULL;
    }

    backend->drm = drm;
    backend->dev = dev;
    backend->session = session;
    wlr_backend_init(&backend->wlr_backend, &backend_impl);

    if (parent) {
        backend->parent = drm_backend_from_wlr_backend(parent);
        /* create renderer to blit buffer */
        if (!drm_mgpu_renderer_init(drm, &drm->mgpu_renderer)) {
            kywc_log(KYWC_ERROR, "Failed to initialize renderer for %s", drm->name);
            wlr_session_close_file(session, dev);
            drm_device_destroy(drm);
            free(backend);
            return NULL;
        }
        backend->parent_destroy.notify = handle_parent_destroy;
        wl_signal_add(&parent->events.destroy, &backend->parent_destroy);
    } else {
        wl_list_init(&backend->parent_destroy.link);
    }

    backend->device_change.notify = handle_device_change;
    wl_signal_add(&dev->events.change, &backend->device_change);
    backend->device_remove.notify = handle_device_remove;
    wl_signal_add(&dev->events.remove, &backend->device_remove);

    backend->session_active.notify = handle_session_active;
    wl_signal_add(&session->events.active, &backend->session_active);
    backend->session_destroy.notify = handle_session_destroy;
    wl_signal_add(&session->events.destroy, &backend->session_destroy);
    backend->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(session->display, &backend->display_destroy);

    drm_lease_device_v1_create(backend);

    return &backend->wlr_backend;
}

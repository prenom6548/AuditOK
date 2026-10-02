// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>
#include <xf86drmMode.h>

#include <wlr/backend/multi.h>

#include <kywc/log.h>

#include "drm_p.h"

static void drm_backend_monitor_destroy(struct drm_backend_monitor *monitor)
{
    wl_list_remove(&monitor->session_new_dev.link);
    wl_list_remove(&monitor->session_destroy.link);
    wl_list_remove(&monitor->primary_drm_destroy.link);
    wl_list_remove(&monitor->multi_destroy.link);
    free(monitor);
}

static void handle_new_dev(struct wl_listener *listener, void *data)
{
    struct session_add_event *event = data;
    struct drm_backend_monitor *backend_monitor =
        wl_container_of(listener, backend_monitor, session_new_dev);

    struct session_device *dev = session_device_create(backend_monitor->session, event->path);
    if (!dev || !drmIsKMS(dev->fd)) {
        kywc_log(KYWC_ERROR, "Can not open %s as KMS device", event->path);
        goto error_session;
    }

    kywc_log(KYWC_DEBUG, "Creating DRM backend for %s after hotplug", event->path);
    struct wlr_backend *child_drm =
        drm_backend_create(backend_monitor->session, dev, backend_monitor->primary_drm);
    if (!child_drm) {
        kywc_log(KYWC_ERROR, "Failed to create DRM backend after hotplug");
        goto error_session;
    }

    if (!wlr_multi_backend_add(backend_monitor->multi, child_drm)) {
        kywc_log(KYWC_ERROR, "Failed to add new drm backend to multi backend");
        goto error_backend;
    }

    if (!wlr_backend_start(child_drm)) {
        kywc_log(KYWC_ERROR, "Failed to start new child DRM backend");
        goto error_backend;
    }

    return;

error_backend:
    wlr_backend_destroy(child_drm);
error_session:
    session_device_destroy(backend_monitor->session, dev);
}

static void handle_session_destroy(struct wl_listener *listener, void *data)
{
    struct drm_backend_monitor *backend_monitor =
        wl_container_of(listener, backend_monitor, session_destroy);
    drm_backend_monitor_destroy(backend_monitor);
}

static void handle_primary_drm_destroy(struct wl_listener *listener, void *data)
{
    struct drm_backend_monitor *backend_monitor =
        wl_container_of(listener, backend_monitor, primary_drm_destroy);
    drm_backend_monitor_destroy(backend_monitor);
}

static void handle_multi_destroy(struct wl_listener *listener, void *data)
{
    struct drm_backend_monitor *backend_monitor =
        wl_container_of(listener, backend_monitor, multi_destroy);
    drm_backend_monitor_destroy(backend_monitor);
}

struct drm_backend_monitor *drm_backend_monitor_create(struct wlr_backend *multi,
                                                       struct wlr_backend *primary_drm,
                                                       struct session *session)
{
    struct drm_backend_monitor *monitor = calloc(1, sizeof(*monitor));
    if (!monitor) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        return NULL;
    }

    monitor->multi = multi;
    monitor->primary_drm = primary_drm;
    monitor->session = session;

    monitor->session_new_dev.notify = handle_new_dev;
    wl_signal_add(&session->events.new_dev, &monitor->session_new_dev);

    monitor->session_destroy.notify = handle_session_destroy;
    wl_signal_add(&session->events.destroy, &monitor->session_destroy);

    monitor->primary_drm_destroy.notify = handle_primary_drm_destroy;
    wl_signal_add(&primary_drm->events.destroy, &monitor->primary_drm_destroy);

    monitor->multi_destroy.notify = handle_multi_destroy;
    wl_signal_add(&multi->events.destroy, &monitor->multi_destroy);

    return monitor;
}

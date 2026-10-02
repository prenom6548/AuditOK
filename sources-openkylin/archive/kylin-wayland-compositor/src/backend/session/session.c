// SPDX-FileCopyrightText: 2026 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <xf86drm.h>

#include <kywc/log.h>

#include "backend/session.h"
#include "session_p.h"

//  0: no wait will occur
// -1: wait indefinitely
// >0: the ms that maximum wait
#define ACTIVATION_TIMEOUT 0

void session_set_active(struct session *s)
{
    s->active = true;
    wl_signal_emit_mutable(&s->events.active, NULL);
}

void session_set_inactive(struct session *s)
{
    s->active = false;
    wl_signal_emit_mutable(&s->events.active, NULL);
}

static int handle_session_event(int fd, uint32_t mask, void *data)
{
    struct session *session = data;

    if (mask & (WL_EVENT_HANGUP | WL_EVENT_ERROR)) {
        if (mask & WL_EVENT_ERROR) {
            kywc_log(KYWC_ERROR, "Failed to wait for session event");
        } else {
            kywc_log(KYWC_INFO, "Failed to wait for session event");
        }
        session_destroy(session);
        return 0;
    }

    if (session->impl->dispatch(session, ACTIVATION_TIMEOUT) < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to dispatch session");
        session_destroy(session);
    }
    return 1;
}

static bool session_init(struct session *session, struct wl_display *disp)
{
    session->impl = calloc(1, sizeof(*session->impl));
    if (!session->impl) {
        return false;
    }

    setenv("XDG_SESSION_TYPE", "wayland", 1);
    if (!logind_create(session)) {
        kywc_log(KYWC_ERROR, "Unable to create logind");
        goto error_alloc;
    }

    const char *seat_name = session->impl->seat_name(session);
    if (!seat_name) {
        kywc_log_errno(KYWC_ERROR, "Unable to get seat info");
        goto error_create;
    }
    snprintf(session->seat, sizeof(session->seat), "%s", seat_name);

    struct wl_event_loop *event_loop = wl_display_get_event_loop(disp);
    session->session_event = wl_event_loop_add_fd(event_loop, session->impl->get_fd(session),
                                                  WL_EVENT_READABLE, handle_session_event, session);
    if (!session->session_event) {
        kywc_log(KYWC_ERROR, "Failed to create session event source");
        goto error_create;
    }

    if (session->impl->dispatch(session, ACTIVATION_TIMEOUT) < 0) {
        kywc_log_errno(KYWC_ERROR, "Session dispatch failed");
        goto error_dispatch;
    }

    kywc_log(KYWC_INFO, "Successfully loaded session");
    return true;

error_dispatch:
    wl_event_source_remove(session->session_event);
    session->session_event = NULL;
error_create:
    session->impl->destroy(session);
error_alloc:
    free(session->impl);
    session->impl = NULL;
    return false;
}

static void session_cleanup(struct session *session)
{
    session->impl->destroy(session);
    free(session->impl);
    session->impl = NULL;

    wl_event_source_remove(session->session_event);
    session->session_event = NULL;
}

static bool is_drm_card(const char *sysname)
{
    const char prefix[] = DRM_PRIMARY_MINOR_NAME;
    if (strncmp(sysname, prefix, strlen(prefix)) != 0) {
        return false;
    }
    for (size_t i = strlen(prefix); sysname[i] != '\0'; i++) {
        if (sysname[i] < '0' || sysname[i] > '9') {
            return false;
        }
    }
    return true;
}

static void read_udev_change_event(struct session_device_change_event *event,
                                   struct udev_device *udev_dev)
{
    const char *hotplug = udev_device_get_property_value(udev_dev, "HOTPLUG");
    if (hotplug != NULL && strcmp(hotplug, "1") == 0) {
        event->type = SESSION_DEVICE_HOTPLUG;
        struct session_device_hotplug_event *hotplug = &event->hotplug;

        const char *connector = udev_device_get_property_value(udev_dev, "CONNECTOR");
        if (connector != NULL) {
            hotplug->connector_id = strtoul(connector, NULL, 10);
        }

        const char *prop = udev_device_get_property_value(udev_dev, "PROPERTY");
        if (prop != NULL) {
            hotplug->prop_id = strtoul(prop, NULL, 10);
        }

        return;
    }

    const char *lease = udev_device_get_property_value(udev_dev, "LEASE");
    if (lease != NULL && strcmp(lease, "1") == 0) {
        event->type = SESSION_DEVICE_LEASE;
        return;
    }
}

static int handle_udev_event(int fd, uint32_t mask, void *data)
{
    struct session *session = data;

    struct udev_device *udev_dev = udev_monitor_receive_device(session->mon);
    if (!udev_dev) {
        return 1;
    }

    const char *sysname = udev_device_get_sysname(udev_dev);
    const char *devnode = udev_device_get_devnode(udev_dev);
    const char *action = udev_device_get_action(udev_dev);
    kywc_log(KYWC_DEBUG, "udev event for %s (%s)", sysname, action);

    if (!is_drm_card(sysname) || !action || !devnode) {
        goto out;
    }

    const char *seat = udev_device_get_property_value(udev_dev, "ID_SEAT");
    if (!seat) {
        seat = "seat0";
    }
    if (session->seat[0] != '\0' && strcmp(session->seat, seat) != 0) {
        goto out;
    }

    dev_t devnum = udev_device_get_devnum(udev_dev);
    if (strcmp(action, "add") == 0) {
        struct session_device *dev;
        wl_list_for_each(dev, &session->devices, link) {
            if (dev->dev == devnum) {
                kywc_log(KYWC_DEBUG, "Skipping duplicate device %s", sysname);
                goto out;
            }
        }

        kywc_log(KYWC_INFO, "DRM device %s added", sysname);
        struct session_add_event event = {
            .path = devnode,
        };
        wl_signal_emit_mutable(&session->events.new_dev, &event);
    } else if (strcmp(action, "change") == 0) {
        struct session_device *dev;
        wl_list_for_each(dev, &session->devices, link) {
            if (dev->dev == devnum) {
                kywc_log(KYWC_INFO, "DRM device %s changed", sysname);
                struct session_device_change_event event = { 0 };
                read_udev_change_event(&event, udev_dev);
                wl_signal_emit_mutable(&dev->events.change, &event);
                break;
            }
        }
    } else if (strcmp(action, "remove") == 0) {
        struct session_device *dev, *tmp;
        wl_list_for_each_safe(dev, tmp, &session->devices, link) {
            if (dev->dev == devnum) {
                kywc_log(KYWC_INFO, "DRM device %s removed", sysname);
                session_device_destroy(session, dev);
                break;
            }
        }
    }

out:
    udev_device_unref(udev_dev);
    return 1;
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct session *session = wl_container_of(listener, session, display_destroy);
    session_destroy(session);
}

struct session *session_create(struct wl_display *disp)
{
    struct session *s = calloc(1, sizeof(*s));
    if (!s) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        return NULL;
    }

    wl_signal_init(&s->events.active);
    wl_signal_init(&s->events.new_dev);
    wl_signal_init(&s->events.destroy);
    wl_list_init(&s->devices);

    if (!session_init(s, disp)) {
        kywc_log(KYWC_ERROR, "Failed to load session backend");
        goto error_alloc;
    }

    s->udev = udev_new();
    if (!s->udev) {
        kywc_log_errno(KYWC_ERROR, "Failed to create udev context");
        goto error_seat;
    }

    s->mon = udev_monitor_new_from_netlink(s->udev, "udev");
    if (!s->mon) {
        kywc_log_errno(KYWC_ERROR, "Failed to create udev monitor");
        goto error_udev;
    }

    udev_monitor_filter_add_match_subsystem_devtype(s->mon, "drm", NULL);
    udev_monitor_enable_receiving(s->mon);

    int fd = udev_monitor_get_fd(s->mon);
    struct wl_event_loop *event_loop = wl_display_get_event_loop(disp);
    s->udev_event = wl_event_loop_add_fd(event_loop, fd, WL_EVENT_READABLE, handle_udev_event, s);
    if (!s->udev_event) {
        kywc_log_errno(KYWC_ERROR, "Failed to create udev event source");
        goto error_mon;
    }

    s->display = disp;
    s->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(disp, &s->display_destroy);

    return s;

error_mon:
    udev_monitor_unref(s->mon);
error_udev:
    udev_unref(s->udev);
error_seat:
    session_cleanup(s);
error_alloc:
    free(s);
    return NULL;
}

void session_destroy(struct session *session)
{
    if (!session) {
        return;
    }

    wl_signal_emit_mutable(&session->events.destroy, session);

    assert(wl_list_empty(&session->events.active.listener_list));
    assert(wl_list_empty(&session->events.new_dev.listener_list));
    assert(wl_list_empty(&session->events.destroy.listener_list));

    wl_list_remove(&session->display_destroy.link);

    wl_event_source_remove(session->udev_event);
    udev_monitor_unref(session->mon);
    udev_unref(session->udev);

    struct session_device *dev, *tmp_dev;
    wl_list_for_each_safe(dev, tmp_dev, &session->devices, link) {
        session_device_destroy(session, dev);
    }

    session_cleanup(session);
    free(session);
}

struct session_device *session_device_create(struct session *session, const char *path)
{
    if (!path) {
        return NULL;
    }

    int fd = session->impl->open(session, path);
    if (fd < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to open device: '%s'", path);
        return NULL;
    }

    struct session_device *dev = malloc(sizeof(*dev));
    if (!dev) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        goto error;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        kywc_log_errno(KYWC_ERROR, "Stat failed");
        goto error;
    }

    dev->fd = fd;
    dev->dev = st.st_rdev;
    wl_signal_init(&dev->events.change);
    wl_signal_init(&dev->events.destroy);
    wl_list_insert(&session->devices, &dev->link);

    return dev;

error:
    session->impl->close(session, fd);
    free(dev);
    return NULL;
}

void session_device_destroy(struct session *session, struct session_device *dev)
{
    if (!dev) {
        return;
    }

    wl_signal_emit_mutable(&dev->events.destroy, NULL);
    assert(wl_list_empty(&dev->events.change.listener_list));
    assert(wl_list_empty(&dev->events.destroy.listener_list));

    session->impl->close(session, dev->fd);
    wl_list_remove(&dev->link);
    free(dev);
}

bool session_change_vt(struct session *session, unsigned vt)
{
    if (!session || !session->impl) {
        return false;
    }
    return session->impl->change_vt(session, vt) == 0;
}

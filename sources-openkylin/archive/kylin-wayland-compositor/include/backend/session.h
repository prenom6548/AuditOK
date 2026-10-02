// SPDX-FileCopyrightText: 2026 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _BACKEND_SESSION_H_
#define _BACKEND_SESSION_H_

#include <wayland-server-core.h>

struct session_interface;

struct session_device {
    struct wl_list link; // session->devices

    int fd;
    dev_t dev;

    struct {
        struct wl_signal change;
        struct wl_signal destroy;
    } events;
};

struct session {
    bool active;

    char seat[256];

    struct udev *udev;
    struct udev_monitor *mon;
    struct wl_event_source *udev_event;

    struct wl_list devices;

    struct wl_display *display;
    struct wl_event_source *session_event;

    struct session_interface *impl;
    void *impl_data;

    struct {
        struct wl_signal active;
        struct wl_signal new_dev;
        struct wl_signal destroy;
    } events;

    struct wl_listener display_destroy;
};

struct session_add_event {
    const char *path;
};

enum session_device_change_type {
    SESSION_DEVICE_HOTPLUG = 1,
    SESSION_DEVICE_LEASE,
};

struct session_device_hotplug_event {
    uint32_t connector_id;
    uint32_t prop_id;
};

struct session_device_change_event {
    enum session_device_change_type type;
    union {
        struct session_device_hotplug_event hotplug;
    };
};

struct session *session_create(struct wl_display *disp);

void session_destroy(struct session *session);

struct session_device *session_device_create(struct session *session, const char *path);

void session_device_destroy(struct session *session, struct session_device *device);

bool session_change_vt(struct session *session, unsigned vt);

#endif /* _BACKEND_SESSION_H_ */

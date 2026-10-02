// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _PORTAL_H_
#define _PORTAL_H_

#include <systemd/sd-bus.h>

#include <kywc/boxes.h>
#include <kywc/log.h>
#include <libkywc.h>

#include "util/event.h"

enum {
    PORTAL_RESPONSE_SUCCESS = 0,
    PORTAL_RESPONSE_CANCELLED = 1,
    PORTAL_RESPONSE_ENDED = 2,
};

enum portal_session_type {
    PORTAL_SESSION_TYPE_REMOTE_DESKTOP = 0,
    PORTAL_SESSION_TYPE_INPUT_CAPTURE,
};

struct portal_output {
    struct wl_list link;
    struct portal_context *ctx;

    kywc_output *output;
    struct kywc_box geometry; // in logical
};

struct portal_context {
    struct event_context *event_loop;
    sd_bus *bus;

    struct {
        struct wl_display *display;
        kywc_context *kywc;
        struct wl_list outputs;
    } wayland;

    struct {
        struct wl_list sessions;
        uint32_t version;
        uint32_t devices;
    } remote_desktop;

    struct {
        struct wl_list sessions;
        uint32_t version;
        uint32_t capabilities;
    } input_capture;
};

struct portal_barrier {
    int id;
    int x1, y1, x2, y2;
};

struct portal_session {
    struct wl_list link;
    sd_bus_slot *slot;
    char *session_handle;
    enum portal_session_type type;
    bool closed;

    char *input_capture_path;
    struct wl_array barriers;
    uint32_t zone_set;
};

struct portal_session *portal_session_create(struct portal_context *ctx,
                                             enum portal_session_type type, char *object_path);

struct portal_session *portal_session_get(struct wl_list *sessions, const char *object_path);

void portal_session_destroy(struct portal_session *session);

bool portal_wayland_init(struct portal_context *ctx);

void portal_wayland_fini(struct portal_context *ctx);

bool portal_remote_desktop_init(struct portal_context *ctx);

void portal_remote_desktop_fini(struct portal_context *ctx);

bool portal_input_capture_init(struct portal_context *ctx);

void portal_input_capture_emit_zones_changed(struct portal_context *ctx);

void portal_input_capture_fini(struct portal_context *ctx);

#endif /* _PORTAL_H_ */

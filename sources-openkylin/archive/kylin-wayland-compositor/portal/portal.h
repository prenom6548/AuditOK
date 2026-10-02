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

struct ei_sender;

enum {
    PORTAL_RESPONSE_SUCCESS = 0,
    PORTAL_RESPONSE_CANCELLED = 1,
    PORTAL_RESPONSE_ENDED = 2,
};

enum portal_session_type {
    PORTAL_SESSION_TYPE_REMOTE_DESKTOP = 0,
    PORTAL_SESSION_TYPE_INPUT_CAPTURE,
    PORTAL_SESSION_TYPE_SCREENCAST,
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
        struct kywc_buffer_helper *helper;
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

    struct {
        uint32_t version;
        struct wl_list requests;
    } screenshot;

    struct {
        struct wl_list sessions;
        uint32_t version;
        uint32_t sources;
        uint32_t cursors;
    } screencast;
};

struct portal_barrier {
    int id;
    int x1, y1, x2, y2;
};

struct portal_stream {
    struct wl_list link;

    kywc_stream *stream;
    uint32_t node;
    uint32_t source_type;
    struct kywc_box geometry;
};

struct screencast_selection {
    bool multiple;
    uint32_t source_types;
    uint32_t cursor_mode;
};

struct portal_session {
    struct wl_list link;
    struct portal_context *ctx;
    sd_bus_slot *slot;
    char *session_handle;

    enum portal_session_type type;
    bool closed;

    void *data;
    void (*destroy)(void *data);

    union {
        struct {
            char *input_capture_path;
            struct wl_array barriers;
            uint32_t zone_set;
        };
        struct {
            uint32_t select_devices;
            uint32_t persist_mode;

            void *restore_data;
            void (*restore_data_destroy)(void *restore_data);

            struct ei_sender *ei_sender;

            bool clipboard_enabled;
            bool screen_share_enabled;
            struct wl_list streams;
            struct screencast_selection select;
            int stream_counts;
        } remote_desktop;
        struct {
            uint32_t persist_mode;
            void *restore_data;
            void (*restore_data_destroy)(void *restore_data);

            struct wl_list streams;
            struct screencast_selection select;
            int stream_counts;
        } screencast;
    };
};

struct portal_request {
    struct wl_list link;
    struct portal_context *ctx;
    sd_bus_slot *slot;
    char *handle;

    void *data;
    void (*destroy)(void *data);
};

struct portal_session *portal_session_create(struct portal_context *ctx,
                                             enum portal_session_type type, char *object_path);

struct portal_session *portal_session_get(struct wl_list *sessions, const char *object_path);

int portal_get_session_type(struct portal_context *ctx, const char *object_path);

void portal_session_destroy(struct portal_session *session);

struct portal_request *portal_request_create(struct portal_context *ctx, const char *object_path,
                                             void *data);

void portal_request_destroy(struct portal_request *request);

bool portal_wayland_init(struct portal_context *ctx);

void portal_wayland_fini(struct portal_context *ctx);

bool portal_remote_desktop_init(struct portal_context *ctx);

void portal_remote_desktop_fini(struct portal_context *ctx);

bool portal_input_capture_init(struct portal_context *ctx);

void portal_input_capture_emit_zones_changed(struct portal_context *ctx);

void portal_input_capture_fini(struct portal_context *ctx);

bool portal_screenshot_init(struct portal_context *ctx);

void portal_screenshot_emit_toplevel_changed(struct portal_context *ctx);

void portal_screenshot_emit_output_changed(struct portal_context *ctx);

void portal_screenshot_fini(struct portal_context *ctx);

bool portal_access_init(struct portal_context *ctx);

void portal_access_fini(struct portal_context *ctx);

bool portal_screencast_init(struct portal_context *ctx);

void portal_screencast_fini(struct portal_context *ctx);

void portal_stream_emit_toplevel_changed(struct portal_context *ctx);

void portal_stream_emit_output_changed(struct portal_context *ctx);

#endif /* _PORTAL_H_ */

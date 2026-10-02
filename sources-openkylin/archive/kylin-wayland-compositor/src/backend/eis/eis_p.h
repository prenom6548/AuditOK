// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EIS_P_H_
#define _EIS_P_H_

#include <libeis.h>

#include <wlr/backend/interface.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/types/wlr_output_layout.h>

#include <kywc/log.h>

enum eis_context_type {
    EIS_CONTEXT_TYPE_NORMAL = 0,
    EIS_CONTEXT_TYPE_REMOTE_DESKTOP,
    EIS_CONTEXT_TYPE_INPUT_CAPTURE,
};

struct eis_backend {
    struct wlr_backend backend;
    struct wl_event_loop *event_loop;
    struct wlr_output_layout *layout;

    struct wl_list contexts;
    struct dbus_object *capture_manager;
    uint32_t capabilities;

    struct eis_context *active_capture;
    int filter_locks;
};

enum {
    VERTICAL = 0,
    HORIZONTAL = 1,
};

struct barrier {
    int position, start, end;
    int orientation;
    int hit;
};

struct eis_context {
    struct eis_backend *backend;
    struct wl_list link;

    struct eis *eis;
    struct wl_event_source *event;
    struct wl_list clients;

    struct dbus_object *dbus;
    const char *dbus_path; // input_capture
    struct sd_bus_slot *watcher;

    enum eis_context_type type;
    uint32_t capabilities;

    struct wl_array barriers;
    uint32_t activation_id;
};

struct eis_context_client {
    struct eis_context *context;
    struct wl_list link;

    struct eis_client *client;
    struct eis_seat *seat;

    struct eis_device *pointer;
    struct wlr_pointer wlr_pointer;

    struct eis_device *absolute;
    struct wlr_pointer wlr_absolute;
    struct wl_listener layout_update;

    struct eis_device *keyboard;
    struct wlr_keyboard wlr_keyboard;
    struct wl_listener keymap_update;
    struct wl_listener modifiers_update;

    bool sender; // sender or receiver
};

struct eis_context *eis_context_create(struct eis_backend *backend, enum eis_context_type type,
                                       uint32_t capabilities, const void *data);

void eis_context_destroy(struct eis_context *context);

struct eis_context_client *eis_context_add_client(struct eis_context *context,
                                                  struct eis_client *eis_client);

void eis_context_remove_client(struct eis_context *context, struct eis_client *eis_client);

void eis_context_set_activated(struct eis_context *context, bool activated, double lx, double ly);

struct eis_context_client *eis_context_get_client(struct eis_context *context,
                                                  struct eis_client *eis_client);

void eis_context_client_process_event(struct eis_context_client *client, struct eis_event *event);

void eis_context_client_feed_event(struct eis_context_client *client, struct eis_event *event);

void eis_backend_lock_filter(struct eis_backend *backend, bool lock);

#endif /* _EIS_P_H_ */

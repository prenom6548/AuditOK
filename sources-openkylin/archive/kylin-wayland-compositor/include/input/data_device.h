// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _DATA_DEVICE_H_
#define _DATA_DEVICE_H_

#include <wayland-server-protocol.h>
#include <wlr/types/wlr_seat.h>

#include "util/client.h"

struct data_source {
    const struct data_source_impl *impl;

    // source metadata
    struct wl_array mime_types;
    int32_t actions;

    // source status
    bool accepted;

    // drag'n'drop status
    enum wl_data_device_manager_dnd_action current_dnd_action;
    uint32_t compositor_action;

    // the owner client, filled at init
    struct client client;

    struct {
        struct wl_signal accepted;
        struct wl_signal dnd_action;
        struct wl_signal destroy;
    } events;
};

struct data_source_impl {
    void (*send)(struct data_source *source, const char *mime_type, int32_t fd);
    void (*accept)(struct data_source *source, uint32_t serial, const char *mime_type);
    void (*destroy)(struct data_source *source);

    void (*dnd_drop)(struct data_source *source);
    void (*dnd_finish)(struct data_source *source);
    void (*dnd_action)(struct data_source *source, enum wl_data_device_manager_dnd_action action);

    /* called at init to fill the client info of the source */
    void (*client)(struct data_source *source, struct client *client);
};

struct data_device_client {
    struct wlr_seat_client *seat_client;
    struct wl_listener seat_client_destroy;
    struct wl_list data_devices; // data_device resource
};

struct drag_icon {
    struct drag *drag;

    struct wlr_surface *surface;
    struct wl_listener surface_destroy;

    struct {
        struct wl_signal destroy;
    } events;
};

enum drag_grab_type {
    DRAG_GRAB_KEYBOARD,
    DRAG_GRAB_KEYBOARD_POINTER,
    DRAG_GRAB_KEYBOARD_TOUCH,
};

struct drag_motion_event {
    struct drag *drag;
    uint32_t time;
    double sx, sy;
};

struct drag_drop_event {
    struct drag *drag;
    uint32_t time;
};

struct drag {
    enum drag_grab_type grab_type;
    struct wlr_seat_keyboard_grab keyboard_grab;
    struct wlr_seat_pointer_grab pointer_grab;
    struct wlr_seat_touch_grab touch_grab;

    struct seat *seat;
    struct wlr_seat_client *focus_client;
    struct wl_listener focus_client_destroy;
    struct wlr_seat_client *seat_client;
    struct wl_listener seat_client_destroy;

    struct drag_icon *icon; // can be NULL
    struct wl_listener icon_destroy;
    struct wlr_surface *focus; // can be NULL
    struct wl_listener focus_destroy;
    struct wlr_surface *denied_focus; // drop denied by security, can be NULL
    struct wl_listener denied_focus_destroy;
    struct data_source *source; // can be NULL
    struct wl_listener source_destroy;

    bool started, dropped, cancelling;
    int32_t grab_touch_id, touch_id; // if DRAG_GRAB_TOUCH

    bool drag_x11;
    struct {
        int16_t cache_x, cache_y;
        bool waiting;
        bool cached;
    } pos;

    struct {
        struct wl_signal focus;
        struct wl_signal motion; // struct drag_motion_event
        struct wl_signal drop;   // struct drag_drop_event
        struct wl_signal destroy;
    } events;
};

void data_source_init(struct data_source *source, const struct data_source_impl *impl);

void data_source_destroy(struct data_source *source);

void data_source_accept(struct data_source *source, uint32_t serial, const char *mime_type);

void data_source_send(struct data_source *source, const char *mime_type, int32_t fd);

void data_source_dnd_drop(struct data_source *source);

void data_source_dnd_finish(struct data_source *source);

void data_source_dnd_action(struct data_source *source,
                            enum wl_data_device_manager_dnd_action action);

/* return true if the destination client is allowed to read the source */
bool data_source_allow_read(struct seat *seat, struct data_source *source,
                            struct client *destination);

/* return true if the destination client is allowed to drop the source */
bool data_source_allow_drop(struct seat *seat, struct data_source *source,
                            struct client *destination);

/* return true if the source owner is allowed to write the selection */
bool data_source_allow_write(struct seat *seat, struct data_source *source);

/* return true if the source owner is allowed to start a drag */
bool data_source_allow_start_drag(struct seat *seat, struct data_source *source);

void seat_request_set_selection(struct seat *seat, struct wlr_seat_client *client,
                                struct data_source *source, uint32_t serial);

void seat_set_selection(struct seat *seat, struct data_source *source, uint32_t serial);

struct drag *drag_create(struct wlr_seat_client *seat_client, struct data_source *source,
                         struct wlr_surface *icon_surface, bool drag_x11);

void seat_request_start_drag(struct seat *seat, struct drag *drag, struct wlr_surface *origin,
                             uint32_t serial);

void seat_start_pointer_drag(struct seat *seat, struct drag *drag, uint32_t serial);

void seat_start_touch_drag(struct seat *seat, struct drag *drag, uint32_t serial,
                           struct wlr_touch_point *point);

void seat_start_drag(struct seat *seat, struct drag *drag, uint32_t serial);

void wlr_seat_client_send_selection(struct wlr_seat_client *seat_client);

struct data_source *data_source_from_resource(struct wl_resource *resource);

struct data_device_client *data_device_client_from_seat_client(struct wlr_seat_client *seat_client);

#endif /* _DATA_DEVICE_H_ */

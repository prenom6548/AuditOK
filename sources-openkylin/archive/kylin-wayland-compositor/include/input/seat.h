// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SEAT_H_
#define _SEAT_H_

#include "input.h"
#include "scene/scene.h"

struct keyboard;
struct seat_pointer_grab;
struct seat_keyboard_grab;
struct seat_touch_grab;

struct seat_pointer_grab_interface {
    bool (*motion)(struct seat_pointer_grab *grab, uint32_t time, double lx, double ly);
    bool (*button)(struct seat_pointer_grab *grab, uint32_t time, uint32_t button, bool pressed);
    bool (*axis)(struct seat_pointer_grab *grab, uint32_t time, bool vertical, double value);
    bool (*frame)(struct seat_pointer_grab *grab);
    void (*cancel)(struct seat_pointer_grab *grab);
};

struct seat_pointer_grab {
    const struct seat_pointer_grab_interface *interface;
    struct seat *seat;
    void *data;
};

struct seat_keyboard_grab_interface {
    bool (*key)(struct seat_keyboard_grab *grab, struct keyboard *keyboard, uint32_t time,
                uint32_t key, bool pressed, uint32_t modifiers);
    void (*cancel)(struct seat_keyboard_grab *grab);
};

struct seat_keyboard_grab {
    const struct seat_keyboard_grab_interface *interface;
    struct seat *seat;
    void *data;
};

struct seat_touch_grab_interface {
    bool (*touch)(struct seat_touch_grab *grab, uint32_t time, bool down, uint32_t serial);
    bool (*motion)(struct seat_touch_grab *grab, uint32_t time, double lx, double ly,
                   uint32_t serial);
    void (*cancel)(struct seat_touch_grab *grab);
};

struct seat_touch_grab {
    const struct seat_touch_grab_interface *interface;
    struct seat *seat;
    void *data;
    uint32_t serial;
};

struct seat_keyboard_shortcuts_inhibitor {
    struct wl_list link;

    struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor;
    struct wl_listener destroy;
};

struct seat_cursor_motion_event {
    struct input *device;
    uint32_t time_msec;
    /* cursor in layout coord */
    double lx, ly;
};

struct seat_cursor_button_event {
    uint32_t time_msec;
    uint32_t button;
    bool pressed;
};

struct seat_keyboard_key_event {
    struct input *device;
    uint32_t time_msec;
    uint32_t keycode;
    bool pressed;
    bool consumed; // this key has been consumed by compositor
};

struct seat_request_set_selection_event {
    struct data_source *source;
    uint32_t serial;
};

struct seat_request_set_primary_selection_event {
    struct primary_selection_source *source;
    uint32_t serial;
};

struct seat_request_start_drag_event {
    struct drag *drag;
    struct wlr_surface *origin;
    uint32_t serial;
};

struct seat {
    struct wlr_seat *wlr_seat;
    char *name;
    struct wl_list link;

    uint32_t caps; // enum wl_seat_capability

    struct input_manager *manager;

    /* input devices attached */
    struct wl_list inputs;

    // TODO: timer to hide cursor
    struct cursor *cursor;
    struct keyboard *keyboard;
    struct wl_list keyboards;

    /* internal grabs */
    struct seat_pointer_grab *pointer_grab;
    struct seat_keyboard_grab *keyboard_grab;
    struct seat_touch_grab *touch_grab;

    /* shortcuts inhibitors */
    struct wl_list keyboard_shortcuts_inhibitors; // struct keyboard_shortcuts_inhibitor.link
    struct seat_keyboard_shortcuts_inhibitor *activated_inhibitor;

    struct scene *scene;
    struct wlr_output_layout *layout;
    struct wlr_pointer_gestures_v1 *pointer_gestures;

    struct input_method_relay *relay;
    struct selection *selection;

    struct data_source *selection_source;
    struct wl_listener selection_source_destroy;
    uint32_t selection_serial;
    struct wl_list selection_offers; // data_offer.link

    // `drag` goes away before `drag_source`, when the implicit grab ends
    struct drag *drag;
    struct data_source *drag_source;
    struct wl_listener drag_source_destroy;
    uint32_t drag_serial;
    struct wl_list drag_offers; // data_offer.link

    struct primary_selection_source *primary_selection_source;
    uint32_t primary_selection_serial;
    struct wl_listener primary_selection_source_destroy;

    struct {
        struct wl_signal cursor_motion;
        struct wl_signal cursor_button;
        struct wl_signal cursor_configure;
        struct wl_signal keyboard_key;
        struct wl_signal keyboard_modifiers;
        struct wl_signal capability;
        struct wl_signal destroy;

        struct wl_signal request_set_selection;
        struct wl_signal set_selection;
        struct wl_signal request_start_drag;
        struct wl_signal start_drag;
        struct wl_signal request_set_primary_selection;
        struct wl_signal set_primary_selection;

    } events;

    struct {
        const char *cursor_theme;
        uint32_t cursor_size;
        uint32_t keyboard_lock_mode;
        uint32_t keyboard_lock;
    } state;

    struct wl_listener server_start;
    struct wl_listener server_active;
    struct wl_listener destroy;
};

struct seat *seat_create(struct input_manager *input_manager, const char *name);

void seat_destroy(struct seat *seat);

void seat_add_input(struct seat *seat, struct input *input);

void seat_remove_input(struct input *input);

void seat_reset_input_gesture(struct seat *seat);

struct seat *seat_from_resource(struct wl_resource *resource);

struct seat *seat_from_wlr_seat(struct wlr_seat *wlr_seat);

void seat_set_cursor(struct seat *seat, const char *cursor_theme, uint32_t cursor_size);

void seat_start_pointer_grab(struct seat *seat, struct seat_pointer_grab *pointer_grab);

void seat_end_pointer_grab(struct seat *seat, struct seat_pointer_grab *pointer_grab);

void seat_start_keyboard_grab(struct seat *seat, struct seat_keyboard_grab *keyboard_grab);

void seat_end_keyboard_grab(struct seat *seat, struct seat_keyboard_grab *keyboard_grab);

void seat_start_touch_grab(struct seat *seat, struct seat_touch_grab *touch_grab);

void seat_end_touch_grab(struct seat *seat, struct seat_touch_grab *touch_grab);

void seat_notify_motion(struct seat *seat, struct wlr_surface *surface, uint32_t time, double sx,
                        double sy, bool first_enter);

void seat_notify_button(struct seat *seat, uint32_t time, uint32_t button, bool pressed);

void seat_notify_leave(struct seat *seat, struct wlr_surface *surface);

void seat_focus_surface(struct seat *seat, struct wlr_surface *surface);

bool seat_is_dragging(struct seat *seat);

void seat_feed_pointer_motion(struct seat *seat, double x, double y, bool absolute);

void seat_feed_pointer_button(struct seat *seat, uint32_t button, bool pressed);

void seat_feed_pointer_axis(struct seat *seat, uint32_t axis, double step);

#endif /* _SEAT_H_ */

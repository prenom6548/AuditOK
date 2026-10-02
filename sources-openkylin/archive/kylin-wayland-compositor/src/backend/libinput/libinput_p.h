// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _LIBINPUT_P_H_
#define _LIBINPUT_P_H_

#include <libinput.h>
#include <pthread.h>
#include <stdatomic.h>

#include <wlr/backend/interface.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_switch.h>
#include <wlr/interfaces/wlr_tablet_pad.h>
#include <wlr/interfaces/wlr_tablet_tool.h>
#include <wlr/interfaces/wlr_touch.h>

#include "input_queue.h"

struct touch_abs {
    int32_t major_minimum;
    int32_t major_maximum;
    int32_t major_resolution;
    int32_t minor_minimum;
    int32_t minor_maximum;
    int32_t minor_resolution;
};

struct libinput_property_exts {
    int (*touch_has_major)(struct libinput_event_touch *touch_event);
    int (*touch_has_minor)(struct libinput_event_touch *touch_event);

    int (*touch_get_major_value)(struct libinput_event_touch *event, double *value);
    int (*touch_get_minor_value)(struct libinput_event_touch *event, double *value);
    int (*device_get_touch_major_minimum)(struct libinput_device *device, int32_t *value);
    int (*device_get_touch_major_maximum)(struct libinput_device *device, int32_t *value);
    int (*device_get_touch_major_resolution)(struct libinput_device *device, int32_t *value);
    int (*device_get_touch_minor_minimum)(struct libinput_device *device, int32_t *value);
    int (*device_get_touch_minor_maximum)(struct libinput_device *device, int32_t *value);
    int (*device_get_touch_minor_resolution)(struct libinput_device *device, int32_t *value);

    bool support_touch_size;
    bool support_touch_abs;
};

enum {
    LIBINPUT_CMD_STOP = 0,
    LIBINPUT_CMD_RESUME = 1,
    LIBINPUT_CMD_SUSPEND = 2,
};

#define LIBINPUT_CMD_QUEUE_CAPACITY 8

struct libinput_backend {
    struct wlr_backend backend;
    struct wl_event_loop *event_loop;

    struct session *session;
    struct wl_listener session_active;
    struct wl_listener session_destroy;

    struct libinput *libinput_context;
    struct wl_event_source *input_event;
    struct libinput_property_exts property_exts;

    struct wl_list devices;

    /* threading */
    pthread_t input_thread;
    bool thread_running;
    pthread_mutex_t mutex;
    int notify_fd;
    int cmd_event_fd;
    int epoll_fd;
    struct input_queue queue;
    struct spsc_chain *cmd_chain;
    atomic_bool notify_pending;
};

struct libinput_input_device {
    struct libinput_backend *backend;
    struct libinput_device *handle;
    struct wl_list link;

    struct wlr_keyboard keyboard;
    struct wlr_pointer pointer;
    struct wlr_switch switch_device;
    struct wlr_touch touch;
    struct wlr_tablet tablet;
    struct wl_list tablet_tools; // see backend/libinput/tablet_tool.c
    struct wlr_tablet_pad tablet_pad;

    struct touch_abs touch_abs;
    double touch_major, touch_minor;
    int32_t touch_size_slot;
    bool touch_size_valid;

    struct {
        struct wl_signal raw_tap;     // struct pointer_raw_tap_event
        struct wl_signal zone_begin;  // struct pointer_zone_gesture_begin_event
        struct wl_signal zone_update; // struct pointer_zone_gesture_update_event
        struct wl_signal zone_end;    // struct pointer_zone_gesture_end_event
    } events;
};

/* events.c */
void handle_libinput_event(struct libinput_backend *backend, struct libinput_event *event);

void dispatch_input_events(struct libinput_backend *backend);

void destroy_libinput_input_device(struct libinput_input_device *dev);

bool button_state_from_libinput(enum libinput_button_state state, enum wlr_button_state *out);

void queue_input_event(struct libinput_input_device *dev, const struct input_event *e);

/* keyboard.c */
extern const struct wlr_keyboard_impl libinput_keyboard_impl;

void init_device_keyboard(struct libinput_input_device *dev);

struct libinput_input_device *device_from_keyboard(struct wlr_keyboard *kb);

void handle_keyboard_key(struct libinput_event *event, struct wlr_keyboard *kb);

/* pointer.c */
extern const struct wlr_pointer_impl libinput_pointer_impl;

void init_device_pointer(struct libinput_input_device *dev);

struct libinput_input_device *device_from_pointer(struct wlr_pointer *wlr_pointer);

void handle_pointer_motion(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_motion_abs(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_button(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_axis(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_axis_value120(struct libinput_event *event, struct wlr_pointer *pointer,
                                  enum wlr_axis_source source);

void handle_pointer_swipe_begin(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_swipe_update(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_swipe_end(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_pinch_begin(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_pinch_update(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_pinch_end(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_hold_begin(struct libinput_event *event, struct wlr_pointer *pointer);

void handle_pointer_hold_end(struct libinput_event *event, struct wlr_pointer *pointer);

/* switch.c */
extern const struct wlr_switch_impl libinput_switch_impl;

void init_device_switch(struct libinput_input_device *dev);

struct libinput_input_device *device_from_switch(struct wlr_switch *wlr_switch);

void handle_switch_toggle(struct libinput_event *event, struct wlr_switch *wlr_switch);

/* tablet_pad.c */
extern const struct wlr_tablet_pad_impl libinput_tablet_pad_impl;

void init_device_tablet_pad(struct libinput_input_device *dev);

void finish_device_tablet_pad(struct libinput_input_device *dev);

struct libinput_input_device *device_from_tablet_pad(struct wlr_tablet_pad *wlr_tablet_pad);

void handle_tablet_pad_button(struct libinput_event *event, struct wlr_tablet_pad *tablet_pad);

void handle_tablet_pad_ring(struct libinput_event *event, struct wlr_tablet_pad *tablet_pad);

void handle_tablet_pad_strip(struct libinput_event *event, struct wlr_tablet_pad *tablet_pad);

/* tablet_tool.c */
extern const struct wlr_tablet_impl libinput_tablet_impl;

void init_device_tablet(struct libinput_input_device *dev);

struct tablet_tool;
void tool_destroy(struct tablet_tool *tool, struct libinput_input_device *dev);
void finish_device_tablet(struct libinput_input_device *dev);

struct libinput_input_device *device_from_tablet(struct wlr_tablet *wlr_tablet);

void handle_tablet_tool_axis(struct libinput_event *event, struct wlr_tablet *wlr_tablet);

void handle_tablet_tool_proximity(struct libinput_event *event, struct wlr_tablet *wlr_tablet);

void handle_tablet_tool_tip(struct libinput_event *event, struct wlr_tablet *wlr_tablet);

void handle_tablet_tool_button(struct libinput_event *event, struct wlr_tablet *wlr_tablet);

/* touch.c */
extern const struct wlr_touch_impl libinput_touch_impl;

void init_device_touch(struct libinput_input_device *dev);

struct libinput_input_device *device_from_touch(struct wlr_touch *wlr_touch);

void handle_touch_down(struct libinput_event *event, struct wlr_touch *touch);

void handle_touch_up(struct libinput_event *event, struct wlr_touch *touch);

void handle_touch_motion(struct libinput_event *event, struct wlr_touch *touch);

void handle_touch_cancel(struct libinput_event *event, struct wlr_touch *touch);

void handle_touch_frame(struct libinput_event *event, struct wlr_touch *touch);

#endif /* _LIBINPUT_P_H_ */

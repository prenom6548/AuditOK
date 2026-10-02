// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _LIBINPUT_P_H_
#define _LIBINPUT_P_H_

#include <libinput.h>

#include <wlr/backend/interface.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_switch.h>
#include <wlr/interfaces/wlr_tablet_pad.h>
#include <wlr/interfaces/wlr_tablet_tool.h>
#include <wlr/interfaces/wlr_touch.h>

struct libinput_backend {
    struct wlr_backend backend;
    struct wl_event_loop *event_loop;

    struct wlr_session *session;
    struct wl_listener session_active;
    struct wl_listener session_destroy;

    struct libinput *libinput_context;
    struct wl_event_source *input_event;

    struct wl_list devices;
};

struct libinput_input_device {
    struct libinput_device *handle;
    struct wl_list link;

    struct wlr_keyboard keyboard;
    struct wlr_pointer pointer;
    struct wlr_switch switch_device;
    struct wlr_touch touch;
    struct wlr_tablet tablet;
    struct wl_list tablet_tools; // see backend/libinput/tablet_tool.c
    struct wlr_tablet_pad tablet_pad;

    struct {
        struct wl_signal raw_tap; // struct pointer_raw_tap_event
    } events;
};

/* events.c */
void handle_libinput_event(struct libinput_backend *backend, struct libinput_event *event);

void destroy_libinput_input_device(struct libinput_input_device *dev);

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

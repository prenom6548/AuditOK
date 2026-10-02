// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _BACKEND_LIBINPUT_H_
#define _BACKEND_LIBINPUT_H_

#include <libinput.h>
#include <wayland-server-core.h>

struct wlr_input_device;
struct wlr_touch;

struct pointer_raw_tap_event {
    uint32_t time_msec;
    uint32_t fingers;
};

enum pointer_zone_gesture_zone {
    POINTER_ZONE_GESTURE_NORMAL = 0,
    POINTER_ZONE_GESTURE_OUTSIDE = 1,
    POINTER_ZONE_GESTURE_LEFT_EDGE = 2,
    POINTER_ZONE_GESTURE_RIGHT_EDGE = 3,
    POINTER_ZONE_GESTURE_TOP_EDGE = 4,
};

struct pointer_zone_gesture_begin_event {
    uint32_t time_msec;
    uint32_t fingers;
    enum pointer_zone_gesture_zone zone;
};

struct pointer_zone_gesture_update_event {
    uint32_t time_msec;
    uint32_t fingers;
    enum pointer_zone_gesture_zone zone;
    double delta;
};

struct pointer_zone_gesture_end_event {
    uint32_t time_msec;
    bool cancelled;
};

struct session;

struct wlr_backend *libinput_backend_create(struct wl_display *display, struct session *session);

bool wlr_backend_is_libinput(struct wlr_backend *backend);

bool wlr_input_device_is_libinput(struct wlr_input_device *device);

struct libinput_device *wlr_libinput_get_device_handle(struct wlr_input_device *wlr_dev);

void wlr_libinput_add_raw_tap_listener(struct wlr_input_device *wlr_dev,
                                       struct wl_listener *listener);

bool wlr_libinput_get_touch_size_value(struct wlr_touch *touch, int32_t touch_id, double *major,
                                       double *minor);

bool wlr_libinput_get_touch_size_abs(struct wlr_touch *touch, int32_t *major_minimum,
                                     int32_t *major_maximum, int32_t *major_resolution,
                                     int32_t *minor_minimum, int32_t *minor_maximum,
                                     int32_t *minor_resolution);

void wlr_libinput_add_zone_gesture_begin_listener(struct wlr_input_device *wlr_dev,
                                                  struct wl_listener *listener);

void wlr_libinput_add_zone_gesture_update_listener(struct wlr_input_device *wlr_dev,
                                                   struct wl_listener *listener);

void wlr_libinput_add_zone_gesture_end_listener(struct wlr_input_device *wlr_dev,
                                                struct wl_listener *listener);

void libinput_backend_lock(struct libinput_device *device);

void libinput_backend_unlock(struct libinput_device *device);

#endif /* _BACKEND_LIBINPUT_H_ */
